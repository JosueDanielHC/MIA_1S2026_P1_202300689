#include "commands/fs/mkfs.h"
#include "manager/mount_manager.h"
#include "disk/mbr.h"
#include "filesystem/superblock.h"
#include "filesystem/inode.h"
#include "filesystem/blocks.h"
#include <cstring>
#include <ctime>
#include <fstream>
#include <string>
#include <vector>

namespace extreamfs {
namespace commands {

namespace {

const int SUPERBLOCK_SIZE = 68;
const int INODE_SIZE = 88;
const int BLOCK_SIZE = 64;
/** PDF: tamaño_particion = sizeOf(superblock) + n + 3n + n*sizeOf(inodos) + 3*n*sizeOf(block) => n = (part_s - 68) / (4 + 88 + 192) */
const int DENOM_N = 4 + INODE_SIZE + 3 * BLOCK_SIZE;  // 284

static bool partitionUsed(const Partition& p) {
    return p.part_status != '0' || p.part_type != '0' || p.part_start >= 0;
}

static std::string getName(const char* name, size_t maxLen) {
    size_t len = 0;
    while (len < maxLen && name[len] != '\0') ++len;
    return std::string(name, len);
}

/** Busca partición primaria por nombre en el MBR. Devuelve part_start y part_s si existe. */
static bool findPrimaryByName(const MBR& mbr, const std::string& name, int& outStart, int& outSize) {
    for (int i = 0; i < 4; ++i) {
        const Partition& p = mbr.mbr_partitions[i];
        if (partitionUsed(p) && p.part_type == 'P' && getName(p.part_name, 16) == name) {
            outStart = p.part_start;
            outSize = p.part_s;
            return true;
        }
    }
    return false;
}

/** Contenido obligatorio de users.txt según PDF. */
static const char USERS_TXT_CONTENT[] = "1,G,root\n1,U,root,root,123\n";
static const int USERS_TXT_LEN = sizeof(USERS_TXT_CONTENT) - 1;  // sin '\0'

} // namespace

std::string runMkfs(const std::string& id) {
    if (id.empty()) {
        return "Error: falta el parámetro -id";
    }

    std::string path, name;
    if (!manager::getMountById(id, path, name)) {
        return "Error: el id no existe o la partición no está montada";
    }

    std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        return "Error: no se pudo abrir el disco";
    }

