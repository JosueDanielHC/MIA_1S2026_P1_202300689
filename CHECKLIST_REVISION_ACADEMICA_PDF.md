# Checklist de revisión académica — Proyecto 1 MIA (Sistema de archivos EXT2 simulado)

Revisión contra los requisitos típicos del enunciado del Proyecto 1 MIA. Cada ítem indica qué exige el PDF, si la implementación actual lo cumple y observaciones.

**Nota:** No pegaste el resumen de tu implementación; el estado se ha inferido del código y de la evaluación previa (`EVALUACION_AVANCE_PROYECTO_PDF.md`). Si tu resumen difiere, ajusta el checklist o compártelo para actualizar este documento.

---

## 1. ESTRUCTURAS OBLIGATORIAS

| # | Requisito PDF | Qué exige el enunciado | ¿Cumple? | Observación |
|---|----------------|------------------------|----------|-------------|
| 1.1 | **MBR** | Estructura en primer sector: mbr_tamano, mbr_fecha_creacion, mbr_dsk_signature, dsk_fit, mbr_partitions[4]. Tamaño y orden según PDF. | **Sí** | Struct en `disk/mbr.h`; auditado y usado en MKDISK y reporte MBR. |
| 1.2 | **Partition** | Campos: part_status, part_type, part_fit, part_start, part_s, part_name, part_correlative, part_id (según PDF). | **Sí** | `disk/partition.h`; part_correlative = -1 para nuevas particiones. |
| 1.3 | **EBR** | Estructura para lógicas: part_mount/part_status, part_fit, part_start, part_s, part_next, part_name. Encadenamiento. | **Sí** | `disk/ebr.h`; usado en FDISK y reporte MBR (cadena EBR). |
| 1.4 | **Superblock** | Campos del superbloque ext2: s_filesystem_type, s_inodes_count, s_blocks_count, s_free_*, s_mtime, s_umtime, s_mnt_count, s_magic, s_inode_s, s_block_s, s_firts_ino, s_first_blo, s_bm_*, s_inode_start, s_block_start. | **Sí** | `filesystem/superblock.h`; tiempos como `int` para tamaño fijo. |
| 1.5 | **Inode** | i_uid, i_gid, i_s, i_atime, i_ctime, i_mtime, i_block[15], i_type, i_perm. | **Sí** | `filesystem/inode.h`; 15 bloques (12 directos + 3 indirectos). |
| 1.6 | **FolderBlock / Content** | Bloque de carpeta con entradas (nombre + inodo); tamaño 64 bytes. | **Sí** | `filesystem/blocks.h`: Content (b_name[12], b_inodo), FolderBlock (4 entradas). |
| 1.7 | **FileBlock** | Bloque de archivo 64 bytes (contenido). | **Sí** | `filesystem/blocks.h`: FileBlock (b_content[64]). |

---

## 2. COMANDOS OBLIGATORIOS (Administración de discos y particiones)

| # | Comando | Qué exige el PDF | ¿Cumple? | Observación |
|---|---------|------------------|----------|-------------|
| 2.1 | **MKDISK** | Crear archivo .mia (disco binario); MBR en sector 0; parámetros -size, -path, -fit, -unit; disco inicializado (ceros). | **Sí** | Implementado y auditado; MBR correcto. |
| 2.2 | **RMDISK** | Eliminar archivo del disco; mensaje si no existe. | **Sí** | Implementado: `runRmdisk(-path)`, no elimina si el disco tiene particiones montadas. |
| 2.3 | **FDISK** | Administrar particiones: primarias, extendida, lógicas (EBR encadenados); -size, -path, -name, -unit, -type, -fit, -delete, -add; validar solapamientos. | **Sí** | Implementado; part_correlative = -1; EBR encadenados. |
| 2.4 | **MOUNT** | Montar partición; ID por disco/partición; solo en RAM; -path, -name; solo particiones primarias (según interpretación del proyecto). | **Sí** | ID formato carnet+part+letra; tabla en RAM; no escribe en disco. |
| 2.5 | **MOUNTED** | Listar particiones montadas. | **Sí** | Cubierto con `mount` sin parámetros → `listMounted()`. |

---

## 3. COMANDOS OBLIGATORIOS (Sistema de archivos y usuarios)

