#ifndef PARTITION_H
#define PARTITION_H

#include <cstddef>

#pragma pack(push, 1)

/**
 * Estructura de una partición (primaria o extendida) en el MBR.
 * Coincide con la especificación del proyecto EXT2.
 */
struct Partition {
    char part_status;    // Indica si la partición está montada o no
    char part_type;      // P = Primaria, E = Extendida
    char part_fit;       // B = Best, F = First, W = Worst
    int part_start;      // Byte donde inicia la partición
    int part_s;          // Tamaño total en bytes
    char part_name[16];  // Nombre de la partición
    int part_correlative;// Correlativo (-1 hasta montar, luego 1, 2, ...)
    char part_id[4];     // ID generado al montar (ej: 34A1)
};

#pragma pack(pop)

#endif // PARTITION_H
