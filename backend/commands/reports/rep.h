#ifndef REP_H
#define REP_H

#include <string>

namespace extreamfs {
namespace commands {

/**
 * Genera reporte según -name (mbr, disk, etc.) en formato Graphviz.
 * -name: tipo de reporte (mbr, disk, inode, file, ...)
 * -path: ruta del archivo de salida (png, jpg o pdf)
 * -id: id de partición montada (para obtener disco)
 * -ruta: ruta del archivo dentro del FS (solo para name=file)
 */
std::string runRep(const std::string& name, const std::string& path, const std::string& id,
                  const std::string& ruta = "");

} // namespace commands
} // namespace extreamfs

#endif // REP_H
