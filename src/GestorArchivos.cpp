#include "GestorArchivos.h"
#include <fstream>
#include <sstream>
#include <algorithm>

GestorArchivos::GestorArchivos(const std::string& dirData) : rutaData(dirData) {}

// Extrae el valor de cadena de una clave JSON en una linea de texto
std::string GestorArchivos::extraerString(const std::string& linea, const std::string& clave) const {
    std::string busqueda = "\"" + clave + "\"";
    size_t pos = linea.find(busqueda);
    if (pos == std::string::npos) return "";
    pos = linea.find("\"", pos + busqueda.size() + 1);
    if (pos == std::string::npos) return "";
    size_t fin = linea.find("\"", pos + 1);
    if (fin == std::string::npos) return "";
    return linea.substr(pos + 1, fin - pos - 1);
}

// Extrae el valor numerico de una clave JSON en una linea de texto
double GestorArchivos::extraerNumero(const std::string& linea, const std::string& clave) const {
    std::string busqueda = "\"" + clave + "\"";
    size_t pos = linea.find(busqueda);
    if (pos == std::string::npos) return 0.0;
    pos = linea.find(":", pos + busqueda.size());
    if (pos == std::string::npos) return 0.0;
    // Avanza espacios
    ++pos;
    while (pos < linea.size() && (linea[pos] == ' ' || linea[pos] == '\t')) ++pos;
    return std::stod(linea.substr(pos));
}

// Extrae el valor booleano (true/false) de una clave JSON
bool GestorArchivos::extraerBool(const std::string& linea, const std::string& clave) const {
    std::string busqueda = "\"" + clave + "\"";
    size_t pos = linea.find(busqueda);
    if (pos == std::string::npos) return false;
    pos = linea.find(":", pos + busqueda.size());
    if (pos == std::string::npos) return false;
    return linea.find("true", pos) != std::string::npos;
}

// Lee buses.json y construye el vector de buses con sus datos basicos
std::vector<Bus> GestorArchivos::cargarBuses() const {
    std::vector<Bus> lista;
    std::ifstream archivo(rutaData + "/buses.json");
    if (!archivo.is_open()) return lista;

    std::string linea;
    int id = 0; std::string placa; int capMax = 0; int capAct = 0; bool est = false;
    while (std::getline(archivo, linea)) {
        if (linea.find("\"idBus\"")           != std::string::npos) id     = (int)extraerNumero(linea, "idBus");
        if (linea.find("\"placa\"")           != std::string::npos) placa  = extraerString(linea, "placa");
        if (linea.find("\"capacidadMaxima\"") != std::string::npos) capMax = (int)extraerNumero(linea, "capacidadMaxima");
        if (linea.find("\"capacidadActual\"") != std::string::npos) capAct = (int)extraerNumero(linea, "capacidadActual");
        if (linea.find("\"estado\"")          != std::string::npos) est    = extraerBool(linea, "estado");
        // Al cerrar el objeto se acumula el bus
        if (linea.find("}") != std::string::npos && id > 0) {
            lista.push_back(Bus(id, placa, capMax, capAct, est));
            id = 0; placa = ""; capMax = 0; capAct = 0; est = false;
        }
    }
    return lista;
}

// Lee paradas.json y construye el vector de paradas con coordenadas GPS
std::vector<Parada> GestorArchivos::cargarParadas() const {
    std::vector<Parada> lista;
    std::ifstream archivo(rutaData + "/paradas.json");
    if (!archivo.is_open()) return lista;

    std::string linea;
    int id = 0; std::string nombre; double lat = 0, lon = 0, alt = 0;
    bool est = false; int orden = 0; bool dentroUbicacion = false;

    while (std::getline(archivo, linea)) {
        if (linea.find("\"idParada\"")   != std::string::npos) id    = (int)extraerNumero(linea, "idParada");
        if (linea.find("\"nombre\"")     != std::string::npos) nombre= extraerString(linea, "nombre");
        if (linea.find("\"ubicacion\"")  != std::string::npos) dentroUbicacion = true;
        if (dentroUbicacion && linea.find("\"latitud\"")  != std::string::npos) lat   = extraerNumero(linea, "latitud");
        if (dentroUbicacion && linea.find("\"longitud\"") != std::string::npos) lon   = extraerNumero(linea, "longitud");
        if (dentroUbicacion && linea.find("\"altitud\"")  != std::string::npos) { alt = extraerNumero(linea, "altitud"); dentroUbicacion = false; }
        if (linea.find("\"estado\"")     != std::string::npos) est   = extraerBool(linea, "estado");
        if (linea.find("\"ordenEnRuta\"")!= std::string::npos) orden = (int)extraerNumero(linea, "ordenEnRuta");
        if (linea.find("}") != std::string::npos && id > 0 && !nombre.empty()) {
            lista.push_back(Parada(id, nombre, lat, lon, alt, est, orden));
            id = 0; nombre = ""; lat = lon = alt = 0; est = false; orden = 0;
        }
    }
    return lista;
}

