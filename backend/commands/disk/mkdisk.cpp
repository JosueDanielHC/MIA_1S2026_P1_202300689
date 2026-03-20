#include "commands/disk/mkdisk.h"
#include "disk/mbr.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <ctime>
#include <chrono>

namespace extreamfs {
namespace commands {

namespace fs = std::filesystem;

static const size_t WRITE_BUFFER_SIZE = 1024;

static std::string runMkdiskImpl(int sizeBytes, const std::string& path,
                                char dskFit) {
    if (fs::exists(path)) {
        return "Error: el archivo ya existe";
    }

    fs::path p(path);
    if (p.has_parent_path()) {
        fs::path parent = p.parent_path();
        std::error_code ec;
        if (!fs::create_directories(parent, ec) && !fs::exists(parent)) {
            return "Error: no se pudieron crear las carpetas de la ruta";
        }
    }

    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return "Error: no se pudo crear el archivo";
    }

    char buffer[WRITE_BUFFER_SIZE];
    std::memset(buffer, 0, sizeof(buffer));
    size_t remaining = static_cast<size_t>(sizeBytes);
    while (remaining > 0) {
        size_t toWrite = (remaining < sizeof(buffer)) ? remaining : sizeof(buffer);
        file.write(buffer, static_cast<std::streamsize>(toWrite));
        if (!file) {
            file.close();
            fs::remove(path);
            return "Error: fallo al escribir el archivo";
        }
        remaining -= toWrite;
    }

    MBR mbr;
    std::memset(&mbr, 0, sizeof(mbr));
    mbr.mbr_tamano = sizeBytes;
    mbr.mbr_fecha_creacion = std::time(nullptr);
    mbr.mbr_dsk_signature = static_cast<int>(std::random_device{}());
    mbr.dsk_fit = dskFit;

    for (int i = 0; i < 4; ++i) {
        mbr.mbr_partitions[i].part_status = '0';
        mbr.mbr_partitions[i].part_type = '0';
        mbr.mbr_partitions[i].part_fit = '0';
        mbr.mbr_partitions[i].part_start = -1;
        mbr.mbr_partitions[i].part_s = 0;
        std::memset(mbr.mbr_partitions[i].part_name, 0, sizeof(mbr.mbr_partitions[i].part_name));
        mbr.mbr_partitions[i].part_correlative = 0;
        std::memset(mbr.mbr_partitions[i].part_id, 0, sizeof(mbr.mbr_partitions[i].part_id));
    }

    file.seekp(0);
    file.write(reinterpret_cast<const char*>(&mbr), sizeof(MBR));
    if (!file) {
        file.close();
        fs::remove(path);
        return "Error: no se pudo escribir el MBR";
    }

    file.close();
    return "Disco creado exitosamente";
}

std::string runMkdisk(int size, const std::string& path,
                     const std::string& unit, const std::string& fit) {
    if (size <= 0) {
        return "Error: size debe ser positivo";
    }

    long long sizeBytes;
    if (unit == "K" || unit == "k") {
        sizeBytes = static_cast<long long>(size) * 1024;
    } else if (unit == "M" || unit == "m") {
        sizeBytes = static_cast<long long>(size) * 1024 * 1024;
    } else {
        return "Error: unit debe ser K o M";
    }

    char dskFit = 'F';
    if (fit == "BF" || fit == "bf") {
        dskFit = 'B';
    } else if (fit == "WF" || fit == "wf") {
        dskFit = 'W';
    } else if (fit == "FF" || fit == "ff") {
        dskFit = 'F';
    } else if (!fit.empty()) {
        return "Error: fit debe ser BF, FF o WF";
    }

    return runMkdiskImpl(static_cast<int>(sizeBytes), path, dskFit);
}

} // namespace commands
} // namespace extreamfs
