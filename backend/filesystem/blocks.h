#ifndef BLOCKS_H
#define BLOCKS_H

#pragma pack(push, 1)

/**
 * Entrada de contenido en un bloque de carpeta.
 * 12 chars + 4 bytes = 16 bytes por entrada.
 */
struct Content {
    char b_name[12];  // Nombre del archivo o carpeta
    int b_inodo;      // Apuntador al inodo asociado
};

/**
 * Bloque de carpeta - 4 entradas = 64 bytes.
 * En el primer apuntador directo, los dos primeros registros son . y ..
 */
struct FolderBlock {
    Content b_content[4];
};

/**
 * Bloque de archivo - Contenido de datos (64 bytes).
 */
struct FileBlock {
    char b_content[64];
};

/**
 * Bloque de apuntadores - Para indirectos simple, doble y triple.
 * 16 int * 4 bytes = 64 bytes.
 */
struct PointerBlock {
    int b_pointers[16];
};

#pragma pack(pop)

#endif // BLOCKS_H
