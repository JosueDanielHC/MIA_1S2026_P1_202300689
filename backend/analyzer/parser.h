#ifndef PARSER_H
#define PARSER_H

#include <string>

namespace extreamfs {
namespace analyzer {

/**
 * Ejecuta el comando contenido en input (línea de comando completa).
 * Reconoce mkdisk y delega en el comando correspondiente.
 * @return Mensaje de salida (éxito o error)
 */
std::string executeCommand(const std::string& input);

} // namespace analyzer
} // namespace extreamfs

#endif // PARSER_H
