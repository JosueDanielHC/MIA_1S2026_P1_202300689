#ifndef MKFILE_H
#define MKFILE_H

#include <string>

namespace extreamfs {
namespace commands {

/**
 * Crea un archivo en la ruta indicada.
 * PDF: mkfile -path=ruta [-p] [-size=N] [-cont=ruta_externa].
 * -cont: contenido desde archivo externo del host. Si además -size: trunca o rellena con 0123456789...
 * Requiere sesión activa. Solo 12 bloques directos (sin indirectos).
 */
std::string runMkfile(const std::string& path, bool createParents, int sizeBytes,
                     const std::string& contPath);

} // namespace commands
} // namespace extreamfs

#endif // MKFILE_H
