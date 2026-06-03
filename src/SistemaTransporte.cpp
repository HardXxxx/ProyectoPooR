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
#include <map>
#include <algorithm>

static const int ANCHO = 80;

// Funciones auxiliares de consola
static void limpiarPantalla() {
    COORD coord = {0, 0}; DWORD celdas, escritas;
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(h, &info);
    celdas = info.dwSize.X * info.dwSize.Y;
    FillConsoleOutputCharacter(h, ' ', celdas, coord, &escritas);
    FillConsoleOutputAttribute(h, info.wAttributes, celdas, coord, &escritas);
    SetConsoleCursorPosition(h, coord);
}
static void setColor(WORD c) { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c); }
static int leerInt() {
    int v;
    if (!(std::cin >> v)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        return -1;
    }
    std::cin.ignore(10000, '\n');
    return v;
}

SistemaTransporte::SistemaTransporte(const std::string& dirData) : gestor(dirData), usuarioActual(nullptr) {}
SistemaTransporte::~SistemaTransporte() {
    for (Ruta* r : rutas) delete r;
    for (Usuario* u : usuarios) delete u;
}

void SistemaTransporte::imprimirSeparador() const {
    setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << std::string(ANCHO, '=') << '\n';
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void SistemaTransporte::imprimirEncabezado() const {
    imprimirSeparador();
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "[ " << horaActual() << " - " << fechaActual() << " ]";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << (usuarioActual ? " | " + usuarioActual->getEncabezado() : " | SISTEMA DE GESTION DE TRANSPORTE") << '\n';
    imprimirSeparador();
}

std::string SistemaTransporte::horaActual() const {
    SYSTEMTIME st; GetLocalTime(&st);
    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << st.wHour << ':'
       << std::setw(2) << std::setfill('0') << st.wMinute << ':'
       << std::setw(2) << std::setfill('0') << st.wSecond;
    return ss.str();
}

std::string SistemaTransporte::fechaActual() const {
    SYSTEMTIME st; GetLocalTime(&st);
    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << st.wDay << '/'
       << std::setw(2) << std::setfill('0') << st.wMonth << '/' << st.wYear;
    return ss.str();
}

Bus* SistemaTransporte::buscarBusPorId(int id) {
    auto it = std::find_if(buses.begin(), buses.end(), [id](const Bus& b) { return b.getIdBus() == id; });
    return it != buses.end() ? &*it : nullptr;
}

Ruta* SistemaTransporte::buscarRutaPorBus(const std::string& placa) {
    auto it = std::find_if(buses.begin(), buses.end(), [&placa](const Bus& b) { return b.getPlaca() == placa; });
    if (it != buses.end() && !rutas.empty()) {
        int idRuta = it->getIdRutaAsignada();
        auto itRuta = std::find_if(rutas.begin(), rutas.end(), [idRuta](Ruta* r) { return r->getIdRuta() == idRuta; });
        if (itRuta != rutas.end()) return *itRuta;
    }
    return (!rutas.empty() ? rutas[0] : nullptr);
}

std::vector<Incidente*> SistemaTransporte::incidentesDeBus(const std::string& placa) {
    std::vector<Incidente*> resultado;
    for (auto& inc : incidentes)
        if (inc.getDescripcion().find(placa) != std::string::npos) resultado.push_back(&inc);
    return resultado;
}

void SistemaTransporte::mostrarProximidadBus(Bus* bus, Ruta* ruta, const std::map<int, std::string>& horasLlegada) {
    if (!bus || !ruta || !bus->getUbicacion()) return;
    
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "\n[ PROXIMIDAD EN RUTA (30 km/h) ]\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    
    int idxActual = bus->getIndiceParadaActual();
    const auto& ids = ruta->getIdsParadas();
    
    for (size_t i = 0; i < ids.size(); ++i) {
        int idP = ids[i];
        auto it = std::find_if(paradas.begin(), paradas.end(), [idP](const Parada& p) { return p.getIdParada() == idP; });
        if (it == paradas.end()) continue;

        auto itHora = horasLlegada.find(idP);
        
        if ((int)i <= idxActual && itHora != horasLlegada.end()) {
            // Parada ya visitada: 0 m, 0 min, hora de llegada
            setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "  * Parada (" << idP << ") " << it->getNombre()
                      << "\n    -> 0 m | 0.0 min | Llego: " << itHora->second << "\n";
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        } else {
            // Parada pendiente: distancia y tiempo reales desde posición actual del bus
            double metros = it->distanciaA(bus->getUbicacion()->getLatitud(), bus->getUbicacion()->getLongitud());
            double minutos = bus->tiempoHastaParada(it->getUbicacion().getLatitud(), it->getUbicacion().getLongitud());
            std::cout << "    Parada (" << idP << ") " << it->getNombre()
                      << "\n    -> " << std::fixed << std::setprecision(0) << metros << " m | "
                      << std::setprecision(1) << minutos << " min\n";
        }
    }
}

int SistemaTransporte::mostrarMenuPrincipal() {
    limpiarPantalla();
    imprimirEncabezado();
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "Seleccione su perfil:\n\n"
              << "  [1] Estudiante\n  [2] Conductor\n  [3] Administrador\n"
              << "  [4] Salir\n\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    imprimirSeparador();
    std::cout << "> Opcion: ";
    return leerInt();
}

Usuario* SistemaTransporte::autenticarUsuario(int tipo) {
    static const std::map<int, std::pair<std::string, std::string>> datos = {
        {1, {"Estudiante", "codigo estudiantil"}},
        {2, {"Conductor", "codigo de operador"}},
        {3, {"Administrador", "codigo de administrador"}}
    };
    
    std::cout << "\nIngrese su " << datos.at(tipo).second << ": ";
    int codigo = leerInt();
    
    auto it = std::find_if(usuarios.begin(), usuarios.end(), [tipo, codigo](Usuario* u) {
        return u->getTipo() == datos.at(tipo).first && u->validarCodigo(codigo);
    });
    
    if (it != usuarios.end()) return *it;
    
    setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    std::cout << "\n  [!] Codigo no valido.\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    Sleep(2000);
    return nullptr;
}

void SistemaTransporte::panelEstudiante() {
    while (true) {
        limpiarPantalla();
        imprimirEncabezado();
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "DIRECTORIO DE RUTAS ACTIVAS\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        std::cout << std::string(ANCHO, '-') << "\nSeleccione una ruta:\n\n";
        
        for (Ruta* r : rutas)
            std::cout << "[ " << std::setw(2) << r->getIdRuta() << "] " << r->getNombre() 
                      << " (" << r->getTipo() << ")\n";
        std::cout << "\n[ 0] Volver\n\n> Ruta: ";
        
        int idRuta = leerInt();
        if (idRuta == 0) break;
        
        // CORRECCIÓN: Usamos un iterador para buscar la ruta correctamente
        auto it = std::find_if(rutas.begin(), rutas.end(), [idRuta](Ruta* r) { return r->getIdRuta() == idRuta; });
        if (it == rutas.end()) {
            setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
            std::cout << "\n  [!] Ruta no existe.\n";
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            Sleep(1500);
            continue;
        }
        
        // Extraemos el puntero del iterador
        Ruta* rutaEncontrada = *it;
        
        limpiarPantalla();
        imprimirSeparador();
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "[ " << horaActual() << " - " << fechaActual() << " ] | VISUALIZADOR DE RUTA\n";
        imprimirSeparador();
        std::cout << "RUTA: [" << rutaEncontrada->getIdRuta() << "] " << rutaEncontrada->getNombre() << "\n";
        if (!rutaEncontrada->getPuntoSalida().empty()) std::cout << "Salida: " << rutaEncontrada->getPuntoSalida() << "\n";
        
        setColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        std::cout << "\n[ PARADAS ]\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        for (int idP : rutaEncontrada->getIdsParadas()) {
            auto itParada = std::find_if(paradas.begin(), paradas.end(), [idP](const Parada& p) { return p.getIdParada() == idP; });
            if (itParada != paradas.end()) std::cout << "  (" << idP << ") " << itParada->getNombre() << "\n";
        }
        
        std::cout << "\n[ HORARIOS ]\n  - Barrio: " << rutaEncontrada->getHorario().formatearBarrio()
                  << "\n  - Unillanos: " << rutaEncontrada->getHorario().formatearUnillanos() << "\n";
        
        std::cout << "\n" << std::string(ANCHO, '-') 
                  << "\n>>> Reservar asiento? (S/N): ";
        char abordar = 'N'; 
        if (std::cin >> abordar) {
            std::cin.ignore(10000, '\n');
        } else {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }
        
        if (toupper(abordar) == 'S') {
            Bus* bus = buscarBusPorId(rutaEncontrada->getIdRuta());
            if (!bus && !buses.empty()) {
                auto itBus = std::find_if(buses.begin(), buses.end(), [](const Bus& b) {
                    return b.getCapacidadActual() < b.getCapacidadMaxima() && b.getEstado();
                });
                if (itBus != buses.end()) bus = &*itBus;
            }
            
            if (bus && bus->getCapacidadActual() < bus->getCapacidadMaxima()) {
                bus->setCapacidadActual(bus->getCapacidadActual() + 1);
                gestor.guardarBuses(buses);
                setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                std::cout << "\n  [EXITO] Asiento reservado en " << bus->getPlaca()
                          << " | Pasajeros: " << bus->getCapacidadActual() << "/" 
                          << bus->getCapacidadMaxima() << " | Disponibles: "
                          << (bus->getCapacidadMaxima() - bus->getCapacidadActual()) << "\n";
            } else {
                setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
                std::cout << "\n  [!] Bus " << (bus ? bus->getPlaca() : "asignado") << " lleno.\n";
            }
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            std::cout << "\n[Enter para continuar]"; std::cin.get();
        }
    }
}

