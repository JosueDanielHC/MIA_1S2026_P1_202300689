#include "commands/disk/fdisk.h"
#include "disk/mbr.h"
#include "disk/ebr.h"
#include <cstring>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>

namespace extreamfs {
namespace commands {

namespace {

struct FreeRegion {
    int start;
    int end;
    int size() const { return end - start; }
};

static bool partitionUsed(const Partition& p) {
    return p.part_status != '0' || p.part_type != '0' || p.part_start >= 0;
}

static void collectFreeRegions(const MBR& mbr, std::vector<FreeRegion>& out) {
    out.clear();
    const int mbrEnd = static_cast<int>(sizeof(MBR));
    const int diskEnd = mbr.mbr_tamano;

    std::vector<std::pair<int, int>> used;
    for (int i = 0; i < 4; ++i) {
        const Partition& p = mbr.mbr_partitions[i];
        if (partitionUsed(p) && p.part_s > 0) {
            used.push_back({p.part_start, p.part_start + p.part_s});
        }
    }
    std::sort(used.begin(), used.end());

    int current = mbrEnd;
    for (const auto& u : used) {
        if (u.first > current) {
            out.push_back({current, u.first});
        }
        if (u.second > current) {
            current = u.second;
        }
    }
    if (current < diskEnd) {
        out.push_back({current, diskEnd});
    }
}

static int chooseRegionByFit(const std::vector<FreeRegion>& regions,
                             int sizeBytes, char dskFit) {
    if (regions.empty()) return -1;

    std::vector<int> valid;
    for (size_t i = 0; i < regions.size(); ++i) {
        if (regions[i].size() >= sizeBytes) {
            valid.push_back(static_cast<int>(i));
        }
    }
    if (valid.empty()) return -1;

    if (dskFit == 'F') {
        return valid[0];
    }
    if (dskFit == 'B') {
        int best = valid[0];
        for (int i : valid) {
            if (regions[i].size() < regions[best].size()) best = i;
        }
        return best;
    }
    if (dskFit == 'W') {
        int worst = valid[0];
        for (int i : valid) {
            if (regions[i].size() > regions[worst].size()) worst = i;
        }
        return worst;
    }
    return valid[0];
}

static bool ebrInUse(const EBR& e) {
    return e.part_s > 0;
}

static std::string runFdiskLogical(std::fstream& file, const MBR& mbr,
                                   int sizeBytes, const std::string& name, char partFit) {
    const Partition* ext = nullptr;
    for (int i = 0; i < 4; ++i) {
        if (mbr.mbr_partitions[i].part_type == 'E' && mbr.mbr_partitions[i].part_status == '1') {
            ext = &mbr.mbr_partitions[i];
            break;
        }
    }
    if (!ext || ext->part_s <= 0) {
        return "Error: no existe partición extendida";
    }

    const int inicio_ext = ext->part_start;
    const int fin_ext = inicio_ext + ext->part_s;
    const int ebrSize = static_cast<int>(sizeof(EBR));

    if (ebrSize + sizeBytes > ext->part_s) {
        return "Error: no hay espacio suficiente en la partición extendida";
    }

    for (int i = 0; i < 4; ++i) {
        const Partition& p = mbr.mbr_partitions[i];
        if (!partitionUsed(p)) continue;
        size_t len = 0;
        while (len < 16 && p.part_name[len] != '\0') ++len;
        if (std::string(p.part_name, len) == name) {
            return "Error: ya existe una partición con ese nombre";
        }
    }

    int currentEbrPos = inicio_ext;
    EBR ebr;
    EBR lastEbr;
    int lastEbrPos = -1;
    bool hasAnyLogical = false;

    while (true) {
        file.seekg(currentEbrPos);
        file.read(reinterpret_cast<char*>(&ebr), sizeof(EBR));
        if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(EBR))) {
            return "Error: no se pudo leer EBR";
        }
        if (ebrInUse(ebr)) {
            hasAnyLogical = true;
            size_t len = 0;
            while (len < 16 && ebr.part_name[len] != '\0') ++len;
            if (std::string(ebr.part_name, len) == name) {
                return "Error: ya existe una partición con ese nombre";
            }
            lastEbr = ebr;
            lastEbrPos = currentEbrPos;
            if (ebr.part_next == -1) break;
            currentEbrPos = ebr.part_next;
        } else {
            break;
        }
    }

    if (!hasAnyLogical) {
        int nuevoEbrPos = inicio_ext;
        int datosStart = nuevoEbrPos + ebrSize;
        if (datosStart + sizeBytes > fin_ext) {
            return "Error: no hay espacio suficiente en la partición extendida";
        }
        std::memset(&ebr, 0, sizeof(EBR));
        ebr.part_mount = '0';
        ebr.part_fit = partFit;
        ebr.part_start = datosStart;
        ebr.part_s = sizeBytes;
        ebr.part_next = -1;
        std::strncpy(ebr.part_name, name.c_str(), 15);
        ebr.part_name[15] = '\0';
        file.seekp(nuevoEbrPos);
        file.write(reinterpret_cast<const char*>(&ebr), sizeof(EBR));
        if (!file) return "Error: no se pudo escribir el EBR";
        return "Partición lógica creada exitosamente";
    }

    int nuevoEbrPos = lastEbr.part_start + lastEbr.part_s;
    if (nuevoEbrPos + ebrSize + sizeBytes > fin_ext) {
        return "Error: no hay espacio suficiente en la partición extendida";
    }
    int datosStart = nuevoEbrPos + ebrSize;

