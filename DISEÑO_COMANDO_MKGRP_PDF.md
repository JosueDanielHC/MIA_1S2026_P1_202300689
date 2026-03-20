# Diseño formal del comando MKGRP — Proyecto 1 MIA 1S2026 (solo PDF oficial)

Basado **estrictamente** en el PDF. Sin comportamiento Linux, sin reglas inventadas, sin validaciones no exigidas. Todo justificado con texto literal o interpretación directa del enunciado.

---

## PARTE 1 — TEXTO LITERAL DEL PDF

### MKGRP

*"10. MKGRP"*  
*"Este comando creará un grupo para los usuarios de la partición y se guardará en el archivo users.txt de la partición, este comando solo lo puede utilizar el usuario root. Si otro usuario lo intenta ejecutar, deberá mostrar un mensaje de error, si el grupo a ingresar ya existe deberá mostrar un mensaje de error. Distinguirá entre mayúsculas y minúsculas. Recibirá los Siguientes parámetros:"*

*"-name       Obligatorio      Indicará el nombre que tendrá el grupo"*

*"#Crea el grupo usuarios en la partición de la sesión actual"*  
*"mkgrp -name=usuarios"*

*"El archivo users.txt debería quedar como el siguiente:"*  
*"1, G, íoot \n"*  
*"1, U, íoot, íoot, 123 \n 2, G, usuaíios \n"*

### users.txt

*"Este archivo lógico almacenado en el disco será llamado users.txt guardado en el sistema ext2 de la raíz de cada partición. Existirán dos tipos de registros, unos para grupos y otros para usuarios. Un id 0 significa que el usuario o grupo está eliminado, el id de grupo o de usuario irá aumentando según se vayan creando usuarios o grupos."*

*"Tendrá la siguiente estructura:"*  
*"GID, Tipo, Gíupo \n"*  
*"UID, Tipo, Gíupo, Usuaíio, Contíaseña \n"*

*"El estado ocupará una letra, el tipo otra, el grupo ocupará como máximo 10 letras al igual que el usuario y la contraseña."*

(En el PDF aparecen OCR "Gíupo", "Usuaíio", "Contíaseña", "íoot", "usuaíios"; se interpretan como Grupo, Usuario, Contraseña, root, usuarios.)

### Formato de registros de grupo

*"GID, Tipo, Gíupo \n"* → para grupos: **GID, Tipo, Grupo \n** (Tipo = "G").

### Regla de eliminación con ID = 0

*"Un id 0 significa que el usuario o grupo está eliminado."*

### Restricción de ejecución solo por root

*"este comando solo lo puede utilizar el usuario root. Si otro usuario lo intenta ejecutar, deberá mostrar un mensaje de error."*

### Límite de longitud del nombre del grupo

*"el grupo ocupará como máximo 10 letras al igual que el usuario y la contraseña."*

---

## PARTE 2 — INTERPRETACIÓN TÉCNICA (sin inventar reglas)

- **MKGRP crea un registro en users.txt.** El PDF dice que el comando *"creará un grupo … y se guardará en el archivo users.txt de la partición"*. Por tanto, MKGRP añade una línea de tipo grupo al archivo users.txt de la partición de la sesión actual.

- **Formato exacto del registro de grupo.** Según la estructura definida para grupos: **GID, Tipo, Grupo** con salto de línea. Con Tipo = "G": **GID,G,NombreGrupo\n** (sin espacios extra salvo que el PDF los indique; los ejemplos usan "1, G, íoot" con espacios después de las comas; el enunciado no exige un único formato de espacios, sí el orden GID, Tipo, Grupo). Interpretación mínima: **GID,G,NombreGrupo** como contenido de la línea, con \n al final.

