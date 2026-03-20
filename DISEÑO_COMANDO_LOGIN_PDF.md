# Diseño formal del comando 8. LOGIN — Proyecto 1 MIA (solo PDF oficial)

Basado **exclusivamente** en el PDF. Sin comportamiento típico de Linux, sin proyectos anteriores, sin asumir estructura de sesión ni validaciones no definidas.

---

## PARTE 1 — Definición formal del comando

### 1) Texto literal del PDF

**Nombre del comando:**  
*"8. LOGIN"*

**Descripción oficial:**  
*"Este comando se utiliza para iniciar sesión en el sistema. No se puede iniciar otra sesión sin haber hecho un LOGOUT antes, en caso contrario debe mostrar un mensaje de error indicando que debe cerrar sesión con anterioridad. Este comando recibirá los siguientes parámetros:"*

**Parámetros obligatorios:**

| PARÁMETRO | CATEGORÍA  | DESCRIPCIÓN (literal) |
|-----------|------------|-------------------------|
| -user     | Obligatorio | *"Especifica el nombre del usuario que iniciará sesión. Si no se encuentra mostrará un mensaje indicando que el usuario no existe. *distinguir mayúsculas de minúsculas."* |
| -pass     | Obligatorio | *"Indicará la contraseña del usuario que inicia sesión. Si no coincide debe mostrar un mensaje de autenticación fallida. *distinguirá entre mayúsculas y minúsculas."* |
| -id       | Obligatorio | *"Indicará el id de la partición montada de la cual van a iniciar sesión. De lograr iniciar sesión todas las acciones se realizarán sobre este id."* |

**Parámetros opcionales:**  
El PDF no indica parámetros opcionales para LOGIN.

**Condiciones de error (literal):**

- Si ya hay sesión activa: *"debe mostrar un mensaje de error indicando que debe cerrar sesión con anterioridad"*.
- Si el usuario no se encuentra: *"mostrará un mensaje indicando que el usuario no existe"*.
- Si la contraseña no coincide: *"debe mostrar un mensaje de autenticación fallida"*.

**Qué sucede si el login es exitoso (literal):**  
*"De lograr iniciar sesión todas las acciones se realizarán sobre este id."*  
Es decir: se establece la sesión y las acciones posteriores se realizan sobre la partición indicada por ese id.

### 2) Confirmaciones textuales

- **¿Permite múltiples sesiones simultáneas?**  
  **No.** Texto literal: *"No se puede iniciar otra sesión sin haber hecho un LOGOUT antes"*. Solo una sesión a la vez.

- **¿Qué pasa si ya existe una sesión activa?**  
  *"debe mostrar un mensaje de error indicando que debe cerrar sesión con anterioridad"*.

- **¿Qué mensaje en cada caso?**  
  - Sesión ya activa: mensaje de error indicando que debe cerrar sesión con anterioridad.  
  - Usuario no encontrado: mensaje indicando que el usuario no existe.  
  - Contraseña no coincide: mensaje de autenticación fallida.  
  El PDF no fija el texto exacto de cada mensaje; solo el sentido (cerrar sesión, usuario no existe, autenticación fallida).

---

## PARTE 2 — Acceso a users.txt

### 1) Dónde está users.txt y cómo localizarlo

**Texto literal (MKFS y Administración de Usuarios):**  
*"También creará un archivo en la raíz llamado users.txt"* (MKFS).  
*"Este archivo lógico almacenado en el disco será llamado users.txt guardado en el sistema ext2 de la raíz de cada partición."*

- **¿LOGIN debe buscar users.txt por path?**  
  El PDF no define un parámetro de path para users.txt en LOGIN. Indica que users.txt está **en la raíz de cada partición**. Por tanto: LOGIN obtiene la partición por **-id** (partición montada); en esa partición, users.txt está **en la raíz**. No se exige un path de usuario para el archivo; sí se exige localizarlo en la raíz del sistema de archivos de esa partición.

- **¿Puede asumir que está en la raíz?**  
  **Sí.** El PDF dice explícitamente que users.txt está guardado *"en el sistema ext2 de la raíz de cada partición"*.

- **¿El PDF indica explícitamente cómo localizarlo?**  
  Indica **dónde** está (raíz de la partición), no un algoritmo paso a paso. Localizarlo = leer el sistema de archivos de la partición correspondiente al -id y obtener el archivo de nombre "users.txt" en el directorio raíz (inodo 0, primer FolderBlock, entrada con b_name "users.txt").

### 2) Formato de users.txt (texto literal)

*"Tendrá la siguiente estructura:"*  
*"GID, Tipo, Gíupo \\n"*  
*"UID, Tipo, Gíupo, Usuaíio, Contíaseña \\n"*  

