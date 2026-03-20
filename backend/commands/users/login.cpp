#include "commands/users/users_commands.h"
#include "manager/session_manager.h"
#include "manager/mount_manager.h"
#include "manager/fs_manager.h"
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

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
        if (!t.empty()) lines.push_back(t);
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

std::string runLogin(const std::string& user, const std::string& pass, const std::string& id) {
    if (manager::isSessionActive()) {
        return "Error: debe cerrar sesión con anterioridad";
    }
    if (id.empty()) {
        return "Error: falta el parámetro -id";
    }
    if (user.empty()) {
        return "Error: falta el parámetro -user";
    }
    if (pass.empty()) {
        return "Error: falta el parámetro -pass";
    }
    std::string path, name;
    if (!manager::getMountById(id, path, name)) {
        return "Error: el id no existe o la partición no está montada";
    }
    int part_start = 0, part_s = 0;
    if (!manager::getPartitionBounds(path, name, part_start, part_s)) {
        return "Error: no se pudo obtener la partición";
    }
    std::string content, err;
    if (!manager::readUsersTxt(path, part_start, part_s, content, err)) {
        return err.empty() ? "Error: no se pudo leer users.txt" : err;
    }
    std::vector<std::string> lines = splitLines(content);
    int foundUid = -1;
    std::string foundGroup;
    for (const std::string& line : lines) {
        std::vector<std::string> p = splitComma(line);
        if (p.size() != 5) continue;
        if (p[1] != "U" && p[1] != "u") continue;
        int uid = parseId(p[0]);
        if (uid == 0) continue;
        if (p[3] != user) continue;
        if (p[4] != pass) {
            return "Error: autenticación fallida";
        }
        foundUid = uid;
        foundGroup = p[2];
        break;
    }
    if (foundUid < 0) {
        return "Error: el usuario no existe";
    }
    int gid = 0;
    for (const std::string& line : lines) {
        std::vector<std::string> p = splitComma(line);
        if (p.size() != 3) continue;
        if (p[1] != "G" && p[1] != "g") continue;
        int g = parseId(p[0]);
        if (g == 0) continue;
        if (p[2] == foundGroup) {
            gid = g;
            break;
        }
    }
    manager::setSession(id, user, foundUid, gid);
    return "Sesión iniciada correctamente.";
}

} // namespace commands
} // namespace extreamfs
