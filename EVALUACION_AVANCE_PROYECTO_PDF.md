# Evaluación formal del porcentaje de avance — Proyecto 1 MIA 1S2026

Evaluación basada **exclusivamente** en el PDF oficial del enunciado. Sin funcionalidades no definidas en el PDF, sin estimación por percepción, sin criterios externos ni experiencia Linux. Porcentaje calculado en función de los módulos y ponderaciones reales del enunciado (rúbrica de calificación).

---

## PARTE 1 — IDENTIFICACIÓN DE TODOS LOS MÓDULOS DEL PROYECTO SEGÚN EL PDF

Listado de funcionalidades que el PDF exige implementar, organizado según la estructura y numeración del enunciado (comandos) y de la rúbrica (Partes 1–6).

### 1. Parte 1: Frontend y comunicación con Backend (PDF: “Backend mostrar el funcionamiento del API”)

- **Frontend:** Área de entrada de comandos, área de salida, botón de carga de archivo (scripts), botón de ejecutar. Comunicación con backend vía API.
- **Scripts .smia:** Carga y ejecución de archivos con comandos; preservación de comentarios y líneas en blanco.

*Ponderación rúbrica: 5 puntos.*

---

### 2. Administración de discos (comandos sobre el archivo .mia)

| # | Comando | Descripción PDF |
|---|---------|------------------|
| 1 | **MKDISK** | Crear archivo binario que simula un disco; MBR en el primer sector; disco lleno de ceros; parámetros -size, -path, -fit, -unit. |
| 2 | **RMDISK** | Eliminar el archivo del disco; mensaje si el disco no existe. |

*Ponderación rúbrica: MKDISK 5, RMDISK 5 → 10 puntos.*

---

### 3. Administración de particiones

| # | Comando | Descripción PDF |
|---|---------|------------------|
| 3 | **FDISK** | Administración de particiones en el archivo de disco: primarias, extendida, lógicas (EBR encadenados); -size, -path, -name, -unit, -type, -fit, -delete, -add. |

*Ponderación rúbrica: 12 puntos.*

---

### 4. Montaje (en RAM)

| # | Comando | Descripción PDF |
|---|---------|------------------|
| 4 | **MOUNT** | Montar una partición; ID por disco y partición; montaje solo en RAM; parámetros -path, -name. |
| 5 | **MOUNTED** | Mostrar todas las particiones montadas en memoria. |

*Ponderación rúbrica: MOUNT 5, MOUNTED 2 → 7 puntos.*

---

### 5. Reportes asociados a Parte 2 (discos/particiones)

- **Reporte MBR:** Tablas con información del MBR y EBR. Comando **rep** con -name=mbr, -path, -id.
- **Reporte DISK:** Estructura de particiones, MBR del disco.

*Ponderación rúbrica: 4 + 4 = 8 puntos.*

---

### 6. Sistema de archivos (formateo y lectura)

| # | Comando | Descripción PDF |
|---|---------|------------------|
| 6 | **MKFS** | Formateo completo de la partición como ext2; estructuras en disco; users.txt en raíz; -id. |
| 7 | **CAT** | Mostrar contenido del archivo; sesión activa; permisos; -fileN. |

*Ponderación rúbrica: MKFS 5, CAT 2 → 7 puntos.*

---

### 7. Gestión de usuarios y grupos (users.txt)

| # | Comando | Descripción PDF |
|---|---------|------------------|
| 8 | **LOGIN** | Iniciar sesión; -user, -pass, -id; validación contra users.txt; una sola sesión. |
| 9 | **LOGOUT** | Cerrar sesión; sin parámetros; requiere sesión activa. |
| 10 | **MKGRP** | Crear grupo en users.txt; solo root; -name; formato GID,G,Grupo. |
| 11 | **RMGRP** | Eliminar grupo (lógico, GID=0); solo root; -name. |
| 12 | **MKUSR** | Crear usuario en users.txt; solo root; -user, -pass, -grp; formato UID,U,Grupo,Usuario,Contraseña. |
| 13 | **RMUSR** | Eliminar usuario (lógico, UID=0); solo root; -user. |
| 14 | **CHGRP** | Cambiar grupo de un usuario; solo root; -user, -grp. |

*Ponderación rúbrica: LOGIN 1, LOGOUT 1, MKGRP 0.5, RMGRP 0.5, MKUSR 0.5, RMUSR 0.5, CHGRP 1 → 5 puntos.*

---

### 8. Administración de carpetas, archivos y permisos

