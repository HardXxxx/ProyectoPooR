#pragma once
#include "Usuario.h"

// Representa a un estudiante universitario con datos academicos
class Estudiante : public Usuario {
private:
    int codigoEstudiantil;
    std::string facultad;
    std::string programa;
    int semestre;


public:
    Estudiante();
    Estudiante(int id, const std::string& nom, const std::string& tel,
               const std::string& cor, int cod, const std::string& fac,
               const std::string& prog, int sem);

    // Getters
    int getCodigoEstudiantil() const;
    std::string getFacultad() const;
    std::string getPrograma() const;
    int getSemestre() const;

    // Setters
    void setCodigoEstudiantil(int cod);
    void setFacultad(const std::string& fac);
    void setPrograma(const std::string& prog);
    void setSemestre(int sem);

    std::string getEncabezado() const override;
    bool validarCodigo(int c)  const override;
};
