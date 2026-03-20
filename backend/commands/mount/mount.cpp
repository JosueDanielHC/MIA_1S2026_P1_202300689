#include "commands/mount/mount.h"
#include "manager/mount_manager.h"
#include "disk/mbr.h"
#include "disk/ebr.h"
#include <cstring>
#include <fstream>
#include <string>

namespace extreamfs {
namespace commands {

namespace {

static bool partitionUsed(const Partition& p) {
    return p.part_status != '0' || p.part_type != '0' || p.part_start >= 0;
}

static std::string getName(const char* name, size_t maxLen) {
    size_t len = 0;
    while (len < maxLen && name[len] != '\0') ++len;
    return std::string(name, len);
}

struct FindResult {
    bool found = false;
    bool isPrimary = false;
    int mbrSlot = -1;
    int ebrPosition = -1;
};

static FindResult findPartitionByName(std::fstream& file, const MBR& mbr,
                                      const std::string& name) {
    FindResult r;
    for (int i = 0; i < 4; ++i) {
        const Partition& p = mbr.mbr_partitions[i];
        if (partitionUsed(p) && p.part_type == 'P') {
            if (getName(p.part_name, 16) == name) {
                r.found = true;
                r.isPrimary = true;
                r.mbrSlot = i;
                return r;
            }
        }
    }
    const Partition* ext = nullptr;
    for (int i = 0; i < 4; ++i) {
        if (mbr.mbr_partitions[i].part_type == 'E' && partitionUsed(mbr.mbr_partitions[i])) {
            ext = &mbr.mbr_partitions[i];
            break;
        }
    }
    if (!ext || ext->part_s <= 0) return r;

    int currentPos = ext->part_start;
    const int maxIter = 100;
    int iter = 0;
    EBR ebr;

    while (iter < maxIter) {
        file.seekg(currentPos);
        file.read(reinterpret_cast<char*>(&ebr), sizeof(EBR));
        if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(EBR))) break;
        if (ebr.part_s <= 0) break;
        if (getName(ebr.part_name, 16) == name) {
            r.found = true;
            r.isPrimary = false;
            r.ebrPosition = currentPos;
            return r;
        }
        if (ebr.part_next == -1) break;
        currentPos = ebr.part_next;
        ++iter;
    }
    return r;
}

} // namespace

std::string runMount(const std::string& path, const std::string& name) {
    if (path.empty()) return "Error: falta parámetro path";
    if (name.empty()) return "Error: falta parámetro name";

    if (manager::isMounted(path, name)) {
        return "Error: la partición ya está montada";
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

    FindResult fr = findPartitionByName(file, mbr, name);
    if (!fr.found) {
        return "Error: no existe la partición con ese nombre";
    }
    // PDF Observación 3: "solo se trabajarán los montajes con particiones primarias"
    if (!fr.isPrimary) {
        return "Error: solo se trabajan los montajes con particiones primarias";
    }

    std::string id = manager::addMounted(path, name);
    if (id.empty()) {
        return "Error: la partición ya está montada";
    }

    // PDF Observación 1: "Este comando debe realizar el montaje en memoria ram no debe escribir esto en el disco."
    // No se escribe MBR ni EBR; el estado de montaje y el ID solo viven en la tabla en RAM.
    file.close();
    return "Partición montada con ID: " + id;
}

} // namespace commands
} // namespace extreamfs