void SistemaTransporte::panelConductor() {
    Conductor* cond = static_cast<Conductor*>(usuarioActual);
    Bus* bus = buscarBusPorId(cond->getBusAsignado());
    Ruta* ruta = bus ? buscarRutaPorBus(bus->getPlaca()) : nullptr;
    std::map<int, std::string> horasLlegada; // idParada -> hora de llegada
    
    while (true) {
        limpiarPantalla();
        imprimirEncabezado();
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "PANEL DEL CONDUCTOR\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        std::cout << std::string(ANCHO, '-') << "\n";
        
        setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "[ VEHICULO ]\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        if (bus) {
            std::cout << "Placa: " << bus->getPlaca() 
                      << " | Capacidad: " << bus->getCapacidadActual() << "/" 
                      << bus->getCapacidadMaxima() << "\n";
            auto inc = incidentesDeBus(bus->getPlaca());
            bool abierto = std::any_of(inc.begin(), inc.end(), [](Incidente* i) { return i->getEstado() == "Abierto"; });
            setColor(abierto ? FOREGROUND_RED | FOREGROUND_INTENSITY : FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "Estado: " << (abierto ? "ALERTA" : "OPERATIVO") << "\n";
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
        
        setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "\n[ RUTA ]\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        if (ruta) {
            std::cout << "Ruta " << ruta->getIdRuta() << " (" << ruta->getNombre() 
                      << ") | " << ruta->getOrigen() << " -> " << ruta->getDestino() << "\n";
            setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "\n[ HORARIOS ]\n";
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            std::cout << "Barrio: " << ruta->getHorario().formatearBarrio() 
                      << " | Unillanos: " << ruta->getHorario().formatearUnillanos() << "\n";
            mostrarProximidadBus(bus, ruta, horasLlegada);
        }
        
        setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "\n[ ESTADO DE RUTA ]\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        if (ruta && bus) {
            int idx = bus->getIndiceParadaActual();
            if (idx == -1) {
                std::cout << "  Ubicacion: En origen / Esperando inicio de ruta\n";
            } else if (idx < (int)ruta->getIdsParadas().size()) {
                int idActual = ruta->getIdsParadas()[idx];
                auto itP = std::find_if(paradas.begin(), paradas.end(), [idActual](const Parada& p){ return p.getIdParada() == idActual; });
                std::cout << "  Ultima parada visitada: " << (itP != paradas.end() ? itP->getNombre() : "Desconocida") << " a las " << horaActual() << "\n";
            } else {
                std::cout << "  Ubicacion: Fin de ruta alcanzado.\n";
            }
        }

        setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "\n[ OPCIONES ]\n";
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "[1] Llegada a parada\n[2] Reportar incidente\n[3] Finalizar turno\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        imprimirSeparador();
        std::cout << "> Opcion: ";
        
        int op = leerInt();
        if (op == 1 && bus && ruta && !ruta->getIdsParadas().empty()) {
            int idxActual = bus->getIndiceParadaActual();
            if (idxActual + 1 < (int)ruta->getIdsParadas().size()) {
                idxActual++;
                int idSig = ruta->getIdsParadas()[idxActual];
                auto it = std::find_if(paradas.begin(), paradas.end(), [idSig](const Parada& p) { return p.getIdParada() == idSig; });
                if (it != paradas.end()) {
                    // Fijar GPS del bus EXACTAMENTE en la parada
                    bus->setUbicacion(it->getUbicacion().getLatitud(), it->getUbicacion().getLongitud(), it->getAltitud());
                    bus->setIndiceParadaActual(idxActual);
                    std::string horaLlg = horaActual();
                    horasLlegada[idSig] = horaLlg;
                    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                    std::cout << "\n  [OK] Llegada registrada a: " << it->getNombre() << " (" << horaLlg << ")\n";
                    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                    gestor.guardarBuses(buses);
                }
            } else {
                bus->setIndiceParadaActual(-1); // Reiniciar recorrido
                
                int idRutaActual = bus->getIdRutaAsignada();
                auto itRuta = std::find_if(rutas.begin(), rutas.end(), [idRutaActual](Ruta* r) { return r->getIdRuta() == idRutaActual; });
                int nextIndex = 0;
                if (itRuta != rutas.end()) {
                    nextIndex = std::distance(rutas.begin(), itRuta) + 1;
                    if (nextIndex >= (int)rutas.size()) nextIndex = 0;
                }
                if (!rutas.empty()) {
                    bus->setIdRutaAsignada(rutas[nextIndex]->getIdRuta());
                    ruta = rutas[nextIndex];
                }
                horasLlegada.clear(); // Limpiar horas al cambiar de ruta

                setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                std::cout << "\n  [OK] Fin de ruta alcanzado. Se le ha asignado la " << (ruta ? ruta->getNombre() : "siguiente ruta") << ".\n";
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                gestor.guardarBuses(buses);
            }
            Sleep(2500);
        } else if (op == 2) {
            std::cout << "\nDescripcion: "; std::string desc; std::getline(std::cin, desc);
            std::cout << "Tipo (Mecanico/Accidente/Otro): "; std::string tipo; std::getline(std::cin, tipo);
            incidentes.emplace_back(incidentes.size() + 1, desc, tipo, "Abierto");
            gestor.guardarIncidentes(incidentes);
            setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "\n  [OK] Incidente registrado.\n";
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            Sleep(1500);
        } else if (op == 3) break;
    }
}

