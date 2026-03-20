# Diseño MKFS – Validación estructural 100% frente al PDF (Proyecto 1 MIA)

Este documento define **únicamente** lo que el PDF exige para MKFS. No se añade nada no pedido ni se simplifica ninguna obligación. Cualquier fórmula o detalle no explícito en el PDF debe **confirmarse con el enunciado** antes de implementar.

---

## 1. Resumen técnico: estado del disco tras MKFS según el PDF

Después de ejecutar MKFS sobre una partición **primaria montada** (identificada por **id**):

- La partición queda formateada como **sistema de archivos ext2**.
- Desde **part_start** hasta **part_start + part_s** de la partición se escribe:
  1. **Superbloque** (una vez, con todos los metadatos del sistema de archivos).
  2. **Bitmap de inodos** (1 byte por inodo: 0 = libre, 1 = ocupado).
  3. **Bitmap de bloques** (1 byte por bloque: 0 = libre, 1 = ocupado).
  4. **Tabla de inodos** (N inodos de tamaño fijo).
  5. **Tabla de bloques** (M bloques de 64 bytes cada uno).
- Se crea la **carpeta raíz** (directorio `/`).
- Se crea en la raíz el archivo **users.txt** con el contenido exacto indicado en el PDF.
- El usuario **root** tiene **privilegios absolutos** (según el PDF: siempre 777 sobre cualquier archivo/carpeta).
- **s_magic = 0xEF53** para identificación ext2.
- Bloques de **64 bytes** (PDF: "para este proyecto de procuró que todos los bloques tengan un tamaño de 64 bytes").

Todo lo que se escribe está **dentro** del rango de la partición; no se escribe fuera de **part_start** ni más allá de **part_start + part_s**.

---

## 2. Estructuras que deben escribirse y orden

Las posiciones son **respecto al inicio del archivo del disco** (no respecto al inicio de la partición), salvo que el PDF indique otra cosa. La partición ocupa desde `part_start` hasta `part_start + part_s - 1`.

**Orden y posiciones (dentro de la partición, desde `part_start`):**

| # | Estructura              | Posición en disco                    | Descripción |
|---|--------------------------|--------------------------------------|-------------|
| 1 | **Superbloque**          | `part_start + 0`                     | Una sola estructura Superblock. Campos según PDF (s_filesystem_type, s_inodes_count, s_blocks_count, s_free_*, s_mtime, s_umtime, s_mnt_count, s_magic, s_inode_s, s_block_s, s_firts_ino, s_first_blo, s_bm_inode_start, s_bm_block_start, s_inode_start, s_block_start). |
| 2 | **Bitmap de inodos**     | `part_start + sizeof(Superblock)`    | Secuencia de **n_inodes** bytes. Valor 0 = libre, 1 = ocupado. |
| 3 | **Bitmap de bloques**    | `s_bm_block_start`                   | Secuencia de **n_blocks** bytes. Valor 0 = libre, 1 = ocupado. |
| 4 | **Tabla de inodos**      | `s_inode_start`                      | **n_inodes** estructuras Inode consecutivas. |
| 5 | **Tabla de bloques**     | `s_block_start`                      | **n_blocks** bloques de 64 bytes (cada uno puede ser FolderBlock, FileBlock o PointerBlock). |

Los punteros del Superbloque (`s_bm_inode_start`, `s_bm_block_start`, `s_inode_start`, `s_block_start`) deben coincidir con las posiciones reales donde se escriben esas estructuras (todas dentro de la partición).

**Cálculo de posiciones (según orden de la ecuación del PDF):**

- `s_bm_inode_start` = `part_start + sizeof(Superblock)`
- Tamaño bitmap inodos = `n_inodes` bytes (1 byte por inodo).
- `s_bm_block_start` = `s_bm_inode_start + n_inodes`
- Tamaño bitmap bloques = `n_blocks` bytes.
- `s_inode_start` = `s_bm_block_start + n_blocks`
- Tamaño tabla inodos = `n_inodes * sizeof(Inode)`.
- `s_block_start` = `s_inode_start + n_inodes * sizeof(Inode)`
- Espacio bloques = `n_blocks * 64` bytes.

