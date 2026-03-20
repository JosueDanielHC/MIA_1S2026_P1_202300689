#include "commands/users/users_commands.h"
#include "manager/session_manager.h"
#include <string>

namespace extreamfs {
namespace commands {

std::string runLogout() {
    if (!manager::isSessionActive()) {
        return "Error: no existe una sesión activa";
    }
    manager::clearSession();
    return "Sesión cerrada correctamente.";
}

} // namespace commands
} // namespace extreamfs