(En el PDF aparecen OCR "Gíupo", "Usuaíio", "Contíaseña"; se interpretan como Grupo, Usuario, Contraseña.)

*"El estado ocupará una letra, el tipo otra, el grupo ocupará como máximo 10 letras al igual que el usuario y la contraseña."*  
*"Al inicio existirá un grupo llamado root, un usuario root y una contraseña (123) para el usuario root. El archivo lógico almacenado en el disco al inicio debería ser como el siguiente:"*  
*"1, G, íoot \\n"*  
*"1, U, íoot, íoot, 123 \\n"*

Además: *"Un id 0 significa que el usuario o grupo está eliminado"*.

### 3) Registros con id = 0 y usuario válido

- **¿Debe ignorar registros con id = 0?**  
  **Sí.** El PDF: *"Un id 0 significa que el usuario o grupo está eliminado"*. Un usuario o grupo eliminado no debe considerarse válido para iniciar sesión.

- **¿Cómo se identifica un usuario válido?**  
  Por registros de tipo usuario (Tipo "U") con UID distinto de 0, donde el campo Usuario coincida con -user (distinguiendo mayúsculas y minúsculas) y el campo Contraseña coincida con -pass (distinguiendo mayúsculas y minúsculas).

- **¿Debe validar grupo además de usuario y contraseña?**  
  El PDF no exige para LOGIN comprobar que el grupo del usuario exista o esté activo (GID != 0). Solo exige encontrar al usuario por nombre y validar contraseña. La validación explícita es: usuario existe (y no eliminado) y contraseña coincide.

### 4) Resolución del GID a partir del registro U (ajuste explícito)

- **Campo "Grupo" en el registro U:** En la estructura *"UID, Tipo, Grupo, Usuario, Contraseña"* el tercer campo es el **nombre del grupo** (no el GID numérico). El usuario queda asociado a un grupo por **nombre** (p. ej. "root", "usuarios").

- **Búsqueda del GID:** Para obtener el GID del usuario logueado se buscan los registros de tipo **G** (*"GID, Tipo, Grupo"*) cuyo **nombre de grupo** coincida con el campo Grupo del registro U. Solo deben considerarse registros con **GID ≠ 0** (grupos no eliminados). Si se encuentra un registro G activo con ese nombre, su GID es el que se asigna a la sesión.

- **Grupo eliminado o inexistente (GID = 0):** Si el grupo al que pertenece el usuario está eliminado (existe un registro G con ese nombre pero GID = 0), o no existe ningún registro G activo con ese nombre, **el PDF no define que LOGIN deba mostrar error**. LOGIN solo exige validar usuario (existente, no eliminado) y contraseña. En este caso se asignará **GID = 0** en la sesión: el enunciado ya define que el id 0 significa "eliminado" y no establece ningún otro comportamiento; así no se inventa un valor ni un grupo nuevo, y la decisión queda acotada al PDF.

**Resumen:** El registro U trae el **nombre del grupo**; el GID se resuelve buscando en registros **G con GID ≠ 0** por ese nombre; si se encuentra, se usa ese GID; si no (grupo eliminado o inexistente), el PDF no exige fallar LOGIN y se asigna **GID = 0** en la sesión.

---

## PARTE 3 — Validaciones obligatorias

Según el PDF:

- **¿Qué pasa si el usuario no existe?**  
  *"Si no se encuentra mostrará un mensaje indicando que el usuario no existe."*

- **¿Qué pasa si la contraseña es incorrecta?**  
  *"Si no coincide debe mostrar un mensaje de autenticación fallida."*

- **¿Qué pasa si el grupo no existe?**  
  El PDF no define un mensaje ni un comportamiento específico para “grupo no existe” en LOGIN. No se considera una validación obligatoria definida en el enunciado.

- **¿Qué pasa si la partición no está montada?**  
  El parámetro -id *"Indicará el id de la partición **montada**"*. Si el id no corresponde a una partición montada (no está en la tabla de montados), no hay “partición montada” sobre la que iniciar sesión. El PDF no escribe literalmente “si el id no está montado mostrar error”, pero -id está definido como id de una partición montada; por tanto, si el id no está montado, debe tratarse como error (p. ej. “el id no corresponde a una partición montada” o similar).

- **¿LOGIN requiere partición montada?**  
  **Sí.** El PDF define -id como *"el id de la partición montada de la cual van a iniciar sesión"*. LOGIN opera sobre una partición montada identificada por ese id.

---

## PARTE 4 — Diseño de sesión en RAM

### 1) ¿El PDF define qué información debe almacenarse en sesión?

El PDF **no** define una estructura explícita de “sesión” ni lista de campos. Sí dice:

