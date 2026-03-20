#ifndef DEBUG_MBR_H
#define DEBUG_MBR_H

#include <string>

namespace extreamfs {
namespace utils {

/**
 * Diagnóstico del MBR: lee el disco, lista particiones activas
 * y verifica que no haya solapamientos.
 * Solo lectura. No modifica el archivo.
 * @return Mensaje con particiones y "MBR válido. No hay solapamientos."
 *         o "ERROR: Particiones solapadas detectadas"
 */
std::string debugMBR(const std::string& path);

} // namespace utils
} // namespace extreamfs

#endif // DEBUG_MBR_H