// Lee incidentes.json y construye el vector de incidentes
std::vector<Incidente> GestorArchivos::cargarIncidentes() const {
    std::vector<Incidente> lista;
    std::ifstream archivo(rutaData + "/incidentes.json");
    if (!archivo.is_open()) return lista;

    std::string linea;
    int id = 0; std::string desc, tipo, est;
    while (std::getline(archivo, linea)) {
        if (linea.find("\"idIncidente\"") != std::string::npos) id   = (int)extraerNumero(linea, "idIncidente");
        if (linea.find("\"descripcion\"") != std::string::npos) desc = extraerString(linea, "descripcion");
        if (linea.find("\"tipo\"")        != std::string::npos) tipo = extraerString(linea, "tipo");
        if (linea.find("\"estado\"")      != std::string::npos) est  = extraerString(linea, "estado");
        if (linea.find("}") != std::string::npos && id > 0) {
            lista.push_back(Incidente(id, desc, tipo, est));
            id = 0; desc = tipo = est = "";
        }
    }
    return lista;
}

// Lee rutas.json y retorna punteros polimorficos segun el tipo de ruta
std::vector<Ruta*> GestorArchivos::cargarRutas() const {
    std::vector<Ruta*> lista;
    std::ifstream archivo(rutaData + "/rutas.json");
    if (!archivo.is_open()) return lista;

    std::string contenido((std::istreambuf_iterator<char>(archivo)),
                           std::istreambuf_iterator<char>());

    // Procesa el JSON buscando bloques de ruta delimitados por { }
    int id = 0; std::string nombre, origen, destino, tipo, puntoSalida, zonaCentro;
    bool estado = true;
    std::vector<int> idsParadas;
    std::vector<std::string> salidasBarrio, salidasUnillanos;
    bool enHorarios = false, enBarrio = false, enUnillanos = false, enParadas = false;

    std::istringstream ss(contenido);
    std::string linea;
    int profundidad = 0;

    while (std::getline(ss, linea)) {
        for (char c : linea) { if (c == '{') profundidad++; else if (c == '}') profundidad--; }

        if (linea.find("\"idRuta\"")      != std::string::npos) id          = (int)extraerNumero(linea, "idRuta");
        if (linea.find("\"nombre\"")      != std::string::npos) nombre      = extraerString(linea, "nombre");
        if (linea.find("\"origen\"")      != std::string::npos) origen      = extraerString(linea, "origen");
        if (linea.find("\"destino\"")     != std::string::npos) destino     = extraerString(linea, "destino");
        if (linea.find("\"tipo\"")        != std::string::npos) tipo        = extraerString(linea, "tipo");
        if (linea.find("\"puntoSalida\"") != std::string::npos) puntoSalida = extraerString(linea, "puntoSalida");
        if (linea.find("\"zonaCentro\"")  != std::string::npos) zonaCentro  = extraerString(linea, "zonaCentro");
        if (linea.find("\"estado\"")      != std::string::npos) estado      = extraerBool(linea, "estado");

        // Detecta seccion de horarios y sus sublistas
        if (linea.find("\"configuracionHorarios\"") != std::string::npos || linea.find("\"horarios\"") != std::string::npos) enHorarios = true;
        if (enHorarios && linea.find("\"salidasBarrio\"")    != std::string::npos) { enBarrio    = true; enUnillanos = false; }
        if (enHorarios && linea.find("\"salidasUnillanos\"") != std::string::npos) { enUnillanos = true; enBarrio    = false; }

        // Detecta seccion de paradas
        if (linea.find("\"paradas\"") != std::string::npos) { enParadas = true; enHorarios = false; enBarrio = false; enUnillanos = false; }

        // Extrae horas individuales dentro de los arrays de horarios
        if (enBarrio && linea.find("\"") != std::string::npos) {
            size_t a = linea.find("\""), b = linea.rfind("\"");
            if (a != b) { std::string h = linea.substr(a + 1, b - a - 1); if (h.size() > 4) salidasBarrio.push_back(h); }
        }
        if (enUnillanos && linea.find("\"") != std::string::npos) {
            size_t a = linea.find("\""), b = linea.rfind("\"");
            if (a != b) { std::string h = linea.substr(a + 1, b - a - 1); if (h.size() > 4) salidasUnillanos.push_back(h); }
        }

        // Extrae IDs de paradas del array
        if (enParadas) {
            for (size_t i = 0; i < linea.size(); ++i) {
                if (std::isdigit(linea[i])) {
                    size_t fin = i;
                    while (fin < linea.size() && std::isdigit(linea[fin])) fin++;
                    idsParadas.push_back(std::stoi(linea.substr(i, fin - i)));
                    i = fin;
                }
            }
            if (linea.find("]") != std::string::npos) enParadas = false;
        }

        // Al regresar a profundidad de objeto raiz se construye la ruta
        if (profundidad == 2 && id > 0 && !nombre.empty()) {
            Ruta* r = nullptr;
            if (tipo == "RutaCentro") {
                r = new RutaCentro(id, nombre, origen, destino, estado, puntoSalida, zonaCentro);
            } else {
                r = new RutaBarrio(id, nombre, origen, destino, estado, puntoSalida);
            }
            r->setIdsParadas(idsParadas);
            r->getHorario().setSalidasBarrio(salidasBarrio);
            r->getHorario().setSalidasUnillanos(salidasUnillanos);
            lista.push_back(r);
            // Reinicia variables para la siguiente ruta
            id = 0; nombre = origen = destino = tipo = puntoSalida = zonaCentro = "";
            estado = true; idsParadas.clear(); salidasBarrio.clear(); salidasUnillanos.clear();
            enHorarios = enBarrio = enUnillanos = enParadas = false;
        }
    }
    return lista;
}

