#include "server/http_server.h"
#include <cstdlib>
#include <iostream>

int main() {
    const int port = 8080;
    std::cout << "ExtreamFS - Servidor en http://0.0.0.0:" << port << std::endl;
    std::cout << "Abrir en el navegador: http://localhost:" << port << std::endl;
    std::cout << "Endpoint API: POST /execute" << std::endl;
    extreamfs::server::runHttpServer(port);
    return EXIT_SUCCESS;
}
