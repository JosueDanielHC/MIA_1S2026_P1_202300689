# Diseño del comando 7. CAT — Proyecto 1 MIA (solo PDF oficial)

Basado **exclusivamente** en el PDF. Sin flujo clásico MIA, sin asumir Linux, sin estructuras no especificadas.

---

## PARTE 1 — Requisitos formales según el PDF

### 1) Texto literal

**Definición oficial de CAT:**

*"7. CAT"*  
*"Este comando permitirá mostrar el contenido del archivo, si el usuario que actualmente está logueado tiene acceso al permiso de lectura."*

**Parámetros:**

| PARÁMETRO | CATEGORÍA  | DESCRIPCIÓN (literal) |
|-----------|------------|-------------------------|
| -filen    | Obligatorio | *"Permitirá admitir como argumentos una lista de N ficheros que hay que enlazar. Estos se deben encadenar en el mismo orden en el cual fueron especificados. Si no existe el archivo o no tiene permiso de lectura, debe mostrarse un mensaje de error."* |

**Formato de parámetros múltiples (ejemplos del PDF):**

*"cat -file1=/home/user/docs/a.txt"*  
*"cat -file1=\"/home/a.txt\" -file2=\"/home/b.txt\" -file3=\"/home/c.txt\""*

Por tanto: **-file1**, **-file2**, … **-fileN** (lista de N ficheros).

**Condiciones de error (literal):**

*"Si no existe el archivo o no tiene permiso de lectura, debe mostrarse un mensaje de error."*

**Restricción de sesión activa (literal, párrafo del apartado):**

*"Los comandos de este apartado simularán el formateo de las particiones, administración de usuarios, carpetas y archivos a excepción de los comandos MKFS y LOGIN todos los demás comandos requieren que exista una sesión activa para su ejecución en caso de no ser así se debería de indicar mediante un error que no existe una sesion activa."*

Tras LOGOUT:

*"Los siguientes comandos que se verán a continuación necesitan que exista una sesión en el sistema ya que se ejecutan sobre la partición en la que inicio sesión. Si no, debe mostrar un mensaje de error indicando que necesita iniciar sesión."*

### 2) Confirmaciones

- **Validación de permisos:** **Sí.** El PDF exige *"si el usuario que actualmente está logueado tiene acceso al permiso de lectura"* y *"Si no existe el archivo o no tiene permiso de lectura, debe mostrarse un mensaje de error"*.
- **Validación de existencia del archivo:** **Sí.** *"Si no existe el archivo … debe mostrarse un mensaje de error."*
- **Mensaje específico o solo "error":** El PDF no define el texto exacto del mensaje; solo exige *"un mensaje de error"* (sin sesión: *"que no existe una sesion activa"* / *"que necesita iniciar sesión"*). Para “no existe el archivo” y “no tiene permiso de lectura” basta un mensaje de error genérico o descriptivo, sin frase literal impuesta.

---

## PARTE 2 — Dependencia de sesión

### 1) Confirmación textual de que CAT requiere sesión

CAT **no** está entre las excepciones. Texto literal:

*"a excepción de los comandos MKFS y LOGIN todos los demás comandos requieren que exista una sesión activa"*

CAT es uno de “todos los demás”, por tanto **requiere sesión activa**. Además, la propia descripción de CAT exige *"el usuario que actualmente está logueado"*, lo que implica sesión iniciada.

### 2) LOGIN previo y simulación de sesión

- **¿Es obligatorio que LOGIN esté implementado para que CAT sea funcional?**  
  **Sí**, según el PDF. La sesión activa se establece con el comando LOGIN (*"Este comando se utiliza para iniciar sesión en el sistema"*). Si no hay LOGIN, no hay “usuario que actualmente está logueado” ni “sesión activa”, y el enunciado exige indicar error cuando no exista sesión. Por tanto, para que CAT sea funcional en los términos del PDF, LOGIN debe estar implementado (o existir algún mecanismo que el enunciado acepte para “sesión activa”; el PDF solo describe LOGIN como medio para iniciar sesión).

- **¿Puede simularse sesión para pruebas?**  
  El PDF no habla de “simular” sesión. Define que los comandos distintos de MKFS y LOGIN requieren sesión activa y que LOGIN inicia sesión. Para cumplir estrictamente el enunciado, la sesión debe existir por el flujo definido (LOGIN); simular sesión para pruebas es una decisión de implementación/pruebas, no un requisito ni una prohibición expresados en el PDF.

