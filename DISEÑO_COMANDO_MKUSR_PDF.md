# Diseño formal del comando MKUSR — Proyecto 1 MIA 1S2026 (solo PDF oficial)

Basado **únicamente** en el PDF oficial. Sin comportamiento Linux, sin reglas adicionales inventadas, sin validaciones no mencionadas. Todo justificado con texto literal o interpretación directa mínima del enunciado. Documento formal listo para implementación.

---

## PARTE 1 — TEXTO LITERAL DEL PDF

### MKUSR

*"12. MKUSR"*  
*"Este comando crea un usuario en la partición. Solo lo puede ejecutar el usuario root, si lo utiliza otro usuario deberá mostrar un error. Recibirá los siguientes parámetros:"*

*"-user             Obligatorio     Indicará el nombre del usuario a crear, si ya existe,"*  
*"                                   deberá mostrar un error indicando que ya existe el"*  
*"                                   usuario."*  
*"                                   Máximo: 10 caracteres."*

*"-pass             Obligatorio     Indicará la contraseña del usuario"*  
*"                                   Máximo 10 Caracteres"*

*"-grp              Obligatorio     Indicará el grupo al que pertenece el usuario."*  
*"                                   Debe de existir en la partición en la que se está"*  
*"                                   creando el usuario, si no debe mostrar un mensaje"*  
*"                                   de error."*  
*"                                   Máximo 10 Caracteres"*

Ejemplo en el PDF:  
*"#Crea usuario user1 en el grupo 'usuarios'"*  
*"mkusr -user=user1 -pass=usuario -grp=usuarios"*

*"#Debe mostrar mensaje de error ya que el usuario ya existe"*  
*"mkusr -user=user1 -pass=usuario -grp=usuarios2"*

### Parámetros obligatorios (-user, -pass, -grp)

Los tres parámetros son obligatorios según la tabla del PDF: -user (nombre del usuario a crear, máximo 10 caracteres), -pass (contraseña, máximo 10 caracteres), -grp (grupo al que pertenece, debe existir en la partición, máximo 10 caracteres).

### Restricción de ejecución solo por root

*"Solo lo puede ejecutar el usuario root, si lo utiliza otro usuario deberá mostrar un error."*

### Estructura de users.txt

*"Este archivo lógico almacenado en el disco será llamado users.txt guardado en el sistema ext2 de la raíz de cada partición. Existirán dos tipos de registros, unos para grupos y otros para usuarios. Un id 0 significa que el usuario o grupo está eliminado, el id de grupo o de usuario irá aumentando según se vayan creando usuarios o grupos."*

*"Tendrá la siguiente estructura:"*  
*"GID, Tipo, Gíupo \n"*  
*"UID, Tipo, Gíupo, Usuaíio, Contíaseña \n"*

(En el PDF aparecen OCR: Gíupo = Grupo, Usuaíio = Usuario, Contíaseña = Contraseña.)

### Formato de registro de usuario

Para usuarios: **UID, Tipo, Grupo, Usuario, Contraseña** con Tipo = "U". Es decir, **UID,U,Grupo,Usuario,Contraseña** (y salto de línea al final de la línea).

### Regla de eliminación con ID = 0

*"Un id 0 significa que el usuario o grupo está eliminado."*

El ejemplo tras RMUSR muestra: *"0, U, usuaíios, useí1, usuaíio \n"* — el registro permanece pero con UID = 0.

### Límite de longitud de usuario y contraseña

En la sección de users.txt: *"el grupo ocupará como máximo 10 letras al igual que el usuario y la contraseña."*

En MKUSR explícitamente:  
- *"Máximo: 10 caracteres"* para -user.  
- *"Máximo 10 Caracteres"* para -pass.  
- *"Máximo 10 Caracteres"* para -grp.

### Distinción de mayúsculas y minúsculas

El PDF **no** indica para MKUSR que se distinga entre mayúsculas y minúsculas (sí lo hace para LOGIN y MKGRP). No hay texto literal en la sección de MKUSR al respecto.

---

## PARTE 2 — INTERPRETACIÓN TÉCNICA (sin inventar reglas)