- *"De lograr iniciar sesión todas las acciones se realizarán sobre este id."*  
- Los comandos posteriores *"se ejecutan sobre la partición en la que inicio sesión"* y requieren *"el usuario que actualmente está logueado"* (CAT), *"el usuario que actualmente ha iniciado sesión"* (MKFILE, MKDIR), y para permisos UGO se usa propietario (i_uid), grupo (i_gid) y categoría U/G/O.

Por tanto, la sesión debe permitir: (1) saber sobre qué partición se actúa, (2) saber qué usuario está logueado y (3) poder evaluar permisos UGO (UID, GID del usuario).

### 2) Estructura mínima justificada por el PDF

Campos que pueden justificarse con el PDF:

- **id_particion_montada (o equivalente):** *"todas las acciones se realizarán sobre este id"* y *"sobre la partición en la que inicio sesión"* → es obligatorio almacenar el id (o path+nombre) de la partición sobre la que se inició sesión.
- **nombre_usuario:** *"el usuario que actualmente está logueado"* / *"usuario que actualmente ha iniciado sesión"* → hace falta identificar al usuario; el PDF usa el nombre (p. ej. root) en los ejemplos.
- **id_usuario (UID):** para permisos UGO, categoría User cuando UID == i_uid; el inodo tiene i_uid. Por tanto hace falta UID del usuario logueado.
- **id_grupo (GID):** para permisos UGO, categoría Grupo cuando GID == i_gid; el inodo tiene i_gid. Por tanto hace falta GID del usuario logueado.
- **indicador_sesion_activa (o “hay sesión”):** LOGOUT exige *"Debe haber una sesión activa anteriormente"* y los demás comandos exigen *"que exista una sesión en el sistema"*; hace falta un indicador de si hay sesión activa y, si la hay, los datos anteriores.

**Resumen mínimo:**  
- indicador_sesion_activa (o equivalente),  
- id_particion_montada,  
- nombre_usuario,  
- id_usuario (UID),  
- id_grupo (GID).  

Todo ello se justifica por las frases citadas del PDF; no se añaden campos sin referencia en el enunciado.

---

## PARTE 5 — Algoritmo formal de LOGIN

Pseudocódigo alineado solo con lo definido en el PDF:

```
COMANDO LOGIN (-user=nombre, -pass=contraseña, -id=id)

1) VERIFICAR QUE NO EXISTA SESIÓN ACTIVA
   SI ya hay sesión activa ENTONCES
     devolver mensaje de error indicando que debe cerrar sesión con anterioridad
     FIN
   FIN

2) VERIFICAR QUE LA PARTICIÓN ESTÉ MONTADA
   Buscar en la tabla de particiones montadas un registro con id = -id
   SI no existe ENTONCES
     devolver mensaje de error (el id no corresponde a una partición montada, o similar)
     FIN
   Obtener path del disco y nombre de la partición (o part_start, part_s) asociados a ese id.
   FIN

3) ABRIR DISCO Y LEER SUPERBLOCK
   Abrir archivo del disco (path) en lectura binaria.
   Leer MBR; localizar la partición por nombre (primaria); obtener part_start, part_s.
   Leer Superblock desde part_start.
   SI la partición no está formateada (p. ej. superblock inválido) ENTONCES
     devolver mensaje de error apropiado
     FIN
   FIN

4) LOCALIZAR users.txt EN LA RAÍZ
   Inodo raíz = inodo 0 (s_inode_start + 0 * s_inode_s).
   Leer inodo 0; es tipo carpeta; leer el/los FolderBlock apuntados por i_block[].
   Buscar en las entradas una cuyo b_name sea "users.txt" (hasta 12 caracteres).
   SI no se encuentra "users.txt" ENTONCES
     devolver mensaje de error (p. ej. users.txt no encontrado en la raíz)
     FIN
   inodo_users = b_inodo de esa entrada.
   FIN

5) LEER CONTENIDO COMPLETO DE users.txt
   Leer el inodo de users.txt; es tipo archivo; obtener i_block[] e i_s.
   Leer los FileBlock (y bloques indirectos si aplica) en orden; concatenar hasta i_s bytes.
   Contenido = texto completo del archivo.
   FIN

6) PARSEAR LÍNEAS
   Dividir Contenido en líneas (separador \n).
   Para cada línea no vacía:
     Si la línea tiene formato "GID, Tipo, Grupo" → registro de grupo (Tipo "G"). Si GID = 0, ignorar (eliminado).
     Si la línea tiene formato "UID, Tipo, Grupo, Usuario, Contraseña" → registro de usuario (Tipo "U"). Si UID = 0, ignorar (eliminado).
   FIN

7) BUSCAR USUARIO ACTIVO Y VALIDAR CONTRASEÑA
   Buscar en los registros de tipo U (usuario) uno donde:
     UID != 0 (no eliminado),
     Usuario == -user (comparación distinguiendo mayúsculas y minúsculas).
   SI no se encuentra ningún registro ENTONCES
     devolver mensaje indicando que el usuario no existe
     FIN
   Para el registro encontrado:
     SI Contraseña != -pass (comparación distinguiendo mayúsculas y minúsculas) ENTONCES
       devolver mensaje de autenticación fallida
       FIN
   UID = UID del registro U encontrado.
   El campo "Grupo" del registro U es el nombre del grupo (no el GID). Para obtener GID: buscar en los registros de tipo G con GID ≠ 0 uno cuyo nombre de grupo coincida con el campo Grupo del usuario; si se encuentra, GID = ese GID; si no se encuentra un grupo activo con ese nombre, el PDF no exige que LOGIN falle y se asignará GID = 0 en la sesión (el documento define id 0 como eliminado y no establece comportamiento adicional).
   FIN

8) CREAR SESIÓN EN RAM
   indicador_sesion_activa = true
   id_particion_montada = -id (o path+nombre según implementación)
   nombre_usuario = -user (o el valor del campo Usuario)
   id_usuario = UID del registro encontrado
   id_grupo = GID resuelto del grupo del usuario (si aplica)
   FIN

9) MOSTRAR MENSAJE DE ÉXITO
   Devolver mensaje indicando que se inició sesión correctamente (el PDF no fija el texto).
   FIN

10) MANEJO DE ERRORES (todos los definidos en el PDF)
    - Sesión ya activa → mensaje de error indicando que debe cerrar sesión con anterioridad.
    - Id no corresponde a partición montada → error.
    - Usuario no encontrado (o eliminado UID=0) → mensaje indicando que el usuario no existe.
    - Contraseña no coincide → mensaje de autenticación fallida.
    FIN
```

