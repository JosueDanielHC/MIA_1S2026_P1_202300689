#include "commands/fs/mkdir.h"
#include "manager/session_manager.h"
#include "manager/mount_manager.h"
#include "manager/fs_manager.h"
#include "filesystem/inode.h"
#include "filesystem/blocks.h"
#include <algorithm>
#include <cstring>
#include <ctime>
#include <sstream>
#include <string>
#include <vector>

namespace extreamfs {
namespace commands {

namespace {

const int MAX_DIRECT_BLOCKS = 12;
const int BLOCK_SIZE = 64;
const int CONTENT_NAME_SIZE = 12;

/** Parsea path en componentes: "/home/user/docs" -> ["home","user","docs"]. */
static std::vector<std::string> parsePathComponents(std::string path) {
    while (!path.empty() && path[0] == '/') path.erase(0, 1);
    if (path.empty()) return {};
    std::vector<std::string> comp;
    std::istringstream iss(path);
    std::string part;
    while (std::getline(iss, part, '/')) {
        if (!part.empty()) comp.push_back(part);
    }
    return comp;
}

/** Copia nombre a b_name (máx 11 caracteres + null). */
static void setContentName(Content& c, const std::string& name) {
    size_t len = std::min(name.size(), static_cast<size_t>(CONTENT_NAME_SIZE - 1));
    std::memcpy(c.b_name, name.c_str(), len);
    c.b_name[len] = '\0';
    for (size_t i = len + 1; i < static_cast<size_t>(CONTENT_NAME_SIZE); ++i)
        c.b_name[i] = '\0';
}

/** Compara b_name con name (hasta 12 chars). */
static bool contentNameEquals(const Content& c, const std::string& name) {
    size_t i = 0;
    while (i < static_cast<size_t>(CONTENT_NAME_SIZE) && c.b_name[i] != '\0' && i < name.size()) {
        if (c.b_name[i] != name[i]) return false;
        ++i;
    }
    if (i < name.size()) return false;
    if (i < static_cast<size_t>(CONTENT_NAME_SIZE) && c.b_name[i] != '\0') return false;
    return true;
}

/** Busca en el inodo (solo bloques directos) una entrada con ese nombre. Retorna índice de inodo o -1. */
static int findEntryInode(const std::string& path, int part_start, const Superblock& sb,
                         const Inode& inode, const std::string& name, std::string& err) {
    for (int b = 0; b < MAX_DIRECT_BLOCKS; ++b) {
        if (inode.i_block[b] < 0) break;
        FolderBlock fb;
        if (!manager::readFolderBlock(path, part_start, sb, inode.i_block[b], fb, err))
            return -2;
        for (int e = 0; e < 4; ++e) {
            if (fb.b_content[e].b_inodo >= 0 && contentNameEquals(fb.b_content[e], name))
                return fb.b_content[e].b_inodo;
        }
    }
    return -1;
}

/** Busca en el inodo un slot libre (b_inodo == -1). Retorna (blockIndex, contentIndex) o (-1,-1). */
static void findFreeSlot(const std::string& path, int part_start, const Superblock& sb,
                        const Inode& inode, int& outBlockIndex, int& outContentIndex, std::string& err) {
    outBlockIndex = -1;
    outContentIndex = -1;
    for (int b = 0; b < MAX_DIRECT_BLOCKS; ++b) {
        if (inode.i_block[b] < 0) break;
        FolderBlock fb;
        if (!manager::readFolderBlock(path, part_start, sb, inode.i_block[b], fb, err))
            return;
        for (int e = 0; e < 4; ++e) {
            if (fb.b_content[e].b_inodo < 0) {
                outBlockIndex = inode.i_block[b];
                outContentIndex = e;
                return;
            }
        }
    }
}

/** Añade entrada (nombre, inodeIndex) al directorio padre. Actualiza bloque(s) e inodo padre. */
static bool addEntryToDir(const std::string& path, int part_start, Superblock& sb,
                          Inode& parentInode, int parentInodeIndex, const std::string& name,
                          int newInodeIndex, std::string& err) {
    int blockIdx = -1, contentIdx = -1;
    findFreeSlot(path, part_start, sb, parentInode, blockIdx, contentIdx, err);
    if (blockIdx >= 0 && contentIdx >= 0) {
        FolderBlock fb;
        if (!manager::readFolderBlock(path, part_start, sb, blockIdx, fb, err)) return false;
        setContentName(fb.b_content[contentIdx], name);
        fb.b_content[contentIdx].b_inodo = newInodeIndex;
        if (!manager::writeFolderBlock(path, part_start, sb, blockIdx, fb, err)) return false;
    } else {
        int newBlockIdx = manager::allocateBlock(path, part_start, sb, err);
        if (newBlockIdx < 0) return false;
        int nextSlot = 0;
        for (; nextSlot < MAX_DIRECT_BLOCKS && parentInode.i_block[nextSlot] >= 0; ++nextSlot) {}
        if (nextSlot >= MAX_DIRECT_BLOCKS) {
            err = "Error: el directorio no admite más bloques directos";
            return false;
        }
        parentInode.i_block[nextSlot] = newBlockIdx;
        FolderBlock fb;
        std::memset(&fb, 0, sizeof(fb));
        for (int e = 0; e < 4; ++e) fb.b_content[e].b_inodo = -1;
        setContentName(fb.b_content[0], name);
        fb.b_content[0].b_inodo = newInodeIndex;
        if (!manager::writeFolderBlock(path, part_start, sb, newBlockIdx, fb, err)) return false;
        parentInode.i_s += BLOCK_SIZE;
    }
    parentInode.i_mtime = static_cast<int>(time(nullptr));
    if (!manager::writeInode(path, part_start, sb, parentInodeIndex, parentInode, err)) return false;
    return true;
}

} // namespace

std::string runMkdir(const std::string& pathParam, bool createParents) {
    if (!manager::isSessionActive()) {
        return "Error: no existe una sesión activa";
    }
    if (pathParam.empty()) {
        return "Error: falta el parámetro -path";
    }
    std::string pathDisk, partName;
    if (!manager::getMountById(manager::getSessionId(), pathDisk, partName)) {
        return "Error: la partición de la sesión no está montada";
    }
    int part_start = 0, part_s = 0;
    if (!manager::getPartitionBounds(pathDisk, partName, part_start, part_s)) {
        return "Error: no se pudo obtener la partición";
    }
    Superblock sb;
    std::string err;
    if (!manager::readSuperblock(pathDisk, part_start, sb, err)) {
        return err.empty() ? "Error: no se pudo leer el Superbloque" : err;
    }
    std::vector<std::string> components = parsePathComponents(pathParam);
    if (components.empty()) {
        return "Error: la ruta no es válida";
    }
    for (const auto& c : components) {
        if (c.size() >= static_cast<size_t>(CONTENT_NAME_SIZE)) {
            return "Error: el nombre de carpeta no puede superar 11 caracteres";
        }
    }
    int currentInodeIndex = 0;
    Inode currentInode;
    if (!manager::readInode(pathDisk, part_start, sb, 0, currentInode, err)) {
        return err.empty() ? "Error: no se pudo leer el inodo raíz" : err;
    }
    const int sessionUid = manager::getSessionUid();
    const int sessionGid = manager::getSessionGid();
    bool alreadyExisted = true;
    for (size_t i = 0; i < components.size(); ++i) {
        const std::string& name = components[i];
        int childInode = findEntryInode(pathDisk, part_start, sb, currentInode, name, err);
        if (childInode == -2) return err;
        if (childInode >= 0) {
            currentInodeIndex = childInode;
            if (!manager::readInode(pathDisk, part_start, sb, currentInodeIndex, currentInode, err))
                return err;
            continue;
        }
        alreadyExisted = false;
        if (i < components.size() - 1 && !createParents) {
            return "Error: no existe la carpeta intermedia";
        }
        int newInodeIdx = manager::allocateInode(pathDisk, part_start, sb, err);
        if (newInodeIdx < 0) return err;
        int newBlockIdx = manager::allocateBlock(pathDisk, part_start, sb, err);
        if (newBlockIdx < 0) return err;
        int now = static_cast<int>(time(nullptr));
        Inode newInode;
        std::memset(&newInode, 0, sizeof(newInode));
        newInode.i_uid = sessionUid;
        newInode.i_gid = sessionGid;
        newInode.i_s = BLOCK_SIZE;
        newInode.i_atime = now;
        newInode.i_ctime = now;
        newInode.i_mtime = now;
        newInode.i_block[0] = newBlockIdx;
        for (int j = 1; j < 15; ++j) newInode.i_block[j] = -1;
        newInode.i_type = 0;
        newInode.i_perm[0] = '6';
        newInode.i_perm[1] = '6';
        newInode.i_perm[2] = '4';
        FolderBlock newBlock;
        std::memset(&newBlock, 0, sizeof(newBlock));
        for (int e = 0; e < 4; ++e) newBlock.b_content[e].b_inodo = -1;
        setContentName(newBlock.b_content[0], ".");
        newBlock.b_content[0].b_inodo = newInodeIdx;
        setContentName(newBlock.b_content[1], "..");
        newBlock.b_content[1].b_inodo = currentInodeIndex;
        if (!manager::writeFolderBlock(pathDisk, part_start, sb, newBlockIdx, newBlock, err))
            return err;
        if (!manager::writeInode(pathDisk, part_start, sb, newInodeIdx, newInode, err))
            return err;
        if (!addEntryToDir(pathDisk, part_start, sb, currentInode, currentInodeIndex, name, newInodeIdx, err))
            return err;
        currentInodeIndex = newInodeIdx;
        if (!manager::readInode(pathDisk, part_start, sb, currentInodeIndex, currentInode, err))
            return err;
    }
    if (alreadyExisted) {
        return "La carpeta ya existe.";
    }
    return "Carpeta creada correctamente.";
}

} // namespace commands
} // namespace extreamfs
