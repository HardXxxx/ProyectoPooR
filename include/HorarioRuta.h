#pragma once
#include <string>
#include <vector>

// Almacena los horarios de salida desde el barrio y desde Unillanos
class HorarioRuta {
private:
    std::vector<std::string> salidasBarrio;
    std::vector<std::string> salidasUnillanos;

public:
    HorarioRuta();

    // Getters
    const std::vector<std::string>& getSalidasBarrio()    const;
    const std::vector<std::string>& getSalidasUnillanos() const;

    // Setters
    void setSalidasBarrio(const std::vector<std::string>& salidas);
    void setSalidasUnillanos(const std::vector<std::string>& salidas);

    // Agrega un horario individual a cada lista
    void agregarSalidaBarrio(const std::string& hora);
    void agregarSalidaUnillanos(const std::string& hora);

    // Devuelve los horarios formateados en una sola linea separados por " | "
    std::string formatearBarrio()     const;
    std::string formatearUnillanos()  const;
};