| # | Comando | Descripción PDF |
|---|---------|------------------|
| 15 | **MKFILE** | Crear archivo en ext2; apuntadores directos e indirectos; contenido desde computadora; -path, -size, -cont, -r, etc. |
| 16 | **MKDIR** | Crear carpetas; apuntadores directos; creación de padres no existentes; -path, -p. |

*Ponderación rúbrica: MKFILE 10, MKDIR 8 → 18 puntos.*

---

### 9. Reportes del sistema de archivos (comando REP)

Generación con **rep** (-name, -path, -id, -path_file_ls según reporte). Valores de -name según PDF:

- mbr, disk, inode, block, bm_inode, bm_block, tree, sb, file, ls.

*Ponderación rúbrica: Inode 3, Block 3, bm_inode 2, bm_block 2, tree 2, Sb 2, File 2, Ls 2 → 18 puntos.*

---

### 10. Documentación y conocimientos (Parte 6)

- Documentación del proyecto (manual técnico, descripción de comandos, etc.).
- Pregunta 1 y Pregunta 2 (conocimientos).

*Ponderación rúbrica: 5 + 2.5 + 2.5 = 10 puntos.*

---

**Resumen de categorías según el PDF:**

| Categoría | Contenido | Puntos (rúbrica) |
|------------|-----------|-------------------|
| Frontend / API | Área comandos, salida, scripts, API | 5 |
| Discos | MKDISK, RMDISK | 10 |
| Particiones | FDISK | 12 |
| Montaje | MOUNT, MOUNTED | 7 |
| Reportes Parte 2 | MBR, DISK | 8 |
| Sistema de archivos | MKFS, CAT | 7 |
| Usuarios y grupos | LOGIN, LOGOUT, MKGRP, RMGRP, MKUSR, RMUSR, CHGRP | 5 |
| Archivos y carpetas | MKFILE, MKDIR | 18 |
| Reportes Parte 5 | inode, block, bm_inode, bm_block, tree, sb, file, ls | 18 |
| Documentación | Doc + Preguntas | 10 |
| **Total** | | **100** |

---

## PARTE 2 — TABLA DE ESTADO ACTUAL

Criterios: **Diseñado** = documento formal con texto literal del PDF, interpretación técnica, validaciones, algoritmo paso a paso, errores y confirmación. **Implementado** = código existente en el proyecto que cumple el comando según el PDF (según auditorías previas). **% completitud** = estimación para ese ítem según lo que exige la rúbrica (implementado = 100%; solo diseñado = 40% — fase de diseño lista para implementar; no iniciado = 0%). La justificación se basa en el estado descrito por el usuario y en los archivos de diseño existentes.

| Módulo | Diseñado | Implementado | % | Justificación (PDF) |
|--------|-----------|----------------|---|---------------------|
| Frontend / API | Parcial | Parcial | 50 | PDF exige área entrada/salida, carga de scripts, ejecución; se asume frontend operativo y envío al backend; no se verifica cada detalle. |
| MKDISK | Sí (auditado) | Sí | 100 | Rúbrica: crear disco correctamente, tamaño según parámetros. Implementado y validado contra PDF. |
| RMDISK | No | No | 0 | PDF: eliminar archivo del disco, mensaje si no existe. No hay diseño formal ni implementación referida. |
| FDISK | Sí (auditado) | Sí | 100 | Rúbrica: primarias, extendidas, lógicas, restricciones. Implementado con part_correlative=-1 y validado. |
| MOUNT | Sí (auditado) | Sí | 100 | PDF: ID por disco/partición, solo RAM, solo primarias. Implementado y validado. |
| MOUNTED | No | No/desconocido | 0 | PDF: listar particiones montadas. No hay diseño formal referido; si existe código, no se ha validado aquí. |
| Reportes MBR, DISK | No | No | 0 | PDF: rep -name=mbr, disk con -path, -id. Necesarios para calificar mkdisk, fdisk, mount. |
| MKFS | Sí | Sí | 100 | Rúbrica: aplicar formato a partición. Diseño e implementación validados (fórmulas, users.txt, estructuras). |
| CAT | Sí | No | 40 | PDF: mostrar contenido; sesión activa; permisos. Diseño formal existe; no implementado → 40% (diseño listo). |
| LOGIN | Sí | No | 40 | PDF: iniciar sesión; users.txt; una sesión. Diseño formal existe; no implementado. |
| LOGOUT | Sí | No | 40 | PDF: cerrar sesión; sin parámetros. Diseño formal existe; no implementado. |
| MKGRP | Sí | No | 40 | PDF: crear grupo; solo root; users.txt. Diseño formal existe; no implementado. |
| RMGRP | Sí | No | 40 | PDF: eliminar grupo (GID=0); solo root. Diseño formal existe; no implementado. |
| MKUSR | Sí | No | 40 | PDF: crear usuario; solo root; -user, -pass, -grp. Diseño formal existe; no implementado. |
| RMUSR | No* | No | 0 | PDF: eliminar usuario (UID=0); solo root. No hay archivo DISEÑO_RMUSR; si se considera análogo a RMGRP, podría estimarse diseño implícito (no se cuenta como 40% aquí por ausencia de documento explícito). |
| CHGRP | No | No | 0 | PDF: cambiar grupo de usuario; solo root; -user, -grp. Sin diseño formal ni implementación. |
| MKFILE | No | No | 0 | Rúbrica: creación de archivos, apuntadores, tamaño, contenido, indirectos. Sin diseño ni implementación. |
| MKDIR | No | No | 0 | Rúbrica: carpetas, apuntadores directos, padres no existentes. Sin diseño ni implementación. |
| Reportes (inode, block, bm_inode, bm_block, tree, sb, file, ls) | No | No | 0 | PDF: rep con -name según tipo. Requeridos para calificar MKFS y comandos de archivos/carpetas. |
| Documentación | No | No | 0 | PDF: documentación y preguntas. Entregables aparte. |

