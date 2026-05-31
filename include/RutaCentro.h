#pragma once
#include "Ruta.h"

// Ruta que opera desde la zona centro de la ciudad hacia la universidad
// Hereda de Ruta agregando la zona de cobertura del centro
class RutaCentro : public Ruta {
private:
    std::string zonaCentro; // zona del centro cubierta por esta ruta

public:
    RutaCentro();
    RutaCentro(int id, const std::string& nom, const std::string& orig,
               const std::string& dest, bool est, const std::string& pSalida,
               const std::string& zona);

    // Getters
    std::string getZonaCentro() const;

    // Setters
    void setZonaCentro(const std::string& zona);

    // Devuelve texto descriptivo propio de rutas de centro
    std::string getInfoAdicional() const override;
};
