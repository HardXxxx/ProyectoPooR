#include "RutaBarrio.h"

RutaBarrio::RutaBarrio() : Ruta() {}

RutaBarrio::RutaBarrio(int id, const std::string& nom, const std::string& orig,
                       const std::string& dest, bool est, const std::string& pSalida)
    : Ruta(id, nom, orig, dest, est, "RutaBarrio", pSalida) {}

// Retorna texto informativo propio de rutas de barrio
std::string RutaBarrio::getInfoAdicional() const {
    return "Tipo: Ruta de Barrio | Trayecto: " + getOrigen() + " -> " + getDestino();
}