    MBR mbr;
    file.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(MBR))) {
        file.close();
        return "Error: no se pudo leer el MBR";
    }

    int part_start = 0, part_s = 0;
    if (!findPrimaryByName(mbr, name, part_start, part_s)) {
        file.close();
        return "Error: no se encontró la partición primaria con ese nombre";
    }

    if (part_s < SUPERBLOCK_SIZE + DENOM_N) {
        file.close();
        return "Error: la partición es demasiado pequeña para formatear";
    }

    int n = (part_s - SUPERBLOCK_SIZE) / DENOM_N;
    if (n < 2) {
        file.close();
        return "Error: la partición no tiene espacio suficiente para inodos y bloques mínimos";
    }

    int n_inodes = n;
    int n_blocks = 3 * n;

    int bm_inode_start = part_start + SUPERBLOCK_SIZE;
    int bm_block_start = bm_inode_start + n_inodes;
    int inode_start = bm_block_start + n_blocks;
    int block_start = inode_start + n_inodes * INODE_SIZE;

    if (block_start + n_blocks * BLOCK_SIZE > part_start + part_s) {
        file.close();
        return "Error: estructuras exceden el tamaño de la partición";
    }

    int now = static_cast<int>(time(nullptr));

    Superblock sb;
    std::memset(&sb, 0, sizeof(sb));
    sb.s_filesystem_type = 2;
    sb.s_inodes_count = n_inodes;
    sb.s_blocks_count = n_blocks;
    sb.s_free_inodes_count = n_inodes - 2;
    sb.s_free_blocks_count = n_blocks - 2;
    sb.s_mtime = now;
    sb.s_umtime = 0;
    sb.s_mnt_count = 0;
    sb.s_magic = 0xEF53;
    sb.s_inode_s = INODE_SIZE;
    sb.s_block_s = BLOCK_SIZE;
    sb.s_firts_ino = inode_start + 2 * INODE_SIZE;   // PDF: "dirección del inodo" = posición en bytes
    sb.s_first_blo = block_start + 2 * BLOCK_SIZE;   // PDF: "dirección del bloque" = posición en bytes
    sb.s_bm_inode_start = bm_inode_start;
    sb.s_bm_block_start = bm_block_start;
    sb.s_inode_start = inode_start;
    sb.s_block_start = block_start;

    file.seekp(part_start);
    file.write(reinterpret_cast<const char*>(&sb), sizeof(sb));
    if (!file) {
        file.close();
        return "Error: no se pudo escribir el Superbloque";
    }

    std::vector<char> bm_inode(static_cast<size_t>(n_inodes), 0);
    bm_inode[0] = 1;
    bm_inode[1] = 1;
    file.seekp(bm_inode_start);
    file.write(bm_inode.data(), n_inodes);
    if (!file) {
        file.close();
        return "Error: no se pudo escribir el bitmap de inodos";
    }

    std::vector<char> bm_block(static_cast<size_t>(n_blocks), 0);
    bm_block[0] = 1;
    bm_block[1] = 1;
    file.seekp(bm_block_start);
    file.write(bm_block.data(), n_blocks);
    if (!file) {
        file.close();
        return "Error: no se pudo escribir el bitmap de bloques";
    }

    Inode inodeRoot;
    std::memset(&inodeRoot, 0, sizeof(inodeRoot));
    inodeRoot.i_uid = 1;
    inodeRoot.i_gid = 1;
    inodeRoot.i_s = BLOCK_SIZE;
    inodeRoot.i_atime = now;
    inodeRoot.i_ctime = now;
    inodeRoot.i_mtime = now;
    inodeRoot.i_block[0] = 0;
    for (int i = 1; i < 15; ++i) inodeRoot.i_block[i] = -1;
    inodeRoot.i_type = 0;
    inodeRoot.i_perm[0] = '7';
    inodeRoot.i_perm[1] = '7';
    inodeRoot.i_perm[2] = '7';

    Inode inodeUsers;
    std::memset(&inodeUsers, 0, sizeof(inodeUsers));
    inodeUsers.i_uid = 1;
    inodeUsers.i_gid = 1;
    inodeUsers.i_s = USERS_TXT_LEN;
    inodeUsers.i_atime = now;
    inodeUsers.i_ctime = now;
    inodeUsers.i_mtime = now;
    inodeUsers.i_block[0] = 1;
    for (int i = 1; i < 15; ++i) inodeUsers.i_block[i] = -1;
    inodeUsers.i_type = 1;
    inodeUsers.i_perm[0] = '7';
    inodeUsers.i_perm[1] = '7';
    inodeUsers.i_perm[2] = '7';

    file.seekp(inode_start);
    file.write(reinterpret_cast<const char*>(&inodeRoot), sizeof(Inode));
    file.write(reinterpret_cast<const char*>(&inodeUsers), sizeof(Inode));
    for (int i = 2; i < n_inodes; ++i) {
        Inode empty;
        std::memset(&empty, 0, sizeof(empty));
        for (int j = 0; j < 15; ++j) empty.i_block[j] = -1;
        file.write(reinterpret_cast<const char*>(&empty), sizeof(Inode));
    }
    if (!file) {
        file.close();
        return "Error: no se pudo escribir la tabla de inodos";
    }

    FolderBlock rootBlock;
    std::memset(&rootBlock, 0, sizeof(rootBlock));
    std::strncpy(rootBlock.b_content[0].b_name, ".", 11);
    rootBlock.b_content[0].b_name[11] = '\0';
    rootBlock.b_content[0].b_inodo = 0;
    std::strncpy(rootBlock.b_content[1].b_name, "..", 11);
    rootBlock.b_content[1].b_name[11] = '\0';
    rootBlock.b_content[1].b_inodo = 0;
    std::strncpy(rootBlock.b_content[2].b_name, "users.txt", 11);
    rootBlock.b_content[2].b_name[11] = '\0';
    rootBlock.b_content[2].b_inodo = 1;
    rootBlock.b_content[3].b_name[0] = '\0';
    rootBlock.b_content[3].b_inodo = -1;

    file.seekp(block_start);
    file.write(reinterpret_cast<const char*>(&rootBlock), sizeof(FolderBlock));
    if (!file) {
        file.close();
        return "Error: no se pudo escribir el bloque de carpeta raíz";
    }

    FileBlock fileUsers;
    std::memset(&fileUsers, 0, sizeof(fileUsers));
    std::memcpy(fileUsers.b_content, USERS_TXT_CONTENT, static_cast<size_t>(USERS_TXT_LEN));
    file.seekp(block_start + BLOCK_SIZE);
    file.write(reinterpret_cast<const char*>(&fileUsers), sizeof(FileBlock));
    if (!file) {
        file.close();
        return "Error: no se pudo escribir el bloque de users.txt";
    }

    file.close();
    return "Partición formateada correctamente como EXT2.";
}

} // namespace commands
} // namespace extreamfs
