# Auditoría estricta: Inode, FolderBlock, FileBlock (PDF Proyecto 1 MIA)

Solo se usa lo definido explícitamente en el PDF. Sin Linux real, ni implementaciones de años anteriores, ni suposiciones.

---

## PARTE 1 — INODE

### 1) Definición textual del PDF (orden exacto)

| # | NOMBRE   | TIPO   | DESCRIPCIÓN (PDF) |
|---|----------|--------|-------------------|
| 1 | i_uid    | int    | UID del usuario propietario del archivo o carpeta |
| 2 | i_gid    | int    | GID del grupo al que pertenece el archivo o carpeta |
| 3 | i_s      | int    | Tamaño del archivo en bytes |
| 4 | i_atime  | time   | Última fecha en que se leyó el inodo sin modificarlo |
| 5 | i_ctime  | time   | Fecha en la que se creó el inodo |
| 6 | i_mtime  | time   | Última fecha en que se modifica el inodo |
| 7 | i_block  | int    | Array: primeros 12 = bloques directos; 13 = simple indirecto; 14 = doble; 15 = triple. No usados = -1. |
| 8 | i_type   | char   | 1 = Archivo, 0 = Carpeta |
| 9 | i_perm   | char[3]| Permisos UGO en forma octal |

### 2) Orden en el PDF

i_uid → i_gid → i_s → i_atime → i_ctime → i_mtime → i_block → i_type → i_perm

### 3) Struct actual del proyecto (tras corrección)

```cpp
#pragma pack(push, 1)
struct Inode {
    int i_uid;
    int i_gid;
    int i_s;
    int i_atime;         // PDF: time → int para layout fijo
    int i_ctime;         // PDF: time → int para layout fijo
    int i_mtime;         // PDF: time → int para layout fijo
    int i_block[15];
    char i_type;
    char i_perm[3];
};
#pragma pack(pop)
```

### 4) Comparación campo por campo

| # | Campo PDF | Tipo PDF | Campo código | Tipo código | ¿Coincide? |
|---|-----------|----------|--------------|-------------|------------|
| 1 | i_uid     | int      | i_uid     | int       | Sí |
| 2 | i_gid     | int      | i_gid     | int       | Sí |
| 3 | i_s       | int      | i_s       | int       | Sí |
| 4 | i_atime   | time     | i_atime   | int       | Sí* |
| 5 | i_ctime   | time     | i_ctime   | int       | Sí* |
| 6 | i_mtime   | time     | i_mtime   | int       | Sí* |
| 7 | i_block   | int (15) | i_block[15] | int[15] | Sí |
| 8 | i_type    | char     | i_type   | char      | Sí |
| 9 | i_perm    | char[3]  | i_perm[3]| char[3]   | Sí |

\* El PDF no define "time" como tipo de lenguaje; se usa **int** para tamaño fijo y compatibilidad binaria (igual que en Superblock).

### 5) sizeof(Inode)

**sizeof(Inode) = 88**

- 6× int (uid, gid, s, atime, ctime, mtime) = 24  
- 15× int (i_block) = 60  
- 1× char (i_type) = 1  
- 3× char (i_perm) = 3  
- Total con #pragma pack(push, 1): **88 bytes**

### 6) Coincidencia con la fórmula de MKFS

La fórmula del PDF es:  
`tamaño_particion = sizeOf(superblock) + n + 3*n + n*sizeOf(inodos) + 3*n*sizeOf(block)`  

**sizeOf(inodos)** = tamaño de un inodo = **sizeof(Inode) = 88**. El cálculo de n usa ese valor; es constante y correcto.

### 7) #pragma pack(push, 1)

Sí. El struct está envuelto en `#pragma pack(push, 1)` / `#pragma pack(pop)`.

---

## PARTE 2 — FOLDERBLOCK

### 1) Composición según el PDF

- **Estructura:** "Bloques de carpetas" con `b_content content[4]` (array de 4 entradas).
- **Estructura Content (b_content):**
  - **b_name** char[12] — Nombre de la carpeta o archivo  
  - **b_inodo** int — Apuntador hacia un inodo asociado al archivo o carpeta  
- **Tamaño en bytes (PDF):** "4 (estructuras b_content) * 12 (chars b_name) * 4 (int b_inodo) = 64"  
  Interpretación correcta: 4 entradas × (12 + 4) = **64 bytes** (cada entrada 16 bytes).

### 2) Verificación

| Requisito              | PDF                    | Código                         | ¿Coincide? |
|-------------------------|------------------------|--------------------------------|------------|
| Cantidad de entradas    | content[4]             | Content b_content[4]           | Sí |
| Tamaño por entrada      | b_name(12) + b_inodo(4)= 16 | b_name[12] + int = 16   | Sí |
| Tamaño total del bloque | 64 bytes               | 4 × 16 = 64                    | Sí |

### 3) Tamaño total 64 bytes

Sí. FolderBlock = 4 × sizeof(Content) = 4 × 16 = **64 bytes**.

### 4) sizeof(FolderBlock)

**sizeof(FolderBlock) = 64**

### 5) Padding

Con `#pragma pack(push, 1)` y Content = 12 + 4 = 16, no hay padding. 4 × 16 = 64.

### 6) Nombre máximo

PDF: **b_name char[12]** → nombre con máximo **12 caracteres** (incl. '\0'). El código usa `char b_name[12]`. Coincide.

---

## PARTE 3 — FILEBLOCK

### 1) Composición según el PDF

- **Estructura:** "Bloques de Archivos"  
- **b_content** char[64] — Array con el contenido del archivo  
- **Tamaño en bytes (PDF):** "64 (chars b_content)."

### 2) Bloque de exactamente 64 bytes

Sí. El bloque es el array b_content[64] = **64 bytes**.

### 3) Tamaño del array de contenido

PDF: **char[64]**. Código: **char b_content[64]**. Coincide.

### 4) sizeof(FileBlock)

**sizeof(FileBlock) = 64**

### 5) Padding

Un solo campo `char b_content[64]`; con pack(1) no hay padding. 64 bytes.

### 6) Coincidencia con sizeOf(block) de la fórmula

El PDF exige que "todos los bloques tengan un tamaño de 64 bytes". La fórmula usa **sizeOf(block)**; en el proyecto todos los tipos de bloque (FolderBlock, FileBlock, PointerBlock) tienen **sizeof = 64**. Correcto.

---

## VALIDACIÓN FINAL

### A) Inode → **CUMPLE**

- Nombres, orden y cantidad de campos coinciden con el PDF.  
- Tipo "time" del PDF representado como **int** para layout binario fijo (como en Superblock).  
- sizeof(Inode) = 88, estable y usado correctamente en la fórmula de MKFS.  
- #pragma pack(push, 1) presente.

### B) FolderBlock → **CUMPLE**

- b_content content[4] con Content = b_name[12] + b_inodo (int).  
- sizeof(FolderBlock) = 64, sin padding.  
- Nombre máximo 12 caracteres según PDF.

### C) FileBlock → **CUMPLE**

- b_content char[64].  
- sizeof(FileBlock) = 64, sin padding.  
- Coincide con sizeOf(block) de la fórmula.

---

## Corrección aplicada

- **Inode:** Se cambiaron **i_atime, i_ctime, i_mtime** de `time_t` a **int** para layout binario fijo y alineado con el criterio usado en Superblock (PDF no define "time" como tipo de implementación).  
- **FolderBlock y FileBlock:** No requirieron cambios; ya cumplían con el PDF.
