# Auditoría y diseño formal del módulo de sesión — Proyecto 1 MIA (solo PDF oficial)

Fuente única: PDF oficial. Sin comportamiento Linux, sin reglas no escritas, sin validaciones no exigidas. Todo justificado con texto del PDF. Se separa **TEXTO LITERAL** e **INTERPRETACIÓN TÉCNICA**.

---

# BLOQUE 1 — Diseño formal de la estructura global de sesión (en RAM)

## 1) Información mínima en RAM para cumplir el PDF

La sesión debe permitir:
- Saber si hay sesión activa (para LOGOUT y para comandos que la exigen).
- Saber “quién” está logueado (para CAT, MKFILE, MKDIR y permisos UGO).
- Saber “sobre qué partición” se actúa (para todos los comandos que operan en la partición de la sesión).
- Saber si el usuario actual es root (para MKGRP, RMGRP, MKUSR, RMUSR, CHGRP).

## 2) Texto literal del PDF

**Qué guarda / implica LOGIN:**

- *"Indicará el id de la partición montada de la cual van a iniciar sesión. **De lograr iniciar sesión todas las acciones se realizarán sobre este id.**"*

**“Usuario que actualmente está logueado”:**

- CAT: *"si el **usuario que actualmente está logueado** tiene acceso al permiso de lectura"*.
- MKFILE: *"El propietario será **el usuario que actualmente ha iniciado sesión**."*
- MKDIR: *"El propietario será **el usuario que actualmente ha iniciado sesión**."*

**“Partición en la que inicio sesión”:**

- Observación tras LOGOUT: *"Los siguientes comandos que se verán a continuación necesitan que exista una sesión en el sistema ya que **se ejecutan sobre la partición en la que inicio sesión**. Si no, debe mostrar un mensaje de error indicando que necesita iniciar sesión."*
- MKGRP: *"en el archivo users.txt **de la partición**"*; ejemplo *"en la **partición de la sesión actual**"*.
- RMGRP: *"en la **partición de la sesión actual**"*.

**Excepción de sesión (qué comandos no la requieren):**

- *"a excepción de los comandos **MKFS y LOGIN** todos los demás comandos requieren que exista una **sesión activa** para su ejecución en caso de no ser así se debería de indicar mediante un error que **no existe una sesion activa**."*

**LOGOUT:**

- *"Este comando se utiliza para **cerrar sesión**. Debe haber una **sesión activa** anteriormente para poder utilizarlo, si no, debe mostrar un mensaje de error."*

## 3) Interpretación técnica

- **“Todas las acciones se realizarán sobre este id”** → La sesión debe almacenar el **id de la partición montada** (o la información necesaria para resolver disco y partición).
- **“Usuario que actualmente está logueado / ha iniciado sesión”** → La sesión debe identificar al **usuario** (al menos nombre y, para permisos UGO, UID y GID).
- **“Partición en la que inicio sesión” / “partición de la sesión actual”** → Misma idea: la sesión está ligada a un **id de partición** sobre el que se ejecutan los comandos.
- **“Sesión activa”** → Debe existir un **indicador** de si hay o no sesión activa (para validar antes de comandos que la requieren y para LOGOUT).
- **Comandos “solo usuario root”** (MKGRP, RMGRP, MKUSR, RMUSR, CHGRP) → Hay que poder saber si el usuario actual es **root**; el PDF usa el nombre *"usuario root"* y el ejemplo *"login -user=root"* → identificación por **nombre de usuario "root"** (o por UID si el enunciado lo fija; en users.txt inicial "1, U, root, root, 123" el root tiene UID 1 y nombre "root").

## 4) Estructura mínima propuesta de sesión (en RAM)

```
sesión {
    activa              (bool)   — Hay sesión activa sí/no.
    id_particion_montada (string) — Id de la partición sobre la que se inició sesión.
    nombre_usuario       (string) — Nombre del usuario logueado.
    UID                  (int)    — Id de usuario (para permisos UGO, categoría U).
    GID                  (int)    — Id de grupo (para permisos UGO, categoría G).
}
```

## 5) Justificación de cada campo con texto literal del PDF

