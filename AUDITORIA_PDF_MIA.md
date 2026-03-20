# Auditoría técnica – Proyecto 1 MIA (vs PDF enunciado)

Evaluación **exclusivamente** según lo exigido en el PDF. Sin buenas prácticas externas ni comportamiento de Linux real.

---

## Resumen por ítem (tras correcciones)

| Ítem | Resultado |
|------|-----------|
| 1) Estructuras (MBR, Partition, EBR, Superblock, Inode, FolderBlock, FileBlock) | **A) CUMPLE COMPLETAMENTE** |
| 2) Lógica MKDISK | **A) CUMPLE COMPLETAMENTE** |
| 3) Lógica FDISK | **A) CUMPLE COMPLETAMENTE** (part_correlative = -1 al crear) |
| 4) Lógica MOUNT | **A) CUMPLE COMPLETAMENTE** (ID por carnet, solo RAM, solo primarias) |
| 5) MKFS (diseño) | **Pendiente de implementación**; diseño debe incluir users.txt, ext2, bitmaps, inodos, raíz, root 777 |

---

## 1) ESTRUCTURAS

### MBR
**PDF:** mbr_tamano (int), mbr_fecha_creacion (time), mbr_dsk_signature (int), dsk_fit (char), mbr_partitions (partition[4]).

**Implementación:** Coincide. `#pragma pack(push,1)` presente.

**Resultado: A) CUMPLE COMPLETAMENTE**

---

### Partition
**PDF:** part_status (char), part_type (char P/E), part_fit (char), part_start (int), part_s (int), part_name (char[16]), part_correlative (int), part_id (char[4]).

**Implementación:** Todos los campos existen y tipos correctos.

**Resultado: A) CUMPLE COMPLETAMENTE**

---

### EBR
**PDF:** part_mount (char), part_fit (char), part_start (int), part_s (int), part_next (int), part_name (char[16]).

**Implementación:** Coincide.

**Resultado: A) CUMPLE COMPLETAMENTE**

---

### Superblock
**PDF:** s_filesystem_type, s_inodes_count, s_blocks_count, s_free_blocks_count, s_free_inodes_count, s_mtime, s_umtime, s_mnt_count, s_magic (0xEF53), s_inode_s, s_block_s, s_firts_ino, s_first_blo, s_bm_inode_start, s_bm_block_start, s_inode_start, s_block_start.

**Implementación:** Todos presentes (s_firts_ino como en el PDF, typo incluido).

**Resultado: A) CUMPLE COMPLETAMENTE**

---

### Inode
**PDF:** i_uid, i_gid, i_s, i_atime, i_ctime, i_mtime, i_block[15], i_type (char), i_perm (char[3]).

**Implementación:** Coincide.

**Resultado: A) CUMPLE COMPLETAMENTE**

---

### FolderBlock / Content
**PDF:** b_content content[4]; Content = b_name char[12], b_inodo int. Tamaño 4×16 = 64 bytes.

**Implementación:** Content con b_name[12] y b_inodo (int); FolderBlock con b_content[4]. Correcto.

**Resultado: A) CUMPLE COMPLETAMENTE**

---

### FileBlock
**PDF:** b_content char[64].

**Implementación:** Coincide.

**Resultado: A) CUMPLE COMPLETAMENTE**

---

### PointerBlock
**PDF:** b_pointers int[16], 64 bytes.

**Implementación:** Coincide.

**Resultado: A) CUMPLE COMPLETAMENTE**

---

## 2) LÓGICA MKDISK

**PDF:** Archivo binario .mia, lleno de ceros binarios; tamaño fijo; MBR en el primer sector; mbr_tamano, mbr_fecha_creacion, mbr_dsk_signature (random), dsk_fit; crear carpetas si no existen; buffer recomendado para escritura.

**Implementación:** Crea archivo con ceros (buffer 1024), crea directorios con std::filesystem, escribe MBR al inicio, inicializa las 4 particiones. Particiones: part_status='0', part_type='0', part_fit='0', part_start=-1, part_s=0, part_name vacío, part_correlative=0, part_id vacío.

**Inconsistencia:** El PDF indica que part_correlative “será inicialmente -1 hasta que sea montado”. Eso aplica a **particiones creadas** (FDISK), no a slots vacíos del MBR. En MKDISK los slots están “vacíos”; el PDF no define un valor concreto para esos casos. No se considera falta de cumplimiento.

**Resultado: A) CUMPLE COMPLETAMENTE**

---

## 3) LÓGICA FDISK

