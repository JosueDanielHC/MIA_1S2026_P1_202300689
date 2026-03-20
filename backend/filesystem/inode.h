#ifndef INODE_H
#define INODE_H

#pragma pack(push, 1)

/**
 * Inodo (index node) - Información de un archivo o carpeta.
 * PDF: i_uid, i_gid, i_s, i_atime, i_ctime, i_mtime (time→int para tamaño fijo), i_block[15], i_type, i_perm[3].
 * Valores no usados en i_block = -1.
 */
struct Inode {
    int i_uid;           // UID del usuario propietario del archivo o carpeta
    int i_gid;           // GID del grupo al que pertenece el archivo o carpeta
    int i_s;             // Tamaño del archivo en bytes
    int i_atime;         // Última fecha en que se leyó el inodo (PDF: time → int)
    int i_ctime;         // Fecha en la que se creó el inodo (PDF: time → int)
    int i_mtime;         // Última fecha en que se modifica el inodo (PDF: time → int)
    int i_block[15];     // [0-11] directos, [12] simple indirecto, [13] doble, [14] triple
    char i_type;         // 1 = Archivo, 0 = Carpeta
    char i_perm[3];      // Permisos UGO en forma octal
};

#pragma pack(pop)

#endif // INODE_H