- **MKUSR crea un nuevo registro tipo U en users.txt.** El PDF dice que el comando *"crea un usuario en la partición"* y que el grupo *"Debe de existir en la partición"*; los usuarios se almacenan en users.txt según la estructura del archivo. Por tanto, MKUSR añade una línea de tipo usuario (U) a users.txt.

- **Formato exacto del registro:** Según la estructura **UID, Tipo, Grupo, Usuario, Contraseña** con Tipo = "U": **UID,U,NombreGrupo,NombreUsuario,Contraseña** (y \n al final). Los nombres y la contraseña son los valores de -grp, -user y -pass respectivamente.

- **El campo "NombreGrupo" es el nombre del grupo, no el GID.** La estructura del PDF es "UID, Tipo, Gíupo, Usuaíio, Contíaseña"; el tercer campo es el nombre del grupo (igual que en LOGIN). Por tanto en el registro se escribe el **nombre** del grupo indicado en -grp, no el GID numérico.

- **Solo root puede ejecutarlo.** *"Solo lo puede ejecutar el usuario root, si lo utiliza otro usuario deberá mostrar un error"* → validar que el usuario actual de la sesión sea root.

- **users.txt está en la raíz de la partición de la sesión.** users.txt está *"en el sistema ext2 de la raíz de cada partición"*; MKUSR crea el usuario *"en la partición"* sobre la que se trabaja, que es la de la sesión actual. Por tanto el archivo a modificar es users.txt en la raíz del sistema de archivos de la partición asociada a la sesión.

- **UID debe incrementarse según los existentes.** *"el id de grupo o de usuario irá aumentando según se vayan creando usuarios o grupos"* → el nuevo UID es el siguiente al mayor UID ya usado en registros tipo U (considerando solo activos, UID ≠ 0).

- **Registros con UID = 0 están eliminados.** *"Un id 0 significa que el usuario o grupo está eliminado"* → para "usuario ya existe" solo cuenta si existe un registro tipo U con ese nombre y UID ≠ 0; los UID = 0 se ignoran como eliminados.

- **No se debe modificar registros anteriores.** MKUSR añade una línea nueva; el PDF no pide reordenar ni alterar líneas existentes, solo crear el usuario y guardarlo en users.txt (agregar al contenido).

**No se agregan reglas como:**

- **Hash de contraseña:** el PDF no pide cifrado ni hash; la contraseña se almacena en texto según el formato del registro.
- **Validación de caracteres especiales:** el PDF no exige validar caracteres en usuario, contraseña o grupo más allá del límite de longitud.
- **Restricciones adicionales no mencionadas:** no se imponen reglas externas (ej. no crear usuario "root", políticas de contraseña, etc.) salvo las del enunciado.

---

## PARTE 3 — VALIDACIONES OBLIGATORIAS (según el PDF)

Antes de crear el usuario se debe validar:

1. **Debe existir sesión activa.** Los comandos que no son MKFS ni LOGIN requieren sesión activa; si no la hay, mostrar error. MKUSR no es excepción.

2. **El usuario activo debe ser root.** *"Solo lo puede ejecutar el usuario root, si lo utiliza otro usuario deberá mostrar un error"* → comprobar que el usuario de la sesión sea root (p. ej. nombre "root").

3. **Debe existir users.txt.** MKUSR escribe en users.txt; si el archivo no existe o no es accesible en la raíz de la partición de la sesión, la operación no puede completarse → validar existencia/accesibilidad de users.txt.

4. **Debe existir un grupo activo (GID ≠ 0) con el nombre indicado en -grp.** *"Debe de existir en la partición en la que se está creando el usuario, si no debe mostrar un mensaje de error"* → debe haber al menos un registro tipo G en users.txt con ese nombre de grupo y GID ≠ 0.

5. **No debe existir un usuario activo (UID ≠ 0) con ese nombre.** *"si ya existe, deberá mostrar un error indicando que ya existe el usuario"* → no debe haber registro tipo U con el mismo nombre de usuario (valor de -user) y UID ≠ 0.

