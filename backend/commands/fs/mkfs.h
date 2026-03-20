#ifndef MKFS_H
#define MKFS_H

#include <string>

namespace extreamfs {
namespace commands {

/**
 * Formatea la partición montada con el ID dado como ext2.
 * PDF: MKFS - formateo completo, users.txt en raíz, contenido inicial obligatorio.
 * @param id ID de la partición montada (ej. 891A)
 * @return Mensaje de éxito o error
 */
std::string runMkfs(const std::string& id);

} // namespace commands
} // namespace extreamfs

#endif // MKFS_H
