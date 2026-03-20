# Diseño formal del comando RMGRP — Proyecto 1 MIA 1S2026 (solo PDF oficial)

Basado **únicamente** en el PDF oficial. Sin comportamiento Linux, sin reglas no mencionadas, sin validaciones adicionales inventadas. Todo justificado con texto literal del enunciado o interpretación directa mínima.

---

## PARTE 1 — TEXTO LITERAL DEL PDF

### RMGRP

*"11. RMGRP"*  
*"Este comando eliminará un grupo para los usuarios de la partición."*  
*"Solo lo puede utilizar el usuario root, si lo utiliza alguien más debe mostrar un error. Recibirá los siguientes parámetros:"*

*"-name           Obligatorio     Indicará el nombre del grupo a eliminar."*  
*"                                 Si el grupo no se encuentra dentro de la partición"*  
*"                                 debe mostrar un error."*

*"Ejemplo:"*  
*"#Elimina el grupo de usuarios en la partición de la sesión actual"*  
*"rmgrp -name=usuarios"*

*"#Debe mostrar mensaje de error ya que el grupo no existe porque ya"*  
*"#fue eliminado"*  
*"rmgrp -name=usuarios"*

*"El archivo users.txt debería quedar como el siguiente:"*  
*"1, G, íoot \n"*  
*"1, U, íoot, íoot, 123 \n 0, G, usuaíios \n"*

### Eliminación lógica con ID = 0

*"Un id 0 significa que el usuario o grupo está eliminado."*  
(En la sección de users.txt.)

El ejemplo de RMGRP muestra que tras eliminar el grupo "usuarios" el registro permanece en users.txt pero con GID 0: *"0, G, usuaíios \n"*.

### users.txt

*"Este archivo lógico almacenado en el disco será llamado users.txt guardado en el sistema ext2 de la raíz de cada partición. Existirán dos tipos de registros, unos para grupos y otros para usuarios. Un id 0 significa que el usuario o grupo está eliminado, el id de grupo o de usuario irá aumentando según se vayan creando usuarios o grupos."*

*"Tendrá la siguiente estructura:"*  
*"GID, Tipo, Gíupo \n"*  
*"UID, Tipo, Gíupo, Usuaíio, Contíaseña \n"*

(OCR: Gíupo = Grupo, Usuaíio = Usuario, Contíaseña = Contraseña, íoot = root, usuaíios = usuarios.)

### Restricción de ejecución solo por root

*"Solo lo puede utilizar el usuario root, si lo utiliza alguien más debe mostrar un error."*

### Manejo de registros de grupo

El PDF no describe paso a paso el procedimiento interno; se infiere del ejemplo: el registro del grupo eliminado sigue en el archivo con su línea modificada (GID pasado a 0). No se indica borrado físico de la línea ni reordenación del archivo.

### Distinción entre mayúsculas y minúsculas

El PDF **no** indica para RMGRP que se distinga entre mayúsculas y minúsculas (sí lo hace para LOGIN y MKGRP). Para RMGRP no hay texto literal al respecto.

---

## PARTE 2 — INTERPRETACIÓN TÉCNICA (sin inventar reglas)

- **RMGRP no borra físicamente el registro.** El ejemplo muestra que tras *"rmgrp -name=usuarios"* en users.txt queda *"0, G, usuaíios \n"*: la línea sigue existiendo, solo cambia el GID. Por tanto, la eliminación es lógica; no se elimina la línea del archivo.

- **Se debe colocar el GID del grupo en 0.** *"Un id 0 significa que el usuario o grupo está eliminado"* y el ejemplo muestra *"0, G, usuaíios \n"*. Por tanto, la operación es: localizar el registro tipo G con ese nombre (y GID ≠ 0) y cambiar su GID a 0, manteniendo Tipo y nombre: de **GID,G,NombreGrupo** a **0,G,NombreGrupo**.

- **Solo root puede ejecutarlo.** *"Solo lo puede utilizar el usuario root, si lo utiliza alguien más debe mostrar un error"* → debe comprobarse que el usuario actual de la sesión sea root (p. ej. por nombre "root") y, si no, mostrar error.

- **El archivo users.txt está en la raíz de la partición donde se inició sesión.** RMGRP actúa sobre *"un grupo para los usuarios de la partición"*; los comandos que modifican users.txt se ejecutan sobre la partición de la sesión. *"users.txt guardado en el sistema ext2 de la raíz de cada partición"* → el archivo a modificar es users.txt en la raíz del sistema de archivos de la partición asociada a la sesión.

