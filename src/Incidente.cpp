#include "Incidente.h"

Incidente::Incidente() : idIncidente(0) {}

Incidente::Incidente(int id, const std::string& desc,
                     const std::string& tip, const std::string& est)
    : idIncidente(id), descripcion(desc), tipo(tip), estado(est) {}

int         Incidente::getIdIncidente()  const { return idIncidente; }
std::string Incidente::getDescripcion()  const { return descripcion; }
std::string Incidente::getTipo()         const { return tipo;        }
std::string Incidente::getEstado()       const { return estado;      }

void Incidente::setIdIncidente(int id)              { idIncidente = id;   }
void Incidente::setDescripcion(const std::string& d){ descripcion  = d;   }
void Incidente::setTipo(const std::string& t)       { tipo         = t;   }
void Incidente::setEstado(const std::string& e)     { estado       = e;   }

// Marca el incidente como cerrado/resuelto
void Incidente::cerrar() {
    estado = "Cerrado";
}