6. **Validar límite de longitud.** Según el PDF: usuario (-user) máximo 10 caracteres, contraseña (-pass) máximo 10 caracteres, grupo (-grp) máximo 10 caracteres. Si se supera alguno, debe mostrarse error (el PDF no fija el texto del mensaje).

7. **Distinción mayúsculas/minúsculas.** El PDF no exige para MKUSR distinción explícita; la búsqueda de "grupo existente" y "usuario ya existe" puede hacerse por coincidencia exacta según implementación. No se añade como requisito obligatorio si el enunciado no lo indica.

No se agregan validaciones no indicadas en el enunciado (ej. caracteres prohibidos, contraseña vacía, etc.) salvo las que se siguen de lo anterior.

---

## PARTE 4 — OBTENCIÓN DEL NUEVO UID

- **Leer todos los registros tipo U** en users.txt (formato UID, U, Grupo, Usuario, Contraseña).

- **Ignorar registros con UID = 0.** *"Un id 0 significa que el usuario o grupo está eliminado"*; para el incremento de IDs solo cuentan los usuarios no eliminados (UID ≠ 0).

- **Obtener el mayor UID activo.** *"el id de grupo o de usuario irá aumentando según se vayan creando usuarios o grupos"* → el siguiente UID es el que sigue al mayor UID ya usado en registros tipo U con UID ≠ 0.

- **El nuevo UID será (mayor UID + 1).** Si no hay ningún usuario activo además de root (p. ej. solo existe "1, U, root, root, 123"), el mayor UID es 1 y el nuevo será 2. Si no hubiera ningún registro U (caso teórico), el mayor sería 0 y el nuevo UID sería 1.

**Fórmula:** nuevo UID = 1 + max{ UID de registros tipo U con UID ≠ 0 }; si no hay ninguno, max = 0 y nuevo UID = 1.

**Justificación:** *"el id de grupo o de usuario irá aumentando según se vayan creando usuarios o grupos"* (texto literal del PDF en la sección de users.txt).

---

## PARTE 5 — ALGORITMO FORMAL PASO A PASO

1. **Verificar sesión activa.** Si no hay sesión activa, mostrar mensaje de error (no existe sesión activa / debe iniciar sesión) y terminar. *Justificación:* comandos sobre la partición de la sesión requieren sesión activa.

2. **Verificar usuario root.** Si el usuario actual de la sesión no es root, mostrar mensaje de error y terminar. *Justificación:* *"Solo lo puede ejecutar el usuario root, si lo utiliza otro usuario deberá mostrar un error."*

3. **Localizar partición de la sesión.** A partir del id de partición montada de la sesión, resolver path del disco y partición (part_start, part_s) y abrir el disco. *Justificación:* MKUSR crea el usuario en la partición; se usa la partición de la sesión.

4. **Localizar users.txt en la raíz.** En el sistema de archivos de esa partición, obtener el archivo "users.txt" en el directorio raíz. Si no existe o no es accesible, mostrar error y terminar. *Justificación:* *"users.txt guardado en el sistema ext2 de la raíz de cada partición"*.

5. **Leer completamente el archivo.** Leer el contenido de users.txt (inodo y bloques según i_block e i_s). *Justificación:* necesario para parsear registros, comprobar grupo activo, usuario ya existe y calcular nuevo UID.

6. **Parsear registros tipo G y tipo U.** Dividir el contenido en líneas; identificar registros tipo G (GID, G, Grupo) y tipo U (UID, U, Grupo, Usuario, Contraseña). *Justificación:* para validar existencia del grupo y no existencia del usuario, y para obtener max UID.

7. **Verificar que el grupo indicado exista activo.** Buscar un registro tipo G con nombre de grupo igual al valor de -grp y GID ≠ 0. Si no existe, mostrar mensaje de error y terminar. *Justificación:* *"Debe de existir en la partición ... si no debe mostrar un mensaje de error."*

8. **Verificar que el usuario no exista activo.** Buscar un registro tipo U con nombre de usuario igual al valor de -user y UID ≠ 0. Si existe, mostrar error indicando que ya existe el usuario y terminar. *Justificación:* *"si ya existe, deberá mostrar un error indicando que ya existe el usuario."*

