#ifndef MKDISK_H
#define MKDISK_H

#include <string>

namespace extreamfs {
namespace commands {

/**
 * Ejecuta el comando MKDISK.
 * Parámetros ya parseados por el analyzer.
 * @return Mensaje de éxito o "Error: descripción"
 */
std::string runMkdisk(int size, const std::string& path,
                     const std::string& unit, const std::string& fit);

} // namespace commands
} // namespace extreamfs

#endif // MKDISK_H