// Lee usuarios.json y retorna punteros polimorficos al tipo correcto de usuario
std::vector<Usuario*> GestorArchivos::cargarUsuarios() const {
    std::vector<Usuario*> lista;
    std::ifstream archivo(rutaData + "/usuarios.json");
    if (!archivo.is_open()) return lista;

    std::string linea;
    int id = 0, codEst = 0, codAdmin = 0, codOp = 0, busId = 0, semestre = 0;
    std::string nombre, telefono, correo, tipo, facultad, programa, experiencia, turno;
    bool carnetActivo = false;

    while (std::getline(archivo, linea)) {
        if (linea.find("\"idUsuario\"")         != std::string::npos) id        = (int)extraerNumero(linea, "idUsuario");
        if (linea.find("\"nombre\"")            != std::string::npos) nombre    = extraerString(linea, "nombre");
        if (linea.find("\"telefono\"")          != std::string::npos) telefono  = extraerString(linea, "telefono");
        if (linea.find("\"correo\"")            != std::string::npos) correo    = extraerString(linea, "correo");
        if (linea.find("\"tipo\"")              != std::string::npos) tipo      = extraerString(linea, "tipo");
        if (linea.find("\"facultad\"")          != std::string::npos) facultad  = extraerString(linea, "facultad");
        if (linea.find("\"programa\"")          != std::string::npos) programa  = extraerString(linea, "programa");
        if (linea.find("\"experiencia\"")       != std::string::npos) experiencia = extraerString(linea, "experiencia");
        if (linea.find("\"turno\"")             != std::string::npos) turno     = extraerString(linea, "turno");
        if (linea.find("\"semestre\"")          != std::string::npos) semestre  = (int)extraerNumero(linea, "semestre");
        if (linea.find("\"codigoEstudiantil\"") != std::string::npos) codEst   = (int)extraerNumero(linea, "codigoEstudiantil");
        if (linea.find("\"codigoAdmin\"")       != std::string::npos) codAdmin = (int)extraerNumero(linea, "codigoAdmin");
        if (linea.find("\"codigoOperador\"")    != std::string::npos) codOp    = (int)extraerNumero(linea, "codigoOperador");
        if (linea.find("\"busAsignado\"")       != std::string::npos) busId    = (int)extraerNumero(linea, "busAsignado");
        if (linea.find("\"carnetActivo\"")      != std::string::npos) carnetActivo = extraerBool(linea, "carnetActivo");

        // Al cerrar el objeto JSON se construye el usuario segun su tipo
        if (linea.find("}") != std::string::npos && id > 0 && !nombre.empty()) {
            if (tipo == "Estudiante")
                lista.push_back(new Estudiante(id, nombre, telefono, correo, codEst, facultad, programa, semestre, carnetActivo));
            else if (tipo == "Administrador")
                lista.push_back(new Administrador(id, nombre, telefono, correo, codAdmin));
            else if (tipo == "Conductor")
                lista.push_back(new Conductor(id, nombre, telefono, correo, codOp, experiencia, turno, busId));
            id = 0; nombre = telefono = correo = tipo = facultad = programa = experiencia = turno = "";
            codEst = codAdmin = codOp = busId = semestre = 0; carnetActivo = false;
        }
    }
    return lista;
}

