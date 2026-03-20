#include "fs_manager.h"
#include "disk/mbr.h"
#include "filesystem/superblock.h"
#include "filesystem/inode.h"
#include "filesystem/blocks.h"
#include <ctime>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace extreamfs {
namespace manager {

namespace {

const int SUPERBLOCK_SIZE = 68;
const int INODE_SIZE = 88;
const int BLOCK_SIZE = 64;
const int USERS_TXT_INODE_INDEX = 1;
const int MAX_DIRECT_BLOCKS = 12;

static bool partitionUsed(const Partition& p) {
    return p.part_status != '0' || p.part_type != '0' || p.part_start >= 0;
}

static std::string getPartName(const char* name, size_t maxLen) {
    size_t len = 0;
    while (len < maxLen && name[len] != '\0') ++len;
    return std::string(name, len);
}

} // namespace

bool getPartitionBounds(const std::string& path, const std::string& partName,
                        int& part_start, int& part_s) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    MBR mbr;
    file.read(reinterpret_cast<char*>(&mbr), sizeof(MBR));
    if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(MBR)))
        return false;
    for (int i = 0; i < 4; ++i) {
        const Partition& p = mbr.mbr_partitions[i];
        if (partitionUsed(p) && p.part_type == 'P' && getPartName(p.part_name, 16) == partName) {
            part_start = p.part_start;
            part_s = p.part_s;
            return true;
        }
    }
    return false;
}

bool readUsersTxt(const std::string& path, int part_start, int part_s,
                  std::string& outContent, std::string& outError) {
    outContent.clear();
    outError.clear();
    std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        outError = "Error: no se pudo abrir el disco";
        return false;
    }
    Superblock sb;
    file.seekg(part_start);
    file.read(reinterpret_cast<char*>(&sb), sizeof(sb));
    if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(sb))) {
        outError = "Error: no se pudo leer el Superbloque";
        return false;
    }
    if (sb.s_magic != 0xEF53) {
        outError = "Error: la partición no está formateada como EXT2";
        return false;
    }
    Inode inode;
    const int inodePos = sb.s_inode_start + USERS_TXT_INODE_INDEX * sb.s_inode_s;
    file.seekg(part_start + inodePos);
    file.read(reinterpret_cast<char*>(&inode), sizeof(Inode));
    if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(Inode))) {
        outError = "Error: no se pudo leer el inodo de users.txt";
        return false;
    }
    const int totalBytes = inode.i_s;
    if (totalBytes <= 0) {
        return true;
    }
    std::vector<char> content(static_cast<size_t>(totalBytes), 0);
    int offset = 0;
    for (int i = 0; i < MAX_DIRECT_BLOCKS && offset < totalBytes; ++i) {
        if (inode.i_block[i] < 0) break;
        const int blockIndex = inode.i_block[i];
        const int blockPos = part_start + sb.s_block_start + blockIndex * sb.s_block_s;
        const int toRead = std::min(BLOCK_SIZE, totalBytes - offset);
        file.seekg(blockPos);
        file.read(content.data() + offset, toRead);
        if (!file) {
            outError = "Error: no se pudo leer bloque de users.txt";
            return false;
        }
        offset += toRead;
    }
    outContent.assign(content.data(), static_cast<size_t>(totalBytes));
    return true;
}

