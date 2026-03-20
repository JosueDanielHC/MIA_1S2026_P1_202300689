#include "utils/debug_ebr.h"
#include "disk/mbr.h"
#include "disk/ebr.h"
#include <fstream>
#include <sstream>
#include <string>

namespace extreamfs {
namespace utils {

static bool partitionUsed(const Partition& p) {
    return p.part_status != '0' || p.part_type != '0' || p.part_start >= 0;
}

std::string debugEBR(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return "Error: no se pudo abrir el disco";
    }

    MBR mbr;
    file.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(MBR))) {
        return "Error: no se pudo leer el MBR";
    }

    const Partition* ext = nullptr;
    for (int i = 0; i < 4; ++i) {
        if (mbr.mbr_partitions[i].part_type == 'E' && partitionUsed(mbr.mbr_partitions[i])) {
            ext = &mbr.mbr_partitions[i];
            break;
        }
    }
    if (!ext || ext->part_s <= 0) {
        return "No existe partición extendida";
    }

    const int inicio_ext = ext->part_start;
    const int fin_ext = inicio_ext + ext->part_s;
    std::ostringstream out;

    int currentPos = inicio_ext;
    const int maxIter = 100;
    int iter = 0;

    while (iter < maxIter) {
        EBR ebr;
        file.seekg(currentPos);
        file.read(reinterpret_cast<char*>(&ebr), sizeof(EBR));
        if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(EBR))) {
            return out.str() + "Error: no se pudo leer EBR en " + std::to_string(currentPos);
        }

        if (ebr.part_s <= 0) {
            break;
        }

        int fin_datos = ebr.part_start + ebr.part_s;
        size_t len = 0;
        while (len < 16 && ebr.part_name[len] != '\0') ++len;
        std::string name(ebr.part_name, len);

        out << "EBR en " << currentPos << "\n";
        out << "Nombre: " << name << "\n";
        out << "DataStart: " << ebr.part_start << "\n";
        out << "Size: " << ebr.part_s << "\n";
        out << "Next: " << ebr.part_next << "\n";
        out << "Fin datos: " << fin_datos << "\n\n";

        if (ebr.part_start < inicio_ext || fin_datos > fin_ext) {
            return out.str() + "Error: bloque de datos fuera del rango extendido";
        }
        if (ebr.part_next != -1) {
            if (ebr.part_next < inicio_ext || ebr.part_next >= fin_ext) {
                return out.str() + "Error: part_next apunta fuera del rango extendido";
            }
            currentPos = ebr.part_next;
        } else {
            break;
        }
        ++iter;
    }

    if (iter >= maxIter) {
        return out.str() + "Error: posible bucle infinito en cadena EBR";
    }

    file.close();
    out << "Cadena EBR válida";
    return out.str();
}

} // namespace utils
} // namespace extreamfs