---

## FORMATO OBLIGATORIO

### A) Texto literal del PDF

- LOGIN: *"Este comando se utiliza para iniciar sesión en el sistema. No se puede iniciar otra sesión sin haber hecho un LOGOUT antes, en caso contrario debe mostrar un mensaje de error indicando que debe cerrar sesión con anterioridad."*
- -user: *"Especifica el nombre del usuario que iniciará sesión. Si no se encuentra mostrará un mensaje indicando que el usuario no existe. *distinguir mayúsculas de minúsculas."*
- -pass: *"Indicará la contraseña del usuario que inicia sesión. Si no coincide debe mostrar un mensaje de autenticación fallida. *distinguirá entre mayúsculas y minúsculas."*
- -id: *"Indicará el id de la partición montada de la cual van a iniciar sesión. De lograr iniciar sesión todas las acciones se realizarán sobre este id."*
- users.txt: *"guardado en el sistema ext2 de la raíz de cada partición"*; *"Un id 0 significa que el usuario o grupo está eliminado"*; estructura *"GID, Tipo, Grupo \n"* y *"UID, Tipo, Grupo, Usuario, Contraseña \n"*.

### B) Interpretación técnica

- LOGIN inicia una única sesión; si ya hay sesión, debe indicarse que se cierre con anterioridad.
- Parámetros obligatorios: -user, -pass, -id (id de partición montada).
- Usuario y contraseña se validan con distinción de mayúsculas y minúsculas; usuario no encontrado → “usuario no existe”; contraseña incorrecta → “autenticación fallida”.
- users.txt está en la raíz de la partición indicada por -id; se localiza por el sistema de archivos (raíz = inodo 0, archivo "users.txt"). Registros con id 0 se consideran eliminados y no válidos.
- Sesión en RAM debe permitir: identificar la partición (id), el usuario logueado (nombre, UID, GID) y si hay sesión activa, para cumplir con los comandos posteriores y permisos UGO.

### C) Validaciones obligatorias

1. No haya ya una sesión activa (si la hay → error “debe cerrar sesión con anterioridad”).  
2. -id corresponda a una partición montada (si no → error).  
3. Usuario exista en users.txt y no esté eliminado (UID != 0) (si no → “usuario no existe”).  
4. Contraseña coincida (si no → “autenticación fallida”).  
No se exige en el PDF validar explícitamente “grupo no existe” en LOGIN.

### D) Algoritmo detallado

Descrito en PARTE 5: comprobar no sesión activa → comprobar id montado → abrir disco y Superblock → localizar users.txt en raíz → leer y parsear users.txt → buscar usuario (Tipo U, UID≠0, Usuario=-user) → validar contraseña → crear sesión en RAM (id, usuario, UID, GID, sesión activa) → mensaje de éxito; en cada paso, aplicar los errores definidos en el PDF.

### E) Confirmación final

**El diseño de LOGIN cumple estrictamente con el PDF:** definición, parámetros, condiciones de error, ubicación y formato de users.txt, ignorar id=0, validaciones de usuario y contraseña, requisito de partición montada, una sola sesión y estructura mínima de sesión justificada por el enunciado.
