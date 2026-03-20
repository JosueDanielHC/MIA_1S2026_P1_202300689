#ifndef CAT_H
#define CAT_H

#include <string>
#include <vector>

namespace extreamfs {
namespace commands {

/**
 * Ejecuta el comando CAT.
 * Muestra el contenido de los archivos indicados por -file1, -file2, ... -fileN.
 * Requiere sesión activa; valida existencia y permiso de lectura.
 * @param filePaths Rutas absolutas dentro del FS (ej. /home/users.txt).
 * @return Contenido concatenado o "Error: descripción"
 */
std::string runCat(const std::vector<std::string>& filePaths);

} // namespace commands
} // namespace extreamfs

#endif // CAT_H
