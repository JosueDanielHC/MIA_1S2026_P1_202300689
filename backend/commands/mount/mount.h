#ifndef MOUNT_CMD_H
#define MOUNT_CMD_H

#include <string>

namespace extreamfs {
namespace commands {

/**
 * Ejecuta mount -path=... -name=...
 * @return "Partición montada con ID: vdXN" o "Error: ..."
 */
std::string runMount(const std::string& path, const std::string& name);

} // namespace commands
} // namespace extreamfs

#endif // MOUNT_CMD_H
