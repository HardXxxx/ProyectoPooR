#pragma once
#include <vector>
#include <string>
#include "GestorArchivos.h"
#include "Bus.h"
#include "Parada.h"
#include "Incidente.h"
#include "Ruta.h"
#include "Usuario.h"

// Clase principal que orquesta todos los modulos del sistema de transporte
class SistemaTransporte {
private:
    GestorArchivos gestor;
    std::vector<Bus> buses;
    std::vector<Parada> paradas;
    std::vector<Incidente> incidentes;
    std::vector<Ruta*> rutas;
    std::vector<Usuario*> usuarios;


    Usuario* usuarioActual; // puntero al usuario que inicio sesion

    // Imprime el separador de linea con el ancho de consola
    void imprimirSeparador() const;

    // Imprime el encabezado con hora real y datos del usuario autenticado
    void imprimirEncabezado() const;

    // Muestra la pantalla de seleccion de perfil y retorna la opcion elegida
    int mostrarMenuPrincipal();

    // Solicita y valida el codigo del usuario segun su tipo
    Usuario* autenticarUsuario(int tipo);

    // Pantalla del estudiante: visor de rutas y paradas paginado
    void panelEstudiante();

    // Pantalla del conductor: informacion del bus, ruta, horarios y opciones
    void panelConductor();

    // Pantalla del administrador: resumen global y menu de modificacion
    void panelAdministrador();

    // Sub-menus del administrador
    void gestionarRutas();
    void gestionarUsuarios();
    void gestionarBuses();
    void gestionarHorarios();
    void gestionarIncidentes();

    // Busca el bus asignado a un conductor usando su idBus (con puntero)
    Bus* buscarBusPorId(int id);

    // Busca la ruta que tiene asignado un bus dado su placa
    Ruta* buscarRutaPorBus(const std::string& placa);

    // Devuelve los incidentes relacionados con la placa dada
    std::vector<Incidente*> incidentesDeBus(const std::string& placa);

    // Devuelve la hora actual del sistema como cadena formateada HH:MM:SS
    std::string horaActual() const;

    // Devuelve la fecha actual del sistema como cadena DD/MM/YYYY
    std::string fechaActual() const;

    // Muestra la distancia y tiempo estimado del bus hacia cada parada de la ruta
    void mostrarProximidadBus(Bus* bus, Ruta* ruta);

public:
    explicit SistemaTransporte(const std::string& dirData);
    ~SistemaTransporte();

    // Carga todos los datos desde los JSON y arranca el bucle principal
    void iniciar();
};
