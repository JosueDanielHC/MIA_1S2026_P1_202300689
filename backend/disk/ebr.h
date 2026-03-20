#ifndef EBR_H
#define EBR_H

#pragma pack(push, 1)

/**
 * Extended Boot Record - Descriptor de unidad lógica.
 * Lista enlazada de EBRs dentro de la partición extendida.
 */
struct EBR {
    char part_mount;     // Indica si la partición está montada o no
    char part_fit;       // B (Best), F (First), W (Worst)
    int part_start;      // Byte donde inicia la partición
    int part_s;          // Tamaño total en bytes
    int part_next;       // Byte del próximo EBR (-1 si no hay siguiente)
    char part_name[16];  // Nombre de la partición
};

#pragma pack(pop)

#endif // EBR_H