- **Comparación del nombre del grupo.** El PDF no exige para RMGRP distinción mayúsculas/minúsculas. La búsqueda del grupo puede hacerse por coincidencia exacta del nombre; si se desea consistencia con MKGRP (mismo archivo), puede usarse la misma convención (case-sensitive), pero no es una regla impuesta por el enunciado para RMGRP.

**No se agregan reglas externas:**

- **Eliminación en cascada:** el PDF no dice que al eliminar un grupo se modifiquen o eliminen usuarios que lo tenían como grupo. No se implementa.

- **Validación de usuarios asociados:** el PDF no exige comprobar si hay usuarios en ese grupo antes de eliminarlo. No se agrega.

- **Protección especial del grupo root:** el PDF no dice que no se pueda eliminar el grupo "root". No se añade restricción; si se desea no permitir eliminar root, habría que justificarlo con texto del enunciado (no hay tal texto).

---

## PARTE 3 — VALIDACIONES OBLIGATORIAS (según el PDF)

Antes de realizar la eliminación lógica se debe validar:

1. **Debe existir sesión activa.** Los comandos que no son MKFS ni LOGIN requieren sesión activa; si no la hay, mostrar error. MKGRP/RMGRP no son excepciones → comprobar sesión activa.

2. **El usuario activo debe ser root.** *"Solo lo puede utilizar el usuario root, si lo utiliza alguien más debe mostrar un error"* → comprobar que el usuario de la sesión sea root.

3. **Debe existir users.txt.** RMGRP modifica users.txt; si el archivo no existe o no se puede localizar/leer en la raíz de la partición de la sesión, la operación no puede realizarse → validar existencia/accesibilidad de users.txt.

4. **Debe existir un grupo activo (GID ≠ 0) con ese nombre.** *"Si el grupo no se encuentra dentro de la partición debe mostrar un error"* y *"#Debe mostrar mensaje de error ya que el grupo no existe porque ya fue eliminado"* → el “grupo” que debe “encontrarse” es un registro tipo G con ese nombre y GID ≠ 0. Si solo existe con GID = 0 (ya eliminado), se considera que no se encuentra y debe mostrarse error.

5. **Comparación del nombre.** El PDF no exige para RMGRP distinción mayúsculas/minúsculas. No se añade como requisito obligatorio; la búsqueda del grupo por nombre se hace según implementación (coincidencia exacta; opcionalmente case-sensitive por consistencia con MKGRP).

No se agregan validaciones no mencionadas en el enunciado (p. ej. nombre vacío, longitud, caracteres especiales, etc.).

---

## PARTE 4 — PROCEDIMIENTO DE ELIMINACIÓN LÓGICA

- **Leer completamente users.txt.** Obtener todo el contenido del archivo users.txt de la raíz de la partición de la sesión (lectura del inodo y bloques correspondientes).

- **Parsear los registros.** Dividir el contenido en líneas e identificar registros tipo G (formato GID, Tipo, Grupo) y tipo U. Para cada registro tipo G, tener GID y nombre de grupo.

- **Localizar el registro tipo G con ese nombre y GID ≠ 0.** Buscar la línea que corresponde a un grupo con el nombre indicado en -name y cuyo GID sea distinto de 0 (grupo activo). Si no existe, se cumple la condición de error “el grupo no se encuentra dentro de la partición”.

- **Modificar únicamente ese registro.** En esa línea, cambiar el valor del GID al número 0, manteniendo el resto (Tipo "G" y nombre del grupo). Es decir:
  - De: **GID,G,NombreGrupo**
  - A: **0,G,NombreGrupo**

- **No eliminar la línea.** El ejemplo del PDF muestra que el registro sigue en el archivo con GID 0. No se borra la línea.

- **No reordenar el archivo.** El PDF no pide reordenar ni compactar; solo se cambia el GID de la línea correspondiente.

- **No modificar otros registros.** Solo se altera el registro del grupo a eliminar; el resto del contenido de users.txt permanece igual.

**Justificación:** *"Un id 0 significa que el usuario o grupo está eliminado"* define el significado de GID (o UID) 0; por tanto, poner GID = 0 en el registro de grupo es la forma definida en el enunciado de marcar ese grupo como eliminado.

---

## PARTE 5 — ALGORITMO FORMAL PASO A PASO

1. **Verificar sesión activa.** Si no hay sesión activa, mostrar mensaje de error (no existe sesión activa / debe iniciar sesión) y terminar. *Justificación:* los comandos que modifican la partición de la sesión requieren sesión activa.

