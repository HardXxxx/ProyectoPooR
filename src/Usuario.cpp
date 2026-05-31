#include "Usuario.h"

Usuario::Usuario() : idUsuario(0) {}

Usuario::Usuario(int id, const std::string& nom, const std::string& tel,
                 const std::string& cor, const std::string& tip)
    : idUsuario(id), nombre(nom), telefono(tel), correo(cor), tipo(tip) {}

int Usuario::getIdUsuario() const { return idUsuario;}
std::string Usuario::getNombre() const { return nombre;}
std::string Usuario::getTelefono() const { return telefono;}
std::string Usuario::getCorreo() const { return correo;}
std::string Usuario::getTipo() const { return tipo;}

void Usuario::setIdUsuario(int id) { idUsuario = id;}
void Usuario::setNombre(const std::string& nom) { nombre = nom;}
void Usuario::setTelefono(const std::string& tel) { telefono = tel;}
void Usuario::setCorreo(const std::string& cor) { correo = cor;}
void Usuario::setTipo(const std::string& tip) { tipo = tip;}