---

## PARTE 3 — Diseño mínimo necesario

### 1) Estructuras que CAT debe leer

Para “mostrar el contenido del archivo” en un sistema ext2 según las estructuras del proyecto:

- **Superblock:** para obtener `s_inode_start`, `s_block_start`, `s_inode_s`, `s_block_s` y poder localizar la raíz (inodo 0) y los bloques. **Sí.**
- **Inodo:** para cada componente del path (carpetas) y para el archivo final; en el inodo del archivo están `i_block[]`, `i_s`, `i_perm`, `i_uid`, `i_gid`, `i_type`. **Sí.**
- **Bloques:** **FolderBlock** para recorrer rutas (resolver path); **FileBlock** para leer el contenido del archivo. **Sí.**
- **Bitmaps:** el PDF no exige que CAT consulte bitmaps. CAT solo debe “mostrar el contenido” y comprobar existencia y permiso de lectura. La existencia se deduce de la resolución del path (inodos y bloques); los permisos están en el inodo. **Solo lectura** si se usan para alguna comprobación adicional; **no son obligatorios** para el mínimo definido en el PDF.

Resumen: **mínimo obligatorio:** Superblock, Inode, FolderBlock, FileBlock (y PointerBlock si el archivo usa bloques indirectos). Bitmaps: opcionales (solo lectura si se usan).

### 2) Confirmaciones

- **¿CAT modifica estructuras?**  
  **No.** El PDF solo dice que CAT “permitirá **mostrar** el contenido del archivo”. No indica escritura ni actualización de metadatos.

- **¿CAT necesita actualizar tiempos (p. ej. i_atime)?**  
  El PDF no dice que CAT actualice fechas de acceso. **No** se exige en el enunciado.

- **¿CAT necesita validar permisos tipo 777?**  
  **Sí**, en el sentido de aplicar las reglas UGO del PDF. El PDF indica que el **usuario root** *"siempre tendrá los permisos 777 sobre cualquier archivo o carpeta"* y *"No se le negará ninguna operación por permisos"* (sección USUARIO ROOT). Para cualquier otro usuario, se deben comprobar los permisos del archivo (UGO) según la categoría del usuario (propietario, grupo, otro). Por tanto: para root no se niega lectura por permisos; para los demás sí debe validarse el permiso de lectura según `i_perm` y la categoría U/G/O.

---

## PARTE 4 — Algoritmo formal de ejecución

Pseudocódigo alineado solo con lo que exige el PDF:

