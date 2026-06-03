#include "SistemaTransporte.h"
#include "Conductor.h"
#include "Estudiante.h"
#include "Administrador.h"
#include "RutaCentro.h"
#include <windows.h>
#include <ctime>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>

// Ancho de la consola para los separadores
static const int ANCHO = 80;

// Constructor y destructor


SistemaTransporte::SistemaTransporte(const std::string& dirData)
    : gestor(dirData), usuarioActual(nullptr) {}

SistemaTransporte::~SistemaTransporte() {
    // Libera memoria de punteros polimorficos
    for (Ruta* r : rutas)    delete r;
    for (Usuario* u : usuarios) delete u;
}

// Utilidades de consola con Windows.h

static void limpiarPantalla() {
    COORD coordInicio = { 0, 0 };
    DWORD celdas, escritas;
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hCon, &info);
    celdas = info.dwSize.X * info.dwSize.Y;
    FillConsoleOutputCharacter(hCon, ' ', celdas, coordInicio, &escritas);
    FillConsoleOutputAttribute(hCon, info.wAttributes, celdas, coordInicio, &escritas);
    SetConsoleCursorPosition(hCon, coordInicio);
}

static void setColor(WORD color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

static void setCursor(int x, int y) {
    COORD pos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void SistemaTransporte::imprimirSeparador() const {
    setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    for (int i = 0; i < ANCHO; ++i) std::cout << '=';
    std::cout << '\n';
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void SistemaTransporte::imprimirEncabezado() const {
    imprimirSeparador();
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "[ " << horaActual() << " - " << fechaActual() << " ]";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    if (usuarioActual)
        std::cout << "  | " << usuarioActual->getEncabezado();
    else
        std::cout << "  | SISTEMA DE GESTION DE TRANSPORTE";
    std::cout << '\n';
    imprimirSeparador();
}


// Hora y fecha real del sistema (avanza en tiempo real)


std::string SistemaTransporte::horaActual() const {
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << st.wHour   << ':'
       << std::setw(2) << std::setfill('0') << st.wMinute << ':'
       << std::setw(2) << std::setfill('0') << st.wSecond;
    return ss.str();
}

std::string SistemaTransporte::fechaActual() const {
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << st.wDay   << '/'
       << std::setw(2) << std::setfill('0') << st.wMonth << '/'
       << st.wYear;
    return ss.str();
}

// Busquedas internas por ID / placa

Bus* SistemaTransporte::buscarBusPorId(int id) {
    for (Bus& b : buses)
        if (b.getIdBus() == id) return &b;
    return nullptr;
}

Ruta* SistemaTransporte::buscarRutaPorBus(const std::string& placa) {
    for (Bus& b : buses) {
        if (b.getPlaca() == placa && !rutas.empty()) {
            int idx = b.getIdBus() - 1;
            if (idx >= 0 && idx < (int)rutas.size()) return rutas[idx];
        }
    }
    return rutas.empty() ? nullptr : rutas[0];
}

std::vector<Incidente*> SistemaTransporte::incidentesDeBus(const std::string& placa) {
    std::vector<Incidente*> resultado;
    for (Incidente& inc : incidentes)
        if (inc.getDescripcion().find(placa) != std::string::npos)
            resultado.push_back(&inc);
    return resultado;
}

// Muestra distancia y tiempo estimado del bus a cada parada de la ruta

void SistemaTransporte::mostrarProximidadBus(Bus* bus, Ruta* ruta) {
    if (!bus || !ruta) return;
    if (!bus->getUbicacion() && !ruta->getIdsParadas().empty()) {
        int idP = ruta->getIdsParadas()[0];
        for (Parada& p : paradas) {
            if (p.getIdParada() == idP) {
                bus->setUbicacion(p.getUbicacion().getLatitud(),
                                  p.getUbicacion().getLongitud(), 470.0, 30.0);
                break;
            }
        }
    }
    if (!bus->getUbicacion()) return;

    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "\n[ PROXIMIDAD EN RUTA (velocidad estimada 30 km/h) ]\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

    int mostradas = 0;
    for (int idP : ruta->getIdsParadas()) {
        for (Parada& p : paradas) {
            if (p.getIdParada() == idP) {
                double dist = bus->tiempoHastaParada(
                    p.getUbicacion().getLatitud(),
                    p.getUbicacion().getLongitud());
                double metros = p.distanciaA(
                    bus->getUbicacion()->getLatitud(),
                    bus->getUbicacion()->getLongitud());
                std::cout << "  Parada (" << idP << ") " << p.getNombre() << "\n"
                          << "    -> Distancia: " << std::fixed << std::setprecision(0)
                          << metros << " m  |  Tiempo estimado: "
                          << std::setprecision(1) << dist << " min\n";
                if (++mostradas >= 4) goto fin_proximidad;
                break;
            }
        }
    }
    fin_proximidad:;
}


// Menu principal de seleccion de perfil


int SistemaTransporte::mostrarMenuPrincipal() {
    limpiarPantalla();
    imprimirEncabezado();
    std::cout << "Por favor, seleccione su perfil para iniciar sesion en el sistema:\n\n";
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "  [1] Ingresar como Estudiante\n"
              << "  [2] Ingresar como Conductor\n"
              << "  [3] Ingresar como Administrador\n"
              << "  [4] Salir del sistema\n\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    imprimirSeparador();
    std::cout << "> Ingrese el numero de su opcion: ";
    int op = 0;
    std::cin >> op;
    std::cin.ignore(100, '\n');
    return op;
}

Usuario* SistemaTransporte::autenticarUsuario(int tipo) {
    std::string tipoBuscado;
    std::string labelCodigo;
    if      (tipo == 1) { tipoBuscado = "Estudiante";    labelCodigo = "codigo estudiantil"; }
    else if (tipo == 2) { tipoBuscado = "Conductor";     labelCodigo = "codigo de operador"; }
    else if (tipo == 3) { tipoBuscado = "Administrador"; labelCodigo = "codigo de administrador"; }

    std::cout << "\nIngrese su " << labelCodigo << ": ";
    int codigo = 0;
    std::cin >> codigo;
    std::cin.ignore(100, '\n');

    for (Usuario* u : usuarios) {
        if (u->getTipo() == tipoBuscado && u->validarCodigo(codigo))
            return u;
    }

    setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    std::cout << "\n  [!] Codigo no valido. El usuario no existe en el sistema.\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    Sleep(2000); 
    return nullptr;
}

// Panel del Estudiante (Modificado: Visualización encapsulada y Reserva)

void SistemaTransporte::panelEstudiante() {
    while (true) {
        limpiarPantalla();
        imprimirEncabezado();
        
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "DIRECTORIO DE RUTAS ACTIVAS\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        std::cout << std::string(ANCHO, '-') << '\n';
        std::cout << "Seleccione una ruta para visualizar sus paradas y horarios disponibles:\n\n";

        // Lista todas las rutas de forma vertical
        for (Ruta* r : rutas) {
            std::cout << "[ " << std::setw(2) << r->getIdRuta() << "] " 
                      << r->getNombre() << " (" << r->getTipo() << ")\n";
        }

        std::cout << "\n[  0] Volver al menu principal\n\n";
        std::cout << "> Ingrese el numero de la ruta: ";
        
        int idRuta = 0;
        std::cin >> idRuta;
        std::cin.ignore(10000, '\n');

        if (idRuta == 0) break;

        // Buscar la ruta seleccionada
        Ruta* rutaSeleccionada = nullptr;
        for (Ruta* r : rutas) {
            if (r->getIdRuta() == idRuta) {
                rutaSeleccionada = r;
                break;
            }
        }

        if (!rutaSeleccionada) {
            setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
            std::cout << "\n  [!] La ruta seleccionada no existe.\n";
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            Sleep(1500);
            continue;
        }

        // Sub-pantalla encapsulada: Detalles de la Ruta
        limpiarPantalla();
        imprimirSeparador();
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "[ " << horaActual() << " - " << fechaActual() << " ]  | VISUALIZADOR DE RUTA\n";
        imprimirSeparador();
        
        std::cout << "RUTA SELECCIONADA: [" << rutaSeleccionada->getIdRuta() << "] " << rutaSeleccionada->getNombre() << "\n";
        if (!rutaSeleccionada->getPuntoSalida().empty()) {
            std::cout << "Punto de Salida  : " << rutaSeleccionada->getPuntoSalida() << "\n";
        }

        setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        std::cout << "\n[ PARADAS ASIGNADAS ]\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        for (int idP : rutaSeleccionada->getIdsParadas()) {
            for (Parada& p : paradas) {
                if (p.getIdParada() == idP) {
                    std::cout << "  (" << idP << ") " << p.getNombre() << "\n";
                    break;
                }
            }
        }

        setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        std::cout << "\n[ HORARIOS DE SALIDA PROGRAMADOS ]\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        std::cout << "  - Desde el Barrio : " << rutaSeleccionada->getHorario().formatearBarrio() << "\n";
        std::cout << "  - Desde Unillanos : " << rutaSeleccionada->getHorario().formatearUnillanos() << "\n";

        std::cout << "\n" << std::string(ANCHO, '-') << "\n";
        std::cout << ">>> Desea registrar su abordaje en el proximo bus de esta ruta? (S/N): ";
        char abordar;
        std::cin >> abordar;
        std::cin.ignore(10000, '\n');

        if (toupper(abordar) == 'S') {
            // Asigna un bus a la ruta. Busca primero un bus con ID idéntico al de la ruta.
            Bus* busAsignado = buscarBusPorId(rutaSeleccionada->getIdRuta());
            
            // Si no encuentra el bus específico, toma el primer bus del sistema que tenga cupo
            if (!busAsignado && !buses.empty()) {
                for (Bus& b : buses) {
                    if (b.getCapacidadActual() < b.getCapacidadMaxima() && b.getEstado() == true) {
                        busAsignado = &b;
                        break;
                    }
                }
            }

            if (busAsignado) {
                if (busAsignado->getCapacidadActual() < busAsignado->getCapacidadMaxima()) {
                    // Aumentar pasajeros (Disminuye capacidad disponible)
                    busAsignado->setCapacidadActual(busAsignado->getCapacidadActual() + 1);
                    gestor.guardarBuses(buses); // Modifica la info en el json

                    int asientosDisponibles = busAsignado->getCapacidadMaxima() - busAsignado->getCapacidadActual();

                    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    std::cout << "\n  [EXITO] Asiento reservado. Puede abordar la unidad en el proximo horario.\n";
                    std::cout << "          Bus asignado al trayecto : " << busAsignado->getPlaca() << "\n";
                    std::cout << "          Pasajeros a bordo        : " << busAsignado->getCapacidadActual() << " / " << busAsignado->getCapacidadMaxima() << "\n";
                    std::cout << "          Asientos disponibles     : " << asientosDisponibles << "\n";
                } else {
                    setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
                    std::cout << "\n  [!] Lo sentimos, el proximo bus asignado (" << busAsignado->getPlaca() << ") ya esta a su capacidad maxima.\n";
                }
            } else {
                setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
                std::cout << "\n  [!] Falla Logistica: No hay buses operativos asignados a esta ruta en este momento.\n";
            }
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            
            std::cout << "\n[Presione 'Enter' para continuar]";
            std::cin.get();
        }
    }
}

// Panel del Conductor: bus asignado, ruta, horarios e incidentes

void SistemaTransporte::panelConductor() {
    Conductor* cond = static_cast<Conductor*>(usuarioActual);
    Bus* bus        = buscarBusPorId(cond->getBusAsignado());
    Ruta* ruta      = bus ? buscarRutaPorBus(bus->getPlaca()) : nullptr;

    while (true) {
        limpiarPantalla();
        imprimirEncabezado();
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "PANEL DE CONTROL DEL CONDUCTOR\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        std::cout << std::string(ANCHO, '-') << '\n';

        setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "[ VEHICULO ASIGNADO ]\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        if (bus) {
            std::cout << "Placa de Bus: " << bus->getPlaca()
                      << " | Capacidad actual: " << bus->getCapacidadActual()
                      << "/" << bus->getCapacidadMaxima() << " pasajeros\n";

            std::vector<Incidente*> inc = incidentesDeBus(bus->getPlaca());
            if (inc.empty()) {
                setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                std::cout << "Estado de la unidad: OPERATIVO\n";
            } else {
                setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
                for (Incidente* i : inc)
                    if (i->getEstado() == "Abierto")
                        std::cout << "Estado de la unidad: ALERTA - " << i->getDescripcion() << "\n";
            }
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }

        setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "\n[ RUTA OPERATIVA ]\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        if (ruta) {
            std::cout << "Asignacion: Ruta " << ruta->getIdRuta()
                      << " (" << ruta->getNombre() << ")\n"
                      << "Trayecto: " << ruta->getOrigen()
                      << " -> " << ruta->getDestino() << "\n";

            setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "\n[ HORARIOS DE SALIDA ASIGNADOS ]\n";
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            std::cout << "Salidas desde el Barrio:   " << ruta->getHorario().formatearBarrio()    << "\n"
                      << "Salidas desde Unillanos:   " << ruta->getHorario().formatearUnillanos() << "\n";

            mostrarProximidadBus(bus, ruta);
        }

        setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "\n[ OPCIONES ]\n";
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "[1] Reportar llegada a parada\n"
                  << "[2] Notificar nuevo incidente\n"
                  << "[3] Finalizar turno\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        imprimirSeparador();
        std::cout << "> Opcion: ";

        int op = 0;
        std::cin >> op;
        std::cin.ignore(100, '\n');

        if (op == 1) {
            if (bus && ruta && !ruta->getIdsParadas().empty()) {
                int idSig = ruta->getIdsParadas()[0];
                for (Parada& p : paradas) {
                    if (p.getIdParada() == idSig) {
                        bus->simularMovimiento(p.getUbicacion().getLatitud(),
                                               p.getUbicacion().getLongitud());
                        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                        std::cout << "\n  [OK] Posicion actualizada hacia parada: " << p.getNombre() << "\n";
                        gestor.guardarBuses(buses);
                        break;
                    }
                }
            }
            Sleep(1500);
        } else if (op == 2) {
            std::cout << "\nDescripcion del incidente: ";
            std::string desc;
            std::getline(std::cin, desc);
            std::cout << "Tipo (Mecanico / Accidente / Otro): ";
            std::string tipo;
            std::getline(std::cin, tipo);
            int nuevoId = (int)incidentes.size() + 1;
            incidentes.push_back(Incidente(nuevoId, desc, tipo, "Abierto"));
            gestor.guardarIncidentes(incidentes);
            setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "\n  [OK] Incidente registrado y guardado en el sistema.\n";
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            Sleep(1500);
        } else if (op == 3) {
            break; 
        }
    }
}