**Condición obligatoria:**  
`part_start + part_s >= s_block_start + (n_blocks * 64)` para que todo quepa en la partición.

---

## 3. Fórmulas exactas (según PDF)

El PDF debe definir explícitamente:

- **Relación (PDF):** "El número de bloques será el triple que el número de inodos." → n_bloques = 3 * n_inodos.

- **Cálculo del número de inodos (y por tanto de bloques) a partir del tamaño de la partición.**  
  La partición tiene tamaño `part_s`. El espacio útil para estructuras es `part_s - sizeof(Superblock)` (si el superbloque es lo único “fijo” antes de los bitmaps).  
  Fórmula literal del PDF (única permitida):  
  - Espacio usado por metadatos e índices = bitmap_inodos + bitmap_bloques + tabla_inodos + tabla_bloques.  
  - Con bytemap (1 byte por inodo, 1 byte por bloque):  
    `n_inodos` bytes + `n_bloques` bytes + `n_inodos * sizeof(Inode)` + `n_bloques * 64`.  
  - Si `n_bloques = 3 * n_inodos`:  
    espacio = `n_inodos * (1 + 3 + sizeof(Inode) + 3*64)` = `n_inodos * (4 + sizeof(Inode) + 192)`.  
  - Ecuación PDF: tamaño_particion = sizeOf(superblock) + n + 3*n + n*sizeOf(inodos) + 3*n*sizeOf(block); numero_estructuras = floor(n). Con part_s y sizeOf(block)=64: **n_inodos = floor((part_s - sizeof(Superblock)) / (4 + sizeof(Inode) + 192))**, **n_bloques = 3 * n_inodos**.  

La fórmula y el orden de estructuras son los indicados literalmente en el PDF (véase VALIDACION_MKFS_PDF_ESTRICTA.md).

---

## 4. Inicialización obligatoria de cada componente

### A) Superbloque

- **s_filesystem_type** = 2 (ext2).
- **s_inodes_count** = n_inodos (calculado según fórmula del PDF).
- **s_blocks_count** = n_bloques.
- **s_free_blocks_count** = n_bloques menos los bloques ya reservados para raíz y users.txt (ver D y E).
- **s_free_inodes_count** = n_inodos menos los inodos ya usados (raíz, users.txt, etc.).
- **s_mtime** = fecha de formateo: `(int)time(nullptr)` al ejecutar MKFS (Superblock usa int para layout fijo).
- **s_umtime** = 0 o `(int)time(nullptr)`; el PDF puede indicar valor inicial.
- **s_mnt_count** = 0 (o el valor que indique el PDF).
- **s_magic** = **0xEF53** (obligatorio en el enunciado).
- **s_inode_s** = sizeof(Inode).
- **s_block_s** = 64.
- **s_firts_ino** = primer inodo libre (índice o dirección según definición del PDF).
- **s_first_blo** = primer bloque libre.
- **s_bm_inode_start**, **s_bm_block_start**, **s_inode_start**, **s_block_start** = posiciones exactas en disco (bytes desde inicio del archivo) calculadas como en el apartado 2.

### B) Bitmap de inodos

- Todos los bytes a 0 (libre), excepto los correspondientes a inodos asignados (raíz, archivo users.txt, etc.), que van a 1.

### C) Bitmap de bloques

- Todos los bytes a 0 (libre), excepto los bloques usados por la raíz y por el contenido de users.txt, que van a 1.

### D) Carpeta raíz

- **Obligatorio según PDF.**  
- Se asigna un inodo para la raíz (por ejemplo inodo 0, si el PDF no reserva el 0 para otro uso).  
- Tipo **carpeta** (i_type = 0 según la estructura actual).  
- Un **FolderBlock** con al menos:
  - `.` → apuntando al inodo de la raíz.
  - `..` → apuntando al inodo de la raíz (raíz es su propio padre).
  - Entrada para `users.txt` → inodo del archivo users.txt.  
