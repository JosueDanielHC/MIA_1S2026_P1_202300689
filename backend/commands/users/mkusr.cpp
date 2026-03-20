#include "commands/users/users_commands.h"
#include "manager/session_manager.h"
#include "manager/mount_manager.h"
#include "manager/fs_manager.h"
#include <sstream>
#include <string>
#include <vector>

namespace extreamfs {
namespace commands {

namespace {

static std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end == std::string::npos ? std::string::npos : end - start + 1);
}

static std::vector<std::string> splitLines(const std::string& content) {
    std::vector<std::string> lines;
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line, '\n')) {
        std::string t = trim(line);
        if (!t.empty()) lines.push_back(line);
    }
    return lines;
}

static std::vector<std::string> splitComma(const std::string& line) {
    std::vector<std::string> parts;
    std::istringstream iss(line);
    std::string part;
    while (std::getline(iss, part, ',')) {
        parts.push_back(trim(part));
    }
    return parts;
}

static int parseId(const std::string& s) {
    try {
        return std::stoi(s);
    } catch (...) {
        return -1;
    }
}

} // namespace

std::string runMkusr(const std::string& user, const std::string& pass, const std::string& grp) {
    if (!manager::isSessionActive()) {
        return "Error: no existe una sesión activa";
    }
    if (!manager::isRoot()) {
        return "Error: solo el usuario root puede ejecutar este comando";
    }
    if (user.empty()) {
        return "Error: falta el parámetro -user";
    }
    if (pass.empty()) {
        return "Error: falta el parámetro -pass";
    }
    if (grp.empty()) {
        return "Error: falta el parámetro -grp";
    }
    if (user.size() > 10) {
        return "Error: el nombre del usuario no puede superar 10 caracteres";
    }
    if (pass.size() > 10) {
        return "Error: la contraseña no puede superar 10 caracteres";
    }
    if (grp.size() > 10) {
        return "Error: el nombre del grupo no puede superar 10 caracteres";
    }
    std::string pathDisk, partName;
    if (!manager::getMountById(manager::getSessionId(), pathDisk, partName)) {
        return "Error: la partición de la sesión no está montada";
    }
    int part_start = 0, part_s = 0;
    if (!manager::getPartitionBounds(pathDisk, partName, part_start, part_s)) {
        return "Error: no se pudo obtener la partición";
    }
    std::string content, err;
    if (!manager::readUsersTxt(pathDisk, part_start, part_s, content, err)) {
        return err.empty() ? "Error: no se pudo leer users.txt" : err;
    }
    std::vector<std::string> lines = splitLines(content);
    bool groupExists = false;
    int maxUid = 0;
    for (const std::string& line : lines) {
        std::vector<std::string> p = splitComma(line);
        if (p.size() == 3 && (p[1] == "G" || p[1] == "g")) {
            int gid = parseId(p[0]);
            if (gid != 0 && p[2] == grp) {
                groupExists = true;
            }
        } else if (p.size() == 5 && (p[1] == "U" || p[1] == "u")) {
            int uid = parseId(p[0]);
            if (uid != 0) {
                if (p[3] == user) {
                    return "Error: ya existe el usuario";
                }
                if (uid > maxUid) maxUid = uid;
            }
        }
    }
    if (!groupExists) {
        return "Error: el grupo no existe en la partición";
    }
    int newUid = maxUid + 1;
    std::string newLine = std::to_string(newUid) + ",U," + grp + "," + user + "," + pass + "\n";
    if (!content.empty() && content.back() != '\n') {
        content += "\n";
    }
    content += newLine;
    if (!manager::writeUsersTxt(pathDisk, part_start, part_s, content, err)) {
        return err.empty() ? "Error: no se pudo escribir users.txt" : err;
    }
    return "Usuario creado correctamente.";
}

} // namespace commands
} // namespace extreamfs