| Campo | Justificación (texto PDF) | Obligatorio / mínimo |
|-------|---------------------------|------------------------|
| **activa** | *"requieren que exista una sesión activa"*; *"Debe haber una sesión activa anteriormente para poder utilizarlo"* (LOGOUT); *"si no, debe mostrar un mensaje de error indicando que no existe una sesion activa"*. | **Obligatorio.** Sin él no se puede validar “hay sesión” antes de comandos que la exigen ni el requisito de LOGOUT. |
| **id_particion_montada** | *"De lograr iniciar sesión todas las acciones se realizarán sobre **este id**"*; *"se ejecutan sobre **la partición en la que inicio sesión**"*; *"partición de la **sesión actual**"* (MKGRP, RMGRP). | **Obligatorio.** El PDF exige que los comandos actúen sobre “este id” / “la partición en la que inicio sesión”. |
| **nombre_usuario** | *"el **usuario que actualmente está logueado**"* (CAT); *"el usuario que actualmente **ha iniciado sesión**"* (MKFILE, MKDIR); *"**usuario root**"* para MKGRP/RMGRP/etc. | **Obligatorio.** Identifica al usuario logueado y permite comprobar si es “usuario root”. |
| **UID** | Inodos tienen *"i_uid"*; permisos UGO dependen de si el usuario es propietario (U), del grupo (G) u otro (O). *"Si es el propietario"* → comparar con i_uid. | **Obligatorio** para evaluar permisos UGO (categoría User) según el PDF. |
| **GID** | *"pertenece al mismo grupo en que está el propietario"*; inodos tienen *"i_gid"*. Para categoría Grupo hay que comparar con i_gid. | **Obligatorio** para permisos UGO (categoría Grupo). |

**Resumen:** Los cinco campos son **obligatorios** para cumplir con lo que el PDF exige (sesión activa, “este id”, usuario actual, root, UGO). No se añaden campos sin referencia en el enunciado.

---

# BLOQUE 2 — Auditoría de comandos que requieren sesión activa

## 1) Regla general del PDF

**Texto literal:**  
*"a excepción de los comandos MKFS y LOGIN todos los demás comandos requieren que exista una sesión activa para su ejecución en caso de no ser así se debería de indicar mediante un error que no existe una sesion activa."*

**Observación tras LOGOUT:**  
*"Los siguientes comandos que se verán a continuación necesitan que exista una sesión en el sistema ya que se ejecutan sobre la partición en la que inicio sesión. Si no, debe mostrar un mensaje de error indicando que necesita iniciar sesión."*

**Interpretación:** Cualquier comando que **no** sea MKFS ni LOGIN **requiere sesión activa**. Si no la hay, debe mostrarse **un mensaje de error** (no existe sesión activa / necesita iniciar sesión).

## 2) Listado de comandos que exigen sesión (y cita literal)

| # | Comando | Texto literal que exige sesión | Por qué depende de sesión |
|---|---------|-------------------------------|----------------------------|
| 1 | **CAT** | Párrafo general: *"todos los demás comandos requieren que exista una sesión activa"*. CAT: *"el **usuario que actualmente está logueado** tiene acceso al permiso de lectura"*. | No es MKFS ni LOGIN; además necesita “usuario logueado” para permisos. |
| 2 | **MKGRP** | Párrafo general; observación: *"necesitan que exista una sesión"*; MKGRP: *"en el archivo users.txt de la partición"* / *"partición de la **sesión actual**"*; *"solo lo puede utilizar el **usuario root**"*. | Opera sobre la partición de la sesión y sobre users.txt; solo root. |
| 3 | **RMGRP** | Párrafo general; *"partición de la sesión actual"*; *"Solo lo puede utilizar el usuario root"*. | Igual que MKGRP. |
| 4 | **MKUSR** | Párrafo general; *"Solo lo puede ejecutar el usuario root"*; *"en la partición en la que se está creando el usuario"*. | Requiere sesión (partición) y usuario root. |
| 5 | **RMUSR** | Párrafo general; *"Solo lo puede ejecutar el usuario root"*. | Requiere sesión y usuario root. |
| 6 | **CHGRP** | Párrafo general; *"Únicamente lo podrá utilizar el **usuario root**"*. | Requiere sesión y usuario root. |
| 7 | **MKFILE** | Párrafo general; *"El propietario será el usuario que actualmente **ha iniciado sesión**"*. | Requiere usuario de la sesión y partición de la sesión. |
| 8 | **MKDIR** | Párrafo general; *"El propietario será el usuario que actualmente **ha iniciado sesión**"*. | Igual que MKFILE. |
| 9 | **REP** | Párrafo general (no es excepción); usa *"-id"* de partición; los reportes pueden ser sobre la partición de la sesión. | No es MKFS ni LOGIN → requiere sesión según la regla general. |

(Se incluyen todos los comandos del enunciado que no son MKFS ni LOGIN y que aparecen en el apartado “Administración del Sistema de Archivos” o posteriores.)

## 3) Qué validar primero (antes de lógica interna)

Para **cualquier comando que requiera sesión activa** (todos salvo MKFS y LOGIN):