// Escribe el vector de incidentes actualizado al archivo JSON
void GestorArchivos::guardarIncidentes(const std::vector<Incidente>& incidentes) const {
    std::ofstream archivo(rutaData + "/incidentes.json");
    if (!archivo.is_open()) return;
    archivo << "{\n    \"incidentes\": [\n";
    for (size_t i = 0; i < incidentes.size(); ++i) {
        const Incidente& inc = incidentes[i];
        archivo << "        {\n"
                << "            \"descripcion\": \"" << inc.getDescripcion() << "\",\n"
                << "            \"estado\": \""      << inc.getEstado()      << "\",\n"
                << "            \"idIncidente\": "   << inc.getIdIncidente() << ",\n"
                << "            \"tipo\": \""        << inc.getTipo()        << "\"\n"
                << "        }" << (i + 1 < incidentes.size() ? "," : "") << "\n";
    }
    archivo << "    ]\n}\n";
}

// Escribe el vector de buses actualizado al archivo JSON
void GestorArchivos::guardarBuses(const std::vector<Bus>& busesList) const {
    std::ofstream archivo(rutaData + "/buses.json");
    if (!archivo.is_open()) return;
    archivo << "{\n    \"buses\": [\n";
    for (size_t i = 0; i < busesList.size(); ++i) {
        const Bus& b = busesList[i];
        archivo << "        {\n"
                << "            \"capacidadActual\": " << b.getCapacidadActual() << ",\n"
                << "            \"capacidadMaxima\": " << b.getCapacidadMaxima() << ",\n"
                << "            \"estado\": "          << (b.getEstado() ? "true" : "false") << ",\n"
                << "            \"idBus\": "           << b.getIdBus()           << ",\n"
                << "            \"placa\": \""         << b.getPlaca()           << "\"\n"
                << "        }" << (i + 1 < busesList.size() ? "," : "") << "\n";
    }
    archivo << "    ]\n}\n";
}