bool writeUsersTxt(const std::string& path, int part_start, int part_s,
                   const std::string& content, std::string& outError) {
    outError.clear();
    std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        outError = "Error: no se pudo abrir el disco";
        return false;
    }
    Superblock sb;
    file.seekg(part_start);
    file.read(reinterpret_cast<char*>(&sb), sizeof(sb));
    if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(sb))) {
        outError = "Error: no se pudo leer el Superbloque";
        return false;
    }
    const int n_blocks = sb.s_blocks_count;
    const int blocksNeeded = (static_cast<int>(content.size()) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    if (blocksNeeded > MAX_DIRECT_BLOCKS) {
        outError = "Error: users.txt excede el tamaño máximo soportado";
        return false;
    }
    Inode inode;
    const int inodePos = sb.s_inode_start + USERS_TXT_INODE_INDEX * sb.s_inode_s;
    file.seekg(part_start + inodePos);
    file.read(reinterpret_cast<char*>(&inode), sizeof(Inode));
    if (!file) {
        outError = "Error: no se pudo leer el inodo de users.txt";
        return false;
    }
    std::vector<char> bm_block(static_cast<size_t>(n_blocks), 0);
    file.seekg(part_start + sb.s_bm_block_start);
    file.read(bm_block.data(), n_blocks);
    if (!file) {
        outError = "Error: no se pudo leer el bitmap de bloques";
        return false;
    }
    std::vector<int> blockIndices;
    for (int i = 0; i < MAX_DIRECT_BLOCKS && inode.i_block[i] >= 0; ++i) {
        blockIndices.push_back(inode.i_block[i]);
    }
    const int initialBlockCount = static_cast<int>(blockIndices.size());
    while (static_cast<int>(blockIndices.size()) < blocksNeeded) {
        int freeBlock = -1;
        for (int b = 0; b < n_blocks; ++b) {
            if (bm_block[static_cast<size_t>(b)] == 0) {
                freeBlock = b;
                break;
            }
        }
        if (freeBlock < 0) {
            outError = "Error: no hay bloques libres en la partición";
            return false;
        }
        bm_block[static_cast<size_t>(freeBlock)] = 1;
        blockIndices.push_back(freeBlock);
    }
    for (int i = 0; i < blocksNeeded; ++i) {
        FileBlock fb;
        std::memset(&fb, 0, sizeof(fb));
        const size_t start = static_cast<size_t>(i * BLOCK_SIZE);
        const size_t len = std::min(static_cast<size_t>(BLOCK_SIZE), content.size() - start);
        if (len > 0) {
            std::memcpy(fb.b_content, content.data() + start, len);
        }
        const int blockPos = part_start + sb.s_block_start + blockIndices[i] * sb.s_block_s;
        file.seekp(blockPos);
        file.write(reinterpret_cast<const char*>(&fb), sizeof(FileBlock));
        if (!file) {
            outError = "Error: no se pudo escribir bloque de users.txt";
            return false;
        }
    }
    for (int i = 0; i < MAX_DIRECT_BLOCKS; ++i) {
        inode.i_block[i] = (i < static_cast<int>(blockIndices.size())) ? blockIndices[i] : -1;
    }
    inode.i_s = static_cast<int>(content.size());
    inode.i_mtime = static_cast<int>(time(nullptr));
    file.seekp(part_start + inodePos);
    file.write(reinterpret_cast<const char*>(&inode), sizeof(Inode));
    if (!file) {
        outError = "Error: no se pudo actualizar el inodo de users.txt";
        return false;
    }
    file.seekp(part_start + sb.s_bm_block_start);
    file.write(bm_block.data(), n_blocks);
    if (!file) {
        outError = "Error: no se pudo actualizar el bitmap de bloques";
        return false;
    }
    const int allocatedNew = blocksNeeded - initialBlockCount;
    if (allocatedNew > 0) {
        sb.s_free_blocks_count = sb.s_free_blocks_count - allocatedNew;
        if (sb.s_free_blocks_count < 0) sb.s_free_blocks_count = 0;
        file.seekp(part_start);
        file.write(reinterpret_cast<const char*>(&sb), sizeof(sb));
    }
    return true;
}

bool readSuperblock(const std::string& path, int part_start, Superblock& sb, std::string& outError) {
    outError.clear();
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        outError = "Error: no se pudo abrir el disco";
        return false;
    }
    file.seekg(part_start);
    file.read(reinterpret_cast<char*>(&sb), sizeof(Superblock));
    if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(Superblock))) {
        outError = "Error: no se pudo leer el Superbloque";
        return false;
    }
    if (sb.s_magic != 0xEF53) {
        outError = "Error: la partición no está formateada como EXT2";
        return false;
    }
    return true;
}

bool readInode(const std::string& path, int part_start, const Superblock& sb,
               int inodeIndex, Inode& out, std::string& outError) {
    outError.clear();
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        outError = "Error: no se pudo abrir el disco";
        return false;
    }
    const int pos = part_start + sb.s_inode_start + inodeIndex * sb.s_inode_s;
    file.seekg(pos);
    file.read(reinterpret_cast<char*>(&out), sizeof(Inode));
    if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(Inode))) {
        outError = "Error: no se pudo leer el inodo";
        return false;
    }
    return true;
}

bool writeInode(const std::string& path, int part_start, const Superblock& sb,
                int inodeIndex, const Inode& inode, std::string& outError) {
    outError.clear();
    std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        outError = "Error: no se pudo abrir el disco";
        return false;
    }
    const int pos = part_start + sb.s_inode_start + inodeIndex * sb.s_inode_s;
    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&inode), sizeof(Inode));
    if (!file) {
        outError = "Error: no se pudo escribir el inodo";
        return false;
    }
    return true;
}

