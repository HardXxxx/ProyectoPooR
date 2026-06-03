#include "GestorArchivos.h"
#include <fstream>
#include <iostream>
#include "../include/json.hpp"

// Alias local para no escribir nlohmann::json en cada linea
using json = nlohmann::json;

GestorArchivos::GestorArchivos(const std::string& dirData) : rutaData(dirData) {}

// Metodos de carga (lectura desde JSON)

// Lee buses.json y construye el vector de buses
std::vector<Bus> GestorArchivos::cargarBuses() const {
    std::vector<Bus> lista;
    std::ifstream archivo(rutaData + "/buses.json");
    if (!archivo.is_open()) return lista;

    json j = json::parse(archivo);
    for (const auto& b : j["buses"]) {
        lista.emplace_back(
            b["idBus"].get<int>(),
            b["placa"].get<std::string>(),
            b["capacidadMaxima"].get<int>(),
            b["capacidadActual"].get<int>(),
            b["estado"].get<bool>()
        );
    }
    return lista;
}

// Lee paradas.json (array raiz) y construye el vector de paradas con coordenadas GPS
std::vector<Parada> GestorArchivos::cargarParadas() const {
    std::vector<Parada> lista;
    std::ifstream archivo(rutaData + "/paradas.json");
    if (!archivo.is_open()) return lista;

    // paradas.json es un array directo, sin clave envolvente
    json j = json::parse(archivo);
    for (const auto& p : j) {
        lista.emplace_back(
            p["idParada"].get<int>(),
            p["nombre"].get<std::string>(),
            p["ubicacion"]["latitud"].get<double>(),
            p["ubicacion"]["longitud"].get<double>(),
            p["ubicacion"]["altitud"].get<double>(),
            p["estado"].get<bool>(),
            p["ordenEnRuta"].get<int>()
        );
    }
    return lista;
}

// Lee incidentes.json y construye el vector de incidentes
std::vector<Incidente> GestorArchivos::cargarIncidentes() const {
    std::vector<Incidente> lista;
    std::ifstream archivo(rutaData + "/incidentes.json");
    if (!archivo.is_open()) return lista;

    json j = json::parse(archivo);
    for (const auto& inc : j["incidentes"]) {
        lista.emplace_back(
            inc["idIncidente"].get<int>(),
            inc["descripcion"].get<std::string>(),
            inc["tipo"].get<std::string>(),
            inc["estado"].get<std::string>()
        );
    }
    return lista;
}

// Lee horarios.json y construye un objeto HorarioRuta con la configuracion global
HorarioRuta GestorArchivos::cargarHorariosPorDefecto() const {
    HorarioRuta global;
    std::ifstream archivo(rutaData + "/horarios.json");
    if (!archivo.is_open()) return global;

    json j = json::parse(archivo);
    if (j.contains("configuracionHorarios")) {
        const auto& conf = j["configuracionHorarios"];
        if (conf.contains("salidasBarrio") && conf["salidasBarrio"].is_array()) {
            for (const auto& h : conf["salidasBarrio"]) {
                global.agregarSalidaBarrio(h.get<std::string>());
            }
        }
        if (conf.contains("salidasUnillanos") && conf["salidasUnillanos"].is_array()) {
            for (const auto& h : conf["salidasUnillanos"]) {
                global.agregarSalidaUnillanos(h.get<std::string>());
            }
        }
    }
    return global;
}

