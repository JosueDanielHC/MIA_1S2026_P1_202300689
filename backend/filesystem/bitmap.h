#ifndef BITMAP_H
#define BITMAP_H

/**
 * Bitmaps: 0 = libre (usable), 1 = ocupado.
 * - Bitmap de inodos: estado de cada inodo.
 * - Bitmap de bloques: estado de cada bloque (carpeta, archivo o apuntador).
 * Los bitmaps se almacenan como secuencia de bits en el disco.
 */
// Estructuras definidas en blocks.h e inode.h; los bitmaps son arrays de char o bits
// manejados por el fs_manager según s_bm_inode_start y s_bm_block_start.

#endif // BITMAP_H