// Panel del Administrador: resumen global y sub-menus de modificacion

void SistemaTransporte::panelAdministrador() {
    while (true) {
        limpiarPantalla();
        imprimirEncabezado();
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "PANEL CENTRAL DE ADMINISTRACION Y MODIFICACION\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        std::cout << std::string(ANCHO, '-') << '\n';

        setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "[ ESTADO GLOBAL DEL SISTEMA ]\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        std::cout << "- Total de Rutas:   " << rutas.size() << " rutas en operacion\n"
                  << "- Total de Paradas: " << paradas.size() << " puntos registrados en la ciudad\n";

        std::cout << "- Flota Activa:     " << buses.size() << " buses operando (";
        for (size_t i = 0; i < buses.size(); ++i) {
            std::cout << buses[i].getPlaca();
            if (i + 1 < buses.size()) std::cout << ", ";
        }
        std::cout << ")\n";

        int abiertos = 0;
        for (Incidente& inc : incidentes)
            if (inc.getEstado() == "Abierto") abiertos++;
        setColor(abiertos > 0 ? FOREGROUND_RED | FOREGROUND_INTENSITY : FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "- Novedades:        " << abiertos << " incidente(s) en estado \"Abierto\"\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "\n[ MENU DE MODIFICACION DE DATOS ]\n";
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "[1] Gestionar y Modificar Rutas / Paradas\n"
                  << "[2] Administrar Base de Usuarios\n"
                  << "[3] Asignar Buses e Inspeccionar Flota\n"
                  << "[4] Modificar Intervalos y Horarios\n"
                  << "[5] Gestionar Estado de Incidentes\n"
                  << "[6] Volver al menu principal\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        imprimirSeparador();
        std::cout << "> Ingrese la seccion que desea modificar: ";

        int op = 0;
        std::cin >> op;
        std::cin.ignore(100, '\n');

        if      (op == 1) gestionarRutas();
        else if (op == 2) gestionarUsuarios();
        else if (op == 3) gestionarBuses();
        else if (op == 4) gestionarHorarios();
        else if (op == 5) gestionarIncidentes();
        else if (op == 6) break;
    }
}

// Sub-menus del Administrador

void SistemaTransporte::gestionarRutas() {
    limpiarPantalla();
    imprimirEncabezado();
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "GESTION DE RUTAS\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << std::string(ANCHO, '-') << '\n';
    for (Ruta* r : rutas) {
        std::cout << "  [" << r->getIdRuta() << "] " << r->getNombre()
                  << "  Estado: " << (r->getEstado() ? "ACTIVA" : "INACTIVA") << '\n';
    }
    std::cout << "\nIngrese ID de ruta a cambiar estado (0 para cancelar): ";
    int id = 0;
    std::cin >> id;
    std::cin.ignore(100, '\n');
    if (id > 0) {
        for (Ruta* r : rutas) {
            if (r->getIdRuta() == id) {
                r->setEstado(!r->getEstado());
                gestor.guardarRutas(rutas);
                setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                std::cout << "\n  [OK] Ruta actualizada. Nuevo estado: "
                          << (r->getEstado() ? "ACTIVA" : "INACTIVA") << '\n';
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                break;
            }
        }
    }
    Sleep(1500);
}

void SistemaTransporte::gestionarUsuarios() {
    limpiarPantalla();
    imprimirEncabezado();
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "ADMINISTRACION DE USUARIOS\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << std::string(ANCHO, '-') << '\n';
    for (Usuario* u : usuarios) {
        std::cout << "  ID: " << u->getIdUsuario()
                  << "  Nombre: " << u->getNombre()
                  << "  Tipo: " << u->getTipo() << '\n';
    }
    std::cout << "\n[ Informacion de usuarios mostrada.]\n";
    std::cout << "\n[Presione 'Enter' para continuar]";
    std::cin.get();
    Sleep(1500);
}

void SistemaTransporte::gestionarBuses() {
    limpiarPantalla();
    imprimirEncabezado();
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "INSPECCION Y ASIGNACION DE FLOTA\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << std::string(ANCHO, '-') << '\n';
    for (Bus& b : buses) {
        std::cout << "  Bus [" << b.getIdBus() << "] " << b.getPlaca()
                  << "  Cap: " << b.getCapacidadActual() << "/" << b.getCapacidadMaxima()
                  << "  Estado: " << (b.getEstado() ? "Activo" : "Inactivo") << '\n';
    }
    std::cout << "\nIngrese ID de bus para actualizar capacidad actual (0 para cancelar): ";
    int id = 0; std::cin >> id; std::cin.ignore(100, '\n');
    if (id > 0) {
        Bus* bus = buscarBusPorId(id);
        if (bus) {
            std::cout << "Nueva capacidad actual (0-" << bus->getCapacidadMaxima() << "): ";
            int cap = 0; std::cin >> cap; std::cin.ignore(100, '\n');
            if (cap >= 0 && cap <= bus->getCapacidadMaxima()) {
                bus->setCapacidadActual(cap);
                gestor.guardarBuses(buses);
                setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                std::cout << "\n  [OK] Capacidad actualizada y guardada en buses.json\n";
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            }
        }
    }
    Sleep(1500);
}

void SistemaTransporte::gestionarHorarios() {
    limpiarPantalla();
    imprimirEncabezado();
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "MODIFICACION DE HORARIOS\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << std::string(ANCHO, '-') << '\n';
    for (Ruta* r : rutas)
        std::cout << "  [" << r->getIdRuta() << "] " << r->getNombre() << '\n';
    std::cout << "\nID de ruta a modificar (0 cancelar): ";
    int id = 0; std::cin >> id; std::cin.ignore(100, '\n');
    if (id > 0) {
        for (Ruta* r : rutas) {
            if (r->getIdRuta() == id) {
                std::cout << "Agregar salida desde [1] Barrio  [2] Unillanos: ";
                int op = 0; std::cin >> op; std::cin.ignore(100, '\n');
                std::cout << "Hora (HH:MM:SS): ";
                std::string hora; std::getline(std::cin, hora);
                if (op == 1) r->getHorario().agregarSalidaBarrio(hora);
                else         r->getHorario().agregarSalidaUnillanos(hora);
                gestor.guardarRutas(rutas);
                setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                std::cout << "\n  [OK] Horario agregado y guardado en rutas.json\n";
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                break;
            }
        }
    }
    Sleep(1500);
}

void SistemaTransporte::gestionarIncidentes() {
    limpiarPantalla();
    imprimirEncabezado();
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "GESTION DE INCIDENTES\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << std::string(ANCHO, '-') << '\n';
    for (Incidente& inc : incidentes) {
        WORD color = inc.getEstado() == "Abierto"
            ? (FOREGROUND_RED | FOREGROUND_INTENSITY)
            : (FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        setColor(color);
        std::cout << "  [" << inc.getIdIncidente() << "] [" << inc.getEstado() << "] "
                  << inc.getTipo() << ": " << inc.getDescripcion() << '\n';
    }
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << "\nID de incidente a cerrar (0 cancelar): ";
    int id = 0; std::cin >> id; std::cin.ignore(100, '\n');
    if (id > 0) {
        for (Incidente& inc : incidentes) {
            if (inc.getIdIncidente() == id) {
                inc.cerrar();
                gestor.guardarIncidentes(incidentes);
                setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                std::cout << "\n  [OK] Incidente cerrado y guardado en incidentes.json\n";
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                break;
            }
        }
    }
    Sleep(1500);
}

// Punto de arranque del sistema

void SistemaTransporte::iniciar() {
    buses      = gestor.cargarBuses();
    paradas    = gestor.cargarParadas();
    incidentes = gestor.cargarIncidentes();
    rutas      = gestor.cargarRutas();
    usuarios   = gestor.cargarUsuarios();

    SetConsoleTitle("Sistema de Gestion de Transporte - Unillanos");
    SetConsoleOutputCP(1252);
    SetConsoleCP(1252);

    while (true) {
        usuarioActual = nullptr;
        int opcion = mostrarMenuPrincipal();

        if (opcion == 4) {
            limpiarPantalla();
            setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "\n  Sistema cerrado. Hasta luego.\n\n";
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            Sleep(1000);
            break;
        }

        if (opcion < 1 || opcion > 3) continue;

        limpiarPantalla();
        imprimirEncabezado();
        usuarioActual = autenticarUsuario(opcion);
        if (!usuarioActual) continue;

        if      (opcion == 1) panelEstudiante();
        else if (opcion == 2) panelConductor();
        else if (opcion == 3) panelAdministrador();
    }
}