- **Si un grupo se elimina se marca con ID = 0.** El PDF: *"Un id 0 significa que el usuario o grupo está eliminado"*. No se dice que se borre la línea; se interpreta que el registro puede conservarse con GID = 0 para indicar “eliminado”. RMGRP marca con 0 (ejemplo: *"0, G, usuaíios \n"*). Para MKGRP, un “grupo que ya existe” debe entenderse como un grupo **activo** (GID ≠ 0); si existe un registro con el mismo nombre pero GID = 0, el PDF no define explícitamente si se reutiliza o se crea otro; la interpretación más directa es que “ya existe” se refiere a grupo activo (GID ≠ 0).

- **Solo root puede ejecutarlo.** *"solo lo puede utilizar el usuario root. Si otro usuario lo intenta ejecutar, deberá mostrar un mensaje de error"* → hay que comprobar que el usuario actual de la sesión sea root (p. ej. por nombre "root") y, si no, mostrar mensaje de error.

- **users.txt está en la raíz de la partición donde se inició sesión.** *"users.txt guardado en el sistema ext2 de la raíz de cada partición"* y MKGRP opera sobre *"users.txt de la partición"* en *"partición de la sesión actual"* → el archivo a modificar es users.txt en la raíz del sistema de archivos de la partición asociada a la sesión (id de partición montada).

- **Nombre del grupo: como máximo 10 letras.** *"el grupo ocupará como máximo 10 letras"* → el nombre del grupo tiene un límite de 10 caracteres.

- **Distinguir mayúsculas y minúsculas.** *"Distinguirá entre mayúsculas y minúsculas"* → la comparación del nombre del grupo (para “ya existe” y para el nuevo registro) debe ser case-sensitive.

- No se añaden comportamientos no descritos (p. ej. permisos sobre el archivo, orden interno de líneas, etc.) salvo los necesarios para cumplir lo anterior.

---

## PARTE 3 — VALIDACIONES OBLIGATORIAS (según el PDF)

Antes de crear el grupo se debe validar, en un orden coherente:

1. **Debe existir sesión activa.** Los comandos distintos de MKFS y LOGIN *"requieren que exista una sesión activa"*; si no hay sesión, *"indicar mediante un error que no existe una sesion activa"* (o que necesita iniciar sesión). MKGRP no es excepción → primera comprobación: sesión activa.

2. **El usuario activo debe ser root.** *"este comando solo lo puede utilizar el usuario root. Si otro usuario lo intenta ejecutar, deberá mostrar un mensaje de error"* → segunda comprobación: usuario de la sesión es root (según criterio definido en el proyecto, p. ej. nombre "root").

3. **El archivo users.txt debe existir.** MKGRP *"se guardará en el archivo users.txt de la partición"*; si users.txt no existe en la raíz de esa partición, no hay dónde guardar el grupo. El PDF no dice explícitamente “si no existe users.txt mostrar error”, pero la operación del comando es escribir en ese archivo; si no existe, la operación no puede completarse → validación obligatoria en la práctica: users.txt debe existir (o tratarse como error si no se puede abrir/localizar).

4. **El grupo no debe existir previamente como grupo activo (GID ≠ 0).** *"si el grupo a ingresar ya existe deberá mostrar un mensaje de error"* → debe comprobarse que no exista ya un registro de tipo G con el mismo nombre de grupo y con GID ≠ 0. Los registros con GID = 0 se consideran eliminados y no cuentan como “grupo existente” para esta regla.

5. **El nombre debe cumplir el límite indicado en el PDF.** *"el grupo ocupará como máximo 10 letras"* → el nombre indicado en -name no debe superar 10 caracteres (o 10 “letras” según el enunciado). Si se supera, el PDF no define mensaje concreto; la interpretación mínima es no aceptar nombres que incumplan el límite y mostrar error.

No se agregan validaciones no mencionadas en el enunciado (p. ej. caracteres prohibidos, nombres vacíos, etc.) salvo las que se siguen directamente de lo anterior.

---

## PARTE 4 — OBTENCIÓN DEL NUEVO GID