| # | Comando | Qué exige el PDF | ¿Cumple? | Observación |
|---|---------|------------------|----------|-------------|
| 3.1 | **MKFS** | Formatear partición como ext2; estructuras (superbloque, bitmaps, inodos, bloques); users.txt en raíz con contenido inicial; fórmulas n_inodos/n_bloques según PDF. | **Sí** | Implementado; users.txt 1,G,root y 1,U,root,root,123; s_firts_ino/s_first_blo como direcciones. |
| 3.2 | **CAT** | Mostrar contenido de archivo(s); sesión activa; validación de permisos; parámetro(s) -fileN. | **Sí** | Implementado: `runCat(-file1, -file2, ... -fileN)`, resolución de path, permisos UGO/root, lectura desde bloques. |
| 3.3 | **LOGIN** | Iniciar sesión; -user, -pass, -id; validar contra users.txt; una sola sesión; GID=0 si grupo no activo (según diseño). | **Sí** | Sesión en RAM; lectura de users.txt; validación usuario/contraseña. |
| 3.4 | **LOGOUT** | Cerrar sesión; sin parámetros; requiere sesión activa. | **Sí** | Limpia sesión. |
| 3.5 | **MKGRP** | Crear grupo en users.txt; solo root; -name; formato GID,G,Nombre. | **Sí** | Persistencia en users.txt dentro de ext2. |
| 3.6 | **RMGRP** | Eliminar grupo (lógico: GID=0); solo root; -name. | **Sí** | Modificación de users.txt. |
| 3.7 | **MKUSR** | Crear usuario; solo root; -user, -pass, -grp; formato UID,U,Grupo,Usuario,Contraseña. | **Sí** | Longitudes y validaciones según diseño. |
| 3.8 | **RMUSR** | Eliminar usuario (lógico: UID=0); solo root; -user. | **Sí** | Implementado. |
| 3.9 | **CHGRP** | Cambiar grupo de un usuario; solo root; -user, -grp. | **Sí** | Modifica campo grupo en registro U de users.txt. |
| 3.10 | **MKDIR** | Crear carpetas; -path; -p (crear padres); apuntadores directos. | **Sí** | Resolución de path; asignación inodo/bloque; actualización del padre. |
| 3.11 | **MKFILE** | Crear archivo; -path; -p; -size; -cont (archivo externo); contenido en bloques directos. | **Sí** | Archivo vacío, con tamaño automático (0123456789...) o desde -cont; combinación -size y -cont. |

---

## 4. REPORTES OBLIGATORIOS (comando REP)

| # | Reporte | Qué exige el PDF | ¿Cumple? | Observación |
|---|---------|------------------|----------|-------------|
| 4.1 | **rep -name=mbr** | Tabla MBR; particiones; EBR si hay extendida; Graphviz DOT; exportar imagen (-path). | **Sí** | Tabla HTML en DOT; MBR + particiones + cadena EBR; .png/.jpg/.pdf. |
| 4.2 | **rep -name=disk** | Reporte de estructura de particiones / disco. | **Sí** | Implementado: REPORTE DISK con ruta del disco, MBR, particiones y cadena EBR. |
| 4.3 | **rep -name=sb** | Superblock completo; todos los campos; fechas legibles. | **Sí** | Tabla con todos los campos; s_mtime/s_umtime en DD/MM/YYYY HH:MM:SS. |
| 4.4 | **rep -name=bm_inode** | Bitmap de inodos; 0/1; orden real; tabla (ej. 20 por fila). | **Sí** | Lectura desde disco; 20 columnas. |
| 4.5 | **rep -name=bm_block** | Bitmap de bloques; 0/1; orden real. | **Sí** | Igual que bm_inode para bloques. |
| 4.6 | **rep -name=tree** | Árbol del sistema de archivos; inodos y bloques; conexiones reales; BFS desde inodo 0. | **Sí** | Nodos inodo/block; aristas inode→block, block→inodo; sin ciclos por "."/"..". |
| 4.7 | **rep -name=file** | Contenido de un archivo; -ruta; búsqueda por path; contenido real desde bloques. | **Sí** | Resolución de path; validación tipo archivo; lectura de FileBlocks; tabla REPORTE FILE. |
| 4.8 | **rep -name=inode** | Reporte de inodo(s) específico(s) (si el PDF lo pide). | **Sí** | Implementado: rep -name=inode -path=... -id=... -ruta=... muestra tabla del inodo resuelto por ruta. |
| 4.9 | **rep -name=block** | Reporte de bloque(s) específico(s) (si el PDF lo pide). | **Sí** | Implementado: rep -name=block -path=... -id=... -ruta=... muestra bloques del archivo/carpeta. |
| 4.10 | **rep -name=ls** | Listado de directorio (path; si el PDF lo pide). | **Sí** | Implementado: rep -name=ls -path=... -id=... -ruta=... lista entradas del directorio (name, inode). |

*Nota:* La rúbrica del PDF puede nombrar "inode", "block", "ls" con otro criterio; conviene cotejar el enunciado exacto (nombre del reporte y parámetros).

---

## 5. VALIDACIONES REQUERIDAS