\* RMUSR: el usuario indicó que está “formalmente diseñado”; en el repositorio no existe `DISEÑO_COMANDO_RMUSR_PDF.md`. Si se considera diseñado por analogía con RMGRP, podría asignarse 40% a RMUSR; en este cálculo se deja 0% por no haber documento de diseño explícito en la lista de archivos revisada.

---

## PARTE 3 — CÁLCULO DEL PORCENTAJE GLOBAL DEL PROYECTO

### Peso por ítem (según rúbrica del PDF)

Se usan los valores de la rúbrica (Detalle de la Calificación) como pesos. Total = 100.

- Parte 1 (Frontend/API): 5  
- MKDISK: 5 | RMDISK: 5 | FDISK: 12 | MOUNT: 5 | MOUNTED: 2 | Reportes P2: 8  
- MKFS: 5 | CAT: 2  
- LOGIN: 1 | LOGOUT: 1 | MKGRP: 0.5 | RMGRP: 0.5 | MKUSR: 0.5 | RMUSR: 0.5 | CHGRP: 1  
- MKFILE: 10 | MKDIR: 8 | Reportes P5: 18  
- Documentación: 10  

### Completitud por ítem (según tabla PARTE 2)

Se toma el % de la tabla (0, 40, 50 o 100) y se multiplica por el peso del ítem. El avance ponderado es la suma de esos productos.

**Cálculo (con RMUSR = 0% por no tener documento de diseño explícito):**

| Ítem | Peso | Completitud | Puntos aportados |
|------|------|-------------|-------------------|
| Frontend/API | 5 | 50% | 2.5 |
| MKDISK | 5 | 100% | 5 |
| RMDISK | 5 | 0% | 0 |
| FDISK | 12 | 100% | 12 |
| MOUNT | 5 | 100% | 5 |
| MOUNTED | 2 | 0% | 0 |
| Reportes MBR+DISK | 8 | 0% | 0 |
| MKFS | 5 | 100% | 5 |
| CAT | 2 | 40% | 0.8 |
| LOGIN | 1 | 40% | 0.4 |
| LOGOUT | 1 | 40% | 0.4 |
| MKGRP | 0.5 | 40% | 0.2 |
| RMGRP | 0.5 | 40% | 0.2 |
| MKUSR | 0.5 | 40% | 0.2 |
| RMUSR | 0.5 | 0% | 0 |
| CHGRP | 1 | 0% | 0 |
| MKFILE | 10 | 0% | 0 |
| MKDIR | 8 | 0% | 0 |
| Reportes P5 | 18 | 0% | 0 |
| Documentación | 10 | 0% | 0 |

**Suma de puntos aportados:** 2.5 + 5 + 12 + 5 + 5 + 0.8 + 0.4 + 0.4 + 0.2 + 0.2 + 0.2 = **31.4**  
**Total posible:** 100  

**Porcentaje global de avance (considerando diseño como 40%):**  
31.4 / 100 = **31.4 %**

