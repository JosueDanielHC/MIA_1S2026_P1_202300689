#include "utils/debug_mbr.h"
#include "disk/mbr.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace extreamfs {
namespace utils {

struct PartInfo {
    int index;
    std::string name;
    int start;
    int size;
    int end;
};

std::string debugMBR(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return "Error: no se pudo abrir el disco";
    }

    MBR mbr;
    file.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    file.close();
    if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(MBR))) {
        return "Error: no se pudo leer el MBR";
    }

    const int mbrSize = static_cast<int>(sizeof(MBR));
    std::vector<PartInfo> active;

    for (int i = 0; i < 4; ++i) {
        const auto& p = mbr.mbr_partitions[i];
        if (p.part_status != '1') continue;
        size_t len = 0;
        while (len < 16 && p.part_name[len] != '\0') ++len;
        std::string name(p.part_name, len);
        int start = p.part_start;
        int size = p.part_s;
        int end = start + size;
        active.push_back({i + 1, name, start, size, end});
    }

    std::sort(active.begin(), active.end(),
              [](const PartInfo& a, const PartInfo& b) { return a.start < b.start; });

    std::ostringstream out;
    for (size_t i = 0; i < active.size(); ++i) {
        const auto& pi = active[i];
        out << "Partición " << (i + 1) << ": start=" << pi.start
            << "  size=" << pi.size << "  end=" << pi.end << "\n";
    }

    for (const auto& pi : active) {
        if (pi.start < mbrSize) {
            return out.str() + "ERROR: Particiones solapadas detectadas";
        }
        if (pi.end > mbr.mbr_tamano) {
            return out.str() + "ERROR: Particiones solapadas detectadas";
        }
    }

    for (size_t i = 0; i + 1 < active.size(); ++i) {
        if (active[i].end > active[i + 1].start) {
            return out.str() + "ERROR: Particiones solapadas detectadas";
        }
    }

    out << "MBR válido. No hay solapamientos.";
    return out.str();
}

} // namespace utils
} // namespace extreamfs
