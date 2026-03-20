#include "manager/mount_manager.h"
#include <algorithm>
#include <sstream>

namespace extreamfs {
namespace manager {

namespace {
    std::vector<MountEntry> mountedPartitions;
    // PDF p.20: "id ... utilizando el número de carnet: Últimos dos dígitos del Carnet + Número de Partición + Letra"
    const int CARNET = 202300689;
}

static std::vector<std::string> distinctPathsInOrder() {
    std::vector<std::string> out;
    for (const auto& e : mountedPartitions) {
        if (std::find(out.begin(), out.end(), e.path) == out.end()) {
            out.push_back(e.path);
        }
    }
    return out;
}

bool isMounted(const std::string& path, const std::string& name) {
    for (const auto& e : mountedPartitions) {
        if (e.path == path && e.name == name) return true;
    }
    return false;
}

bool isDiskPathMounted(const std::string& diskPath) {
    for (const auto& e : mountedPartitions) {
        if (e.path == diskPath) return true;
    }
    return false;
}

int getDiskLetterIndex(const std::string& path) {
    std::vector<std::string> paths = distinctPathsInOrder();
    auto it = std::find(paths.begin(), paths.end(), path);
    if (it != paths.end()) {
        return static_cast<int>(it - paths.begin());
    }
    return static_cast<int>(paths.size());
}

int getPartitionNumberForPath(const std::string& path) {
    int count = 0;
    for (const auto& e : mountedPartitions) {
        if (e.path == path) ++count;
    }
    return count + 1;
}

static std::string generateId(const std::string& path) {
    int letterIndex = getDiskLetterIndex(path);
    int partNum = getPartitionNumberForPath(path);
    if (letterIndex >= 26 || partNum < 1 || partNum > 9) {
        return "";
    }
    int lastTwo = CARNET % 100;  // Últimos dos dígitos del carnet
    char letter = static_cast<char>('A' + letterIndex);
    std::string id = (lastTwo < 10 ? "0" : "") + std::to_string(lastTwo) + std::to_string(partNum) + letter;
    return id;  // Ej: 341A, 342A, 341B (PDF ejemplo carnet 202401234)
}

std::string addMounted(const std::string& path, const std::string& name) {
    if (isMounted(path, name)) return "";
    std::string id = generateId(path);
    if (id.empty()) return "";
    mountedPartitions.push_back({path, name, id});
    return id;
}

std::string listMounted() {
    std::ostringstream out;
    for (const auto& e : mountedPartitions) {
        out << e.id << " -> " << e.name << "\n";
    }
    std::string s = out.str();
    if (!s.empty()) s.pop_back();
    return s.empty() ? "(ninguna partición montada)" : s;
}

bool getMountById(const std::string& id, std::string& outPath, std::string& outName) {
    for (const auto& e : mountedPartitions) {
        if (e.id == id) {
            outPath = e.path;
            outName = e.name;
            return true;
        }
    }
    return false;
}

} // namespace manager
} // namespace extreamfs