// Lee rutas.json y retorna punteros polimorficos segun el tipo de ruta
std::vector<Ruta*> GestorArchivos::cargarRutas() const {
    std::vector<Ruta*> lista;
    std::ifstream archivo(rutaData + "/rutas.json");
    if (!archivo.is_open()) return lista;

    HorarioRuta horarioGlobal = cargarHorariosPorDefecto();

    json j = json::parse(archivo);
    for (const auto& r : j["rutas"]) {
        std::string tipo    = r["tipo"].get<std::string>();
        // puntoSalida puede ser null o no existir en el JSON
        std::string pSalida = (r.contains("puntoSalida") && !r["puntoSalida"].is_null()) ? r["puntoSalida"].get<std::string>() : "";
        // zonaCentro solo existe en RutaCentro
        std::string zona    = r.value("zonaCentro", "");

        Ruta* ruta = nullptr;
        if (tipo == "RutaCentro") {
            ruta = new RutaCentro(
                r["idRuta"].get<int>(),  r["nombre"].get<std::string>(),
                r["origen"].get<std::string>(), r["destino"].get<std::string>(),
                r["estado"].get<bool>(), pSalida, zona
            );
        } else {
            ruta = new RutaBarrio(
                r["idRuta"].get<int>(),  r["nombre"].get<std::string>(),
                r["origen"].get<std::string>(), r["destino"].get<std::string>(),
                r["estado"].get<bool>(), pSalida
            );
        }

        // Carga los IDs de paradas que conforman la ruta
        for (int idP : r["paradas"])
            ruta->agregarParada(idP);

        // Carga los horarios si el objeto existe y tiene contenido, sino usa el global
        if (r.contains("horarios") && r["horarios"].is_object() && !r["horarios"].empty()) {
            const auto& hor = r["horarios"];
            if (hor.contains("salidasBarrio") && hor["salidasBarrio"].is_array())
                for (const auto& h : hor["salidasBarrio"])
                    ruta->getHorario().agregarSalidaBarrio(h.get<std::string>());
            if (hor.contains("salidasUnillanos") && hor["salidasUnillanos"].is_array())
                for (const auto& h : hor["salidasUnillanos"])
                    ruta->getHorario().agregarSalidaUnillanos(h.get<std::string>());
        } else {
            ruta->getHorario() = horarioGlobal;
        }

        lista.push_back(ruta);
    }
    return lista;
}

// Lee usuarios.json y retorna punteros polimorficos al subtipo correcto
std::vector<Usuario*> GestorArchivos::cargarUsuarios() const {
    std::vector<Usuario*> lista;
    std::ifstream archivo(rutaData + "/usuarios.json");
    if (!archivo.is_open()) {
        std::cerr << "\nERROR CRITICO: No se encontro usuarios.json en: " << rutaData << std::endl;
        return lista;
    }

    json j = json::parse(archivo);
    for (const auto& u : j["usuarios"]) {
        std::string tipo = u["tipo"].get<std::string>();
        int         id   = u["idUsuario"].get<int>();
        std::string nom  = u["nombre"].get<std::string>();
        std::string tel  = u["telefono"].get<std::string>();
        std::string cor  = u["correo"].get<std::string>();

        if (tipo == "Estudiante") {
            lista.push_back(new Estudiante(
                id, nom, tel, cor,
                u["codigoEstudiantil"].get<int>(),
                u["facultad"].get<std::string>(),
                u["programa"].get<std::string>(),
                u["semestre"].get<int>(),
                u["carnetActivo"].get<bool>()
            ));
        } else if (tipo == "Administrador") {
            lista.push_back(new Administrador(
                id, nom, tel, cor,
                u["codigoAdmin"].get<int>()
            ));
        } else if (tipo == "Conductor") {
            lista.push_back(new Conductor(
                id, nom, tel, cor,
                u["codigoOperador"].get<int>(),
                u["experiencia"].get<std::string>(),
                u["turno"].get<std::string>(),
                u["busAsignado"].get<int>()
            ));
        }
    }
    return lista;
}

// -------------------------------------------------------------------------
// Metodos de guardado (escritura al JSON)
// -------------------------------------------------------------------------

// Serializa y escribe el vector de incidentes al archivo incidentes.json
void GestorArchivos::guardarIncidentes(const std::vector<Incidente>& incidentes) const {
    json j;
    j["incidentes"] = json::array();
    for (const Incidente& inc : incidentes) {
        j["incidentes"].push_back({
            {"descripcion", inc.getDescripcion()},
            {"estado",      inc.getEstado()},
            {"idIncidente", inc.getIdIncidente()},
            {"tipo",        inc.getTipo()}
        });
    }
    std::ofstream archivo(rutaData + "/incidentes.json");
    if (archivo.is_open()) archivo << j.dump(4);
}

