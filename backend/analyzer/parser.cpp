#include "parser.h"
#include "commands/disk/mkdisk.h"
#include "commands/disk/rmdisk.h"
#include "commands/disk/fdisk.h"
#include "commands/fs/mkfs.h"
#include "commands/fs/mkdir.h"
#include "commands/fs/mkfile.h"
#include "commands/fs/cat.h"
#include "commands/reports/rep.h"
#include "commands/users/users_commands.h"
#include "utils/debug_mbr.h"
#include "utils/debug_ebr.h"
#include "commands/mount/mount.h"
#include "manager/mount_manager.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace extreamfs {
namespace analyzer {

static std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end == std::string::npos ? std::string::npos : end - start + 1);
}

static std::string toLower(const std::string& s) {
    std::string r = s;
    for (char& c : r) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return r;
}

static std::vector<std::string> tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    const size_t n = input.size();
    size_t i = 0;
    while (i < n) {
        while (i < n && (input[i] == ' ' || input[i] == '\t')) ++i;
        if (i >= n) break;
        if (input[i] == '"') {
            ++i;
            size_t start = i;
            while (i < n && input[i] != '"') ++i;
            tokens.push_back(input.substr(start, i - start));
            if (i < n) ++i;
        } else {
            size_t start = i;
            while (i < n && input[i] != ' ' && input[i] != '\t') ++i;
            tokens.push_back(input.substr(start, i - start));
        }
    }
    return tokens;
}

static void parseParam(const std::string& token,
                       std::string& size, std::string& path,
                       std::string& unit, std::string& fit,
                       std::string* name = nullptr, std::string* type = nullptr) {
    size_t eq = token.find('=');
    if (eq == std::string::npos) return;
    std::string key = toLower(trim(token.substr(0, eq)));
    std::string val = trim(token.substr(eq + 1));
    if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
        val = val.substr(1, val.size() - 2);
    }
    if (key == "-size") size = val;
    else if (key == "-path") path = val;
    else if (key == "-unit") unit = val;
    else if (key == "-fit") fit = val;
    else if (name && key == "-name") *name = val;
    else if (type && key == "-type") *type = val;
}

static std::string parseMkdisk(const std::vector<std::string>& tokens) {
    std::string sizeStr, pathStr, unitStr = "M", fitStr = "FF";
    for (size_t i = 1; i < tokens.size(); ++i) {
        parseParam(tokens[i], sizeStr, pathStr, unitStr, fitStr);
    }
    if (sizeStr.empty() || pathStr.empty()) {
        return "Error: faltan parámetros obligatorios (size y path)";
    }
    if (unitStr.empty()) unitStr = "M";
    if (fitStr.empty()) fitStr = "FF";
    int sizeVal = 0;
    try {
        sizeVal = std::stoi(sizeStr);
    } catch (...) {
        return "Error: size debe ser un número entero";
    }
    return commands::runMkdisk(sizeVal, pathStr, unitStr, fitStr);
}

static std::string parseRmdisk(const std::vector<std::string>& tokens) {
    std::string pathStr;
    for (size_t i = 1; i < tokens.size(); ++i) {
        size_t eq = tokens[i].find('=');
        if (eq == std::string::npos) continue;
        std::string key = toLower(trim(tokens[i].substr(0, eq)));
        std::string val = trim(tokens[i].substr(eq + 1));
        if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
            val = val.substr(1, val.size() - 2);
        }
        if (key == "-path") { pathStr = val; break; }
    }
    return commands::runRmdisk(pathStr);
}

static std::string parseFdisk(const std::vector<std::string>& tokens) {
    std::string sizeStr, pathStr, nameStr, unitStr = "K", fitStr = "FF", typeStr = "P";
    for (size_t i = 1; i < tokens.size(); ++i) {
        parseParam(tokens[i], sizeStr, pathStr, unitStr, fitStr, &nameStr, &typeStr);
    }
    if (sizeStr.empty() || pathStr.empty() || nameStr.empty()) {
        return "Error: faltan parámetros obligatorios (size, path y name)";
    }
    if (unitStr.empty()) unitStr = "K";
    if (fitStr.empty()) fitStr = "FF";
    if (typeStr.empty()) typeStr = "P";
    int sizeVal = 0;
    try {
        sizeVal = std::stoi(sizeStr);
    } catch (...) {
        return "Error: size debe ser un número entero";
    }
    return commands::runFdisk(sizeVal, pathStr, nameStr, unitStr, fitStr, typeStr);
}

