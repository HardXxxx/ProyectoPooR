#pragma once
#include "Usuario.h"

// Representa al administrador del sistema con acceso total
class Administrador : public Usuario {
private:
    int codigoAdmin;

public:
    Administrador();
    Administrador(int id, const std::string& nom, const std::string& tel,
                  const std::string& cor, int codAdmin);

    // Getters
    int getCodigoAdmin() const;

    // Setters
    void setCodigoAdmin(int cod);

    std::string getEncabezado() const override;
    bool validarCodigo(int c) const override;
};
