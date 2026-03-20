#ifndef FS_MANAGER_H
#define FS_MANAGER_H

#include "filesystem/superblock.h"
#include "filesystem/inode.h"
#include "filesystem/blocks.h"
#include <string>

namespace extreamfs {
namespace manager {

/**
 * Obtiene part_start y part_s de la partición primaria con el nombre dado.
 * @return true si se encontró la partición
 */
bool getPartitionBounds(const std::string& path, const std::string& partName,
                        int& part_start, int& part_s);

/** Lee el superbloque de la partición. */
bool readSuperblock(const std::string& path, int part_start, Superblock& sb, std::string& outError);

/** Lee un inodo por índice (0-based). */
bool readInode(const std::string& path, int part_start, const Superblock& sb,
               int inodeIndex, Inode& out, std::string& outError);

/** Escribe un inodo por índice. */
bool writeInode(const std::string& path, int part_start, const Superblock& sb,
                int inodeIndex, const Inode& inode, std::string& outError);

/** Lee un bloque de carpeta por índice. */
bool readFolderBlock(const std::string& path, int part_start, const Superblock& sb,
                     int blockIndex, FolderBlock& out, std::string& outError);

/** Escribe un bloque de carpeta por índice. */
bool writeFolderBlock(const std::string& path, int part_start, const Superblock& sb,
                      int blockIndex, const FolderBlock& block, std::string& outError);

/** Lee un bloque de archivo por índice. */
bool readFileBlock(const std::string& path, int part_start, const Superblock& sb,
                   int blockIndex, FileBlock& out, std::string& outError);

/** Escribe un bloque de archivo por índice. */
bool writeFileBlock(const std::string& path, int part_start, const Superblock& sb,
                    int blockIndex, const FileBlock& block, std::string& outError);

/** Asigna un inodo libre (marca bitmap, actualiza sb). Retorna índice o -1. */
int allocateInode(const std::string& path, int part_start, Superblock& sb, std::string& outError);

/** Asigna un bloque libre (marca bitmap, actualiza sb). Retorna índice o -1. */
int allocateBlock(const std::string& path, int part_start, Superblock& sb, std::string& outError);

/**
 * Lee el contenido completo de users.txt desde la partición ext2.
 * users.txt está en inodo 1 (creado por MKFS).
 * @param path Ruta del disco .mia
 * @param part_start Byte donde inicia la partición
 * @param part_s Tamaño de la partición en bytes
 * @param outContent Contenido del archivo (vacío si error)
 * @param outError Mensaje de error si falla
 * @return true si se leyó correctamente
 */
bool readUsersTxt(const std::string& path, int part_start, int part_s,
                  std::string& outContent, std::string& outError);

/**
 * Escribe el contenido completo en users.txt (inodo 1).
 * Asigna bloques adicionales si el contenido crece (hasta 12 bloques directos).
 * @return true si se escribió correctamente
 */
bool writeUsersTxt(const std::string& path, int part_start, int part_s,
                   const std::string& content, std::string& outError);

} // namespace manager
} // namespace extreamfs

#endif // FS_MANAGER_H
