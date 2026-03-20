#include "commands/fs/cat.h"
#include "manager/session_manager.h"
#include "manager/mount_manager.h"
#include "manager/fs_manager.h"
#include "filesystem/blocks.h"
#include <sstream>
#include <string>
#include <vector>

namespace extreamfs {
namespace commands {

namespace {

static bool contentNameEquals(const Content& c, const std::string& name) {
    size_t i = 0;
    const size_t maxName = 12;
    while (i < maxName && c.b_name[i] != '\0' && i < name.size()) {
        if (c.b_name[i] != name[i]) return false;
        ++i;
    }
    if (i < name.size()) return false;
    if (i < maxName && c.b_name[i] != '\0') return false;
    return true;
}

static std::vector<std::string> splitPathComponents(const std::string& ruta) {
    std::vector<std::string> out;
    std::string s = ruta;
    while (!s.empty() && s[0] == '/') s.erase(0, 1);
    if (s.empty()) return out;
    std::istringstream iss(s);
    std::string part;
    while (std::getline(iss, part, '/')) {
        if (!part.empty()) out.push_back(part);
    }
    return out;
}

static int resolvePathToInode(const std::string& diskPath, int part_start, const Superblock& sb,
                              const std::string& ruta, std::string& outError) {
    std::vector<std::string> components = splitPathComponents(ruta);
    if (components.empty()) return -1;
    const int maxDirectBlocks = 12;
    int currentIdx = 0;
    Inode current;
    for (size_t i = 0; i + 1 < components.size(); ++i) {
        if (!manager::readInode(diskPath, part_start, sb, currentIdx, current, outError))
            return -2;
        if (current.i_type != 0) return -1;
        int childIdx = -1;
        for (int b = 0; b < maxDirectBlocks && current.i_block[b] >= 0; ++b) {
            FolderBlock fb;
            if (!manager::readFolderBlock(diskPath, part_start, sb, current.i_block[b], fb, outError))
                return -2;
            for (int e = 0; e < 4; ++e) {
                if (fb.b_content[e].b_inodo >= 0 && contentNameEquals(fb.b_content[e], components[i])) {
                    childIdx = fb.b_content[e].b_inodo;
                    break;
                }
            }
            if (childIdx >= 0) break;
        }
        if (childIdx < 0) return -1;
        currentIdx = childIdx;
    }
    if (!manager::readInode(diskPath, part_start, sb, currentIdx, current, outError))
        return -2;
    if (current.i_type != 0) return -1;
    const std::string& fileName = components.back();
    for (int b = 0; b < maxDirectBlocks && current.i_block[b] >= 0; ++b) {
        FolderBlock fb;
        if (!manager::readFolderBlock(diskPath, part_start, sb, current.i_block[b], fb, outError))
            return -2;
        for (int e = 0; e < 4; ++e) {
            if (fb.b_content[e].b_inodo >= 0 && contentNameEquals(fb.b_content[e], fileName))
                return fb.b_content[e].b_inodo;
        }
    }
    return -1;
}

/** Permiso de lectura: root siempre; si no, UGO según i_perm (octal, bit 4 = read). */
static bool canRead(const Inode& ino) {
    if (manager::isRoot()) return true;
    int uid = manager::getSessionUid();
    int gid = manager::getSessionGid();
    int owner = (ino.i_perm[0] >= '0' && ino.i_perm[0] <= '7') ? (ino.i_perm[0] - '0') : 0;
    int group = (ino.i_perm[1] >= '0' && ino.i_perm[1] <= '7') ? (ino.i_perm[1] - '0') : 0;
    int other = (ino.i_perm[2] >= '0' && ino.i_perm[2] <= '7') ? (ino.i_perm[2] - '0') : 0;
    int perm = (uid == ino.i_uid) ? owner : ((gid == ino.i_gid) ? group : other);
    return (perm & 4) != 0;
}

static bool readFileContent(const std::string& diskPath, int part_start, const Superblock& sb,
                            int inodeIdx, std::string& outContent, std::string& outError) {
    Inode ino;
    if (!manager::readInode(diskPath, part_start, sb, inodeIdx, ino, outError))
        return false;
    if (ino.i_type != 1) {
        outError = "No es un archivo";
        return false;
    }
    if (!canRead(ino)) {
        outError = "No tiene permiso de lectura";
        return false;
    }
    outContent.clear();
    outContent.reserve(static_cast<size_t>(ino.i_s > 0 ? ino.i_s : 0));
    int remaining = ino.i_s;
    for (int i = 0; i < 15 && remaining > 0; ++i) {
        if (ino.i_block[i] < 0) break;
        FileBlock fblk;
        if (!manager::readFileBlock(diskPath, part_start, sb, ino.i_block[i], fblk, outError))
            return false;
        size_t toTake = static_cast<size_t>(std::min(64, remaining));
        outContent.append(fblk.b_content, toTake);
        remaining -= static_cast<int>(toTake);
    }
    return true;
}

} // namespace

std::string runCat(const std::vector<std::string>& filePaths) {
    if (!manager::isSessionActive()) {
        return "Error: no existe una sesión activa";
    }
    std::string diskPath, partName;
    if (!manager::getMountById(manager::getSessionId(), diskPath, partName)) {
        return "Error: la partición de la sesión no está montada";
    }
    int part_start = 0, part_s = 0;
    if (!manager::getPartitionBounds(diskPath, partName, part_start, part_s)) {
        return "Error: no se pudo obtener la partición";
    }
    Superblock sb;
    std::string err;
    if (!manager::readSuperblock(diskPath, part_start, sb, err)) {
        return "Error: " + (err.empty() ? "no se puede leer el superbloque" : err);
    }
    if (sb.s_magic != 0xEF53) {
        return "Error: partición no es EXT2";
    }

    if (filePaths.empty()) {
        return "Error: se requiere al menos un archivo (-file1, -file2, ...)";
    }

    std::ostringstream out;
    for (size_t i = 0; i < filePaths.size(); ++i) {
        const std::string& ruta = filePaths[i];
        std::string pathError;
        int inodeIdx = resolvePathToInode(diskPath, part_start, sb, ruta, pathError);
        if (inodeIdx == -2) {
            return "Error: " + (pathError.empty() ? "error al leer" : pathError);
        }
        if (inodeIdx == -1) {
            return "Error: no existe el archivo o no tiene permiso de lectura";
        }
        std::string content;
        if (!readFileContent(diskPath, part_start, sb, inodeIdx, content, pathError)) {
            return "Error: " + (pathError.empty() ? "no se pudo leer el archivo" : pathError);
        }
        out << content;
    }
    return out.str();
}

} // namespace commands
} // namespace extreamfs