// Serializa y escribe el vector de buses al archivo buses.json
void GestorArchivos::guardarBuses(const std::vector<Bus>& busesList) const {
    json j;
    j["buses"] = json::array();
    for (const Bus& b : busesList) {
        j["buses"].push_back({
            {"capacidadActual", b.getCapacidadActual()},
            {"capacidadMaxima", b.getCapacidadMaxima()},
            {"estado",          b.getEstado()},
            {"idBus",           b.getIdBus()},
            {"placa",           b.getPlaca()}
        });
    }
    std::ofstream archivo(rutaData + "/buses.json");
    if (archivo.is_open()) archivo << j.dump(4);
}

// Serializa y escribe el vector de usuarios al archivo usuarios.json
// Incluye los campos especificos de cada subtipo mediante static_cast
void GestorArchivos::guardarUsuarios(const std::vector<Usuario*>& usuariosList) const {
    json j;
    j["usuarios"] = json::array();
    for (const Usuario* u : usuariosList) {
        // Campos comunes a todos los tipos de usuario
        json obj = {
            {"idUsuario", u->getIdUsuario()},
            {"nombre",    u->getNombre()},
            {"telefono",  u->getTelefono()},
            {"correo",    u->getCorreo()},
            {"tipo",      u->getTipo()}
        };
        // Campos especificos segun el subtipo
        if (u->getTipo() == "Estudiante") {
            const Estudiante* e = static_cast<const Estudiante*>(u);
            obj["codigoEstudiantil"] = e->getCodigoEstudiantil();
            obj["facultad"]          = e->getFacultad();
            obj["programa"]          = e->getPrograma();
            obj["semestre"]          = e->getSemestre();
            obj["carnetActivo"]      = e->isCarnetActivo();
        } else if (u->getTipo() == "Administrador") {
            const Administrador* a = static_cast<const Administrador*>(u);
            obj["codigoAdmin"] = a->getCodigoAdmin();
        } else if (u->getTipo() == "Conductor") {
            const Conductor* c = static_cast<const Conductor*>(u);
            obj["codigoOperador"] = c->getCodigoOperador();
            obj["experiencia"]    = c->getExperiencia();
            obj["turno"]          = c->getTurno();
            obj["busAsignado"]    = c->getBusAsignado();
        }
        j["usuarios"].push_back(obj);
    }
    std::ofstream archivo(rutaData + "/usuarios.json");
    if (archivo.is_open()) archivo << j.dump(2);
}

// Serializa y escribe el vector de rutas al archivo rutas.json pSalida
void GestorArchivos::guardarRutas(const std::vector<Ruta*>& rutasList) const {
    json j;
    j["rutas"] = json::array();
    for (const Ruta* r : rutasList) {
        // Construye el objeto de horarios con sus dos arrays
        json horarios = {
            {"salidasBarrio",    r->getHorario().getSalidasBarrio()},
            {"salidasUnillanos", r->getHorario().getSalidasUnillanos()}
        };
        json obj = {
            {"destino",     r->getDestino()},
            {"estado",      r->getEstado()},
            {"horarios",    horarios},
            {"idRuta",      r->getIdRuta()},
            {"nombre",      r->getNombre()},
            {"origen",      r->getOrigen()},
            {"paradas",     r->getIdsParadas()},
            {"tipo",        r->getTipo()},
            {"puntoSalida", r->getPuntoSalida().empty() ? json(nullptr) : json(r->getPuntoSalida())}
        };
        // Campo exclusivo de RutaCentro
        if (r->getTipo() == "RutaCentro") {
            const RutaCentro* rc = static_cast<const RutaCentro*>(r);
            obj["zonaCentro"] = rc->getZonaCentro();
        }
        j["rutas"].push_back(obj);
    }
    std::ofstream archivo(rutaData + "/rutas.json");
    if (archivo.is_open()) archivo << j.dump(2);
}