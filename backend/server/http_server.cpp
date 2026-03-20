#include "http_server.h"
#include "httplib.h"
#include "analyzer/parser.h"
#include <fstream>
#include <sstream>
#include <string>
#include <regex>

namespace extreamfs {
namespace server {

/**
 * Parsea el campo "input" de un JSON simple.
 * Esperado: {"input": "comando aquí"}
 */
static std::string parseInputFromJson(const std::string& body) {
    std::regex re(R"re("input"\s*:\s*"((?:[^"\\]|\\.)*)")re");
    std::smatch match;
    if (std::regex_search(body, match, re) && match.size() > 1) {
        return match[1].str();
    }
    return "";
}

/**
 * Construye JSON de respuesta: {"output": "respuesta"}
 */
static std::string buildOutputJson(const std::string& output) {
    std::string escaped;
    escaped.reserve(output.size() + 16);
    escaped += "{\"output\":\"";
    for (char c : output) {
        if (c == '"') escaped += "\\\"";
        else if (c == '\\') escaped += "\\\\";
        else if (c == '\n') escaped += "\\n";
        else if (c == '\r') escaped += "\\r";
        else if (c == '\t') escaped += "\\t";
        else escaped += c;
    }
    escaped += "\"}";
    return escaped;
}

static bool readFile(const std::string& path, std::string& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    out = ss.str();
    return true;
}

static std::string contentType(const std::string& path) {
    if (path.find(".css") != std::string::npos) return "text/css; charset=utf-8";
    if (path.find(".js") != std::string::npos) return "application/javascript; charset=utf-8";
    return "text/html; charset=utf-8";
}

void runHttpServer(int port) {
    httplib::Server svr;
    // Buscar frontend en el directorio actual o en el padre (si se ejecuta desde build/)
    std::string frontendDir = "frontend";
    {
        std::string test;
        if (!readFile(frontendDir + "/index.html", test) && readFile("../frontend/index.html", test)) {
            frontendDir = "../frontend";
        }
    }

    // Servir la página principal (desde la raíz del proyecto)
    svr.Get("/", [&frontendDir](const httplib::Request&, httplib::Response& res) {
        std::string body;
        if (readFile(frontendDir + "/index.html", body)) {
            res.set_content(body, contentType("index.html"));
        } else {
            res.status = 404;
            res.set_content("No se encontró frontend/index.html. Ejecute el servidor desde la raíz del proyecto.", "text/plain; charset=utf-8");
        }
    });

    svr.Get("/index.html", [&frontendDir](const httplib::Request&, httplib::Response& res) {
        std::string body;
        if (readFile(frontendDir + "/index.html", body)) {
            res.set_content(body, contentType("index.html"));
        } else {
            res.status = 404;
        }
    });

    // Archivos estáticos (CSS, JS)
    svr.Get("/styles.css", [&frontendDir](const httplib::Request&, httplib::Response& res) {
        std::string body;
        if (readFile(frontendDir + "/styles.css", body)) {
            res.set_content(body, contentType("styles.css"));
        } else {
            res.status = 404;
        }
    });

    svr.Get("/app.js", [&frontendDir](const httplib::Request&, httplib::Response& res) {
        std::string body;
        if (readFile(frontendDir + "/app.js", body)) {
            res.set_content(body, contentType("app.js"));
        } else {
            res.status = 404;
        }
    });

    svr.Post("/execute", [](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Content-Type", "application/json; charset=utf-8");
        res.set_header("Access-Control-Allow-Origin", "*");

        std::string input = parseInputFromJson(req.body);
        for (size_t p = 0; (p = input.find("\\\"", p)) != std::string::npos; p += 1) {
            input.replace(p, 2, "\"");
        }
        std::string output = extreamfs::analyzer::executeCommand(input);
        if (output.empty()) {
            output = "Servidor funcionando correctamente";
        }
        res.set_content(buildOutputJson(output), "application/json");
    });

    svr.Options("/execute", [](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.status = 204;
    });

    svr.listen("0.0.0.0", static_cast<int>(port));
}

} // namespace server
} // namespace extreamfs
