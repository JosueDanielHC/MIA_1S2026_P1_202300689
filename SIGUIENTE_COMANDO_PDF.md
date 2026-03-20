# Siguiente comando después de MKFS según el PDF oficial (Proyecto 1 MIA)

Determinación basada **exclusivamente** en el texto literal del PDF. Sin flujo clásico MIA, sin asumir LOGIN, sin proyectos de años anteriores ni referencias externas.

---

## PARTE 1 — Orden oficial en el proyecto

### 1) Orden de presentación de los comandos en el PDF

En la sección **"A continuación se detallan los comandos disponibles"** (tras "Administración de discos" y "Administración del Sistema de Archivos"), el enunciado presenta los comandos en este orden numérico:

- 1. MKDISK  
- 2. RMDISK  
- 3. FDISK  
- 4. MOUNT  
- 5. MOUNTED  
- **6. MKFS**  
- **7. CAT**  
- 8. LOGIN  
- 9. LOGOUT  
- 10. MKGRP  
- …

**Texto literal (orden 6 → 7):**  
Tras el apartado **"6. MKFS"** y sus observaciones (*"Este comando es el que agrega las estructuras requeridas en el disco"*), el siguiente apartado es:

*"7. CAT"*  
*"Este comando permitirá mostrar el contenido del archivo, si el usuario que actualmente está logueado tiene acceso al permiso de lectura."*

En la **rúbrica de calificación (8.3 Detalle)** aparece la misma secuencia en "Parte 3: Administración del Sistema de Archivos Ext2":

- 5. MKFS — 5 pts  
- **6. CAT** — 2 pts  

Y en "Parte 4: Administración de Usuarios y Grupos": 7. LOGIN, 8. LOGOUT, etc.

No existe en el PDF una sección que diga "fases" o "etapas de implementación" con otro orden. El orden explícito es el de la lista numerada (1, 2, 3… 6. MKFS, 7. CAT…).

### 2) Respuestas explícitas

- **¿Cuál es el siguiente comando que el enunciado describe después de MKFS?**  
  **7. CAT.** Es el siguiente en la lista numerada y en la rúbrica.

- **¿El PDF exige implementar todos los comandos o solo algunos?**  
  En "4.3 Requerimientos técnicos" / listado de entregables el PDF indica que se deben implementar correctamente los comandos definidos (mkdisk, rmdisk, fdisk, mount, etc.). No establece un subconjunto “obligatorio primero”; el orden de descripción es el de la lista (1 a 7…).

- **¿Existe dependencia obligatoria entre comandos?**  
  Sí, de forma **textual**:  
  *"Los comandos de este apartado simularán el formateo de las particiones, administración de usuarios, carpetas y archivos* **a excepción de los comandos MKFS y LOGIN** *todos los demás comandos requieren que exista una sesión activa para su ejecución en caso de no ser así se debería de indicar mediante un error que no existe una sesion activa."*  
  Es decir: MKFS y LOGIN **no** requieren sesión; **todos los demás** (incluido CAT) **sí** requieren sesión activa.

---

## PARTE 2 — Definición formal del siguiente comando (CAT)

### 1) Texto literal del PDF

**Nombre:**  
*"7. CAT"*

**Descripción:**  
*"Este comando permitirá mostrar el contenido del archivo, si el usuario que actualmente está logueado tiene acceso al permiso de lectura."*

**Parámetros:**

| PARÁMETRO | CATEGORÍA | DESCRIPCIÓN (literal) |
|-----------|-----------|------------------------|
| -filen    | Obligatorio | *"Permitirá admitir como argumentos una lista de N ficheros que hay que enlazar. Estos se deben encadenar en el mismo orden en el cual fueron especificados. Si no existe el archivo o no tiene permiso de lectura, debe mostrarse un mensaje de error."* |

Ejemplos del PDF:  
*"cat -file1=/home/user/docs/a.txt"*  
*"cat -file1=\"/home/a.txt\" -file2=\"/home/b.txt\" -file3=\"/home/c.txt\""*

**Parámetros opcionales:**  
El PDF no indica parámetros opcionales para CAT; la lista de N ficheros se da con -file1, -file2, … -fileN.

**Restricciones:**  
- El usuario que ejecuta debe ser el que *"actualmente está logueado"*.  
- Debe tener *"acceso al permiso de lectura"* sobre el archivo.

**Errores definidos (literal):**  
- *"Si no existe el archivo o no tiene permiso de lectura, debe mostrarse un mensaje de error."*

### 2) Confirmaciones según el PDF

- **¿Requiere sesión iniciada?**  
  **Sí.** El PDF dice que, salvo MKFS y LOGIN, *"todos los demás comandos requieren que exista una sesión activa"*; CAT no está entre las excepciones. Además, la descripción de CAT exige *"el usuario que actualmente está logueado"*.

