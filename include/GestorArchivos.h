#pragma once
#include <string>
#include <vector>
#include "Bus.h"
#include "Parada.h"
#include "Incidente.h"
#include "Ruta.h"
#include "RutaBarrio.h"
#include "RutaCentro.h"
#include "Usuario.h"
#include "Estudiante.h"
#include "Conductor.h"
#include "Administrador.h"

// Gestiona la lectura y escritura de todos los archivos JSON del sistema
class GestorArchivos {
private:
    std::string rutaData; // directorio base donde estan los JSON

    // Extrae el valor de una clave JSON de tipo string en una linea dada
    std::string extraerString(const std::string& linea, const std::string& clave) const;

    // Extrae el valor de una clave JSON de tipo numerico en una linea dada
    double extraerNumero(const std::string& linea, const std::string& clave) const;

    // Extrae el valor booleano de una clave JSON en una linea dada
    bool extraerBool(const std::string& linea, const std::string& clave) const;

public:
    explicit GestorArchivos(const std::string& dirData);

    // Lee el archivo buses.json y retorna el vector de buses cargados
    std::vector<Bus> cargarBuses() const;

    // Lee el archivo paradas.json y retorna el vector de paradas
    std::vector<Parada> cargarParadas() const;

    // Lee el archivo incidentes.json y retorna el vector de incidentes
    std::vector<Incidente> cargarIncidentes() const;

    // Lee el archivo rutas.json y retorna punteros polimorficos a las rutas
    std::vector<Ruta*> cargarRutas() const;

    // Lee el archivo usuarios.json y retorna punteros polimorficos a usuarios
    std::vector<Usuario*> cargarUsuarios() const;

    // Guarda el estado actualizado de incidentes al archivo incidentes.json
    void guardarIncidentes(const std::vector<Incidente>& incidentes) const;

    // Guarda los buses actualizados (capacidad, estado) al archivo buses.json
    void guardarBuses(const std::vector<Bus>& buses) const;

    // Guarda los usuarios actualizados al archivo usuarios.json
    void guardarUsuarios(const std::vector<Usuario*>& usuarios) const;

    // Guarda las rutas actualizadas al archivo rutas.json
    void guardarRutas(const std::vector<Ruta*>& rutas) const;
};