static std::string parseDebugmbr(const std::vector<std::string>& tokens) {
    std::string pathStr;
    std::string dummy1, dummy2, dummy3;
    for (size_t i = 1; i < tokens.size(); ++i) {
        parseParam(tokens[i], dummy1, pathStr, dummy2, dummy3);
    }
    if (pathStr.empty()) {
        return "Error: falta parámetro -path";
    }
    return utils::debugMBR(pathStr);
}

static std::string parseDebugebr(const std::vector<std::string>& tokens) {
    std::string pathStr;
    std::string dummy1, dummy2, dummy3;
    for (size_t i = 1; i < tokens.size(); ++i) {
        parseParam(tokens[i], dummy1, pathStr, dummy2, dummy3);
    }
    if (pathStr.empty()) {
        return "Error: falta parámetro -path";
    }
    return utils::debugEBR(pathStr);
}

static std::string parseMkfs(const std::vector<std::string>& tokens) {
    std::string idStr;
    for (size_t i = 1; i < tokens.size(); ++i) {
        size_t eq = tokens[i].find('=');
        if (eq == std::string::npos) continue;
        std::string key = toLower(trim(tokens[i].substr(0, eq)));
        std::string val = trim(tokens[i].substr(eq + 1));
        if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
            val = val.substr(1, val.size() - 2);
        }
        if (key == "-id") idStr = val;
    }
    if (idStr.empty()) {
        return "Error: mkfs requiere el parámetro -id";
    }
    return commands::runMkfs(idStr);
}

static std::string parseMount(const std::vector<std::string>& tokens) {
    if (tokens.size() == 1) {
        return manager::listMounted();
    }
    std::string pathStr, nameStr;
    for (size_t i = 1; i < tokens.size(); ++i) {
        size_t eq = tokens[i].find('=');
        if (eq == std::string::npos) continue;
        std::string key = toLower(trim(tokens[i].substr(0, eq)));
        std::string val = trim(tokens[i].substr(eq + 1));
        if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
            val = val.substr(1, val.size() - 2);
        }
        if (key == "-path") pathStr = val;
        else if (key == "-name") nameStr = val;
    }
    if (pathStr.empty() || nameStr.empty()) {
        return "Error: mount requiere -path y -name";
    }
    return commands::runMount(pathStr, nameStr);
}

static std::string getParam(const std::vector<std::string>& tokens, const std::string& key) {
    std::string k = toLower(key);
    for (size_t i = 1; i < tokens.size(); ++i) {
        size_t eq = tokens[i].find('=');
        if (eq == std::string::npos) continue;
        std::string tokKey = toLower(trim(tokens[i].substr(0, eq)));
        std::string val = trim(tokens[i].substr(eq + 1));
        if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
            val = val.substr(1, val.size() - 2);
        }
        if (tokKey == k) return val;
    }
    return "";
}

static std::string parseLogin(const std::vector<std::string>& tokens) {
    std::string user = getParam(tokens, "-user");
    std::string pass = getParam(tokens, "-pass");
    std::string id = getParam(tokens, "-id");
    return commands::runLogin(user, pass, id);
}

static std::string parseLogout(const std::vector<std::string>& tokens) {
    (void)tokens;
    return commands::runLogout();
}

static std::string parseMkgrp(const std::vector<std::string>& tokens) {
    std::string name = getParam(tokens, "-name");
    return commands::runMkgrp(name);
}

static std::string parseRmgrp(const std::vector<std::string>& tokens) {
    std::string name = getParam(tokens, "-name");
    return commands::runRmgrp(name);
}

static std::string parseMkusr(const std::vector<std::string>& tokens) {
    std::string user = getParam(tokens, "-user");
    std::string pass = getParam(tokens, "-pass");
    std::string grp = getParam(tokens, "-grp");
    return commands::runMkusr(user, pass, grp);
}

