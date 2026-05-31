#include "Coordenada.h"
#include <cmath>

// Constante para conversion de grados a radianes
static const double PI = 3.14159265358979323846;
static const double RADIO_TIERRA = 6371000.0; // metros

Coordenada::Coordenada() : latitud(0.0), longitud(0.0) {}

Coordenada::Coordenada(double lat, double lon) : latitud(lat), longitud(lon) {}

double Coordenada::getLatitud()  const { return latitud; }
double Coordenada::getLongitud() const { return longitud; }

void Coordenada::setLatitud(double lat)  { latitud  = lat; }
void Coordenada::setLongitud(double lon) { longitud = lon; }

// Calcula distancia en metros entre dos coordenadas usando Haversine
double Coordenada::distanciaHacia(const Coordenada& otra) const {
    double dLat = (otra.latitud  - latitud)  * PI / 180.0;
    double dLon = (otra.longitud - longitud) * PI / 180.0;
    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(latitud * PI / 180.0) * std::cos(otra.latitud * PI / 180.0) *
               std::sin(dLon / 2) * std::sin(dLon / 2);
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return RADIO_TIERRA * c;
}
