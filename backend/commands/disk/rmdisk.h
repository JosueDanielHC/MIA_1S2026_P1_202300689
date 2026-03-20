#ifndef RMDISK_H
#define RMDISK_H

#include <string>

namespace extreamfs {
namespace commands {

/**
 * Ejecuta el comando RMDISK.
 * Elimina el archivo de disco (.mia) indicado por -path.
 * @return Mensaje de éxito o "Error: descripción"
 */
std::string runRmdisk(const std::string& path);

} // namespace commands
} // namespace extreamfs

#endif // RMDISK_H