static std::string parseRmusr(const std::vector<std::string>& tokens) {
    std::string user = getParam(tokens, "-user");
    return commands::runRmusr(user);
}

static std::string parseChgrp(const std::vector<std::string>& tokens) {
    std::string user = getParam(tokens, "-user");
    std::string grp = getParam(tokens, "-grp");
    return commands::runChgrp(user, grp);
}

static std::string parseMkdir(const std::vector<std::string>& tokens) {
    std::string pathStr = getParam(tokens, "-path");
    bool hasP = false;
    for (size_t i = 1; i < tokens.size(); ++i) {
        if (toLower(trim(tokens[i])) == "-p") {
            hasP = true;
            break;
        }
    }
    return commands::runMkdir(pathStr, hasP);
}

static std::string parseMkfile(const std::vector<std::string>& tokens) {
    std::string pathStr = getParam(tokens, "-path");
    bool hasP = false;
    for (size_t i = 1; i < tokens.size(); ++i) {
        if (toLower(trim(tokens[i])) == "-p") {
            hasP = true;
            break;
        }
    }
    std::string sizeStr = getParam(tokens, "-size");
    int sizeVal = -1;
    if (!sizeStr.empty()) {
        try {
            sizeVal = std::stoi(sizeStr);
            if (sizeVal < 0) {
                return "Error: -size debe ser mayor o igual a 0";
            }
        } catch (...) {
            return "Error: -size debe ser un número entero";
        }
    }
    std::string contStr = getParam(tokens, "-cont");
    return commands::runMkfile(pathStr, hasP, sizeVal, contStr);
}

static std::string parseCat(const std::vector<std::string>& tokens) {
    std::vector<std::string> paths;
    for (int n = 1; n <= 20; ++n) {
        std::string key = "-file" + std::to_string(n);
        std::string val = getParam(tokens, key);
        if (val.size() >= 2 && val.front() == '"' && val.back() == '"') {
            val = val.substr(1, val.size() - 2);
        }
        if (!val.empty()) paths.push_back(val);
    }
    return commands::runCat(paths);
}

static std::string parseRep(const std::vector<std::string>& tokens) {
    std::string name = getParam(tokens, "-name");
    std::string path = getParam(tokens, "-path");
    std::string id = getParam(tokens, "-id");
    std::string ruta = getParam(tokens, "-ruta");
    return commands::runRep(name, path, id, ruta);
}

std::string executeCommand(const std::string& input) {
    std::string line = trim(input);
    if (line.empty()) {
        return "";
    }
    std::vector<std::string> tokens = tokenize(line);
    if (tokens.empty()) return "";

    std::string cmd = toLower(tokens[0]);
    if (cmd == "mkdisk") {
        return parseMkdisk(tokens);
    }
    if (cmd == "rmdisk") {
        return parseRmdisk(tokens);
    }
    if (cmd == "fdisk") {
        return parseFdisk(tokens);
    }
    if (cmd == "debugmbr") {
        return parseDebugmbr(tokens);
    }
    if (cmd == "debugebr") {
        return parseDebugebr(tokens);
    }
    if (cmd == "mount") {
        return parseMount(tokens);
    }
    if (cmd == "mkfs") {
        return parseMkfs(tokens);
    }
    if (cmd == "login") {
        return parseLogin(tokens);
    }
    if (cmd == "logout") {
        return parseLogout(tokens);
    }
    if (cmd == "mkgrp") {
        return parseMkgrp(tokens);
    }
    if (cmd == "rmgrp") {
        return parseRmgrp(tokens);
    }
    if (cmd == "mkusr") {
        return parseMkusr(tokens);
    }
    if (cmd == "rmusr") {
        return parseRmusr(tokens);
    }
    if (cmd == "chgrp") {
        return parseChgrp(tokens);
    }
    if (cmd == "mkdir") {
        return parseMkdir(tokens);
    }
    if (cmd == "mkfile") {
        return parseMkfile(tokens);
    }
    if (cmd == "cat") {
        return parseCat(tokens);
    }
    if (cmd == "rep") {
        return parseRep(tokens);
    }

    return "Error: comando no reconocido";
}

} // namespace analyzer
} // namespace extreamfs
