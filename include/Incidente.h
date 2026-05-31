#pragma once
#include <string>

// Registra un incidente reportado en el sistema (mecanico, accidente, etc.)
class Incidente {
private:
    int         idIncidente;
    std::string descripcion;
    std::string tipo;    // "Mecanico", "Accidente", etc.
    std::string estado;  // "Abierto" o "Cerrado"

public:
    Incidente();
    Incidente(int id, const std::string& desc, const std::string& tipo,
              const std::string& est);

    // Getters
    int         getIdIncidente()  const;
    std::string getDescripcion()  const;
    std::string getTipo()         const;
    std::string getEstado()       const;

    // Setters
    void setIdIncidente(int id);
    void setDescripcion(const std::string& desc);
    void setTipo(const std::string& tipo);
    void setEstado(const std::string& est);

    // Cierra el incidente marcandolo como resuelto
    void cerrar();
};
