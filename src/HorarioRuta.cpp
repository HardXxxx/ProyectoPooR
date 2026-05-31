#include "HorarioRuta.h"

HorarioRuta::HorarioRuta() {}

const std::vector<std::string>& HorarioRuta::getSalidasBarrio() const { return salidasBarrio; }
const std::vector<std::string>& HorarioRuta::getSalidasUnillanos() const { return salidasUnillanos; }

void HorarioRuta::setSalidasBarrio(const std::vector<std::string>& s) { salidasBarrio = s; }
void HorarioRuta::setSalidasUnillanos(const std::vector<std::string>& s) { salidasUnillanos = s; }

void HorarioRuta::agregarSalidaBarrio(const std::string& hora) { salidasBarrio.push_back(hora); }
void HorarioRuta::agregarSalidaUnillanos(const std::string& hora) { salidasUnillanos.push_back(hora); }


// Concatena los horarios del barrio separados por " | "
std::string HorarioRuta::formatearBarrio() const {
    std::string resultado;
    for (size_t i = 0; i < salidasBarrio.size(); ++i) {
        if (i > 0) resultado += " | ";
        resultado += salidasBarrio[i];
    }
    return resultado.empty() ? "Sin horarios asignados" : resultado;
}

// Concatena los horarios de Unillanos separados por " | "
std::string HorarioRuta::formatearUnillanos() const {
    std::string resultado;
    for (size_t i = 0; i < salidasUnillanos.size(); ++i) {
        if (i > 0) resultado += " | ";
        resultado += salidasUnillanos[i];
    }
    return resultado.empty() ? "Sin horarios asignados" : resultado;
}
