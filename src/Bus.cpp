#include "Bus.h"
#include <cstdlib>

Bus::Bus() : idBus(0), capacidadMaxima(0), capacidadActual(0), estado(false), ubicacion(nullptr), indiceParadaActual(-1), idRutaAsignada(0) {}

Bus::Bus(int id, const std::string& plc, int capMax, int capAct, bool est)
    : idBus(id), placa(plc), capacidadMaxima(capMax), capacidadActual(capAct),
      estado(est), ubicacion(nullptr), indiceParadaActual(-1), idRutaAsignada(id) {}

// Libera la memoria del puntero de ubicacion GPS al destruir el bus
Bus::~Bus() {
    delete ubicacion;
}

int Bus::getIdBus() const { return idBus; }
std::string Bus::getPlaca() const { return placa; }
int Bus::getCapacidadMaxima() const { return capacidadMaxima; }
int Bus::getCapacidadActual() const { return capacidadActual; }
bool Bus::getEstado() const { return estado; }
UbicacionGPS* Bus::getUbicacion() const { return ubicacion; }
int Bus::getIndiceParadaActual() const { return indiceParadaActual; }
int Bus::getIdRutaAsignada() const { return idRutaAsignada; }

void Bus::setIdBus(int id) { idBus = id; }
void Bus::setPlaca(const std::string& p) { placa = p; }
void Bus::setCapacidadMaxima(int cap) { capacidadMaxima = cap; }
void Bus::setCapacidadActual(int cap) { capacidadActual = cap; }
void Bus::setEstado(bool est) { estado = est; }
void Bus::setIndiceParadaActual(int idx) { indiceParadaActual = idx; }
void Bus::setIdRutaAsignada(int idRuta) { idRutaAsignada = idRuta; }


// Inicializa o actualiza el puntero de ubicacion GPS con nuevas coordenadas
void Bus::setUbicacion(double lat, double lon, double alt, double vel) {
    delete ubicacion;
    ubicacion = new UbicacionGPS(lat, lon, alt, vel);
}

// Mueve la posicion GPS del bus un paso hacia las coordenadas destino
void Bus::simularMovimiento(double latDest, double lonDest) {
    if (!ubicacion) return;
    double deltaLat = (latDest - ubicacion->getLatitud())  * 0.05;
    double deltaLon = (lonDest - ubicacion->getLongitud()) * 0.05;
    ubicacion->setLatitud(ubicacion->getLatitud()   + deltaLat);
    ubicacion->setLongitud(ubicacion->getLongitud() + deltaLon);
}

// Calcula el tiempo en minutos desde la posicion actual hasta la parada indicada
double Bus::tiempoHastaParada(double latParada, double lonParada) const {
    if (!ubicacion) return -1.0;
    Coordenada destino(latParada, lonParada);
    double dist = ubicacion->distanciaHacia(destino);
    return ubicacion->tiempoEstimadoMinutos(dist);
}

// Compara buses por capacidad disponible (mayor disponibilidad es "menor")
bool Bus::operator<(const Bus& otro) const {
    return (capacidadMaxima - capacidadActual) > (otro.capacidadMaxima - otro.capacidadActual);
}