**PDF:** Primarias (P), extendida (E), lógicas (L); una sola extendida; lógicas dentro de la extendida; EBR encadenado con part_next; restricciones de teoría (máx. 4, una E, etc.); part_type P/E; ajustes BF/FF/WF; nombre obligatorio; -1 si no hay siguiente EBR.

**Implementación:** Primarias y extendida con regiones libres y FIT; una sola extendida; lógicas con EBR en part_start de la extendida, part_next encadenado; no escribe en part_start de la extendida más que el primer EBR; reescribe MBR/EBR correctamente.

**Inconsistencia:** El PDF dice: “part_correlative … será inicialmente -1 hasta que sea montado (luego la primera partición montada empezará en 1 e irán incrementando)”. Al **crear** una partición P o E con FDISK debe asignarse part_correlative = **-1**, no el número de slot. La implementación usa `part_correlative = freeSlot + 1` (1,2,3,4). El correlativo debe asignarse al **montar**, no al crear.

**Resultado: B) CUMPLE PARCIALMENTE** — Falta: en FDISK, al crear partición P o E, asignar **part_correlative = -1** en lugar de freeSlot+1.

---

## 4) LÓGICA MOUNT

**PDF (literal):**
- “Cada partición se identificará por un id que tendrá la siguiente estructura **utilizando el número de carnet**: **Últimos dos dígitos del Carnet + Número de Partición + Letra**. Ejemplo: carnet = 202401234. Id´s = 341A, 341B, 341C, 342A, 343A.”
- “Si es una partición del mismo disco se incrementa en uno el número de partición. Si la partición es de otro disco se debe colocar la letra siguiente (A,B,C,D…) y volver a iniciar en uno el número de partición.”
- **Observación 1:** “Este comando debe realizar el montaje **en memoria ram** no debe **escribir esto en el disco**.”
- **Observación 2:** “actualizar el atributo estatus de la estructura de partición y ajustar su correlativo … y … el valor del ID por el nuevo valor generado.”
- **Observación 3:** “**solo se trabajarán los montajes con particiones primarias**.”
- Número de partición inicia en 1.

**Implementación actual (tras correcciones):**
- ID con formato **últimos dos dígitos del carnet + número de partición + letra** (ej. 341A, 342A). Constante `CARNET` en `mount_manager.cpp` (cambiar por el carnet del estudiante).
- **No** se escribe en disco al montar (Observación 1); el montaje solo se mantiene en la tabla en RAM.
- Solo se permite montar **particiones primarias**; si se intenta montar una lógica se devuelve error (Observación 3).

**Resultado: A) CUMPLE COMPLETAMENTE** (con las correcciones aplicadas).

---

## 5) DISEÑO PROPUESTO PARA MKFS (no implementado aún)

Requisitos del PDF que debe cumplir el diseño:

- Formateo completo como **ext2**.
- Crear archivo en la **raíz** llamado **users.txt** con usuarios y contraseñas.
- Estructura users.txt: “GID, Tipo, Grupo \n” y “UID, Tipo, Grupo, Usuario, Contraseña \n”.
- **Al inicio** el contenido debe ser exactamente:
  - `1,G,root` (o “1, G, root” con espacios según redacción)
  - `1,U,root,root,123`
  (el PDF muestra “íoot” por OCR; debe ser “root”.)
- Sistema ext2: superbloque, bitmaps (inodos y bloques: 0 usable, 1 ocupado), tabla de inodos, tabla de bloques; bloques 64 bytes; número de bloques = 3× número de inodos; fórmula de tamaño de partición del PDF.
- Crear **carpeta raíz** (inodo 0 o el que se asigne a /).
- Permisos UGO en octal; usuario root con **privilegios absolutos** (siempre 777 sobre cualquier archivo/carpeta).

**Resultado (diseño):** No implementado. El diseño debe incluir: users.txt obligatorio con exactamente las dos líneas iniciales indicadas; ext2; bitmaps; inodos; carpeta raíz; permisos conforme al PDF; root con privilegios absolutos.

---

## CORRECCIONES APLICADAS EN CÓDIGO

1. **FDISK** (`backend/commands/disk/fdisk.cpp`): Al crear partición P o E se asigna **part_correlative = -1** (PDF: "inicialmente -1 hasta que sea montado").
2. **MOUNT** (`backend/commands/mount/mount.cpp` y `backend/manager/mount_manager.cpp`):  
   - Formato de ID según PDF: **últimos dos dígitos del carnet + número de partición + letra** (ej. 341A). Constante `CARNET` en `mount_manager.cpp` para que el estudiante ponga su carnet.  
   - No se escribe en disco al montar (Observación 1); solo tabla en RAM.  
   - Se rechaza el montaje de particiones lógicas; solo se permiten primarias (Observación 3).
