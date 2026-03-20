#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <string>

namespace extreamfs {
namespace manager {

/**
 * Estructura de sesión en RAM según diseño formal (PDF Parte 4).
 * Solo puede existir una sesión activa. LOGIN activa; LOGOUT desactiva.
 */
struct Session {
    bool active = false;
    std::string id_particion;
    std::string username;
    int uid = 0;
    int gid = 0;
};

/** Establece la sesión activa (tras LOGIN exitoso). */
void setSession(const std::string& idPartition, const std::string& username, int uid, int gid);

/** Desactiva y limpia la sesión (LOGOUT). */
void clearSession();

/** Devuelve la sesión actual (solo lectura). */
Session getSession();

/** true si hay sesión activa. */
bool isSessionActive();

/** true si el usuario actual es root (nombre "root"). */
bool isRoot();

/** ID de la partición montada de la sesión. */
std::string getSessionId();

/** Nombre del usuario logueado. */
std::string getSessionUsername();

/** UID del usuario logueado. */
int getSessionUid();

/** GID del usuario logueado. */
int getSessionGid();

} // namespace manager
} // namespace extreamfs

#endif // SESSION_MANAGER_H
