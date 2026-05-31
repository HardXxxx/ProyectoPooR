#pragma once
#include "Coordenada.h"

// Extiende Coordenada con altitud y velocidad para simular posicion GPS en tiempo real
class UbicacionGPS : public Coordenada {
private:
    double altitud;
    double velocidadKmh; // velocidad actual del bus en km/h

public:
    UbicacionGPS();
    UbicacionGPS(double lat, double lon, double alt, double vel = 30.0);

    // Getters
    double getAltitud() const;
    double getVelocidadKmh() const;

    // Setters
    void setAltitud(double alt);
    void setVelocidadKmh(double vel);

    // Estima el tiempo en minutos para recorrer distancia dada la velocidad actual
    double tiempoEstimadoMinutos(double distanciaMetros) const;
};