- **Leer todos los registros tipo G** en users.txt. Los registros de grupo tienen el formato GID, Tipo, Grupo; Tipo = "G".

- **Ignorar registros con GID = 0.** *"Un id 0 significa que el usuario o grupo está eliminado"*; los IDs activos son los distintos de 0. Para “ir aumentando según se vayan creando” solo cuentan los grupos no eliminados.

- **Obtener el mayor GID activo.** *"el id de grupo o de usuario irá aumentando según se vayan creando usuarios o grupos"* → los nuevos IDs se asignan en orden creciente. Por tanto, el siguiente GID disponible es el que sigue al mayor GID ya usado en registros de tipo G con GID ≠ 0. Si no hay ningún grupo activo, el mayor GID puede considerarse 0 (y el nuevo será 1, como en el ejemplo inicial "1, G, root").

- **El nuevo GID será (mayor GID + 1).** Así se cumple que el id “irá aumentando según se vayan creando” grupos. El ejemplo del PDF muestra que tras root (GID 1) el nuevo grupo "usuarios" recibe GID 2 → nuevo GID = mayor GID activo + 1.

**Resumen:** Nuevo GID = 1 + max{ GID de registros tipo G con GID ≠ 0 }; si no hay ninguno, max = 0 y nuevo GID = 1.

---

## PARTE 5 — ALGORITMO FORMAL PASO A PASO

1. **Verificar sesión activa.** Si no hay sesión activa, mostrar mensaje de error indicando que no existe una sesión activa (o que necesita iniciar sesión) y terminar. *Justificación:* todos los comandos excepto MKFS y LOGIN requieren sesión activa.

2. **Verificar usuario root.** Si el usuario actual de la sesión no es root, mostrar mensaje de error y terminar. *Justificación:* *"solo lo puede utilizar el usuario root. Si otro usuario lo intenta ejecutar, deberá mostrar un mensaje de error."*

3. **Localizar partición de la sesión.** A partir del id de partición montada de la sesión, resolver path del disco y partición (part_start, part_s) y abrir el disco. *Justificación:* *"se guardará en el archivo users.txt de la partición"* y *"partición de la sesión actual"*.

4. **Localizar users.txt en la raíz.** En el sistema de archivos de esa partición, obtener el archivo "users.txt" en el directorio raíz (inodo 0, entrada correspondiente en el/los FolderBlock de la raíz). Si no existe, mostrar error y terminar. *Justificación:* *"users.txt guardado en el sistema ext2 de la raíz de cada partición"*.

5. **Leer completamente el archivo.** Leer el contenido de users.txt (inodo del archivo, bloques según i_block e i_s). *Justificación:* necesario para parsear registros, comprobar “ya existe” y calcular el nuevo GID.

6. **Parsear registros.** Dividir el contenido en líneas; para cada línea, identificar si es registro de grupo (formato GID, G, Grupo) o de usuario (UID, U, Grupo, Usuario, Contraseña). Extraer GID y nombre de grupo de cada registro tipo G. *Justificación:* se necesita para validar “grupo ya existe” y para obtener el mayor GID activo.

7. **Validar que no exista grupo activo con ese nombre.** Si existe un registro tipo G con GID ≠ 0 y nombre de grupo igual al parámetro -name (comparación distinguiendo mayúsculas y minúsculas), mostrar mensaje de error y terminar. *Justificación:* *"si el grupo a ingresar ya existe deberá mostrar un mensaje de error"* y *"Distinguirá entre mayúsculas y minúsculas"*.

8. **Validar longitud del nombre.** Si el nombre del grupo supera 10 caracteres (o 10 letras según el enunciado), mostrar error y terminar. *Justificación:* *"el grupo ocupará como máximo 10 letras"*.

9. **Obtener nuevo GID.** Entre los registros tipo G con GID ≠ 0, calcular max_gid; nuevo GID = max_gid + 1 (si no hay ninguno, nuevo GID = 1). *Justificación:* *"el id de grupo o de usuario irá aumentando según se vayan creando usuarios o grupos"* (PARTE 4).