void SistemaTransporte::panelAdministrador() {
    while (true) {
        limpiarPantalla();
        imprimirEncabezado();
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "PANEL DE ADMINISTRACION\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        std::cout << std::string(ANCHO, '-') << "\n";
        
        setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "[ ESTADO GLOBAL ]\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        int abiertos = std::count_if(incidentes.begin(), incidentes.end(), 
                                     [](const Incidente& i) { return i.getEstado() == "Abierto"; });
        std::cout << "- Rutas: " << rutas.size() << "\n- Paradas: " << paradas.size() 
                  << "\n- Buses: " << buses.size() << " (";
        for (size_t i = 0; i < buses.size(); ++i) {
            std::cout << buses[i].getPlaca();
            if (i + 1 < buses.size()) std::cout << ", ";
        }
        std::cout << ")\n";
        setColor(abiertos > 0 ? FOREGROUND_RED | FOREGROUND_INTENSITY : FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "- Incidentes abiertos: " << abiertos << "\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        
        setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "\n[ MENU ]\n";
        setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "[1] Gestionar Rutas\n[2] Gestionar Usuarios\n[3] Gestionar Buses\n"
                  << "[4] Gestionar Horarios\n[5] Gestionar Incidentes\n[6] Volver\n";
        setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        imprimirSeparador();
        std::cout << "> Opcion: ";
        
        int op = leerInt();
        if (op == 1) gestionarRutas();
        else if (op == 2) gestionarUsuarios();
        else if (op == 3) gestionarBuses();
        else if (op == 4) gestionarHorarios();
        else if (op == 5) gestionarIncidentes();
        else if (op == 6) break;
    }
}