- Permisos del root según PDF: **privilegios absolutos**. En el esquema UGO en octal esto se traduce en **777** para la raíz (y para cualquier recurso donde el PDF indique que root tiene control total).

### E) Archivo users.txt en la raíz

- **Obligatorio según PDF.**  
- Nombre exacto: **users.txt**.  
- Ubicación: **raíz** (entrada en el FolderBlock de la raíz).  
- Contenido **exacto** (sin líneas extra al inicio):

  ```
  1,G,root
  1,U,root,root,123
  ```

  (Una línea por grupo, una por usuario; sin espacios extra salvo que el PDF los especifique; el PDF puede mostrar “íoot” por OCR pero debe implementarse “root”.)

### F) Permisos del root

- El PDF indica que el usuario root tiene **privilegios absolutos** (siempre 777 sobre cualquier archivo/carpeta).  
- En los inodos que pertenezcan a root (raíz y users.txt, y en general cuando el propietario sea root), **i_perm** debe reflejar **777** (por ejemplo `i_perm[0]='7', i_perm[1]='7', i_perm[2]='7'`).  
- **i_uid** / **i_gid** según corresponda al root (por ejemplo 1 si el primer usuario/grupo es root).

### G) Tabla de inodos y bloques

- Inodos no usados: inicializados con valores por defecto (por ejemplo i_type indicando “libre” o i_block con -1; según definición del PDF).  
- Bloques no usados: no es obligatorio rellenarlos; el bitmap ya indica que están libres (0).

---

## 5. Posiciones exactas (resumen)

Todas las posiciones son **en bytes desde el inicio del archivo del disco**:

- **Superbloque:** `part_start`.
- **Bitmap inodos:** `part_start + sizeof(Superblock)` (= `s_bm_inode_start`).
- **Bitmap bloques:** `s_bm_block_start` = `part_start + sizeof(Superblock) + n_inodes`.
- **Tabla inodos:** `s_inode_start` = `part_start + sizeof(Superblock) + n_inodes + n_blocks`.
- **Tabla bloques:** `s_block_start` = `s_inode_start + n_inodes * sizeof(Inode)`.

El Superbloque debe contener exactamente estos valores en `s_bm_inode_start`, `s_bm_block_start`, `s_inode_start`, `s_block_start` para que un lector pueda ubicar cada estructura.

---

## 6. Contradicciones con el diseño previo y correcciones

- **Nada de lo anterior contradice** MKDISK, FDISK ni MOUNT ya validados. MKFS solo escribe **dentro** de la partición (desde `part_start`), no modifica MBR, EBR ni particiones no formateadas.
- **Fórmula y orden:** Son los indicados literalmente en el PDF (ecuación de tamaño_particion y orden de sus sumandos). No se usa ninguna otra fuente.

---

## 7. Confirmación explícita de cumplimiento con el PDF

- **A) Sistema de archivos:** ext2, según el PDF.  
- **B) Cálculo de inodos y bloques:** Usar **únicamente** la fórmula que venga en el PDF; si no hay fórmula, validar con el docente la aquí propuesta.  
- **C) Inicialización:** Superbloque, bitmap de inodos, bitmap de bloques, tabla de inodos y bloques, con valores y posiciones como arriba (y como detalle el PDF).  
- **D) Carpeta raíz:** Creada obligatoriamente.  
- **E) users.txt:** Creado obligatoriamente en la raíz.  
- **F) Contenido de users.txt:** Exactamente las dos líneas indicadas.  
- **G) Permisos root:** 777 (privilegios absolutos) según el PDF.  
- **H) Fechas y conteos en el Superbloque:** s_mtime (y s_umtime, s_mnt_count si aplica) según el enunciado; conteos libres actualizados según inodos/bloques reservados.  
- **I) Posiciones:** Todas dentro de la partición; punteros del Superbloque coherentes con el orden y tamaños definidos.

**Validación:** La fórmula, el orden de estructuras y el tamaño de bloque provienen exclusivamente del texto del PDF (véase VALIDACION_MKFS_PDF_ESTRICTA.md). El diseño cumple estrictamente con el PDF.