10. **Construir nuevo registro.** Construir la línea del nuevo grupo en el formato definido: GID,G,NombreGrupo (con el nuevo GID, "G", y el nombre indicado en -name), terminada en \n. *Justificación:* estructura *"GID, Tipo, Grupo \n"* del PDF.

11. **Agregar el registro al final del archivo.** Añadir la nueva línea al contenido actual de users.txt. Si el contenido actual no termina en \n, puede añadirse \n antes de la nueva línea para mantener una línea por registro. *Justificación:* MKGRP “creará un grupo” y “se guardará en el archivo users.txt”; el ejemplo muestra la nueva línea después de las existentes.

12. **Reescribir bloques necesarios del sistema de archivos.** Escribir el contenido actualizado de users.txt en los bloques correspondientes del sistema de archivos (actualizar FileBlock(s) y, si cambia el tamaño, el inodo de users.txt). *Justificación:* el archivo users.txt es un archivo lógico en ext2; modificarlo implica actualizar los bloques y metadatos según el diseño del proyecto.

13. **Mostrar mensaje de éxito.** Devolver un mensaje indicando que el grupo se creó correctamente. El PDF no fija el texto. *Justificación:* feedback al usuario; el enunciado no exige mensaje concreto de éxito.

Cada paso está justificado con una referencia al comportamiento definido en el PDF.

---

## PARTE 6 — MANEJO DE ERRORES

Se especifican **únicamente** los errores que el PDF exige o que se siguen de él:

| Condición | Acción | Base en el PDF |
|-----------|--------|-----------------|
| No hay sesión activa | Mostrar mensaje de error (no existe sesión activa / necesita iniciar sesión). | *"requieren que exista una sesión activa … indicar mediante un error que no existe una sesion activa"*. |
| Usuario no es root | Mostrar mensaje de error. | *"Si otro usuario lo intenta ejecutar, deberá mostrar un mensaje de error."* |
| Grupo ya existe (activo, GID ≠ 0) | Mostrar mensaje de error. | *"si el grupo a ingresar ya existe deberá mostrar un mensaje de error."* |
| Nombre inválido (longitud > 10) | Mostrar mensaje de error. | *"el grupo ocupará como máximo 10 letras"* → no aceptar nombres que incumplan el límite. |
| No existe users.txt (o no se puede localizar/leer) | Mostrar mensaje de error. | El comando escribe en users.txt; si no existe o no es accesible, la operación no puede completarse. |

No se inventan mensajes adicionales ni errores no derivados del enunciado (p. ej. “nombre vacío”, “caracteres no permitidos”, etc.) salvo los que se consideren necesarios para cumplir lo anterior sin añadir reglas externas.

---

## PARTE 7 — CONFIRMACIÓN FINAL

- **El diseño cumple estrictamente con el PDF:** definición de MKGRP, parámetro -name, guardado en users.txt de la partición, solo usuario root, error si el grupo ya existe, distinción mayúsculas/minúsculas, formato de registros de grupo, regla id 0 = eliminado, límite de 10 letras para el grupo, y obtención del nuevo GID como mayor GID activo + 1 están tomados del enunciado o de interpretación directa del mismo.

- **No se asumieron reglas externas:** no se usa comportamiento típico de Linux ni de otros proyectos; solo se usan las reglas definidas en el enunciado (users.txt en la raíz, estructura GID,Tipo,Grupo, id 0 eliminado, solo root, grupo ya existe, máximo 10 letras, ids aumentando al crear).

- **No se agregó comportamiento Linux:** no se introducen permisos, convenciones de nombres ni reglas de sistema de archivos distintas de las del PDF.

- **Solo se utilizaron reglas definidas en el enunciado:** cada validación, cada paso del algoritmo y cada error tienen su justificación en el texto literal o en una interpretación directa del PDF del Proyecto 1 MIA 1S2026.
