# Validación estricta MKFS contra el PDF oficial (Proyecto 1 MIA – 1S2026)

**Fuente única:** texto extraído del archivo `MIA_Enunciado_Proyecto1_1S2026 (1).pdf`.  
No se usan fórmulas típicas, suposiciones de años anteriores ni referencias externas.

---

## 1) Ubicación textual de la sección MKFS en el PDF

- **Comando MKFS:** Apartado **"6. MKFS"** dentro de "Comandos del sistema de archivos".  
  Texto: *"Este comando realiza un formateo completo de la partición, se formatea como ext2. También creará un archivo en la raíz llamado users.txt que tendrá los usuarios y contraseñas del sistema de archivos."*
- **Fórmulas de estructuras y número de inodos/bloques:** Sección **"Sistema de Archivos Ext2"** / **"EXT2"**, antes de "Súper Bloque", donde se describe la estructura en bloques y el cálculo.
- **Bloques 64 bytes:** Sección **"Bloques"**: *"para este proyecto de procuró que todos los bloques tengan un tamaño de 64 bytes"*.
- **Bitmap:** Sección **"Bitmap"**: 0 usable, 1 ocupado; ejemplo con "10 bits" / "20 bits".
- **users.txt inicial:** Apartado **"Administración de Usuarios y Grupos"**: *"Al inicio existirá un grupo llamado root, un usuario root y una contraseña (123) para el usuario root. El archivo lógico almacenado en el disco al inicio debería ser como el siguiente:"* seguido de las dos líneas (en el PDF con OCR "íoot" = root).
- **Usuario root 777:** Sección **"USUARIO ROOT"**: *"Este usuario es especial y no importando que permisos tiene el archivo o carpeta, se manejan permisos UGO en su forma octal y este siempre tendrá los permisos 777 ... No se le negará ninguna operación por permisos, ya que él los tiene todos."*

---

## 2) Extracción literal del PDF

### Fórmula para número de inodos y bloques

**Texto literal (PDF):**

- *"El número de bloques será el triple que el número de inodos. El número de inodos y bloques a crear se puede calcular despejando n de la primera ecuación y aplicando la función floor al resultado:"*
- *"● tamaño_particion = sizeOf(superblock) + n + 3 * n + n * sizeOf(inodos) + 3 * n * sizeOf(block)"*
- *"● numero_estructuras = floor(n)"*

**Observaciones del PDF:**
- *"1. sizeof es el tamaño de los Structs."*
- *"2. En el Bitmap de Bloques y Bloques se multiplica por tres debido a que existen tres tipos de bloque que son: bloques carpetas, bloques archivos y bloques de contenido."*

### Relación entre inodos y bloques

**Texto literal:** *"El número de bloques será el triple que el número de inodos."*

### Tamaño del bloque

**Texto literal:** *"para este proyecto de procuró que todos los bloques tengan un tamaño de 64 bytes"* (Bloques de carpetas, archivos y apuntadores: 64 bytes cada uno).

### Orden de estructuras

El PDF **no** escribe explícitamente una frase del tipo "el orden es: superbloque, luego…".  
El orden se deduce **únicamente** de la ecuación de **tamaño_particion**, cuyos sumandos aparecen en este orden:

1. sizeOf(superblock)  
2. n  
3. 3 * n  
4. n * sizeOf(inodos)  
5. 3 * n * sizeOf(block)  

Por tanto, el orden en disco es: **Superbloque → espacio para bitmap de inodos (n) → espacio para bitmap de bloques (3n) → tabla de inodos → tabla de bloques.**

---

## 3) Lo que el PDF NO define

- **Bitmap en bytes vs bits:** El ejemplo dice "10 bits" y "20 bits". La ecuación usa **n** y **3*n** como sumandos de **tamaño_particion** (en bytes), por lo que ese espacio son **n bytes** y **3*n bytes** respectivamente. Es decir: 1 byte por inodo y 1 byte por bloque (bytemap). El PDF no escribe "1 byte por inodo" de forma literal.
- **Orden explícito:** No hay una lista numerada "primero superbloque, segundo…"; el orden se infiere del orden de los sumandos en la fórmula.