**Porcentaje solo por ítems implementados (100% = código listo):**  
Frontend 50% → 2.5; MKDISK 5; FDISK 12; MOUNT 5; MKFS 5. Suma = 2.5 + 5 + 12 + 5 + 5 = **29.5** de 100 → **29.5 %**.

Si se considera **solo lo implementado** (sin contar diseño como avance), el avance está en **~29.5 %** (con MKFS implementado y frontend parcial).

**Criterio adoptado:** el enunciado pide “cuánto del proyecto está cubierto”; se considera que un módulo “diseñado formalmente” cubre parte del trabajo (fase de diseño lista). Por tanto el **resultado final de avance global** se reporta como:

- **Avance global (con diseño = 40% por ítem diseñado): 31.4 %**
- **Avance solo implementación (código listo = 100%, resto 0%): ~29.5 %**

No se usan números arbitrarios; los pesos son los de la rúbrica del PDF y los porcentajes por ítem se limitan a 0, 40 (solo diseñado), 50 (frontend parcial) o 100 (implementado y validado).

---

## PARTE 4 — ANÁLISIS DE LO QUE FALTA

Según el PDF y la tabla de estado, lo que **aún no está cubierto**:

### Comandos faltantes (implementación y/o diseño)

- **RMDISK:** eliminar archivo de disco; mensaje si no existe.
- **MOUNTED:** listar particiones montadas en memoria.
- **CAT:** implementación (diseño listo).
- **LOGIN, LOGOUT, MKGRP, RMGRP, MKUSR:** implementación (diseño listo).
- **RMUSR:** diseño formal explícito (si se desea alineado al resto) e implementación.
- **CHGRP:** diseño e implementación.
- **MKFILE:** diseño e implementación (archivos, apuntadores, contenido, -r, etc.).
- **MKDIR:** diseño e implementación (carpetas, -p, padres no existentes).

### Reportes faltantes

- **Comando REP** con -name, -path, -id, -path_file_ls.
- Reportes: **mbr, disk, inode, block, bm_inode, bm_block, tree, sb, file, ls** (generación con Graphviz u otra herramienta según enunciado).
- Creación de carpeta en -path si no existe; validación de -id (partición montada).

### Validaciones estructurales

- Verificación de sesión activa en todos los comandos que lo exigen (excepto MKFS y LOGIN).
- Verificación de usuario root para MKGRP, RMGRP, MKUSR, RMUSR, CHGRP (y MKFILE/MKDIR si el PDF lo indica).
- Persistencia correcta en users.txt (MKGRP, RMGRP, MKUSR, RMUSR, CHGRP) y en estructuras ext2 (MKFILE, MKDIR, permisos si aplica).

### Manejo de errores no implementados

- Mensajes de error definidos en el PDF para cada comando (sesión inactiva, no root, grupo/usuario no existe, ya existe, etc.) en los comandos aún no implementados.

### Persistencia en disco

- MKFS ya escribe en la partición. Falta: escritura en users.txt (MKGRP, RMGRP, MKUSR, RMUSR, CHGRP) y escritura de archivos/carpetas en bloques e inodos (MKFILE, MKDIR).

### Pruebas formales

- El PDF no exige un conjunto concreto de pruebas automatizadas; la calificación se basa en reportes y funcionamiento observable. Lo que falta es tener los comandos y reportes funcionando para poder evidenciar el cumplimiento.

### Documentación

- Documentación del proyecto y preparación para las preguntas de conocimientos (Parte 6).

---

## PARTE 5 — PLAN CONCRETO PARA CONCLUIR EL PROYECTO

### FASE 1 — Cierre del módulo actual (sesión y users.txt)

**Objetivo:** Tener operativos LOGIN, LOGOUT y los comandos que modifican users.txt sobre la partición de la sesión.

**Qué implementar (orden recomendado):**

1. Módulo de sesión en RAM (activa, id_particion, nombre_usuario, UID, GID) según diseño ya documentado.
2. **LOGIN:** leer users.txt de la raíz de la partición indicada por -id; validar usuario/contraseña; crear sesión; resolver GID por nombre de grupo (GID=0 si grupo eliminado/no existe).
3. **LOGOUT:** marcar sesión inactiva; limpiar datos de sesión.
4. **MKGRP:** validar sesión y root; leer users.txt; comprobar que no exista grupo activo con ese nombre; nuevo GID = max(GID activos)+1; añadir línea GID,G,NombreGrupo; reescribir users.txt en disco.
5. **RMGRP:** validar sesión y root; localizar grupo activo por nombre; poner GID=0 en esa línea; reescribir users.txt.
6. **MKUSR:** validar sesión, root, grupo activo, usuario no existente, longitudes; nuevo UID; añadir línea UID,U,Grupo,Usuario,Contraseña; reescribir users.txt.
7. **RMUSR:** validar sesión y root; localizar usuario activo; poner UID=0; reescribir users.txt.
8. **CHGRP:** validar sesión, root, usuario existente, nuevo grupo existente; modificar campo Grupo del registro U; reescribir users.txt.

