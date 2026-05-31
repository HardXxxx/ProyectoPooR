#pragma once
// Representa un punto geografico con latitud y longitud
class Coordenada {
private:
    double latitud;
    double longitud;
public:
    Coordenada();
    Coordenada(double lat, double lon);

    // Getters
    double getLatitud() const;
    double getLongitud() const;

    // Setters
    void setLatitud(double lat);
    void setLongitud(double lon);

    // Calcula la distancia en metros hacia otra coordenada (formula Haversine)
    double distanciaHacia(const Coordenada& otra) const;
};
