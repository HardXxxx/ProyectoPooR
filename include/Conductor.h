#pragma once
#include "Usuario.h"

// Representa a un conductor con su turno y bus asignado
class Conductor : public Usuario {
private:
    int codigoOperador;
    std::string experiencia;
    std::string turno;
    int busAsignado; // idBus del bus que opera

public:
    Conductor();
    Conductor(int id, const std::string& nom, const std::string& tel,
              const std::string& cor, int codOp, const std::string& exp,
              const std::string& turno, int busId);

    // Getters
    int getCodigoOperador() const;
    std::string getExperiencia() const;
    std::string getTurno() const;
    int getBusAsignado() const;

    // Setters
    void setCodigoOperador(int cod);
    void setExperiencia(const std::string& exp);
    void setTurno(const std::string& turno);
    void setBusAsignado(int busId);

    std::string getEncabezado() const override;
    bool validarCodigo(int c) const override;
};
