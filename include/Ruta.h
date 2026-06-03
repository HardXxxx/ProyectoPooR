#pragma once
#include <string>
#include <vector>
#include "HorarioRuta.h"
#include "Parada.h"

// Clase base para todos los tipos de ruta del sistema 

class Ruta {
private:
    int idRuta;
    std::string nombre;
    std::string origen;
    std::string destino;
    bool estado;
    std::string tipo;
    std::string puntoSalida;
    HorarioRuta horario;
    std::vector<int> idsParadas; // IDs de paradas que conforman la ruta

public:
    Ruta();
    Ruta(int id, const std::string& nom, const std::string& orig,
         const std::string& dest, bool est, const std::string& tip,
         const std::string& pSalida);
    virtual ~Ruta() {}

    // Getters
    int getIdRuta() const;
    std::string getNombre() const;
    std::string getOrigen() const;
    std::string getDestino() const;
    bool getEstado() const;
    std::string getTipo() const;
    std::string getPuntoSalida() const;
    HorarioRuta& getHorario(); 
    const HorarioRuta& getHorario() const;
    const std::vector<int>& getIdsParadas() const;

    // Setters
    void setIdRuta(int id);
    void setNombre(const std::string& nom);
    void setOrigen(const std::string& orig);
    void setDestino(const std::string& dest);
    void setEstado(bool est);
    void setTipo(const std::string& tip);
    void setPuntoSalida(const std::string& ps);
    void setIdsParadas(const std::vector<int>& ids);

    // Agrega una parada a la ruta
    void agregarParada(int idParada);

    // Muestra la informacion especifica del tipo de ruta (polimorfismo)
    virtual std::string getInfoAdicional() const = 0;

    // Operador de comparacion por ID de ruta
    bool operator<(const Ruta& otra) const;
    bool operator==(const Ruta& otra) const;
};
