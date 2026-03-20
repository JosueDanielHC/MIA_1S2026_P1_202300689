#ifndef MBR_H
#define MBR_H

#include <ctime>
#include "partition.h"

#pragma pack(push, 1)

/**
 * Master Boot Record - Primer sector del disco.
 * Contiene información del sistema de archivos y las 4 particiones.
 */
struct MBR {
    int mbr_tamano;              // Tamaño total del disco en bytes
    time_t mbr_fecha_creacion;   // Fecha y hora de creación
    int mbr_dsk_signature;       // Número único que identifica el disco
    char dsk_fit;                // Ajuste: B (Best), F (First), W (Worst)
    Partition mbr_partitions[4]; // Las 4 particiones
};

#pragma pack(pop)

#endif // MBR_H
