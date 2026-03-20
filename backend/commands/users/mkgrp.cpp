#include "commands/users/users_commands.h"
#include "manager/session_manager.h"
#include "manager/mount_manager.h"
#include "manager/fs_manager.h"
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

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

std::string runMkgrp(const std::string& name) {
    if (!manager::isSessionActive()) {
        return "Error: no existe una sesión activa";
    }
    if (!manager::isRoot()) {
        return "Error: solo el usuario root puede ejecutar este comando";
    }
    if (name.empty()) {
        return "Error: falta el parámetro -name";
    }
    if (name.size() > 10) {
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
    int maxGid = 0;
    for (const std::string& line : lines) {
        std::vector<std::string> p = splitComma(line);
        if (p.size() != 3) continue;
        if (p[1] != "G" && p[1] != "g") continue;
        int gid = parseId(p[0]);
        if (gid == 0) continue;
        if (p[2] == name) {
            return "Error: el grupo ya existe";
        }
        if (gid > maxGid) maxGid = gid;
    }
    int newGid = maxGid + 1;
    std::string newLine = std::to_string(newGid) + ",G," + name + "\n";
    if (!content.empty() && content.back() != '\n') {
        content += "\n";
    }
    content += newLine;
    if (!manager::writeUsersTxt(pathDisk, part_start, part_s, content, err)) {
        return err.empty() ? "Error: no se pudo escribir users.txt" : err;
    }
    return "Grupo creado correctamente.";
}

} // namespace commands
} // namespace extreamfs