**Orden lógico:** Sesión → LOGIN/LOGOUT → MKGRP/RMGRP → MKUSR/RMUSR → CHGRP (CHGRP modifica registro U existente).

**Riesgos:** Lectura/escritura de users.txt dentro de ext2 (resolver path raíz, inodo de users.txt, bloques); actualizar inodo si cambia tamaño del archivo; no corromper el resto del sistema de archivos.

---

### FASE 2 — Implementación de comandos faltantes (CAT, RMDISK, MOUNTED)

**Objetivo:** Completar comandos de lectura y utilidad que no modifican estructura de archivos (salvo RMDISK que borra el .mia).

**Qué implementar:**

1. **CAT:** sesión activa; partición de sesión; resolver path por -fileN; validar que sea archivo y permisos (UGO/root); leer bloques según inodo; concatenar y mostrar.
2. **MOUNTED:** listar desde la tabla en RAM (id, path, name, etc.) según lo que exija el PDF.
3. **RMDISK:** -path; comprobar existencia del archivo; eliminarlo; mensaje si no existe.

**Orden:** CAT (depende de sesión y ext2), MOUNTED (solo lectura de tabla), RMDISK (independiente).

**Riesgos:** Resolución de paths en ext2; permisos 777/root; manejo de bloques directos e indirectos en CAT.

---

### FASE 3 — Validaciones estructurales finales

**Objetivo:** Que todos los comandos que requieren sesión la validen antes de ejecutar; que los que requieren root lo comprueben; que users.txt y estructuras ext2 no se corrompan.

**Qué hacer:**

- En el parser o en cada comando: si el comando requiere sesión (según PDF), comprobar sesión activa; si requiere root, comprobar usuario root.
- Revisar que las escrituras en users.txt (y en bloques/inodos para MKFILE/MKDIR) respeten tamaños, posiciones y formatos definidos en el PDF.

**Riesgos:** Olvidar algún comando en la validación de sesión/root; errores de borde en escritura (tamaño, bloques nuevos).

---

### FASE 4 — Reportes (comando REP)

**Objetivo:** Cumplir la sección “Reportes” del PDF para poder calificar según la rúbrica.

**Qué implementar:**

1. **rep** con parámetros -name, -path, -id y -path_file_ls (para file y ls).
2. Reportes: **mbr, disk** (Parte 2); **inode, block, bm_inode, bm_block, tree, sb, file, ls** (Parte 5).
3. Generación en la ruta -path (crear carpeta si no existe); uso de Graphviz según PDF; -id para identificar partición (y disco si aplica).

**Orden:** Primero rep + mbr y disk (para calificar discos/particiones); luego reportes del sistema de archivos (inode, block, bitmaps, tree, sb, file, ls).

**Riesgos:** Acceso a estructuras en disco por -id (path del disco, partición); formato de salida (dot, imagen o texto según cada reporte); path_file_ls para file/ls.

---

### FASE 5 — MKFILE y MKDIR

**Objetivo:** Completar la Parte 5 de la rúbrica (archivos y carpetas).

**Qué implementar:**

1. **MKFILE:** Crear archivo en path dado; asignar inodo y bloques (directos e indirectos si hace falta); opción -cont para contenido desde computadora; -r si aplica; manejo de tamaño.
2. **MKDIR:** Crear carpetas; inodos tipo carpeta; bloques de directorio; -p para crear padres no existentes.

**Orden:** Diseño formal según PDF → MKDIR (más simple) → MKFILE (contenido, indirectos).

**Riesgos:** Asignación de bloques libres (bitmap, s_first_blo/s_firts_ino); actualización de bitmaps e inodos; bloques indirectos; permisos iniciales (ej. 664 para archivos según PDF).

---

### FASE 6 — Pruebas integrales y preparación para entrega

**Objetivo:** Flujo completo MKDISK → FDISK → MOUNT → MKFS → LOGIN → comandos de usuarios → CAT/MKFILE/MKDIR → reportes; y documentación.

