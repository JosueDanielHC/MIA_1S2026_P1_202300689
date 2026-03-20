#include "commands/fs/mkfile.h"
#include "commands/fs/mkdir.h"
#include "manager/session_manager.h"
#include "manager/mount_manager.h"
#include "manager/fs_manager.h"
#include "filesystem/inode.h"
#include "filesystem/blocks.h"
#include <cstring>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace extreamfs {
namespace commands {

namespace {

const int MAX_DIRECT_BLOCKS = 12;
const int BLOCK_SIZE = 64;
const int CONTENT_NAME_SIZE = 12;

/** Parsea path en componentes; el último es el nombre de archivo. */
static void parseFilePath(const std::string& pathParam,
                          std::vector<std::string>& dirComponents,
                          std::string& fileName) {
    dirComponents.clear();
    fileName.clear();
    std::string path = pathParam;
    while (!path.empty() && path[0] == '/') path.erase(0, 1);
    if (path.empty()) return;
    std::vector<std::string> all;
    std::istringstream iss(path);
    std::string part;
    while (std::getline(iss, part, '/')) {
        if (!part.empty()) all.push_back(part);
    }
    if (all.empty()) return;
    fileName = all.back();
    all.pop_back();
    dirComponents = std::move(all);
}

/** Construye ruta de directorio: ["home","user","docs"] -> "/home/user/docs". */
static std::string buildDirPath(const std::vector<std::string>& dirComponents) {
    if (dirComponents.empty()) return "/";
    std::string r = "/";
    for (size_t i = 0; i < dirComponents.size(); ++i) {
        if (i > 0) r += "/";
        r += dirComponents[i];
    }
    return r;
}

static void setContentName(Content& c, const std::string& name) {
    size_t len = std::min(name.size(), static_cast<size_t>(CONTENT_NAME_SIZE - 1));
    std::memcpy(c.b_name, name.c_str(), len);
    c.b_name[len] = '\0';
    for (size_t i = len + 1; i < static_cast<size_t>(CONTENT_NAME_SIZE); ++i)
        c.b_name[i] = '\0';
}

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

/** Resuelve componentes de directorio al inodo del directorio. Retorna índice o -1 si no existe. */
static int resolveDirPath(const std::string& pathDisk, int part_start, const Superblock& sb,
                          const std::vector<std::string>& dirComponents, std::string& err) {
    int currentIdx = 0;
    Inode current;
    if (!manager::readInode(pathDisk, part_start, sb, 0, current, err))
        return -2;
    for (const std::string& name : dirComponents) {
        int childIdx = -1;
        for (int b = 0; b < MAX_DIRECT_BLOCKS && current.i_block[b] >= 0; ++b) {
            FolderBlock fb;
            if (!manager::readFolderBlock(pathDisk, part_start, sb, current.i_block[b], fb, err))
                return -2;
            for (int e = 0; e < 4; ++e) {
                if (fb.b_content[e].b_inodo >= 0 && contentNameEquals(fb.b_content[e], name)) {
                    childIdx = fb.b_content[e].b_inodo;
                    break;
                }
            }
            if (childIdx >= 0) break;
        }
        if (childIdx < 0) return -1;
        currentIdx = childIdx;
        if (!manager::readInode(pathDisk, part_start, sb, currentIdx, current, err))
            return -2;
    }
    return currentIdx;
}

/** Busca en el inodo una entrada con ese nombre. Retorna índice de inodo o -1. */
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

/** Genera contenido automático: repite "0123456789" hasta tener exactamente size bytes. */
static std::string buildAutoContent(int size) {
    const char seq[] = "0123456789";
    const size_t seqLen = sizeof(seq) - 1;
    std::string out;
    out.reserve(static_cast<size_t>(size));
    for (int i = 0; i < size; ++i) {
        out += seq[i % seqLen];
    }
    return out;
}

/** Lee el archivo externo del host en modo binario. Retorna contenido o string vacío y err. */
static bool readExternalFile(const std::string& path, std::string& outContent, std::string& err) {
    outContent.clear();
    err.clear();
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        err = "Error: el archivo externo no existe";
        return false;
    }
    f.seekg(0, std::ios::end);
    std::streamsize sz = f.tellg();
    f.seekg(0, std::ios::beg);
    if (sz < 0) {
        err = "Error: no se pudo leer el archivo externo";
        return false;
    }
    outContent.resize(static_cast<size_t>(sz), '\0');
    f.read(&outContent[0], sz);
    if (!f) {
        err = "Error: no se pudo leer el archivo externo";
        return false;
    }
    return true;
}

} // namespace

