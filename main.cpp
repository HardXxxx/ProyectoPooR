#include <iostream>
#include <stdexcept>
#include "SistemaTransporte.h"

// Punto de entrada del sistema de gestion de transporte universitario
// Toda la logica esta delegada en la clase SistemaTransporte
int main() {
    try {
        // El directorio "data" debe estar junto al ejecutable
        SistemaTransporte sistema("data");
        sistema.iniciar();
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR CRITICO] " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