2. **Verificar usuario root.** Si el usuario actual de la sesión no es root, mostrar mensaje de error y terminar. *Justificación:* *"Solo lo puede utilizar el usuario root, si lo utiliza alguien más debe mostrar un error."*

3. **Localizar partición de la sesión.** A partir del id de partición montada de la sesión, resolver path del disco y partición (part_start, part_s) y abrir el disco. *Justificación:* RMGRP actúa sobre users.txt de la partición; la partición es la de la sesión.

4. **Localizar users.txt en la raíz.** En el sistema de archivos de esa partición, obtener el archivo "users.txt" en el directorio raíz. Si no existe o no es accesible, mostrar error y terminar. *Justificación:* *"users.txt guardado en el sistema ext2 de la raíz de cada partición"*.

5. **Leer completamente el archivo.** Leer el contenido de users.txt (inodo y bloques según i_block e i_s). *Justificación:* necesario para parsear, buscar el grupo y luego reescribir el contenido modificado.

6. **Parsear registros.** Dividir el contenido en líneas e identificar registros tipo G (GID, G, Grupo). *Justificación:* para encontrar la línea del grupo a eliminar.

7. **Buscar grupo activo con ese nombre.** Buscar un registro tipo G cuyo nombre coincida con el parámetro -name y cuyo GID sea ≠ 0. *Justificación:* *"Si el grupo no se encuentra dentro de la partición debe mostrar un error"*; “encontrarse” = existir como activo (GID ≠ 0).

8. **Si no existe → error.** Si no hay ningún registro tipo G con ese nombre y GID ≠ 0, mostrar mensaje de error y terminar. *Justificación:* *"Si el grupo no se encuentra dentro de la partición debe mostrar un error"* y ejemplo *"el grupo no existe porque ya fue eliminado"*.

9. **Modificar su GID a 0.** En el contenido en memoria, en la línea correspondiente a ese registro, sustituir el GID por 0 (formato resultante: 0,G,NombreGrupo). *Justificación:* *"Un id 0 significa que el usuario o grupo está eliminado"* y ejemplo *"0, G, usuaíios \n"*.

10. **Reescribir el archivo en los bloques correspondientes.** Escribir el contenido actualizado de users.txt en los bloques del sistema de archivos (y actualizar inodo si cambia tamaño). *Justificación:* la modificación debe persistir en disco.

11. **Mostrar mensaje de éxito.** Devolver un mensaje indicando que el grupo se eliminó correctamente. El PDF no fija el texto. *Justificación:* feedback al usuario.

Cada paso está justificado con el comportamiento definido en el PDF.

---

## PARTE 6 — MANEJO DE ERRORES

Solo los errores que el PDF exige o que se siguen directamente del enunciado:

| Condición | Acción | Base en el PDF |
|-----------|--------|-----------------|
| No hay sesión activa | Mostrar mensaje de error (no existe sesión activa / debe iniciar sesión). | Comandos sobre la partición de la sesión requieren sesión activa. |
| Usuario no es root | Mostrar mensaje de error. | *"Solo lo puede utilizar el usuario root, si lo utiliza alguien más debe mostrar un error."* |
| Grupo no existe (activo) | Mostrar mensaje de error. | *"Si el grupo no se encuentra dentro de la partición debe mostrar un error"*; ejemplo: *"el grupo no existe porque ya fue eliminado"*. |
| users.txt no existe o no accesible | Mostrar mensaje de error. | RMGRP modifica users.txt; si no existe o no se puede acceder, la operación no puede completarse. |

No se inventan mensajes adicionales ni reglas no descritas en el enunciado.

---

## PARTE 7 — CONFIRMACIÓN FINAL

- **El diseño cumple estrictamente con el PDF:** definición de RMGRP (eliminar un grupo, solo root, parámetro -name, error si el grupo no se encuentra), eliminación lógica (GID = 0, registro conservado), ubicación de users.txt en la raíz de la partición, y manejo de errores (no sesión, no root, grupo no encontrado) están tomados del enunciado o de interpretación directa mínima.

- **No se asumieron reglas externas:** no se introduce eliminación en cascada, validación de usuarios en el grupo ni protección especial del grupo root; solo lo que el PDF establece o implica.

- **No se añadió comportamiento Linux:** no se incorporan reglas de sistema ni convenciones ajenas al enunciado.

- **Solo se utilizaron las instrucciones explícitas del enunciado:** cada validación, cada paso del algoritmo y cada error están justificados con el texto literal o la interpretación directa mínima del PDF del Proyecto 1 MIA 1S2026.