9. **Validar longitudes.** Comprobar que longitud(-user) ≤ 10, longitud(-pass) ≤ 10, longitud(-grp) ≤ 10. Si no se cumple, mostrar error y terminar. *Justificación:* *"Máximo: 10 caracteres"* / *"Máximo 10 Caracteres"* en cada parámetro.

10. **Obtener nuevo UID.** Entre los registros tipo U con UID ≠ 0, calcular max_uid; nuevo UID = max_uid + 1 (si no hay ninguno, nuevo UID = 1). *Justificación:* PARTE 4 y regla del PDF sobre aumento de ids.

11. **Construir nueva línea.** Construir la línea del nuevo usuario en el formato: **UID,U,Grupo,Usuario,Contraseña** (con el nuevo UID, "U", y los valores de -grp, -user, -pass), terminada en \n. *Justificación:* estructura *"UID, Tipo, Grupo, Usuario, Contraseña"* del PDF.

12. **Agregar la línea al final del archivo.** Añadir la nueva línea al contenido actual de users.txt (respetando una línea por registro; si hace falta \n antes de la nueva línea, añadirlo). *Justificación:* MKUSR crea un usuario y lo guarda en users.txt; el ejemplo muestra la línea nueva después de las existentes.

13. **Reescribir los bloques necesarios del sistema de archivos.** Escribir el contenido actualizado de users.txt en los bloques correspondientes (y actualizar inodo si cambia el tamaño). *Justificación:* el archivo es lógico en ext2; la modificación debe persistir en disco.

14. **Mostrar mensaje de éxito.** Devolver un mensaje indicando que el usuario se creó correctamente. El PDF no fija el texto. *Justificación:* feedback al usuario.

Cada paso está justificado con el comportamiento definido en el PDF.

---

## PARTE 6 — MANEJO DE ERRORES

Solo los errores que el PDF exige o que se siguen directamente del enunciado:

| Condición | Acción | Base en el PDF |
|-----------|--------|-----------------|
| No hay sesión activa | Mostrar mensaje de error (no existe sesión activa / debe iniciar sesión). | Comandos sobre la partición requieren sesión activa. |
| Usuario no es root | Mostrar mensaje de error. | *"Solo lo puede ejecutar el usuario root, si lo utiliza otro usuario deberá mostrar un error."* |
| Grupo no existe (activo) | Mostrar mensaje de error. | *"Debe de existir en la partición ... si no debe mostrar un mensaje de error."* |
| Usuario ya existe (activo) | Mostrar error indicando que ya existe el usuario. | *"si ya existe, deberá mostrar un error indicando que ya existe el usuario."* |
| Longitud inválida (user, pass o grp > 10) | Mostrar mensaje de error. | *"Máximo: 10 caracteres"* / *"Máximo 10 Caracteres"* para -user, -pass, -grp. |
| users.txt no existe o no accesible | Mostrar mensaje de error. | MKUSR escribe en users.txt; si no existe o no se puede acceder, la operación no puede completarse. |

No se inventan mensajes adicionales ni condiciones de error no descritas en el enunciado.

---

## PARTE 7 — CONFIRMACIÓN FINAL

- **El diseño cumple estrictamente con el PDF:** definición de MKUSR (crear usuario en la partición, solo root, parámetros -user, -pass, -grp), formato de registro U (UID,U,Grupo,Usuario,Contraseña), regla UID = 0 para eliminado, límites de 10 caracteres, existencia del grupo y no existencia previa del usuario están tomados del enunciado o de interpretación directa mínima.

- **No se asumieron reglas externas:** no se introduce hash de contraseña, validación de caracteres especiales ni restricciones no mencionadas; solo lo que el PDF establece o implica.

- **No se añadió comportamiento Linux:** no se incorporan políticas de contraseña, UID/GID de sistema ni convenciones ajenas al enunciado.

- **Solo se utilizaron instrucciones explícitas del enunciado:** cada validación, cada paso del algoritmo y cada error están justificados con el texto literal o la interpretación directa mínima del PDF del Proyecto 1 MIA 1S2026. El documento es formal y está listo para implementación.