- **¿Requiere partición montada?**  
  La sesión se inicia con LOGIN sobre un *"id de la partición montada"*; CAT se ejecuta *"sobre la partición en la que inicio sesión"* (texto que aparece en la observación tras LOGOUT). Por tanto, de forma implícita la partición debe estar montada (tener un id usado en LOGIN).

- **¿Modifica estructuras del sistema de archivos?**  
  **No.** El PDF define CAT como mostrar el contenido del archivo; no indica escritura ni modificación de estructuras.

---

## PARTE 3 — Validación de dependencias

- **¿El siguiente comando (CAT) depende explícitamente de MKFS?**  
  El PDF no dice literalmente que CAT dependa de MKFS. Dependencia lógica: para que existan archivos que leer, la partición debe tener sistema de archivos (MKFS). Esa dependencia no está expresada como “CAT requiere haber ejecutado MKFS” en el enunciado.

- **¿Depende de MOUNT?**  
  No se dice de forma literal “CAT depende de MOUNT”. Sí se indica que los comandos que requieren sesión *"se ejecutan sobre la partición en la que inicio sesión"* y que LOGIN usa el *"id de la partición montada"*. Por tanto, para tener sesión sobre una partición, esa partición debe estar montada. La dependencia de CAT respecto de MOUNT es indirecta (vía sesión y LOGIN).

- **¿Depende de LOGIN?**  
  **Sí, de forma textual.** El PDF establece que todos los comandos excepto MKFS y LOGIN *"requieren que exista una sesión activa"*; si no, *"se debería de indicar mediante un error que no existe una sesion activa"*. CAT no es excepción, por lo que **requiere sesión activa**, y la sesión se establece con el comando LOGIN. Así, la dependencia de CAT respecto de LOGIN está indicada en el enunciado.

- **¿El PDF lo indica de forma textual?**  
  La exigencia de **sesión activa** para CAT (y por tanto la dependencia de LOGIN) sí está indicada textualmente en el párrafo que excepciona solo a MKFS y LOGIN.

---

## FORMATO OBLIGATORIO DE RESPUESTA

### A) Texto literal del PDF

- Orden: *"6. MKFS"* → *"7. CAT"* (siguiente apartado tras MKFS).  
- Excepción de sesión: *"a excepción de los comandos MKFS y LOGIN todos los demás comandos requieren que exista una sesión activa para su ejecución en caso de no ser así se debería de indicar mediante un error que no existe una sesion activa."*  
- CAT: *"Este comando permitirá mostrar el contenido del archivo, si el usuario que actualmente está logueado tiene acceso al permiso de lectura."*  
- Parámetro: *"-filen Obligatorio Permitirá admitir como argumentos una lista de N ficheros que hay que enlazar. Estos se deben encadenar en el mismo orden en el cual fueron especificados. Si no existe el archivo o no tiene permiso de lectura, debe mostrarse un mensaje de error."*  
- Observación tras LOGOUT: *"Los siguientes comandos que se verán a continuación necesitan que exista una sesión en el sistema ya que se ejecutan sobre la partición en la que inicio sesión. Si no, debe mostrar un mensaje de error indicando que necesita iniciar sesión."*

### B) Interpretación técnica correcta

- El **siguiente comando descrito** después de MKFS es **CAT (7. CAT)**.  
- CAT **requiere sesión activa** (no está entre las excepciones MKFS y LOGIN).  
- CAT requiere **usuario logueado** y **permiso de lectura** sobre los archivos; si el archivo no existe o no hay permiso, debe mostrarse **mensaje de error**.  
- Parámetros: lista de ficheros con **-file1**, **-file2**, … **-fileN** (el PDF escribe "-filen" en la tabla y usa -file1, -file2, -file3 en los ejemplos).  
- CAT **no modifica** estructuras; solo muestra contenido.

### C) Confirmación del siguiente comando a implementar

**El siguiente comando a implementar después de MKFS, según el orden y la descripción del PDF, es: 7. CAT.**

### D) Dependencias exactas según el PDF

- **Sesión activa:** obligatoria para CAT (texto que excepciona solo MKFS y LOGIN).  
- **LOGIN:** obligatorio para tener sesión; por tanto CAT depende de que se haya ejecutado LOGIN.  
- **Partición montada:** necesaria para poder iniciar sesión sobre un id; no se dice “CAT depende de MOUNT” de forma directa, pero la cadena sesión → LOGIN → id de partición montada lo implica.  
- **MKFS:** el PDF no dice textualmente que CAT dependa de MKFS; solo se infiere que debe existir sistema de archivos para que haya archivos que leer.

### E) Confirmación final

**El siguiente paso está determinado exclusivamente por el PDF:** el enunciado presenta **7. CAT** inmediatamente después de **6. MKFS** y lo define como comando que muestra el contenido de archivos con usuario logueado y permiso de lectura; además, establece de forma textual que CAT requiere **sesión activa** (y por tanto LOGIN).