---

## 4) Respuesta en el formato solicitado

### A) Fórmula textual encontrada en el PDF (copia literal)

- *"El número de bloques será el triple que el número de inodos."*
- *"El número de inodos y bloques a crear se puede calcular despejando n de la primera ecuación y aplicando la función floor al resultado:"*
- *"tamaño_particion = sizeOf(superblock) + n + 3 * n + n * sizeOf(inodos) + 3 * n * sizeOf(block)"*
- *"numero_estructuras = floor(n)"*
- *"sizeof es el tamaño de los Structs."*
- *"para este proyecto de procuró que todos los bloques tengan un tamaño de 64 bytes"*

### B) Interpretación técnica correcta

- **n** = número de inodos. **numero_estructuras = floor(n)** es el número de inodos a usar (n entero tras el floor).
- **Número de bloques:** `n_blocks = 3 * n` (triple que el número de inodos).
- **tamaño_particion** es el tamaño de la partición en bytes (`part_s`). Se cumple:  
  `part_s = sizeof(Superblock) + n + 3*n + n*sizeof(Inode) + 3*n*64`
- Despejando **n**:  
  `part_s - sizeof(Superblock) = n*(1 + 3 + sizeof(Inode) + 3*64)`  
  `n = (part_s - sizeof(Superblock)) / (4 + sizeof(Inode) + 192)`  
  **n_inodos = floor((part_s - sizeof(Superblock)) / (4 + sizeof(Inode) + 192))**  
  **n_bloques = 3 * n_inodos**
- **Tamaño del bloque:** 64 bytes (explícito en el PDF).
- **Orden en disco:** el orden de los sumandos en la ecuación implica: Superbloque → bitmap inodos (n bytes) → bitmap bloques (3n bytes) → tabla inodos → tabla bloques (cada bloque 64 bytes).

### C) Confirmación de que el diseño actual coincide 100% con esa fórmula

- La relación **n_bloques = 3 * n_inodos** y la ecuación de **tamaño_particion** con los cinco sumandos en ese orden están recogidas en DISEÑO_MKFS_PDF.md.
- Tras sustituir la “fórmula típica” por la fórmula literal del PDF (con floor y los mismos términos), el diseño coincide al 100% con lo que exige el PDF.

### D) Corrección aplicada al diseño

- Se eliminó toda referencia a “fórmulas típicas de MIA”, “en muchos enunciados” y “validar con el docente”.
- La única fórmula usada es la del PDF:  
  `n = floor((part_s - sizeof(Superblock)) / (4 + sizeof(Inode) + 192))`  
  `n_inodos = n`, `n_bloques = 3 * n`.
- El orden de estructuras se deja explícito como **derivado del orden de la ecuación** (no como suposición externa).
- Tamaño de bloque **64 bytes** y contenido inicial de **users.txt** se marcan como **explícitos en el PDF**; root 777 como **explícito en la sección USUARIO ROOT**.

### E) Confirmación final

**Ahora el diseño cumple estrictamente con el PDF.**

---

## 5) Resumen de confirmaciones

| Aspecto | ¿En el PDF? | Texto / conclusión |
|--------|-------------|---------------------|
| Fórmula n_inodos / n_bloques | Sí | Ecuación tamaño_particion + numero_estructuras = floor(n); n_bloques = 3*n. |
| Tamaño bloque 64 bytes | Sí | "todos los bloques tengan un tamaño de 64 bytes". |
| Orden de estructuras | Implícito | Orden de sumandos: superblock, n, 3n, n*sizeOf(inodos), 3*n*sizeOf(block). |
| Bitmap 0 usable 1 ocupado | Sí | "0 como usable y 1 como ocupado". |
| users.txt en raíz, contenido inicial | Sí | Dos líneas (1,G,root y 1,U,root,root,123; "íoot" = root por OCR). |
| Root siempre 777 | Sí | "este siempre tendrá los permisos 777". |
