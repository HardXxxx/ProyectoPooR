#include "RutaCentro.h"

RutaCentro::RutaCentro() : Ruta() {}

RutaCentro::RutaCentro(int id, const std::string& nom, const std::string& orig,
                       const std::string& dest, bool est, const std::string& pSalida,
                       const std::string& zona)
    : Ruta(id, nom, orig, dest, est, "RutaCentro", pSalida), zonaCentro(zona) {}

std::string RutaCentro::getZonaCentro() const          { return zonaCentro;  }
void        RutaCentro::setZonaCentro(const std::string& z) { zonaCentro = z; }

// Retorna texto informativo incluyendo la zona del centro que cubre la ruta
std::string RutaCentro::getInfoAdicional() const {
    return "Tipo: Ruta Centro | Zona: " + zonaCentro + " | " + getOrigen() + " -> " + getDestino();
}
