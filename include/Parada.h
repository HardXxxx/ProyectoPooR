#pragma once
#include <string>
#include "Coordenada.h"

// Representa una parada fisica dentro de una ruta
class Parada {
private:
    int idParada;
    std::string nombre;
    Coordenada ubicacion;
    double altitud;
    bool estado;      // true = activa
    int ordenEnRuta;  // posición dentro de la ruta

public:
    Parada();
    Parada(int id, const std::string& nom, double lat, double lon,
           double alt, bool est, int orden);

    // Getters
    int getIdParada() const;
    std::string getNombre() const;
    Coordenada getUbicacion() const;
    double getAltitud() const;
    bool getEstado() const;
    int getOrdenEnRuta() const;

    // Setters
    void setIdParada(int id);
    void setNombre(const std::string& nom);
    void setUbicacion(double lat, double lon);
    void setAltitud(double alt);
    void setEstado(bool est);
    void setOrdenEnRuta(int orden);

    // Calcula distancia en metros hacia otro punto geografico
    double distanciaA(double lat, double lon) const;

    // Operador de comparacion por orden en ruta
    bool operator<(const Parada& otra) const;
};
