# Diseño formal del comando 9. LOGOUT — Proyecto 1 MIA (solo PDF oficial)

Basado **exclusivamente** en el PDF. Sin comportamiento típico de Linux, sin validaciones ni efectos no definidos en el enunciado.

---

## PARTE 1 — Definición formal del comando

### 1) Texto literal del PDF

**Nombre del comando:**  
*"9. LOGOUT"*

**Descripción oficial:**  
*"Este comando se utiliza para cerrar sesión. Debe haber una sesión activa anteriormente para poder utilizarlo, si no, debe mostrar un mensaje de error. Este comando no recibe parámetros."*

**Parámetros:**  
*"Este comando no recibe parámetros."* → No tiene parámetros.

**Condiciones de error (literal):**  
*"Debe haber una sesión activa anteriormente para poder utilizarlo, si no, debe mostrar un mensaje de error."*

**Qué sucede si el LOGOUT es exitoso:**  
El PDF no describe explícitamente el efecto (mensaje de éxito, texto, etc.). Solo indica que el comando *"se utiliza para cerrar sesión"*. Tras un LOGOUT exitoso, deja de haber sesión activa, por lo que la restricción de LOGIN (*"No se puede iniciar otra sesión sin haber hecho un LOGOUT antes"*) queda satisfecha y los comandos posteriores que requieren sesión volverán a exigir iniciar sesión.

**Ejemplos (literal):**  
*"#Termina la sesión del usuario"* → `logout`  
*"#Si se vuelve a ejecutar deberá mostrar un error ya que no hay sesión actualmente"* → `Logout`

### 2) Confirmaciones textuales

- **¿LOGOUT requiere parámetros?**  
  **No.** *"Este comando no recibe parámetros."*

- **¿Qué debe pasar si no hay sesión activa?**  
  *"si no, debe mostrar un mensaje de error"*.

- **¿Debe mostrarse un mensaje específico?**  
  El PDF exige *"un mensaje de error"* cuando no hay sesión activa; no fija el texto exacto del mensaje ni el de éxito.

---

## PARTE 2 — Relación con LOGIN

### 1) Texto donde se menciona que no se puede iniciar otra sesión sin LOGOUT

En la definición de **LOGIN (8. LOGIN):**  
*"No se puede iniciar otra sesión sin haber hecho un LOGOUT antes, en caso contrario debe mostrar un mensaje de error indicando que debe cerrar sesión con anterioridad."*

Es decir: solo puede haber una sesión; para iniciar una nueva debe haberse ejecutado LOGOUT antes.

### 2) Cómo LOGOUT afecta esa restricción

LOGOUT *"se utiliza para cerrar sesión"*. Una vez ejecutado LOGOUT de forma válida (con sesión activa), ya no hay sesión activa; en ese estado, LOGIN puede iniciar una nueva sesión sin violar la regla anterior. Por tanto, **LOGOUT es el único comando que restablece el estado “sin sesión”**, permitiendo un nuevo LOGIN.

---

## PARTE 3 — Efecto en sesión en RAM

### 1) Qué debe cambiar al ejecutar LOGOUT

El PDF solo dice que el comando *"se utiliza para cerrar sesión"* y que *"Debe haber una sesión activa anteriormente para poder utilizarlo"*. No enumera operaciones concretas (borrar usuario, borrar id, etc.).  
**Interpretación mínima:** tras LOGOUT exitoso no debe haber sesión activa; es decir, el estado debe ser el mismo que antes de cualquier LOGIN (ningún usuario logueado, ninguna partición de sesión).

### 2) Qué indica el PDF sobre borrar datos de sesión

El PDF **no** dice literalmente:

- “Borrar usuario actual”
- “Borrar id de partición”
- “Reiniciar UID/GID”
- “Solo marcar sesión como inactiva”

Solo exige **cerrar sesión**. Para cumplir el enunciado basta con que, después de LOGOUT:

- No exista “sesión activa” (para que los demás comandos exijan de nuevo “iniciar sesión” y LOGIN pueda usarse de nuevo).
- No quede un “usuario que actualmente está logueado” ni “partición en la que inicio sesión” utilizables.

**Solución mínima justificada por el PDF:**

- **Marcar sesión como inactiva:** necesario para que se cumpla “si no [hay sesión activa], debe mostrar un mensaje de error” en futuras ejecuciones de comandos que requieren sesión, y para que LOGIN pueda abrir una nueva sesión.  
- **Dejar de considerar los datos de sesión como vigentes:** el PDF no exige borrar explícitamente cada campo, pero sí que “cerrar sesión” implique que ya no hay usuario logueado ni partición de sesión. La forma más simple y segura es: además de marcar sesión como inactiva, no usar (o limpiar) id_particion_montada, nombre_usuario, UID y GID para ningún comando hasta el próximo LOGIN. Implementativamente eso equivale a “limpiar” o “invalidar” la estructura de sesión (p. ej. indicador_sesion_activa = false y, opcionalmente, vaciar o no usar el resto de campos hasta el siguiente LOGIN).

