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

std::string runChgrp(const std::string& user, const std::string& grp) {
    if (!manager::isSessionActive()) {
        return "Error: no existe una sesión activa";
    }
    if (!manager::isRoot()) {
        return "Error: solo el usuario root puede ejecutar este comando";
    }
    if (user.empty()) {
        return "Error: falta el parámetro -user";
    }
    if (grp.empty()) {
        return "Error: falta el parámetro -grp";
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
    std::istringstream iss(content);
    std::string line;
    std::string newContent;
    bool userFound = false;
    bool grpExists = false;
    std::istringstream iss2(content);
    while (std::getline(iss2, line, '\n')) {
        if (line.empty()) continue;
        std::vector<std::string> p = splitComma(line);
        if (p.size() == 3 && (p[1] == "G" || p[1] == "g")) {
            int gid = parseId(p[0]);
            if (gid != 0 && p[2] == grp) grpExists = true;
        }
    }
    iss.clear();
    iss.str(content);
    while (std::getline(iss, line, '\n')) {
        if (line.empty()) continue;
        std::vector<std::string> p = splitComma(line);
        if (p.size() == 5 && (p[1] == "U" || p[1] == "u")) {
            int uid = parseId(p[0]);
            if (uid != 0 && p[3] == user) {
                userFound = true;
                newContent += p[0] + ",U," + grp + "," + p[3] + "," + p[4] + "\n";
                continue;
            }
        }
        newContent += line + "\n";
    }
    if (!userFound) {
        return "Error: el usuario no existe";
    }
    if (!grpExists) {
        return "Error: el grupo no existe o está eliminado";
    }
    if (!manager::writeUsersTxt(pathDisk, part_start, part_s, newContent, err)) {
        return err.empty() ? "Error: no se pudo escribir users.txt" : err;
    }
    return "Grupo del usuario cambiado correctamente.";
}

} // namespace commands
} // namespace extreamfs
