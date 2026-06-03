#include "Estudiante.h"

Estudiante::Estudiante() : Usuario(), codigoEstudiantil(0), semestre(0), carnetActivo(false) {}

Estudiante::Estudiante(int id, const std::string& nom, const std::string& tel,
                       const std::string& cor, int cod, const std::string& fac,
                       const std::string& prog, int sem, bool carnet)
    : Usuario(id, nom, tel, cor, "Estudiante"),
      codigoEstudiantil(cod), facultad(fac), programa(prog),
      semestre(sem), carnetActivo(carnet) {}

int Estudiante::getCodigoEstudiantil() const { return codigoEstudiantil;}
std::string Estudiante::getFacultad() const { return facultad;}
std::string Estudiante::getPrograma() const { return programa;}
int Estudiante::getSemestre() const { return semestre;}
bool Estudiante::isCarnetActivo() const { return carnetActivo;}      

void Estudiante::setCodigoEstudiantil(int cod) { codigoEstudiantil = cod;}
void Estudiante::setFacultad(const std::string& fac) { facultad = fac;}  
void Estudiante::setPrograma(const std::string& prog) { programa = prog;} 
void Estudiante::setSemestre(int sem) { semestre = sem;}
void Estudiante::setCarnetActivo(bool activo) { carnetActivo = activo;} 

std::string Estudiante::getEncabezado() const {  
    return "Usuario: " + getNombre() + " (Estudiante - " + programa + ")";
}
  
// Valida que el codigo ingresado coincida con el codigo estudiantil
bool Estudiante::validarCodigo(int c) const {
    return c == codigoEstudiantil;
}