std::string runMkfile(const std::string& pathParam, bool createParents, int sizeBytes,
                     const std::string& contPath) {
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
    if (sb.s_magic != 0xEF53) {
        return "Error: la partición no está formateada como EXT2";
    }
    std::vector<std::string> dirComponents;
    std::string fileName;
    parseFilePath(pathParam, dirComponents, fileName);
    if (fileName.empty()) {
        return "Error: la ruta debe incluir el nombre del archivo";
    }
    if (fileName.size() >= static_cast<size_t>(CONTENT_NAME_SIZE)) {
        return "Error: el nombre del archivo no puede superar 11 caracteres";
    }
    if (sizeBytes < 0) {
        sizeBytes = -1;
    }
    const bool hasCont = !contPath.empty();
    std::string externalContent;
    if (hasCont) {
        std::string readErr;
        if (!readExternalFile(contPath, externalContent, readErr)) {
            return readErr;
        }
    }
    const int externalSize = static_cast<int>(externalContent.size());

    int sizeFinal;
    std::string content;
    if (hasCont && sizeBytes < 0) {
        sizeFinal = externalSize;
        content = externalContent;
    } else if (hasCont && sizeBytes >= 0) {
        sizeFinal = sizeBytes;
        if (sizeBytes > externalSize) {
            content = externalContent + buildAutoContent(sizeBytes - externalSize);
        } else {
            content = externalContent.substr(0, static_cast<size_t>(sizeBytes));
        }
    } else if (sizeBytes >= 0) {
        sizeFinal = sizeBytes;
        content = buildAutoContent(sizeBytes);
    } else {
        sizeFinal = 0;
        content.clear();
    }

    const int blockSize = sb.s_block_s;
    int blocksNeeded = 0;
    if (sizeFinal > 0) {
        blocksNeeded = (sizeFinal + blockSize - 1) / blockSize;
        if (blocksNeeded > MAX_DIRECT_BLOCKS) {
            return "Error: el tamaño requiere más de 12 bloques (no se usan bloques indirectos)";
        }
        if (sb.s_free_blocks_count < blocksNeeded) {
            return "Error: no hay suficientes bloques libres en la partición";
        }
    }
    if (createParents && !dirComponents.empty()) {
        std::string parentPath = buildDirPath(dirComponents);
        std::string mkdirResult = runMkdir(parentPath, true);
        if (mkdirResult.size() >= 6 && mkdirResult.compare(0, 6, "Error:") == 0) {
            return mkdirResult;
        }
    }
    int parentInodeIndex = resolveDirPath(pathDisk, part_start, sb, dirComponents, err);
    if (parentInodeIndex == -2) return err;
    if (parentInodeIndex == -1) {
        return "Error: no existe la carpeta intermedia";
    }
    Inode parentInode;
    if (!manager::readInode(pathDisk, part_start, sb, parentInodeIndex, parentInode, err)) {
        return err;
    }
    int existing = findEntryInode(pathDisk, part_start, sb, parentInode, fileName, err);
    if (existing == -2) return err;
    if (existing >= 0) {
        return "El archivo ya existe.";
    }
    int newInodeIdx = manager::allocateInode(pathDisk, part_start, sb, err);
    if (newInodeIdx < 0) return err;
    int now = static_cast<int>(time(nullptr));
    Inode newInode;
    std::memset(&newInode, 0, sizeof(newInode));
    newInode.i_uid = manager::getSessionUid();
    newInode.i_gid = manager::getSessionGid();
    newInode.i_s = sizeFinal;
    newInode.i_atime = now;
    newInode.i_ctime = now;
    newInode.i_mtime = now;
    for (int j = 0; j < 15; ++j) newInode.i_block[j] = -1;
    newInode.i_type = 1;
    newInode.i_perm[0] = '6';
    newInode.i_perm[1] = '6';
    newInode.i_perm[2] = '4';

    if (sizeFinal > 0 && blocksNeeded > 0) {
        std::vector<int> blockIndices;
        for (int b = 0; b < blocksNeeded; ++b) {
            int blockIdx = manager::allocateBlock(pathDisk, part_start, sb, err);
            if (blockIdx < 0) return err;
            blockIndices.push_back(blockIdx);
        }
        for (int b = 0; b < blocksNeeded; ++b) {
            FileBlock fb;
            std::memset(&fb, 0, sizeof(fb));
            const int offset = b * blockSize;
            const int len = std::min(blockSize, sizeFinal - offset);
            if (len > 0) {
                std::memcpy(fb.b_content, content.data() + offset, static_cast<size_t>(len));
            }
            if (!manager::writeFileBlock(pathDisk, part_start, sb, blockIndices[b], fb, err)) {
                return err;
            }
            newInode.i_block[b] = blockIndices[b];
        }
    }

    if (!manager::writeInode(pathDisk, part_start, sb, newInodeIdx, newInode, err)) {
        return err;
    }
    if (!addEntryToDir(pathDisk, part_start, sb, parentInode, parentInodeIndex, fileName, newInodeIdx, err)) {
        return err;
    }
    return "Archivo creado correctamente.";
}

} // namespace commands
} // namespace extreamfs
