#include "Conductor.h"

Conductor::Conductor() : Usuario(), codigoOperador(0), busAsignado(0) {}

Conductor::Conductor(int id, const std::string& nom, const std::string& tel,
                     const std::string& cor, int codOp, const std::string& exp,
                     const std::string& tur, int busId)
    : Usuario(id, nom, tel, cor, "Conductor"),
      codigoOperador(codOp), experiencia(exp), turno(tur), busAsignado(busId) {}

int         Conductor::getCodigoOperador() const { return codigoOperador; }
std::string Conductor::getExperiencia()    const { return experiencia;    }
std::string Conductor::getTurno()          const { return turno;          }
int         Conductor::getBusAsignado()    const { return busAsignado;    }

void Conductor::setCodigoOperador(int cod)           { codigoOperador = cod; }
void Conductor::setExperiencia(const std::string& e) { experiencia    = e;   }
void Conductor::setTurno(const std::string& t)       { turno          = t;   }
void Conductor::setBusAsignado(int busId)             { busAsignado    = busId; }

std::string Conductor::getEncabezado() const {
    return "Usuario: " + getNombre() + " (Conductor - Turno: " + turno + ")";
}

// Valida que el codigo ingresado coincida con el codigo de operador
bool Conductor::validarCodigo(int c) const {
    return c == codigoOperador;
}
