#ifndef MOUNT_MANAGER_H
#define MOUNT_MANAGER_H

#include <string>
#include <vector>

namespace extreamfs {
namespace manager {

struct MountEntry {
    std::string path;
    std::string name;
    std::string id;
};

/**
 * Añade una partición montada (path + name) y asigna ID vdXN.
 * @return ID asignado o string vacío si ya estaba montada
 */
std::string addMounted(const std::string& path, const std::string& name);

/** Devuelve true si (path, name) ya está en la tabla. */
bool isMounted(const std::string& path, const std::string& name);

/** Devuelve true si el disco (path del .mia) tiene alguna partición montada. */
bool isDiskPathMounted(const std::string& diskPath);

/** Lista todas las particiones montadas: "vdX1 -> Name\nvdX2 -> Name\n..." */
std::string listMounted();

/**
 * Obtiene path y name de la partición montada con el ID dado.
 * @return true si el ID existe en la tabla de montados
 */
bool getMountById(const std::string& id, std::string& outPath, std::string& outName);

/** Obtiene el número correlativo (1-based) para este path según la tabla actual. */
int getPartitionNumberForPath(const std::string& path);

/** Obtiene el índice de disco (0-based) para este path. */
int getDiskLetterIndex(const std::string& path);

} // namespace manager
} // namespace extreamfs

#endif // MOUNT_MANAGER_H