// Escribe el vector de usuarios actualizado al archivo JSON
void GestorArchivos::guardarUsuarios(const std::vector<Usuario*>& usuariosList) const {
    std::ofstream archivo(rutaData + "/usuarios.json");
    if (!archivo.is_open()) return;
    archivo << "{\n  \"usuarios\": [\n";
    for (size_t i = 0; i < usuariosList.size(); ++i) {
        Usuario* u = usuariosList[i];
        archivo << "    {\n"
                << "      \"idUsuario\": "  << u->getIdUsuario() << ",\n"
                << "      \"nombre\": \""   << u->getNombre()    << "\",\n"
                << "      \"telefono\": \"" << u->getTelefono()  << "\",\n"
                << "      \"correo\": \""   << u->getCorreo()    << "\",\n"
                << "      \"tipo\": \""     << u->getTipo()      << "\"";
        if (u->getTipo() == "Estudiante") {
            Estudiante* e = static_cast<Estudiante*>(u);
            archivo << ",\n      \"codigoEstudiantil\": " << e->getCodigoEstudiantil()
                    << ",\n      \"facultad\": \""  << e->getFacultad()  << "\""
                    << ",\n      \"programa\": \""  << e->getPrograma()  << "\""
                    << ",\n      \"semestre\": "    << e->getSemestre()
                    << ",\n      \"carnetActivo\": "<< (e->isCarnetActivo() ? "true" : "false");
        } else if (u->getTipo() == "Administrador") {
            Administrador* a = static_cast<Administrador*>(u);
            archivo << ",\n      \"codigoAdmin\": " << a->getCodigoAdmin();
        } else if (u->getTipo() == "Conductor") {
            Conductor* c = static_cast<Conductor*>(u);
            archivo << ",\n      \"codigoOperador\": " << c->getCodigoOperador()
                    << ",\n      \"experiencia\": \""  << c->getExperiencia() << "\""
                    << ",\n      \"turno\": \""        << c->getTurno()       << "\""
                    << ",\n      \"busAsignado\": "    << c->getBusAsignado();
        }
        archivo << "\n    }" << (i + 1 < usuariosList.size() ? "," : "") << "\n";
    }
    archivo << "  ]\n}\n";
}

// Escribe el vector de rutas actualizado al archivo JSON
void GestorArchivos::guardarRutas(const std::vector<Ruta*>& rutasList) const {
    std::ofstream archivo(rutaData + "/rutas.json");
    if (!archivo.is_open()) return;
    archivo << "{\n  \"rutas\": [\n";
    for (size_t i = 0; i < rutasList.size(); ++i) {
        Ruta* r = rutasList[i];
        archivo << "    {\n"
                << "      \"destino\": \""     << r->getDestino()     << "\",\n"
                << "      \"estado\": "        << (r->getEstado() ? "true" : "false") << ",\n"
                << "      \"horarios\": {\n"
                << "        \"salidasBarrio\": [";
        const std::vector<std::string>& sb = r->getHorario().getSalidasBarrio();
        for (size_t j = 0; j < sb.size(); ++j) archivo << "\"" << sb[j] << "\"" << (j+1<sb.size()?",":"");
        archivo << "],\n        \"salidasUnillanos\": [";
        const std::vector<std::string>& su = r->getHorario().getSalidasUnillanos();
        for (size_t j = 0; j < su.size(); ++j) archivo << "\"" << su[j] << "\"" << (j+1<su.size()?",":"");
        archivo << "]\n      },\n"
                << "      \"idRuta\": "        << r->getIdRuta()      << ",\n"
                << "      \"nombre\": \""      << r->getNombre()      << "\",\n"
                << "      \"origen\": \""      << r->getOrigen()      << "\",\n";
        const std::vector<int>& ids = r->getIdsParadas();
        archivo << "      \"paradas\": [";
        for (size_t j = 0; j < ids.size(); ++j) archivo << ids[j] << (j+1<ids.size()?", ":"");
        archivo << "],\n"
                << "      \"tipo\": \""        << r->getTipo()        << "\",\n"
                << "      \"puntoSalida\": ";
        if (r->getPuntoSalida().empty()) archivo << "null";
        else archivo << "\"" << r->getPuntoSalida() << "\"";
        if (r->getTipo() == "RutaCentro") {
            RutaCentro* rc = static_cast<RutaCentro*>(r);
            archivo << ",\n      \"zonaCentro\": \"" << rc->getZonaCentro() << "\"";
        }
        archivo << "\n    }" << (i + 1 < rutasList.size() ? "," : "") << "\n";
    }
    archivo << "  ]\n}\n";
}