**Resumen:** El PDF exige que tras LOGOUT no haya sesión activa. La implementación mínima es: **indicador_sesion_activa = false** (obligatorio) y tratar como inexistentes los datos de usuario/partición hasta el próximo LOGIN (limpiar o no usar el resto de campos, según diseño de la sesión).

---

## PARTE 4 — Validaciones obligatorias

- **¿Qué pasa si se ejecuta LOGOUT sin sesión activa?**  
  *"si no, debe mostrar un mensaje de error"*.

- **¿Debe mostrar error?**  
  **Sí.** *"Debe haber una sesión activa anteriormente para poder utilizarlo, si no, debe mostrar un mensaje de error."*

- **¿El PDF define texto específico del mensaje?**  
  **No.** Solo exige *"un mensaje de error"*; no se indica la frase exacta.

---

## PARTE 5 — Algoritmo formal de LOGOUT

Pseudocódigo alineado solo con el PDF:

```
COMANDO LOGOUT (sin parámetros)

1) VERIFICAR SI HAY SESIÓN ACTIVA
   SI no hay sesión activa ENTONCES
     devolver mensaje de error (el PDF no fija el texto)
     FIN
   FIN

2) CERRAR SESIÓN
   Marcar sesión como inactiva (indicador_sesion_activa = false).
   Opcional / según diseño: limpiar o invalidar el resto de campos de sesión
   (id_particion_montada, nombre_usuario, UID, GID) para que ningún comando
   considere que hay usuario logueado o partición de sesión hasta el próximo LOGIN.
   FIN

3) MOSTRAR MENSAJE DE ÉXITO
   Devolver mensaje indicando que se cerró sesión (el PDF no fija el texto).
   FIN

4) CONFIRMACIÓN: NO MODIFICAR ESTRUCTURAS DEL SISTEMA DE ARCHIVOS
   LOGOUT solo afecta el estado de sesión en RAM. No se escribe en disco,
   no se modifican MBR, particiones, Superblock, inodos, bloques ni users.txt.
   El PDF no menciona ningún efecto sobre el sistema de archivos.
   FIN
```

---

## FORMATO OBLIGATORIO

### A) Texto literal del PDF

- LOGOUT: *"Este comando se utiliza para cerrar sesión. Debe haber una sesión activa anteriormente para poder utilizarlo, si no, debe mostrar un mensaje de error. Este comando no recibe parámetros."*
- LOGIN (relación): *"No se puede iniciar otra sesión sin haber hecho un LOGOUT antes, en caso contrario debe mostrar un mensaje de error indicando que debe cerrar sesión con anterioridad."*

### B) Interpretación técnica correcta

- LOGOUT cierra la sesión actual; no tiene parámetros.
- Requiere sesión activa; si no la hay, debe mostrarse un mensaje de error.
- Tras LOGOUT exitoso no hay sesión activa, por lo que se puede ejecutar LOGIN de nuevo y los comandos que requieren sesión volverán a exigir iniciar sesión.
- El PDF no define texto concreto ni para el error ni para el éxito; no describe efectos sobre disco.

### C) Cambios en estructura de sesión

- **Obligatorio:** Marcar la sesión como inactiva (p. ej. `indicador_sesion_activa = false`).
- **Mínimo para cumplir el enunciado:** Que ningún comando considere que hay usuario logueado ni partición de sesión hasta el próximo LOGIN; en la práctica eso implica no usar (o limpiar) id_particion_montada, nombre_usuario, UID y GID hasta que se ejecute un LOGIN válido. El PDF no exige explícitamente “borrar” cada campo; sí exige que la sesión quede cerrada.

### D) Validaciones obligatorias

- Comprobar que exista sesión activa antes de cerrar sesión.
- Si no hay sesión activa → mostrar un mensaje de error (texto no especificado en el PDF).

### E) Confirmación final

**El diseño de LOGOUT cumple estrictamente con el PDF:** definición, ausencia de parámetros, exigencia de sesión activa, mensaje de error cuando no la hay, efecto de “cerrar sesión” (sesión inactiva y datos de sesión no vigentes hasta el próximo LOGIN), sin modificación de estructuras del sistema de archivos y sin añadir validaciones o efectos no definidos en el enunciado.
