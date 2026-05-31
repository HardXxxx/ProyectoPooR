#pragma once
#include <string>

// Clase base abstracta para todos los tipos de usuario del sistema
class Usuario {
private:
    int    idUsuario;
    std::string nombre;
    std::string telefono;
    std::string correo;
    std::string tipo;

public:
    Usuario();
    Usuario(int id, const std::string& nom, const std::string& tel,
            const std::string& cor, const std::string& tip);
    virtual ~Usuario() {}

    // Getters
    int         getIdUsuario()  const;
    std::string getNombre()     const;
    std::string getTelefono()   const;
    std::string getCorreo()     const;
    std::string getTipo()       const;

    // Setters
    void setIdUsuario(int id);
    void setNombre(const std::string& nom);
    void setTelefono(const std::string& tel);
    void setCorreo(const std::string& cor);
    void setTipo(const std::string& tip);

    // Muestra el encabezado de sesion segun el tipo de usuario (polimorfismo)
    virtual std::string getEncabezado() const = 0;

    // Verifica si el codigo de acceso ingresado es valido para este usuario
    virtual bool validarCodigo(int codigo) const = 0;
};