```
COMANDO CAT (parámetros: -file1=ruta1, -file2=ruta2, … -fileN=rutaN)

1) VERIFICAR SESIÓN ACTIVA
   SI no hay sesión activa ENTONCES
     devolver mensaje de error indicando que no existe una sesión activa / que necesita iniciar sesión
     FIN
   Obtener de la sesión: usuario logueado (UID, GID, tal vez nombre), id de partición (o path+nombre de partición montada).

2) OBTENER PARTICION DE SESIÓN
   A partir del id de sesión, resolver disco (path) y partición (part_start, part_s).
   Abrir archivo del disco en lectura (binario).
   Leer Superblock desde part_start (posición según MKFS).

3) PARA CADA fichero en la lista (-file1, -file2, … -fileN) EN ORDEN:

   4) BUSCAR ARCHIVO POR PATH
      Resolver la ruta (ej. /home/user/docs/a.txt o /users.txt):
      - Inicio en inodo raíz (inodo 0: en s_inode_start + 0 * s_inode_s).
      - Para cada componente del path (excepto raíz):
        - Si el inodo actual no es tipo carpeta → error "no existe" o "no tiene permiso" según corresponda.
        - Buscar en los FolderBlock del inodo una entrada cuyo b_name coincida con el componente.
        - Si no hay entrada → archivo/carpeta no existe → ir a 9) con mensaje de error.
        - Siguiente inodo = b_inodo de esa entrada; leer ese inodo.
      - Al terminar el path, el inodo actual es el del fichero solicitado.

   5) VALIDAR QUE SEA ARCHIVO
      SI i_type != 1 (archivo) ENTONCES
        mostrar mensaje de error (p. ej. no es un archivo o no existe)
        continuar con el siguiente fichero o FIN según diseño
      FIN

   6) VALIDAR PERMISO DE LECTURA
      Determinar categoría del usuario logueado respecto al archivo:
        User (U) si UID == i_uid
        Grupo (G) si UID != i_uid y GID == i_gid
        Otro (O) si no es U ni G
      Si el usuario es root (según PDF: "siempre tendrá los permisos 777"): permitir lectura.
      Si no es root: según categoría, leer el dígito correspondiente en i_perm (U=0, G=1, O=2); si el dígito incluye lectura (4 o 5 o 6 o 7) permitir; si no, denegar.
      SI no tiene permiso de lectura ENTONCES
        ir a 9) con mensaje de error (no tiene permiso de lectura)
      FIN

   7) LEER BLOQUES DEL ARCHIVO
      Desde el inodo del archivo, obtener i_block[0..11] (directos); si hay indirectos (i_block[12], [13], [14]) resolver según el diseño de bloques (PointerBlock, etc.).
      Para cada bloque b_i (número lógico 0, 1, 2, …):
        Calcular posición en disco: s_block_start + b_i * s_block_s.
        Leer FileBlock (64 bytes); concatenar b_content (o hasta i_s si el archivo es menor que el total de bloques).

   8) CONCATENAR Y MOSTRAR
      Concatenar el contenido de todos los bloques del archivo en orden, hasta un total de i_s bytes.
      En la salida del comando, mostrar ese contenido (para este fichero).
      Si hay más de un fichero en la lista, los ejemplos del PDF indican que cada archivo va separado por salto de línea: mostrar contenido del file1, luego salto de línea, contenido del file2, etc.

9) MANEJO DE ERRORES (definidos en el PDF)
   - No hay sesión activa → mensaje de error indicando que no existe una sesión activa o que necesita iniciar sesión.
   - No existe el archivo (path no encontrado o componente no es archivo cuando debe serlo) → mensaje de error.
   - No tiene permiso de lectura → mensaje de error.
   El PDF no especifica el texto exacto; solo exige "un mensaje de error" en cada caso.
```

---

## FORMATO OBLIGATORIO

### A) Texto literal del PDF

- CAT: *"Este comando permitirá mostrar el contenido del archivo, si el usuario que actualmente está logueado tiene acceso al permiso de lectura."*
- Parámetro: *"-filen Obligatorio Permitirá admitir como argumentos una lista de N ficheros que hay que enlazar. Estos se deben encadenar en el mismo orden en el cual fueron especificados. Si no existe el archivo o no tiene permiso de lectura, debe mostrarse un mensaje de error."*
- Sesión: *"a excepción de los comandos MKFS y LOGIN todos los demás comandos requieren que exista una sesión activa para su ejecución en caso de no ser así se debería de indicar mediante un error que no existe una sesion activa."*
- Root: *"este siempre tendrá los permisos 777 sobre cualquier archivo o carpeta … No se le negará ninguna operación por permisos."*

### B) Interpretación técnica

- CAT muestra el contenido de uno o más archivos, en orden (-file1, -file2, … -fileN), separados por salto de línea.
- Requiere sesión activa y usuario logueado; debe validar existencia del archivo y permiso de lectura (UGO; root siempre permitido).
- Solo lectura; no modifica estructuras ni actualiza tiempos a menos que el PDF lo indique (no lo hace).
- Estructuras mínimas: Superblock, Inode, FolderBlock, FileBlock (y PointerBlock si hay indirectos); bitmaps no obligatorios.

### C) Algoritmo detallado

Descrito en la PARTE 4 (verificación de sesión → obtención de partición → por cada -fileN: resolución de path desde raíz → validar que sea archivo → validar permiso UGO/root → leer bloques → concatenar hasta i_s → mostrar; errores según PDF).

### D) Dependencias formales

- **Sesión activa:** obligatoria (texto del apartado).
- **LOGIN:** necesario para tener sesión y “usuario que actualmente está logueado”.
- **Partición montada y formateada:** la sesión está asociada a un id de partición montada; el path se resuelve en el sistema de archivos de esa partición (MKFS).

### E) Confirmación

**El diseño de CAT cumple estrictamente con el PDF:** requisitos literales (definición, parámetros -file1…-fileN, errores, sesión), estructuras mínimas (Superblock, Inode, bloques), no modificación de estructuras, validación de existencia y permiso de lectura (UGO y excepción root 777), y algoritmo anterior basado solo en lo que el enunciado exige.