| # | Validación | Qué exige el PDF | ¿Cumple? | Observación |
|---|------------|------------------|----------|-------------|
| 5.1 | Sesión activa | Comandos (salvo MKFS y LOGIN) requieren sesión activa. | **Sí** | LOGIN, MKGRP, RMGRP, MKUSR, RMUSR, CHGRP, MKDIR, MKFILE y rep validan sesión donde aplica. |
| 5.2 | Usuario root | MKGRP, RMGRP, MKUSR, RMUSR, CHGRP solo por root. | **Sí** | Comprobación con isRoot() en cada uno. |
| 5.3 | Partición montada | Operaciones sobre FS y reportes requieren -id montado. | **Sí** | getMountById / getPartitionBounds antes de acceder al disco. |
| 5.4 | s_magic EXT2 | Reportes y operaciones sobre FS validan 0xEF53. | **Sí** | En reportes sb, bm_inode, bm_block, tree, file. |
| 5.5 | users.txt existe y formato | LOGIN/MKGRP/… asumen users.txt en raíz y formato G/U. | **Sí** | MKFS crea users.txt; comandos de usuarios leen/escriben según diseño. |

---

## 6. RESTRICCIONES TÉCNICAS

| # | Restricción | Qué exige el PDF | ¿Cumple? | Observación |
|---|-------------|------------------|----------|-------------|
| 6.1 | Montaje solo en RAM | MOUNT no debe escribir en disco. | **Sí** | Tabla de montajes solo en memoria. |
| 6.2 | ID de montaje | Formato según enunciado (ej. últimos 2 carnet + número partición + letra). | **Sí** | Constante CARNET; ID tipo 891A, 892A. |
| 6.3 | Solo particiones primarias para montar | Si el PDF lo indica. | **Sí** | MOUNT valida tipo primaria. |
| 6.4 | Bloques directos en MKDIR/MKFILE | Sin bloques indirectos (según decisión del proyecto). | **Sí** | Solo i_block[0..11]; sin indirectos. |
| 6.5 | Tamaños fijos de estructuras | MBR, Partition, EBR, Superblock, Inode, bloques con tamaños coherentes. | **Sí** | #pragma pack(push,1); tiempos en int donde se definió. |

---

## 7. FRONTEND Y API

| # | Requisito | Qué exige el PDF | ¿Cumple? | Observación |
|---|-----------|------------------|----------|-------------|
| 7.1 | Área de comandos y salida | Interfaz para ingresar comandos y ver resultados. | **Parcial** | Hay frontend y servidor HTTP; no se verifica detalle de UI. |
| 7.2 | Comunicación con backend | API para enviar comandos y recibir respuesta. | **Sí** | Backend expone API; executeCommand procesa entrada. |
| 7.3 | Scripts .smia | Carga y ejecución de archivos de comandos. | **Parcial** | Depende de si el frontend implementa carga y envío de script; no revisado en detalle. |

---

## 8. DOCUMENTACIÓN Y ENTREGABLES

| # | Requisito | Qué exige el PDF | ¿Cumple? | Observación |
|---|-----------|------------------|----------|-------------|
| 8.1 | Documentación del proyecto | Manual técnico, descripción de comandos, etc. | **No evaluado** | Depende de entregables; existen .md de diseño y auditoría. |
| 8.2 | Preguntas de conocimientos | Parte 6 del enunciado. | **No aplica** | Fuera del alcance del código. |

---

## RESUMEN DE CUMPLIMIENTO

| Categoría | Ítems | Cumple | Parcial | No cumple |
|-----------|-------|--------|---------|-----------|
| Estructuras | 7 | 7 | 0 | 0 |
| Comandos disco/particiones | 5 | 5 | 0 | 0 |
| Comandos FS y usuarios | 11 | 11 | 0 | 0 |
| Reportes REP | 10 | 10 | 0 | 0 |
| Validaciones | 5 | 5 | 0 | 0 |
| Restricciones técnicas | 5 | 5 | 0 | 0 |
| Frontend/API | 3 | 1 | 2 | 0 |

---

## VEREDICTO

- **Comandos núcleo:** MKDISK, FDISK, MOUNT, MOUNTED, MKFS, LOGIN, LOGOUT, MKGRP, RMGRP, MKUSR, RMUSR, CHGRP, MKDIR, MKFILE están implementados y alineados al diseño/PDF.
- **Reportes:** MBR, disk, sb, bm_inode, bm_block, tree, file, inode, block y ls están implementados.
- **Comandos:** RMDISK y CAT implementados.

**Conclusión:** **Proyecto completo y defendible** respecto a comandos y reportes del enunciado.

- Para **proyecto sólido y bien estructurado:** revisión final contra cada frase del PDF (mensajes de error, permisos, parámetros), documentación y preguntas de conocimientos si la rúbrica lo exige.

Si compartes el resumen completo de tu implementación o el fragmento del PDF con la lista exacta de reportes y comandos, se puede ajustar este checklist y el veredicto con precisión.
