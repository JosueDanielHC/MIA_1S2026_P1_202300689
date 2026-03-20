# Validación final: s_firts_ino, s_first_blo, s_mnt_count (PDF Proyecto 1 MIA)

Solo se usa el texto literal del PDF. Sin suposiciones, Linux real ni proyectos de años anteriores.

---

## PARTE 1 — s_firts_ino y s_first_blo

### 1) y 2) Definición textual en el PDF

Del enunciado (tabla del Súper Bloque):

- **s_firts_ino** — int — *"Primer inodo libre (dirección del inodo)"*
- **s_first_blo** — int — *"Primer bloque libre (dirección del bloque)"*

Campos del mismo Superbloque para comparar:

- **s_bm_inode_start** — int — *"Guardará el inicio del bitmap de inodos"*
- **s_bm_block_start** — int — *"Guardará el inicio del bitmap de bloques"*
- **s_inode_start** — int — *"Guardará el inicio de la tabla de inodos"*
- **s_block_start** — int — *"Guardará el inicio de la tabla de bloques"*

En la misma tabla, **part_start** (Partición) se define como: *"Indica en qué byte del disco inicia la partición"*.

### 3) Índice (A) vs dirección/posición (B)

El PDF no usa la palabra "índice" para s_firts_ino ni s_first_blo.

Sí usa:

- **"dirección del inodo"** y **"dirección del bloque"** para estos dos campos.
- **"inicio"** / **"Guardará el inicio de"** para s_bm_inode_start, s_inode_start, s_block_start (posiciones en disco).
- **"en qué byte del disco"** para part_start.

En el mismo documento, "inicio" y "byte del disco" indican **posición en bytes**. "Dirección" se usa en el mismo contexto (campos del Superbloque que guardan posiciones). Por tanto, el enunciado define estos campos como **dirección/posición absoluta dentro de la partición (en bytes)**, no como número de inodo/bloque (índice lógico).

**Conclusión: B) Dirección/posición absoluta dentro de la partición.**

### 4) Valor que debe guardarse

Según el PDF:

- **s_firts_ino** debe ser la **dirección (en bytes)** del primer inodo libre.
- **s_first_blo** debe ser la **dirección (en bytes)** del primer bloque libre.

Tras MKFS, los primeros libres son el inodo de índice 2 y el bloque de índice 2. Sus posiciones son:

- Primer inodo libre: `inode_start + (2 * sizeof(Inode))` = `inode_start + (2 * 88)`.
- Primer bloque libre: `block_start + (2 * 64)`.

Por tanto:

- **s_firts_ino** = `inode_start + (2 * sizeof(Inode))` (dirección en bytes).
- **s_first_blo** = `block_start + (2 * sizeof(Block))` (dirección en bytes).

No debe guardarse solo **2** (índice).

### 5) Implementación actual vs PDF

- **Antes:** se guardaba `s_firts_ino = 2` y `s_first_blo = 2` (índice).
- **Exigencia del PDF:** guardar la **dirección** (posición en bytes).
- **Corrección aplicada:** asignar dirección absoluta:

  - `s_firts_ino = inode_start + 2 * INODE_SIZE`
  - `s_first_blo = block_start + 2 * BLOCK_SIZE`

---

## PARTE 2 — s_mnt_count

### 1) Descripción en el PDF

- **s_mnt_count** — int — *"Indica cuantas veces se ha montado el sistema"*

### 2) Inicio e interpretación

El PDF no dice:

- que deba iniciar en 1,
- que deba incrementarse en MKFS,
- ni que solo se modifique al montar.

Solo define el **significado**: número de veces que el sistema (esta partición formateada) **se ha montado**.

Tras ejecutar MKFS se ha **formateado** la partición; el comando **mount** es el que registra un montaje. Inmediatamente después de MKFS, la partición no se ha montado aún con ese comando, por tanto el número de veces que se ha montado es **0**.

### 3) Valor correcto tras MKFS

**s_mnt_count = 0** inmediatamente después de ejecutar MKFS.

La implementación actual ya asigna `sb.s_mnt_count = 0`. **Cumple.**

---

## FORMATO OBLIGATORIO DE RESPUESTA

### s_firts_ino

**A) Texto literal del PDF:**  
*"s_firts_ino — int — Primer inodo libre (dirección del inodo)"*

**B) Interpretación técnica:**  
El campo debe contener la **dirección (posición en bytes)** del primer inodo libre dentro de la partición, no el índice. Es decir: `inode_start + (índice_del_primer_libre * sizeof(Inode))`. Tras MKFS, primer libre índice 2: `inode_start + 2 * 88`.

**C) ¿La implementación actual cumple?**  
**Sí**, tras la corrección aplicada (antes guardaba 2 como índice).

**D) Corrección exacta aplicada en el código:**

```cpp
sb.s_firts_ino = inode_start + 2 * INODE_SIZE;   // PDF: "dirección del inodo"
sb.s_first_blo = block_start + 2 * BLOCK_SIZE;   // PDF: "dirección del bloque"
```

(Se reemplazó `sb.s_firts_ino = 2` y `sb.s_first_blo = 2` por las líneas anteriores.)

---

### s_first_blo

**A) Texto literal del PDF:**  
*"s_first_blo — int — Primer bloque libre (dirección del bloque)"*

**B) Interpretación técnica:**  
El campo debe contener la **dirección (posición en bytes)** del primer bloque libre: `block_start + (índice_del_primer_libre * 64)`. Tras MKFS: `block_start + 2 * 64`.

**C) ¿La implementación actual cumple?**  
**Sí**, tras la corrección aplicada (antes guardaba 2 como índice).

**D) Corrección exacta:**  
Usar `block_start + 2 * BLOCK_SIZE` en lugar de `2`. Ya aplicada en el código.

---

### s_mnt_count

**A) Texto literal del PDF:**  
*"s_mnt_count — int — Indica cuantas veces se ha montado el sistema"*

**B) Interpretación técnica:**  
Contador de veces que esta partición ha sido montada. Tras MKFS aún no se ha ejecutado mount sobre ella, por tanto debe ser 0.

**C) ¿La implementación actual cumple?**  
**Sí.** Se asigna `sb.s_mnt_count = 0` tras MKFS.

**D)** No requiere corrección.

---

## E) Confirmación final

**MKFS ahora cumple estrictamente con el PDF** en los tres campos:

- **s_firts_ino:** se guarda la dirección en bytes del primer inodo libre (`inode_start + 2 * INODE_SIZE`).
- **s_first_blo:** se guarda la dirección en bytes del primer bloque libre (`block_start + 2 * BLOCK_SIZE`).
- **s_mnt_count:** se deja en 0 tras MKFS, según el significado dado en el enunciado.
