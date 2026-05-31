#include "Administrador.h"

Administrador::Administrador() : Usuario(), codigoAdmin(0) {}

Administrador::Administrador(int id, const std::string& nom, const std::string& tel,
                             const std::string& cor, int codAdmin)
    : Usuario(id, nom, tel, cor, "Administrador"), codigoAdmin(codAdmin) {}

int  Administrador::getCodigoAdmin() const { return codigoAdmin; }
void Administrador::setCodigoAdmin(int cod) { codigoAdmin = cod; }

std::string Administrador::getEncabezado() const {
    return "Usuario: " + getNombre() + " (Administrador)";
}

// Valida que el codigo ingresado coincida con el codigo de administrador
bool Administrador::validarCodigo(int c) const {
    return c == codigoAdmin;
}
