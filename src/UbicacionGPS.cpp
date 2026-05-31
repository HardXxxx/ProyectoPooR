#include "UbicacionGPS.h"

UbicacionGPS::UbicacionGPS() : Coordenada(), altitud(0.0), velocidadKmh(30.0) {}

UbicacionGPS::UbicacionGPS(double lat, double lon, double alt, double vel)
    : Coordenada(lat, lon), altitud(alt), velocidadKmh(vel) {}

double UbicacionGPS::getAltitud()      const { return altitud; }
double UbicacionGPS::getVelocidadKmh() const { return velocidadKmh; }

void UbicacionGPS::setAltitud(double alt)      { altitud      = alt; }
void UbicacionGPS::setVelocidadKmh(double vel) { velocidadKmh = vel; }

// Convierte distancia en metros a tiempo en minutos segun velocidad actual
double UbicacionGPS::tiempoEstimadoMinutos(double distanciaMetros) const {
    if (velocidadKmh <= 0.0) return 0.0;
    double distanciaKm = distanciaMetros / 1000.0;
    return (distanciaKm / velocidadKmh) * 60.0;
}