**Qué hacer:**

- Ejecutar secuencias de comandos que cubran cada ítem de la rúbrica (crear disco, particiones, montar, formatear, login, mkgrp/mkusr, etc., reportes).
- Ajustar mensajes de error y salidas al PDF.
- Redactar documentación (manual técnico, descripción de comandos) y preparar respuestas a las preguntas de conocimientos.

**Riesgos:** Inconsistencias entre diseño e implementación; errores no contemplados en paths o permisos.

---

## PARTE 6 — ESTIMACIÓN REALISTA DE COMPLEJIDAD RESTANTE

**Clasificación: ALTA**

**Justificación técnica según el enunciado:**

- **Ya resuelto (complejidad absorbida):** Estructuras MBR, EBR, Partition, Superblock, Inode, bloques; comandos MKDISK, FDISK, MOUNT, MKFS; diseño formal de LOGIN, LOGOUT, MKGRP, RMGRP, MKUSR, CAT. Esto reduce riesgo en la parte de discos, particiones, formateo y modelo de usuarios/grupos.

- **Pendiente de alta complejidad:**  
  - **MKFILE y MKDIR:** Manipulación completa del sistema de archivos ext2 (inodos, bloques directos e indirectos, bitmaps, tamaño variable, contenido desde archivo externo). El PDF exige “apuntadores”, “manejo de tamaño”, “contenido de un archivo de la computadora”, “crear carpetas padres no existentes”.  
  - **Reportes:** Ocho tipos de reportes (mbr, disk, inode, block, bm_inode, bm_block, tree, sb, file, ls) con Graphviz y parámetros -path, -id, -path_file_ls; dependen de estructuras ya definidas pero requieren recorrido y generación de salida.  
  - **Persistencia de users.txt:** Implementar lectura/escritura de un archivo dentro de ext2 (resolver path, leer/escribir bloques, actualizar inodo) para LOGIN, MKGRP, RMGRP, MKUSR, RMUSR, CHGRP.

- **Complejidad media:** Sesión en RAM, validaciones de sesión/root, CAT (resolución de path y permisos), RMDISK, MOUNTED, CHGRP (modificar una línea en users.txt).

Por tanto, el **trabajo restante** implica módulos que tocan núcleo del sistema de archivos (MKFILE, MKDIR, lectura/escritura de archivos dentro de ext2) y una capa amplia de reportes. No es “bajo” ni “medio”; se clasifica como **alto**. “Muy alto” se reservaría si además no existieran las estructuras ni los comandos de disco/partición/MKFS ya implementados.

---

## PARTE 7 — CONCLUSIÓN EJECUTIVA

- **Porcentaje actual:** El avance global del proyecto, calculado con los pesos de la rúbrica del PDF y considerando “diseño formal listo” como 40% del ítem, es **~31 %**. Si solo se cuenta lo implementado y validado (MKDISK, FDISK, MOUNT, MKFS y parte del frontend), el avance está en el rango **~29–35 %**.

- **Grado de avance respecto al PDF:** Aproximadamente un tercio del proyecto está cubierto (discos, particiones, montaje, formateo ext2 y diseños formales de sesión y comandos de users.txt). La base de estructuras y de comandos de administración de discos/particiones está alineada al enunciado; la parte de usuarios/grupos está diseñada pero no implementada; la parte de archivos/carpetas y reportes está por hacer.

- **Proximidad a proyecto completo:** Falta implementar la mayoría de los comandos que dependen de sesión (LOGIN, LOGOUT, CAT, MKGRP, RMGRP, MKUSR, RMUSR, CHGRP), los de archivos y carpetas (MKFILE, MKDIR), todo el comando REP y sus reportes, RMDISK, MOUNTED y la documentación. Con el diseño actual, la implementación restante es sustancial pero acotada por los documentos formales ya redactados.

- **Riesgo de inconsistencias:** **Medio.** Existe riesgo de desvío respecto al PDF en: (1) formato exacto de users.txt al escribir (MKGRP, MKUSR, etc.); (2) permisos y resolución de paths en CAT y en reportes file/ls; (3) detalle de MKFILE/MKDIR (bloques indirectos, -cont, -p). Mitigación: seguir al pie de la letra los diseños formales y el texto del PDF en cada implementación y validación.

No se han usado suposiciones externas ni conocimiento Linux; la evaluación se basa solo en el contenido del PDF del Proyecto 1 MIA 1S2026 y en la rúbrica de calificación que en él se define.