void SistemaTransporte::gestionarRutas() {
    limpiarPantalla(); imprimirEncabezado();
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY); std::cout << "GESTION DE RUTAS\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << std::string(ANCHO, '-') << "\n";
    for (Ruta* r : rutas)
        std::cout << "  [" << r->getIdRuta() << "] " << r->getNombre() 
                  << " | " << (r->getEstado() ? "ACTIVA" : "INACTIVA") << "\n";
    std::cout << "\nID ruta a cambiar (0 cancelar): ";
    int id = leerInt();
    if (id > 0) {
        for (Ruta* r : rutas) {
            if (r->getIdRuta() == id) {
                r->setEstado(!r->getEstado());
                gestor.guardarRutas(rutas);
                setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                std::cout << "\n  [OK] Estado: " << (r->getEstado() ? "ACTIVA" : "INACTIVA") << "\n";
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                break;
            }
        }
    }
    Sleep(1500);
}

void SistemaTransporte::gestionarUsuarios() {
    limpiarPantalla(); imprimirEncabezado();
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY); std::cout << "ADMINISTRACION DE USUARIOS\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << std::string(ANCHO, '-') << "\n";
    for (Usuario* u : usuarios)
        std::cout << "  ID: " << u->getIdUsuario() << " | " << u->getNombre() 
                  << " | " << u->getTipo() << "\n";
    std::cout << "\n[Info mostrada. Enter para continuar]";
    std::cin.get(); Sleep(1500);
}

