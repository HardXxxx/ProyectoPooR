#pragma once
#include "Ruta.h"

// Ruta que opera entre un barrio de la ciudad y la universidad
class RutaBarrio : public Ruta {
public:
    RutaBarrio();
    RutaBarrio(int id, const std::string& nom, const std::string& orig,
               const std::string& dest, bool est, const std::string& pSalida);

    // Devuelve texto descriptivo propio de rutas de barrio
    std::string getInfoAdicional() const override;
};