bool readFolderBlock(const std::string& path, int part_start, const Superblock& sb,
                     int blockIndex, FolderBlock& out, std::string& outError) {
    outError.clear();
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        outError = "Error: no se pudo abrir el disco";
        return false;
    }
    const int pos = part_start + sb.s_block_start + blockIndex * sb.s_block_s;
    file.seekg(pos);
    file.read(reinterpret_cast<char*>(&out), sizeof(FolderBlock));
    if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(FolderBlock))) {
        outError = "Error: no se pudo leer el bloque";
        return false;
    }
    return true;
}

bool writeFolderBlock(const std::string& path, int part_start, const Superblock& sb,
                      int blockIndex, const FolderBlock& block, std::string& outError) {
    outError.clear();
    std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        outError = "Error: no se pudo abrir el disco";
        return false;
    }
    const int pos = part_start + sb.s_block_start + blockIndex * sb.s_block_s;
    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&block), sizeof(FolderBlock));
    if (!file) {
        outError = "Error: no se pudo escribir el bloque";
        return false;
    }
    return true;
}

bool readFileBlock(const std::string& path, int part_start, const Superblock& sb,
                   int blockIndex, FileBlock& out, std::string& outError) {
    outError.clear();
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        outError = "Error: no se pudo abrir el disco";
        return false;
    }
    const int pos = part_start + sb.s_block_start + blockIndex * sb.s_block_s;
    file.seekg(pos);
    file.read(reinterpret_cast<char*>(&out), sizeof(FileBlock));
    if (!file || file.gcount() != static_cast<std::streamsize>(sizeof(FileBlock))) {
        outError = "Error: no se pudo leer el bloque";
        return false;
    }
    return true;
}

bool writeFileBlock(const std::string& path, int part_start, const Superblock& sb,
                    int blockIndex, const FileBlock& block, std::string& outError) {
    outError.clear();
    std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        outError = "Error: no se pudo abrir el disco";
        return false;
    }
    const int pos = part_start + sb.s_block_start + blockIndex * sb.s_block_s;
    file.seekp(pos);
    file.write(reinterpret_cast<const char*>(&block), sizeof(FileBlock));
    if (!file) {
        outError = "Error: no se pudo escribir el bloque";
        return false;
    }
    return true;
}

int allocateInode(const std::string& path, int part_start, Superblock& sb, std::string& outError) {
    outError.clear();
    std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        outError = "Error: no se pudo abrir el disco";
        return -1;
    }
    const int n = sb.s_inodes_count;
    std::vector<char> bm(static_cast<size_t>(n), 0);
    file.seekg(part_start + sb.s_bm_inode_start);
    file.read(bm.data(), n);
    if (!file) {
        outError = "Error: no se pudo leer el bitmap de inodos";
        return -1;
    }
    int idx = -1;
    for (int i = 0; i < n; ++i) {
        if (bm[static_cast<size_t>(i)] == 0) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        outError = "Error: no hay inodos libres";
        return -1;
    }
    bm[static_cast<size_t>(idx)] = 1;
    file.seekp(part_start + sb.s_bm_inode_start);
    file.write(bm.data(), n);
    if (!file) {
        outError = "Error: no se pudo actualizar el bitmap de inodos";
        return -1;
    }
    sb.s_free_inodes_count--;
    if (sb.s_free_inodes_count < 0) sb.s_free_inodes_count = 0;
    file.seekp(part_start);
    file.write(reinterpret_cast<const char*>(&sb), sizeof(Superblock));
    return idx;
}

int allocateBlock(const std::string& path, int part_start, Superblock& sb, std::string& outError) {
    outError.clear();
    std::fstream file(path, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        outError = "Error: no se pudo abrir el disco";
        return -1;
    }
    const int n = sb.s_blocks_count;
    std::vector<char> bm(static_cast<size_t>(n), 0);
    file.seekg(part_start + sb.s_bm_block_start);
    file.read(bm.data(), n);
    if (!file) {
        outError = "Error: no se pudo leer el bitmap de bloques";
        return -1;
    }
    int idx = -1;
    for (int i = 0; i < n; ++i) {
        if (bm[static_cast<size_t>(i)] == 0) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        outError = "Error: no hay bloques libres";
        return -1;
    }
    bm[static_cast<size_t>(idx)] = 1;
    file.seekp(part_start + sb.s_bm_block_start);
    file.write(bm.data(), n);
    if (!file) {
        outError = "Error: no se pudo actualizar el bitmap de bloques";
        return -1;
    }
    sb.s_free_blocks_count--;
    if (sb.s_free_blocks_count < 0) sb.s_free_blocks_count = 0;
    file.seekp(part_start);
    file.write(reinterpret_cast<const char*>(&sb), sizeof(Superblock));
    return idx;
}

} // namespace manager
} // namespace extreamfs