void SistemaTransporte::gestionarBuses() {
    limpiarPantalla(); imprimirEncabezado();
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY); std::cout << "INSPECCION DE FLOTA\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << std::string(ANCHO, '-') << "\n";
    for (Bus& b : buses)
        std::cout << "  [" << b.getIdBus() << "] " << b.getPlaca() 
                  << " | " << b.getCapacidadActual() << "/" << b.getCapacidadMaxima()
                  << " | " << (b.getEstado() ? "Activo" : "Inactivo") << "\n";
    std::cout << "\nID bus (0 cancelar): ";
    int id = leerInt();
    if (id > 0) {
        Bus* bus = buscarBusPorId(id);
        if (bus) {
            std::cout << "Nueva capacidad (0-" << bus->getCapacidadMaxima() << "): ";
            int cap = leerInt();
            if (cap >= 0 && cap <= bus->getCapacidadMaxima()) {
                bus->setCapacidadActual(cap);
                gestor.guardarBuses(buses);
                setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                std::cout << "\n  [OK] Capacidad actualizada.\n";
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            }
        }
    }
    Sleep(1500);
}

void SistemaTransporte::gestionarHorarios() {
    limpiarPantalla(); imprimirEncabezado();
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY); std::cout << "MODIFICACION DE HORARIOS\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << std::string(ANCHO, '-') << "\n";
    for (Ruta* r : rutas) std::cout << "  [" << r->getIdRuta() << "] " << r->getNombre() << "\n";
    std::cout << "\nID ruta (0 cancelar): ";
    int id = leerInt();
    if (id > 0) {
        for (Ruta* r : rutas) {
            if (r->getIdRuta() == id) {
                std::cout << "Salida [1] Barrio [2] Unillanos: "; int op = leerInt();
                std::cout << "Hora (HH:MM:SS): "; std::string hora; std::getline(std::cin, hora);
                if (op == 1) r->getHorario().agregarSalidaBarrio(hora);
                else r->getHorario().agregarSalidaUnillanos(hora);
                gestor.guardarRutas(rutas);
                setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                std::cout << "\n  [OK] Horario agregado.\n";
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                break;
            }
        }
    }
    Sleep(1500);
}

void SistemaTransporte::gestionarIncidentes() {
    limpiarPantalla(); imprimirEncabezado();
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY); std::cout << "GESTION DE INCIDENTES\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << std::string(ANCHO, '-') << "\n";
    for (Incidente& inc : incidentes) {
        setColor(inc.getEstado() == "Abierto" ? FOREGROUND_RED | FOREGROUND_INTENSITY : FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "  [" << inc.getIdIncidente() << "] [" << inc.getEstado() << "] "
                  << inc.getTipo() << ": " << inc.getDescripcion() << "\n";
    }
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << "\nID incidente a cerrar (0 cancelar): ";
    int id = leerInt();
    if (id > 0) {
        for (Incidente& inc : incidentes) {
            if (inc.getIdIncidente() == id) {
                inc.cerrar();
                gestor.guardarIncidentes(incidentes);
                setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
                std::cout << "\n  [OK] Incidente cerrado.\n";
                setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
                break;
            }
        }
    }
    Sleep(1500);
}

void SistemaTransporte::iniciar() {
    buses = gestor.cargarBuses(); paradas = gestor.cargarParadas();
    incidentes = gestor.cargarIncidentes(); rutas = gestor.cargarRutas(); usuarios = gestor.cargarUsuarios();
    
    SetConsoleTitle("Sistema de Gestion de Transporte - Unillanos");
    SetConsoleOutputCP(65001); SetConsoleCP(65001); 
    
    while (true) {
        usuarioActual = nullptr;
        int op = mostrarMenuPrincipal();
        if (op == 4) {
            limpiarPantalla(); setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
            std::cout << "\n  Sistema cerrado. Hasta luego.\n\n";
            setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); Sleep(1000); break;
        }
        if (op < 1 || op > 3) continue;
        limpiarPantalla(); imprimirEncabezado();
        usuarioActual = autenticarUsuario(op);
        if (!usuarioActual) continue;
        if (op == 1) panelEstudiante();
        else if (op == 2) panelConductor();
        else panelAdministrador();
    }
}