1. **Comprobar** que exista sesión activa (p. ej. `sesión.activa == true`).
2. **Si no existe** → mostrar mensaje de error (que no existe una sesión activa / que necesita iniciar sesión) y **no** ejecutar la lógica interna del comando.
3. **Si existe** → continuar con la lógica del comando (y, si aplica, validar después “solo root”, permisos, etc.).

## 4) Regla formal general

**Regla:**  
*Si el comando X requiere sesión activa según el PDF y `sesión.activa == false`, entonces el sistema debe mostrar un mensaje de error (indicando que no existe una sesión activa o que necesita iniciar sesión) y no ejecutar la lógica propia del comando X.*

**Confirmación:** Esta validación **no contradice** el PDF; lo **aplica** literalmente: *"en caso de no ser así se debería de indicar mediante un error que no existe una sesion activa"* y *"Si no, debe mostrar un mensaje de error indicando que necesita iniciar sesión"*.

---

# BLOQUE 3 — Preparación para MKGRP / RMGRP y users.txt

## 1) Definiciones en el PDF (texto literal)

**users.txt:**

- *"Este archivo lógico almacenado en el disco será llamado **users.txt** guardado en el sistema ext2 de la **raíz de cada partición**."*
- *"Existirán dos tipos de registros, unos para grupos y otros para usuarios."*
- Estructura: *"GID, Tipo, Grupo \n"* y *"UID, Tipo, Grupo, Usuario, Contraseña \n"*.
- *"Un id 0 significa que el usuario o grupo está eliminado."*

**MKGRP:**

- *"Este comando **creará un grupo** para los usuarios de la partición y se guardará en el archivo **users.txt de la partición**, este comando **solo lo puede utilizar el usuario root**. Si otro usuario lo intenta ejecutar, deberá mostrar un mensaje de error."*
- Parámetro: *"-name Obligatorio"* (nombre del grupo).
- Ejemplo: *"#Crea el grupo usuarios en la **partición de la sesión actual**"* → `mkgrp -name=usuarios`.

**RMGRP:**

- *"Este comando **eliminará un grupo** para los usuarios de la partición. **Solo lo puede utilizar el usuario root**, si lo utiliza alguien más debe mostrar un error."*
- Parámetro: *"-name Obligatorio"* (nombre del grupo a eliminar).
- Ejemplo: *"#Elimina el grupo de usuarios en la **partición de la sesión actual**"*.

**Permisos / restricciones:**

- MKGRP y RMGRP: **solo usuario root** puede ejecutarlos; si otro usuario lo intenta, **mostrar mensaje de error**.

## 2) Respuestas según el PDF

- **¿Solo root puede ejecutar MKGRP?** **Sí.** *"este comando solo lo puede utilizar el usuario root. Si otro usuario lo intenta ejecutar, deberá mostrar un mensaje de error."*
- **¿Solo root puede ejecutar RMGRP?** **Sí.** *"Solo lo puede utilizar el usuario root, si lo utiliza alguien más debe mostrar un error."*
- **¿Cómo se identifica root según el PDF?** El PDF habla de *"usuario root"* y en users.txt inicial aparece *"1, U, root, root, 123"* (usuario de nombre **"root"**). No se define explícitamente “root = UID 1”; sí se usa reiteradamente el **nombre** "root". Interpretación mínima: **usuario actual es root si `nombre_usuario == "root"`** (o, si el enunciado lo fijara, por UID; con lo escrito, el nombre es suficiente y está respaldado por los ejemplos).

## 3) Cómo la estructura de sesión permite validar MKGRP/RMGRP

- **Sesión activa:** Se comprueba `sesión.activa == true` antes de ejecutar MKGRP/RMGRP (regla del BLOQUE 2).
- **Solo root:** Se comprueba que el usuario actual sea root usando `sesión.nombre_usuario` (p. ej. `nombre_usuario == "root"`). Si no es root → mensaje de error según el PDF.
- **Partición donde modificar users.txt:** Es la partición de la sesión, identificada por `sesión.id_particion_montada` (disco y partición se resuelven a partir de ese id, como en LOGIN).

Con la estructura propuesta (activa, id_particion_montada, nombre_usuario, UID, GID) se pueden aplicar ambas validaciones y localizar users.txt en la partición correcta.

## 4) Campos de sesión indispensables

- **Para validar que el usuario sea root:** **nombre_usuario** (comparar con "root"). Opcionalmente UID si se define root por UID en el enunciado.
- **Para saber en qué partición modificar users.txt:** **id_particion_montada** (y, en implementación, la resolución a path de disco + partición que ya se usa en LOGIN).

## 5) Confirmaciones (solo lo que dice el PDF)

