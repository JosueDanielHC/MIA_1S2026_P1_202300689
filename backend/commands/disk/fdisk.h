#ifndef FDISK_H
#define FDISK_H

#include <string>

namespace extreamfs {
namespace commands {

/**
 * Ejecuta el comando FDISK - Fase 2A (solo particiones primarias).
 * Parámetros ya parseados por el analyzer.
 * @return Mensaje de éxito o "Error: descripción"
 */
std::string runFdisk(int size, const std::string& path, const std::string& name,
                    const std::string& unit, const std::string& fit,
                    const std::string& type);

} // namespace commands
} // namespace extreamfs

#endif // FDISK_H
