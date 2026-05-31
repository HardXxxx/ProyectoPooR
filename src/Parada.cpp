#include "Parada.h"

Parada::Parada() : idParada(0), altitud(0.0), estado(false), ordenEnRuta(0) {}

Parada::Parada(int id, const std::string& nom, double lat, double lon,
               double alt, bool est, int orden)
    : idParada(id), nombre(nom), ubicacion(lat, lon),
      altitud(alt), estado(est), ordenEnRuta(orden) {}

int Parada::getIdParada() const { return idParada; }
std::string Parada::getNombre() const { return nombre; }
Coordenada Parada::getUbicacion() const { return ubicacion; }
double Parada::getAltitud() const { return altitud; }
bool Parada::getEstado() const { return estado; }
int Parada::getOrdenEnRuta() const { return ordenEnRuta; }

void Parada::setIdParada(int id) { idParada = id; }
void Parada::setNombre(const std::string& nom) { nombre = nom; }
void Parada::setUbicacion(double lat, double lon) { ubicacion = Coordenada(lat, lon); }
void Parada::setAltitud(double alt) { altitud = alt; }
void Parada::setEstado(bool est) { estado = est; }
void Parada::setOrdenEnRuta(int orden) { ordenEnRuta = orden; }


// Calcula la distancia en metros desde esta parada al punto indicado
double Parada::distanciaA(double lat, double lon) const {
    Coordenada otro(lat, lon);
    return ubicacion.distanciaHacia(otro);
}

// Compara paradas por su orden dentro de la ruta
bool Parada::operator<(const Parada& otra) const {
    return ordenEnRuta < otra.ordenEnRuta;
}