- **¿LOGOUT afecta users.txt?** **No.** El PDF solo dice que LOGOUT “se utiliza para cerrar sesión”; no menciona escritura en disco ni users.txt. LOGOUT no modifica users.txt.
- **¿LOGIN solo lee users.txt?** **Sí.** LOGIN valida usuario y contraseña leyendo users.txt; no se indica que LOGIN escriba o modifique users.txt.
- **¿MKGRP/RMGRP modifican users.txt?** **Sí.** *"se guardará en el archivo users.txt de la partición"* (MKGRP); los ejemplos muestran cómo “debería quedar” users.txt tras mkgrp/rmgrp, por tanto ambos **modifican** users.txt.

---

# FORMATO OBLIGATORIO DE RESPUESTA

## A) Texto literal del PDF (citado claramente)

- Sesión activa y excepciones: *"a excepción de los comandos MKFS y LOGIN todos los demás comandos requieren que exista una sesión activa para su ejecución en caso de no ser así se debería de indicar mediante un error que no existe una sesion activa."*
- Partición de la sesión: *"De lograr iniciar sesión todas las acciones se realizarán sobre este id."*; *"se ejecutan sobre la partición en la que inicio sesión"*; *"partición de la sesión actual"*.
- Usuario logueado: *"usuario que actualmente está logueado"* (CAT); *"usuario que actualmente ha iniciado sesión"* (MKFILE, MKDIR).
- LOGOUT: *"Este comando se utiliza para cerrar sesión. Debe haber una sesión activa anteriormente para poder utilizarlo, si no, debe mostrar un mensaje de error. Este comando no recibe parámetros."*
- MKGRP: *"solo lo puede utilizar el usuario root. Si otro usuario lo intenta ejecutar, deberá mostrar un mensaje de error"*; *"en el archivo users.txt de la partición"*; *"partición de la sesión actual"*.
- RMGRP: *"Solo lo puede utilizar el usuario root, si lo utiliza alguien más debe mostrar un error."*
- users.txt: *"users.txt guardado en el sistema ext2 de la raíz de cada partición"*; estructura GID,Tipo,Grupo y UID,Tipo,Grupo,Usuario,Contraseña.

## B) Interpretación técnica estricta

- Una sola sesión a la vez; solo MKFS y LOGIN no requieren sesión activa; el resto debe comprobar sesión activa y, si no la hay, mostrar error.
- La sesión debe almacenar: si está activa, id de la partición sobre la que se actúa, usuario actual (nombre y, para UGO, UID y GID).
- Root se identifica por el nombre de usuario "root" según ejemplos y redacción del PDF.
- LOGOUT solo cierra sesión en RAM; no toca users.txt. LOGIN solo lee users.txt. MKGRP y RMGRP modifican users.txt en la partición de la sesión y solo pueden ser ejecutados por root.

## C) Diseño estructural propuesto

```
sesión {
    activa               bool    — Requerido por "sesión activa" en el PDF.
    id_particion_montada string  — Requerido por "todas las acciones sobre este id" / "partición en la que inicio sesión".
    nombre_usuario       string  — Requerido por "usuario que actualmente está logueado" y para identificar root.
    UID                  int     — Requerido para permisos UGO (categoría User, i_uid).
    GID                  int     — Requerido para permisos UGO (categoría Grupo, i_gid).
}
```

Todos los campos están justificados por citas del PDF (ver BLOQUE 1).

## D) Validaciones formales

1. **Comandos que requieren sesión (todos excepto MKFS y LOGIN):**  
   Si `sesión.activa == false` → mostrar mensaje de error (no existe sesión activa / necesita iniciar sesión) y no ejecutar el comando.

2. **MKGRP y RMGRP (y MKUSR, RMUSR, CHGRP cuando aplique):**  
   Además de sesión activa: si `sesión.nombre_usuario != "root"` (o el criterio que fije el enunciado para root) → mostrar mensaje de error (solo usuario root puede utilizarlo).

3. **LOGOUT:**  
   Si `sesión.activa == false` → mostrar mensaje de error (debe haber sesión activa).

Ninguna de estas validaciones añade requisitos no expresados en el PDF.

## E) Confirmación final

**El módulo de sesión propuesto cumple estrictamente con el PDF oficial sin añadir comportamiento no definido:** la estructura (activa, id_particion_montada, nombre_usuario, UID, GID) está justificada con texto literal; la regla “sesión activa obligatoria para todos los comandos excepto MKFS y LOGIN” y el mensaje de error cuando no hay sesión son los del enunciado; la restricción “solo root” para MKGRP/RMGRP (y los demás que lo indiquen) y la identificación de root por nombre están basadas en el PDF; LOGOUT no afecta users.txt, LOGIN solo lee users.txt y MKGRP/RMGRP modifican users.txt en la partición de la sesión, tal como describe el enunciado.