    lastEbr.part_next = nuevoEbrPos;
    file.seekp(lastEbrPos);
    file.write(reinterpret_cast<const char*>(&lastEbr), sizeof(EBR));
    if (!file) return "Error: no se pudo actualizar el EBR anterior";

    std::memset(&ebr, 0, sizeof(EBR));
    ebr.part_mount = '0';
    ebr.part_fit = partFit;
    ebr.part_start = datosStart;
    ebr.part_s = sizeBytes;
    ebr.part_next = -1;
    std::strncpy(ebr.part_name, name.c_str(), 15);
    ebr.part_name[15] = '\0';
    file.seekp(nuevoEbrPos);
    file.write(reinterpret_cast<const char*>(&ebr), sizeof(EBR));
    if (!file) return "Error: no se pudo escribir el EBR";
    return "Partición lógica creada exitosamente";
}

} // namespace

std::string runFdisk(int size, const std::string& path, const std::string& name,
                     const std::string& unit, const std::string& fit,
                     const std::string& typeParam) {
    if (size <= 0) {
        return "Error: size debe ser positivo";
    }

    std::string typeStr = typeParam;
    while (!typeStr.empty() && (typeStr.back() == ' ' || typeStr.back() == '\t')) typeStr.pop_back();
    while (!typeStr.empty() && (typeStr.front() == ' ' || typeStr.front() == '\t')) typeStr.erase(0, 1);
    if (typeStr.empty()) typeStr = "P";
    char typeChar = typeStr.empty() ? 'P' : static_cast<char>(typeStr[0]);
    if (typeChar != 'P' && typeChar != 'p' && typeChar != 'E' && typeChar != 'e' && typeChar != 'L' && typeChar != 'l') {
        return "Error: tipo no soportado en esta fase";
    }
    const bool isLogical = (typeChar == 'L' || typeChar == 'l');
    const bool isExtended = (typeChar == 'E' || typeChar == 'e');

    long long sizeBytes;
    if (unit == "B" || unit == "b") {
        sizeBytes = size;
    } else if (unit == "K" || unit == "k") {
        sizeBytes = static_cast<long long>(size) * 1024;
    } else if (unit == "M" || unit == "m") {
        sizeBytes = static_cast<long long>(size) * 1024 * 1024;
    } else {
        return "Error: unit debe ser B, K o M";
    }

    char partFit = 'F';
    if (fit == "BF" || fit == "bf") partFit = 'B';
    else if (fit == "WF" || fit == "wf") partFit = 'W';
    else if (fit == "FF" || fit == "ff") partFit = 'F';
    else if (!fit.empty()) {
        return "Error: fit debe ser BF, FF o WF";
    }

    if (name.empty()) {
        return "Error: name es obligatorio";
    }
    if (name.size() > 15) {
        return "Error: name no puede exceder 15 caracteres";
    }

    std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        return "Error: no se pudo abrir el disco";
    }

    MBR mbr;
    file.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(MBR))) {
        return "Error: no se pudo leer el MBR";
    }

    if (isLogical) {
        std::string result = runFdiskLogical(file, mbr, static_cast<int>(sizeBytes), name, partFit);
        file.close();
        return result;
    }

    int freeSlot = -1;
    for (int i = 0; i < 4; ++i) {
        if (!partitionUsed(mbr.mbr_partitions[i])) {
            freeSlot = i;
            break;
        }
    }
    if (freeSlot < 0) {
        return "Error: no hay espacio para más particiones (máximo 4)";
    }

    if (isExtended) {
        for (int i = 0; i < 4; ++i) {
            if (mbr.mbr_partitions[i].part_type == 'E') {
                return "Error: ya existe una partición extendida";
            }
        }
    }

    for (int i = 0; i < 4; ++i) {
        if (!partitionUsed(mbr.mbr_partitions[i])) continue;
        const Partition& p = mbr.mbr_partitions[i];
        size_t len = 0;
        while (len < 16 && p.part_name[len] != '\0') ++len;
        std::string existing(p.part_name, len);
        if (existing == name) {
            return "Error: ya existe una partición con ese nombre";
        }
    }

    std::vector<FreeRegion> regions;
    collectFreeRegions(mbr, regions);

    int idx = chooseRegionByFit(regions, static_cast<int>(sizeBytes), mbr.dsk_fit);
    if (idx < 0) {
        return "Error: no hay espacio suficiente en el disco";
    }

    int partStart = regions[idx].start;

    Partition& p = mbr.mbr_partitions[freeSlot];
    std::memset(&p, 0, sizeof(Partition));
    p.part_status = '1';
    p.part_type = isExtended ? 'E' : 'P';
    p.part_fit = partFit;
    p.part_start = partStart;
    p.part_s = static_cast<int>(sizeBytes);
    std::strncpy(p.part_name, name.c_str(), 15);
    p.part_name[15] = '\0';
    p.part_correlative = -1;  // PDF: "inicialmente -1 hasta que sea montado"
    std::memset(p.part_id, 0, sizeof(p.part_id));

    file.seekp(0);
    file.write(reinterpret_cast<const char*>(&mbr), sizeof(MBR));
    if (!file) {
        return "Error: no se pudo escribir el MBR";
    }
    file.close();

    return isExtended ? "Partición extendida creada exitosamente"
                      : "Partición primaria creada exitosamente";
}

} // namespace commands
} // namespace extreamfs
