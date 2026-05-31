#include "Ruta.h"

Ruta::Ruta() : idRuta(0), estado(false) {}

Ruta::Ruta(int id, const std::string& nom, const std::string& orig,
           const std::string& dest, bool est, const std::string& tip,
           const std::string& pSalida)
    : idRuta(id), nombre(nom), origen(orig), destino(dest),
      estado(est), tipo(tip), puntoSalida(pSalida) {}

int Ruta::getIdRuta() const { return idRuta; }
std::string Ruta::getNombre() const { return nombre; }
std::string Ruta::getOrigen() const { return origen; }
std::string Ruta::getDestino() const { return destino; }
bool Ruta::getEstado() const { return estado; }
std::string Ruta::getTipo() const { return tipo; }
std::string Ruta::getPuntoSalida() const { return puntoSalida; }
HorarioRuta& Ruta::getHorario() { return horario; }
const std::vector<int>& Ruta::getIdsParadas() const { return idsParadas; }

void Ruta::setIdRuta(int id) { idRuta = id; }
void Ruta::setNombre(const std::string& nom) { nombre = nom; }
void Ruta::setOrigen(const std::string& orig) { origen = orig; }
void Ruta::setDestino(const std::string& dest) { destino = dest; }
void Ruta::setEstado(bool est) { estado = est; }
void Ruta::setTipo(const std::string& tip) { tipo = tip; }
void Ruta::setPuntoSalida(const std::string& ps) { puntoSalida = ps; }
void Ruta::setIdsParadas(const std::vector<int>& ids) { idsParadas = ids; }


// Agrega el ID de una parada al vector de paradas de la ruta
void Ruta::agregarParada(int idParada) {
    idsParadas.push_back(idParada);
}

// Compara rutas por su ID numerico
bool Ruta::operator<(const Ruta& otra) const  { return idRuta <  otra.idRuta; }
bool Ruta::operator==(const Ruta& otra) const { return idRuta == otra.idRuta; }
