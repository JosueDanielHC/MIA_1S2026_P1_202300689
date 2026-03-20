#ifndef MKDIR_H
#define MKDIR_H

#include <string>

namespace extreamfs {
namespace commands {

/**
 * Crea una carpeta en la ruta indicada dentro de la partición de la sesión.
 * PDF: mkdir -path=ruta [-p]. Sin -p falla si no existe carpeta intermedia.
 * Requiere sesión activa.
 */
std::string runMkdir(const std::string& path, bool createParents);

} // namespace commands
} // namespace extreamfs

#endif // MKDIR_H
