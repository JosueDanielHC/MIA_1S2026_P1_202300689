#include "commands/disk/rmdisk.h"
#include "manager/mount_manager.h"
#include <filesystem>
#include <string>

namespace extreamfs {
namespace commands {

std::string runRmdisk(const std::string& path) {
    if (path.empty()) {
        return "Error: se requiere -path";
    }
    if (!std::filesystem::exists(path)) {
        return "Error: el disco no existe";
    }
    if (manager::isDiskPathMounted(path)) {
        return "Error: no se puede eliminar el disco, tiene particiones montadas";
    }
    std::error_code ec;
    if (!std::filesystem::remove(path, ec)) {
        return "Error: no se pudo eliminar el disco";
    }
    return "Disco eliminado exitosamente";
}

} // namespace commands
} // namespace extreamfs
