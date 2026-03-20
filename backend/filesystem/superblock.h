#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#pragma pack(push, 1)

/**
 * Superbloque EXT2 - Información del sistema de archivos en la partición.
 * Se escribe una sola vez por partición formateada.
 * PDF: tipo "time" para s_mtime/s_umtime; se usa int para layout binario fijo.
 */
struct Superblock {
    int s_filesystem_type;   // 2 = EXT2
    int s_inodes_count;      // Número total de inodos
    int s_blocks_count;      // Número total de bloques
    int s_free_blocks_count; // Bloques libres
    int s_free_inodes_count; // Inodos libres
    int s_mtime;             // Última fecha de montaje (PDF: time → int para tamaño fijo)
    int s_umtime;            // Última fecha de desmontaje (PDF: time → int para tamaño fijo)
    int s_mnt_count;         // Veces que se ha montado
    int s_magic;             // 0xEF53 identificación EXT2
    int s_inode_s;           // Tamaño del inodo
    int s_block_s;           // Tamaño del bloque
    int s_firts_ino;         // Primer inodo libre (dirección)
    int s_first_blo;         // Primer bloque libre (dirección)
    int s_bm_inode_start;    // Inicio del bitmap de inodos
    int s_bm_block_start;    // Inicio del bitmap de bloques
    int s_inode_start;       // Inicio de la tabla de inodos
    int s_block_start;       // Inicio de la tabla de bloques
};

#pragma pack(pop)

#endif // SUPERBLOCK_H
