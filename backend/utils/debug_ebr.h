#ifndef DEBUG_EBR_H
#define DEBUG_EBR_H

#include <string>

namespace extreamfs {
namespace utils {

/**
 * Diagnóstico de la cadena EBR dentro de la partición extendida.
 * Solo lectura. Valida que los EBR estén encadenados y dentro del rango.
 * @return Mensaje con lista de EBR y "Cadena EBR válida" o descripción de error
 */
std::string debugEBR(const std::string& path);

} // namespace utils
} // namespace extreamfs

#endif // DEBUG_EBR_H
