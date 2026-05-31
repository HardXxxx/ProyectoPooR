#pragma once
#include <string>
#include "UbicacionGPS.h"

// Representa un bus de la flota con su ubicacion GPS en tiempo real
class Bus {
private:
    int idBus;
    std::string  placa;
    int capacidadMaxima;
    int capacidadActual;
    bool estado;          // true = activo
    UbicacionGPS* ubicacion;      // puntero a la ubicacion GPS actual del bus

public:
    Bus();
    Bus(int id, const std::string& placa, int capMax, int capAct, bool est);
    ~Bus();

    // Getters
    int getIdBus() const;
    std::string  getPlaca() const;
    int getCapacidadMaxima() const;
    int getCapacidadActual() const;
    bool getEstado() const;
    UbicacionGPS* getUbicacion() const;

    // Setters
    void setIdBus(int id);
    void setPlaca(const std::string& placa);
    void setCapacidadMaxima(int cap);
    void setCapacidadActual(int cap);
    void setEstado(bool est);
    void setUbicacion(double lat, double lon, double alt, double vel = 30.0);

    // Simula el movimiento del bus hacia unas coordenadas destino
    void simularMovimiento(double latDest, double lonDest);

    // Estima el tiempo en minutos hasta la siguiente parada dada su ubicacion
    double tiempoHastaParada(double latParada, double lonParada) const;

    // Operador de comparacion por capacidad disponible
    bool operator<(const Bus& otro) const;
};
