# Documentación Técnica y Educativa Completa — Sistema de Gestión de Rutas de Transporte Universitario

> **Universidad de los Llanos — Villavicencio, Meta, Colombia**  
> **Lenguaje:** C++17  
> **Plataforma:** Windows (API de consola)  
> **Compilación:** CMake ≥ 3.16 / MinGW  
> **Propósito:** Sistema de gestión de rutas de transporte universitario con perfiles de Estudiante, Conductor y Administrador  
> **Perfil del lector:** Estudiante sin conocimientos previos del proyecto  
> **Tono:** Profesor universitario — se asume cero conocimiento, se explica todo desde los fundamentos

---

## Tabla de Contenidos

1. [Visión General y Arquitectura](#sección-1-visión-general-y-arquitectura)
2. [Estructura de Carpetas](#sección-2-estructura-de-carpetas)
3. [Explicación Archivo por Archivo](#sección-3-explicación-archivo-por-archivo)
4. [Análisis de Clases y Estructuras](#sección-4-análisis-de-clases-y-estructuras)
5. [Análisis Línea por Línea (Bloques Lógicos)](#sección-5-análisis-línea-por-línea-bloques-lógicos)
6. [Guía de Sintaxis del Lenguaje (C++)](#sección-6-guía-de-sintaxis-del-lenguaje-c)
7. [Guía de la STL y Librerías Estándar](#sección-7-guía-de-la-stl-y-librerías-estándar)
8. [APIs Externas y Dependencias](#sección-8-apis-externas-y-dependencias)
9. [Desglose de Instrucciones Complejas](#sección-9-desglose-de-instrucciones-complejas)
10. [Flujo de Ejecución Completo (Call Graph)](#sección-10-flujo-de-ejecución-completo-call-graph)
11. [Conceptos Teóricos y Buenas Prácticas](#sección-11-conceptos-teóricos-y-buenas-prácticas)
12. [Resumen y Ruta de Estudio](#sección-12-resumen-y-ruta-de-estudio)
13. [Análisis Focalizado: GestorArchivos.cpp](#sección-13-análisis-focalizado-gestorarchivoscpp)
14. [Análisis Focalizado: SistemaTransporte.cpp](#sección-14-análisis-focalizado-sistemtransportecpp)
15. [Vulnerabilidades y Debilidades del Diseño](#sección-15-vulnerabilidades-y-debilidades-del-diseño)

---

## Credenciales de Prueba (tomadas de usuarios.json)

| Perfil | Nombre | Código | Bus Asignado |
|--------|--------|--------|--------------|
| Estudiante | Pedro Rodríguez | 2024001 | — |
| Estudiante | Breiner Montaña | 160005557 | — |
| Estudiante | Danna Martinez | 160005552 | — |
| Estudiante | Harold Padilla | 160005567 | — |
| Conductor | Carlos García | 101 | Bus 1 (VST-001) |
| Conductor | Juan Pérez | 102 | Bus 2 (VST-002) |
| Administrador | María López | 1 | — |

---

## Funcionalidades Principales

- **Estudiante**: Visor paginado de rutas y paradas, reserva de asientos en buses con capacidad disponible.
- **Conductor**: Panel con bus asignado, incidentes, horarios, simulación GPS con distancia Haversine en tiempo real.
- **Administrador**: Gestión de rutas (activar/desactivar), usuarios (solo lectura), flota (modificar capacidad), horarios (agregar salidas) e incidentes (cerrar); cambios reflejados en JSON inmediatamente.
- **Tiempo real**: Hora y fecha obtenidas con `GetLocalTime` de Windows.h; avanzan en cada refresco de pantalla.
- **GPS**: Cálculo de distancia (Haversine) y tiempo estimado de llegada del bus a cada parada (velocidad 30 km/h).

---

# Sección 1: Visión General y Arquitectura

## 1.1 ¿Qué hace el sistema?

El **SistemaTransporte** es una aplicación de **consola para Windows** que permite gestionar las rutas de transporte que llevan a los estudiantes desde diversos barrios de Villavicencio hasta el campus de la Universidad de los Llanos (Unillanos). Tiene **tres perfiles de usuario**:

| Perfil | ¿Qué puede hacer? |
|--------|-------------------|
| **Estudiante** | Ver listado de rutas activas, consultar paradas de cada ruta, ver horarios de salida (barrio→universidad y viceversa), y **reservar un asiento** en un bus disponible. |
| **Conductor** | Ver su bus asignado, consultar la ruta, ver en **tiempo real** distancia y tiempo estimado hasta cada parada (simulación GPS), reportar incidentes (mecánicos, accidentes), y simular llegada a una parada. |
| **Administrador** | Activar/desactivar rutas, visualizar usuarios, modificar capacidad de buses, agregar horarios de salida, y cerrar incidentes. |

Los datos se guardan de forma **permanente** en archivos **JSON** dentro de `data/`. Si el programa se cierra y se vuelve a abrir, toda la información persiste.

## 1.2 Arquitectura en tres capas

```
 ┌──────────────────────────────────────────────────────────────┐
 │                     INTERFAZ DE USUARIO (UI)                 │
 │   main.cpp -> SistemaTransporte.cpp (menús, colores, input)   │
 │   Dependencia: windows.h (API de consola de Windows)         │
 └──────────────────────────┬───────────────────────────────────┘
                            │  llamadas a métodos
                            ▼
 ┌──────────────────────────────────────────────────────────────┐
 │                   LÓGICA DE NEGOCIO (Core)                   │
 │   Coordenada, UbicacionGPS, Bus, Parada, Ruta,              │
 │   RutaBarrio, RutaCentro, Usuario, Estudiante,              │
 │   Conductor, Administrador, Incidente, HorarioRuta           │
 │   Lenguaje: C++ puro (clases, herencia, polimorfismo)       │
 └──────────────────────────┬───────────────────────────────────┘
                            │  carga/guarda datos
                            ▼
 ┌──────────────────────────────────────────────────────────────┐
 │                   PERSISTENCIA DE DATOS                      │
 │   GestorArchivos.cpp + nlohmann/json (json.hpp)             │
 │   Archivos: buses.json, paradas.json, incidentes.json,      │
 │             rutas.json, horarios.json, usuarios.json         │
 └──────────────────────────────────────────────────────────────┘
```

- **Capa 1 — UI**: `main.cpp` crea un objeto `SistemaTransporte` y llama a `iniciar()`. Maneja menús, entrada del usuario, colores y limpieza de pantalla.
- **Capa 2 — Lógica de Negocio**: Clases que representan conceptos del mundo real. Aquí ocurren los cálculos (distancia Haversine, tiempo estimado, simulación) y reglas de negocio (validar código, verificar capacidad).
- **Capa 3 — Persistencia**: `GestorArchivos` lee/escribe archivos JSON usando `nlohmann/json`.

## 1.3 Flujo General de Alto Nivel

```
  INICIO
    │
    ▼
  main() crea SistemaTransporte("data")
    │
    ▼
  SistemaTransporte::iniciar()
    │
    ├── 1. Cargar datos desde JSON:
    │       gestor.cargarBuses()       ->   buses[]
    │       gestor.cargarParadas()     ->   paradas[]
    │       gestor.cargarIncidentes()  ->   incidentes[]
    │       gestor.cargarRutas()       ->   rutas[]
    │       gestor.cargarUsuarios()    ->   usuarios[]
    │
    ├── 2. Configurar consola:
    │       SetConsoleTitle("...")
    │       SetConsoleOutputCP(65001)   <- UTF-8
    │
    └── 3. BUCLE PRINCIPAL (while true):
            ├── mostrarMenuPrincipal()
            │     └── [1] Estudiante  [2] Conductor  [3] Administrador  [4] Salir
            ├── autenticarUsuario(opción)
            │     └── Pide código -> busca en usuarios[] -> si no existe: error
            └── Según el perfil:
                  ├── panelEstudiante()
                  ├── panelConductor()
                  └── panelAdministrador()

  FIN (opción 4)
    └── ~SistemaTransporte()
          └── delete cada Ruta*
          └── delete cada Usuario*
```

## 1.4 Diagrama de Interacción Completo

```
main.cpp
  └-> SistemaTransporte::iniciar()
       ├-> GestorArchivos::cargarBuses()     ->   buses.json   -> vector<Bus>
       ├-> GestorArchivos::cargarParadas()   -> paradas.json   -> vector<Parada>
       ├-> GestorArchivos::cargarIncidentes()-> incidentes.json -> vector<Incidente>
       ├-> GestorArchivos::cargarRutas()     ->   rutas.json   -> vector<Ruta*>
       │   ├-> cargo HorarioRuta global de horarios.json
       │   └-> por cada ruta: new RutaBarrio() o new RutaCentro()
       └-> GestorArchivos::cargarUsuarios() -> usuarios.json  -> vector<Usuario*>
            ├-> new Estudiante(...)
            ├-> new Conductor(...)
            └-> new Administrador(...)
       └-> Loop: menu -> autenticar -> panel
            ├-> panelEstudiante()
            │   └-> mostrar rutas -> seleccionar -> ver paradas/horarios -> reservar asiento
            └-> panelConductor()
            │   └-> buscar bus asignado -> mostrar ruta -> proximidad GPS -> simular llegada -> reportar incidente
            └-> panelAdministrador()
                └-> gestionar rutas/usuarios/buses/horarios/incidentes -> guardar cambios
```

---

# Sección 2: Estructura de Carpetas

## 2.1 Árbol del proyecto

```
SistemaTransporte/
├── .gitignore                <- Indica a Git qué archivos ignorar
├── CMakeLists.txt            <- Instrucciones de compilación con CMake
├── README.md                 <- Documentación breve
├── main.cpp                  <- Punto de entrada del programa
├── include/                  <- CABECERAS (.h) — declaraciones de clases
│   ├── Coordenada.h
│   ├── UbicacionGPS.h
│   ├── Usuario.h
│   ├── Estudiante.h
│   ├── Conductor.h
│   ├── Administrador.h
│   ├── Bus.h
│   ├── Parada.h
│   ├── Incidente.h
│   ├── HorarioRuta.h
│   ├── Ruta.h
│   ├── RutaBarrio.h
│   ├── RutaCentro.h
│   ├── GestorArchivos.h
│   ├── SistemaTransporte.h
│   └── json.hpp               <- Librería nlohmann/json (26.000 líneas, terceros)
├── src/                       <- IMPLEMENTACIONES (.cpp) — código de cada método
│   ├── Coordenada.cpp
│   ├── UbicacionGPS.cpp
│   ├── Usuario.cpp
│   ├── Estudiante.cpp
│   ├── Conductor.cpp
│   ├── Administrador.cpp
│   ├── Bus.cpp
│   ├── Parada.cpp
│   ├── Incidente.cpp
│   ├── HorarioRuta.cpp
│   ├── Ruta.cpp
│   ├── RutaBarrio.cpp
│   ├── RutaCentro.cpp
│   ├── GestorArchivos.cpp
│   └── SistemaTransporte.cpp
└── data/                      <- DATOS PERSISTENTES (JSON)
    ├── buses.json
    ├── horarios.json
    ├── incidentes.json
    ├── paradas.json
    ├── rutas.json
    └── usuarios.json
```

## 2.2 ¿Por qué separar `.h` de `.cpp`?

En C++ (a diferencia de Java o Python):

- **Archivos de cabecera (`.h`)**: Contienen las **declaraciones** de las clases. Es como el **índice** de un libro: dice qué métodos existen, qué parámetros reciben y qué devuelven, pero no contiene el código de cada método.
- **Archivos de implementación (`.cpp`)**: Contienen el **cuerpo** de cada método. Es el **contenido** del libro: el código real que se ejecuta.

**Razones:**
1. **Tiempo de compilación**: Si todo estuviera en un solo archivo, cualquier cambio obligaría a recompilar todo. Al separar, solo se recompila el `.cpp` modificado.
2. **Organización**: Se puede leer la interfaz de una clase sin ver los detalles de implementación.
3. **Reutilización**: Otros proyectos pueden incluir solo los `.h` y enlazar los objetos compilados.
4. **Ocultación de información**: Quien usa una clase solo necesita saber qué métodos tiene (el `.h`), no cómo están implementados (el `.cpp`).

## 2.3 Responsabilidad de cada directorio

| Directorio | Función | ¿Qué pasaría si falta? |
|------------|---------|------------------------|
| `include/` | Definiciones de clases + librería json | Error de compilación total: el compilador no sabría qué es `Bus`, `Ruta`, etc. |
| `src/` | Implementación de cada método | Error del enlazador (linker): "símbolo sin definir" |
| `data/` | Datos iniciales en JSON | El programa se ejecuta pero con listas vacías |
| `main.cpp` | Punto de entrada | No hay `main` -> no se genera ejecutable |
| `CMakeLists.txt` | Instrucciones de compilación | No se puede compilar con CMake |

---

# Sección 3: Explicación Archivo por Archivo

## 3.1 `main.cpp`

**Ubicación:** `SistemaTransporte/main.cpp`  
**Propósito:** Punto de entrada del programa.

```cpp
#include <iostream>
#include <stdexcept>
#include "SistemaTransporte.h"

int main() {
    try {
        SistemaTransporte sistema("data");
        sistema.iniciar();
    } catch (const std::exception& ex) {
        std::cerr << "[ERROR CRITICO] " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
```

**Dependencias:**
- `<iostream>` -> `std::cerr` (salida de error estándar)
- `<stdexcept>` -> `std::exception` (clase base de excepciones)
- `"SistemaTransporte.h"` -> clase `SistemaTransporte`

**Explicación:**
- `try { ... } catch (...) { ... }`: Manejo de excepciones. Si algo inesperado ocurre, el flujo salta al `catch`.
- `SistemaTransporte sistema("data")`: Crea un objeto `sistema` pasando `"data"` como ruta de los archivos JSON.
- `sistema.iniciar()`: Arranca el menú principal del sistema.

## 3.2 `CMakeLists.txt`

**Ubicación:** `SistemaTransporte/CMakeLists.txt`  
**Propósito:** Archivo de configuración de CMake.

| Línea | Instrucción | Significado |
|-------|-------------|-------------|
| 1 | `cmake_minimum_required(VERSION 3.16)` | Requiere CMake versión 3.16 como mínimo |
| 2 | `project(SistemaTransporte CXX)` | El proyecto se llama "SistemaTransporte" y usa C++ |
| 4 | `set(CMAKE_CXX_STANDARD 17)` | Usa el estándar C++17 |
| 5 | `set(CMAKE_CXX_STANDARD_REQUIRED ON)` | Si no hay soporte para C++17, error de compilación |
| 8 | `include_directories(include)` | Los headers se buscarán en la carpeta `include/` |
| 11-28 | `set(SOURCES ...)` | Lista de todos los archivos `.cpp` a compilar |
| 30 | `add_executable(SistemaTransporte ${SOURCES})` | Crea el ejecutable con todos los fuentes |
| 33-37 | `add_custom_command(...)` | Después de compilar, copia la carpeta `data/` junto al ejecutable |

## 3.3 `include/Coordenada.h` + `src/Coordenada.cpp`

**Propósito:** Representar un punto geográfico (latitud/longitud). Es la **clase base de UbicacionGPS** y se usa por **composición** dentro de `Parada`.

```cpp
class Coordenada {
private:
    double latitud;
    double longitud;
public:
    Coordenada();                          // Constructor por defecto (0,0)
    Coordenada(double lat, double lon);    // Constructor paramétrico
    double getLatitud() const;
    double getLongitud() const;
    void setLatitud(double lat);
    void setLongitud(double lon);
    double distanciaHacia(const Coordenada& otra) const;  // <- FÓRMULA DE HAVERSINE
};
```

**`distanciaHacia()`** implementa la **fórmula del Haversine**:
```cpp
double dLat = (otra.latitud - latitud) * PI / 180.0;
double dLon = (otra.longitud - longitud) * PI / 180.0;
double a = sin(dLat/2)^2 + cos(lat) * cos(otra.lat) * sin(dLon/2)^2;
double c = 2 * atan2(sqrt(a), sqrt(1-a));
return RADIO_TIERRA * c;  // 6371000 metros
```

**¿Dónde se usa Coordenada?**
- **Composición en `Parada`**: `Parada::ubicacion` (tipo `Coordenada`) -> almacena dónde está cada parada
- **Herencia en `UbicacionGPS`**: `UbicacionGPS : public Coordenada` -> añade altitud y velocidad
- **En `Bus::tiempoHastaParada()`**: crea una `Coordenada destino(lat, lon)` para medir distancia
- **En `Parada::distanciaA()`**: crea `Coordenada otro(lat, lon)` y llama a `ubicacion.distanciaHacia(otro)`

## 3.4 `include/UbicacionGPS.h` + `src/UbicacionGPS.cpp`

**Propósito:** Extiende `Coordenada` agregando altitud y velocidad para cálculos de tiempo de viaje.

```cpp
class UbicacionGPS : public Coordenada {  // <- HEREDA DE Coordenada
private:
    double altitud;
    double velocidadKmh;        // 30 km/h por defecto
public:
    UbicacionGPS(double lat, double lon, double alt, double vel = 30.0);
    double getAltitud() const;
    double getVelocidadKmh() const;
    void setAltitud(double alt);
    void setVelocidadKmh(double vel);
    double tiempoEstimadoMinutos(double distanciaMetros) const;
};
```

**`tiempoEstimadoMinutos()`**: `(distanciaKm / velocidadKmh) * 60.0` -> simple regla de 3.

**¿Dónde se usa UbicacionGPS?**
- **Como puntero dinámico en `Bus`**: `UbicacionGPS* ubicacion` -> se crea con `new` en `setUbicacion()` y se destruye en el destructor de Bus
- **En `Bus::simularMovimiento()`**: mueve el bus un 5% hacia una coordenada destino
- **En `Bus::tiempoHastaParada()`**: calcula minutos hasta una parada
- **En `SistemaTransporte::mostrarProximidadBus()`**: muestra distancia y tiempo a cada parada

## 3.5 `include/Usuario.h` + `src/Usuario.cpp`

**Propósito:** Clase **abstracta base** para los 3 roles del sistema. Define la interfaz polimórfica.

```cpp
class Usuario {
private:
    int idUsuario;
    std::string nombre, telefono, correo, tipo;  // tipo = "Estudiante"|"Conductor"|"Administrador"
public:
    virtual ~Usuario() {}                         // Destructor virtual para polimorfismo
    virtual std::string getEncabezado() const = 0;  // <- PURO: cada rol muestra info diferente
    virtual bool validarCodigo(int codigo) const = 0; // <- PURO: cada rol valida su código
    // Getters/Setters...
};
```

**¿Dónde se usa Usuario?**
- **Como base polimórfica**: `std::vector<Usuario*> usuarios` en SistemaTransporte
- **En `autenticarUsuario()`**: itera sobre `usuarios` y llama a `u->validarCodigo(codigo)` y `u->getTipo()`
- **En `imprimirEncabezado()`**: `usuarioActual->getEncabezado()` muestra el encabezado personalizado
- **En `gestionarUsuarios()`**: itera mostrando `u->getNombre()` y `u->getTipo()`

## 3.6 `include/Estudiante.h` + `src/Estudiante.cpp`

```cpp
class Estudiante : public Usuario {
private:
    int codigoEstudiantil;
    std::string facultad, programa;
    int semestre;
public:
    std::string getEncabezado() const override {
        return "Usuario: " + getNombre() + " (Estudiante - " + programa + ")";
    }
    bool validarCodigo(int c) const override {
        return c == codigoEstudiantil;
    }
};
```

**¿Dónde se usa Estudiante?**
- **En `GestorArchivos::cargarUsuarios()`**: cuando `tipo == "Estudiante"`, crea `new Estudiante(...)` y lo pushea al vector
- **En `panelEstudiante()`**: el flujo que permite ver rutas y reservar asientos
- **En `GestorArchivos::guardarUsuarios()`**: usa `static_cast<const Estudiante*>(u)` para acceder a campos específicos

## 3.7 `include/Conductor.h` + `src/Conductor.cpp`

```cpp
class Conductor : public Usuario {
private:
    int codigoOperador;
    std::string experiencia, turno;  // turno = "Mañana"|"Tarde"
    int busAsignado;         // <- id del bus que conduce (FK a Bus.idBus)
public:
    std::string getEncabezado() const override {
        return "Usuario: " + getNombre() + " (Conductor - Turno: " + turno + ")";
    }
    bool validarCodigo(int c) const override {
        return c == codigoOperador;
    }
};
```

**¿Dónde se usa Conductor?**
- **En `panelConductor()`**: `Conductor* cond = static_cast<Conductor*>(usuarioActual)` -> casting down para obtener `getBusAsignado()`, que se usa para buscar el bus: `Bus* bus = buscarBusPorId(cond->getBusAsignado())`

## 3.8 `include/Administrador.h` + `src/Administrador.cpp`

```cpp
class Administrador : public Usuario {
private:
    int codigoAdmin;
public:
    std::string getEncabezado() const override {
        return "Usuario: " + getNombre() + " (Administrador)";
    }
    bool validarCodigo(int c) const override {
        return c == codigoAdmin;
    }
};
```

**¿Dónde se usa Administrador?**
- **En `panelAdministrador()`**: menú con 6 opciones -> gestionarRutas(), gestionarUsuarios(), gestionarBuses(), gestionarHorarios(), gestionarIncidentes()
- **En `gestionarRutas()`**: toggle estado activo/inactivo de rutas
- **En `gestionarBuses()`**: modificar capacidad actual de un bus
- **En `gestionarHorarios()`**: agregar salidas a rutas
- **En `gestionarIncidentes()`**: cerrar incidentes

## 3.9 `include/Bus.h` + `src/Bus.cpp`

```cpp
class Bus {
private:
    int idBus;
    std::string placa;
    int capacidadMaxima, capacidadActual;
    bool estado;                 // true=activo, false=inactivo
    UbicacionGPS* ubicacion;     // <- PUNTERO DINÁMICO (se crea con new)
    int indiceParadaActual;      // -1 = no iniciada
    int idRutaAsignada;
public:
    Bus();
    Bus(int id, const std::string& plc, int capMax, int capAct, bool est);
    ~Bus();                      // delete ubicacion;
    void setUbicacion(double lat, double lon, double alt, double vel);
    void simularMovimiento(double latDest, double lonDest);
    double tiempoHastaParada(double latParada, double lonParada) const;
    bool operator<(const Bus& otro) const;  // Ordena por espacio disponible
    // Getters/Setters...
};
```

**Detalles clave:**
- `ubicacion` es un **puntero crudo** (raw pointer) -> `new UbicacionGPS(lat, lon, alt, vel)` en `setUbicacion()`
- `~Bus()` hace `delete ubicacion` -> libera memoria
- `simularMovimiento()` mueve 5% hacia el destino en cada llamada
- `operator<` ordena buses por **mayor espacio libre** (capacidadMax - capacidadActual), invertido intencionalmente

## 3.10 `include/Parada.h` + `src/Parada.cpp`

```cpp
class Parada {
private:
    int idParada;
    std::string nombre;
    Coordenada ubicacion;           // <- COMPOSICIÓN: usa Coordenada
    double altitud;
    bool estado;
    int ordenEnRuta;               // <- posición en la ruta
public:
    double distanciaA(double lat, double lon) const;
    bool operator<(const Parada& otra) const;  // ordena por ordenEnRuta
    // Getters/Setters...
};
```

**Los datos JSON** contienen **67 paradas** reales de Villavicencio con coordenadas geográficas precisas (desde "Primax La Coralina" hasta "Segundo puente peatonal Samán de Rivera").

## 3.11 `include/Incidente.h` + `src/Incidente.cpp`

```cpp
class Incidente {
private:
    int idIncidente;
    std::string descripcion, tipo, estado;  // estado = "Abierto"|"Cerrado"
public:
    void cerrar();  // estado = "Cerrado"
    // Getters/Setters...
};
```

**¿Dónde se usa Incidente?**
- **En `panelConductor()`**: el conductor reporta incidentes -> `incidentes.emplace_back(id, desc, tipo, "Abierto")` y `gestor.guardarIncidentes()`
- **En `panelAdministrador()` -> `gestionarIncidentes()`**: lista todos los incidentes y permite cerrarlos (`inc.cerrar()`)
- **En `incidentesDeBus()`**: filtra incidentes cuya descripción contiene la placa del bus
- **En `panelConductor()`**: `std::any_of()` revisa si hay incidentes abiertos para mostrar "ALERTA" o "OPERATIVO"

## 3.12 `include/HorarioRuta.h` + `src/HorarioRuta.cpp`

```cpp
class HorarioRuta {
private:
    std::vector<std::string> salidasBarrio;     // horas "HH:MM:SS"
    std::vector<std::string> salidasUnillanos;
public:
    void agregarSalidaBarrio(const std::string& hora);
    void agregarSalidaUnillanos(const std::string& hora);
    std::string formatearBarrio() const;
    std::string formatearUnillanos() const;
};
```

## 3.13 `include/Ruta.h` + `src/Ruta.cpp`

```cpp
class Ruta {
private:
    int idRuta;
    std::string nombre, origen, destino;
    bool estado;
    std::string tipo;           // "RutaBarrio"|"RutaCentro"
    std::string puntoSalida;
    HorarioRuta horario;        // <- COMPOSICIÓN
    std::vector<int> idsParadas;  // <- IDs de paradas (no objetos completos)
public:
    virtual ~Ruta() {}
    virtual std::string getInfoAdicional() const = 0;  // <- PURO
    bool operator<(const Ruta& otra) const;
    bool operator==(const Ruta& otra) const;
    // Getters/Setters...
};
```

## 3.14 `include/RutaBarrio.h` + `src/RutaBarrio.cpp`

```cpp
class RutaBarrio : public Ruta {
public:
    RutaBarrio(...) : Ruta(id, nom, orig, dest, est, "RutaBarrio", pSalida) {}
    std::string getInfoAdicional() const override {
        return "Tipo: Ruta de Barrio | Trayecto: " + getOrigen() + " -> " + getDestino();
    }
};
```

## 3.15 `include/RutaCentro.h` + `src/RutaCentro.cpp`

```cpp
class RutaCentro : public Ruta {
private:
    std::string zonaCentro;
public:
    RutaCentro(...) : Ruta(...), zonaCentro(zona) {}
    std::string getInfoAdicional() const override {
        return "Tipo: Ruta Centro | Zona: " + zonaCentro + " | " + getOrigen() + " -> " + getDestino();
    }
};
```

## 3.16 `include/GestorArchivos.h` + `src/GestorArchivos.cpp`

**Propósito:** CRUD completo sobre archivos JSON. Es el **puente de persistencia**. 300 líneas.

```cpp
class GestorArchivos {
private:
    std::string rutaData;     // "data" (directorio de JSONs)
public:
    explicit GestorArchivos(const std::string& dirData);
    
    std::vector<Bus> cargarBuses() const;
    std::vector<Parada> cargarParadas() const;
    std::vector<Incidente> cargarIncidentes() const;
    std::vector<Ruta*> cargarRutas() const;
    std::vector<Usuario*> cargarUsuarios() const;
    HorarioRuta cargarHorariosPorDefecto() const;
    
    void guardarIncidentes(const std::vector<Incidente>&);
    void guardarBuses(const std::vector<Bus>&);
    void guardarUsuarios(const std::vector<Usuario*>&);
    void guardarRutas(const std::vector<Ruta*>&);
};
```

**Operaciones con nlohmann/json:**

| Operación | Código | Ejemplo en GestorArchivos.cpp |
|-----------|--------|-------------------------------|
| Parsear archivo | `json j = json::parse(archivo)` | Línea 16: `json j = json::parse(archivo);` |
| Acceder a arreglo | `j["buses"]` | Línea 18: `for (const auto& b : j["buses"])` |
| Extraer valor | `b["idBus"].get<int>()` | Línea 19: `b["idBus"].get<int>()` |
| Valor con default | `r.value("zonaCentro", "")` | Línea 70: `std::string zona = r.value("zonaCentro", "");` |
| Verificar existencia | `r.contains("horarios")` | Línea 80: `if (r.contains("horarios") && r["horarios"].is_object() && !r["horarios"].empty())` |
| Crear arreglo vacío | `j["buses"] = json::array()` | Línea 131: `j["buses"] = json::array();` |
| Agregar objeto | `j["buses"].push_back({...})` | Línea 133: `j["buses"].push_back({{"idBus", b.getIdBus()}, ...})` |
| Serializar | `archivo << j.dump(4)` | Línea 137: `archivo << j.dump(4)` |
| Verificar tipo | `r["puntoSalida"].is_null()` | Línea 63: `!r["puntoSalida"].is_null()` |

## 3.17 `include/SistemaTransporte.h` + `src/SistemaTransporte.cpp`

**Propósito:** Orquestador principal. 560 líneas, el archivo más grande del proyecto.

```cpp
class SistemaTransporte {
private:
    GestorArchivos gestor;
    std::vector<Bus> buses;
    std::vector<Parada> paradas;
    std::vector<Incidente> incidentes;
    std::vector<Ruta*> rutas;
    std::vector<Usuario*> usuarios;
    Usuario* usuarioActual;
    // + métodos privados...
public:
    SistemaTransporte(const std::string& dirData);
    ~SistemaTransporte();  // delete rutas[i], delete usuarios[i]
    void iniciar();
    // Paneles y gestión...
};
```

---

# Sección 4: Análisis de Clases y Estructuras

## 4.1 Clase `Coordenada`

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `latitud` | `double` | Coordenada norte-sur (-90 a 90) |
| `longitud` | `double` | Coordenada este-oeste (-180 a 180) |

### Método `distanciaHacia(const Coordenada& otra) const`

**Descripción:** Calcula la distancia en **metros** entre este punto y otro usando la **fórmula de Haversine**.

```
INPUT:  otra (referencia constante a Coordenada) — punto de destino
OUTPUT: double — distancia en metros

PASO A PASO:
1. Convertir diferencia de latitud a radianes:
   dLat = (otra.latitud - this.latitud) * (PI / 180)
2. Convertir diferencia de longitud a radianes:
   dLon = (otra.longitud - this.longitud) * (PI / 180)
3. Calcular 'a' usando la fórmula de Haversine:
   a = sen²(dLat/2) + cos(latitud en rad) * cos(otra.latitud en rad) * sen²(dLon/2)
4. Calcular distancia angular 'c':
   c = 2 * atan2(√a, √(1-a))
5. Distancia final = RADIO_TIERRA * c  (6.371.000 metros)

EFICIENCIA: O(1) — siempre la misma cantidad de cálculos
```

**Código real:**
```cpp
double Coordenada::distanciaHacia(const Coordenada& otra) const {
    double dLat = (otra.latitud  - latitud)  * PI / 180.0;
    double dLon = (otra.longitud - longitud) * PI / 180.0;
    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(latitud * PI / 180.0) * std::cos(otra.latitud * PI / 180.0) *
               std::sin(dLon / 2) * std::sin(dLon / 2);
    double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    return RADIO_TIERRA * c;
}
```

## 4.2 Clase `UbicacionGPS` (hereda de `Coordenada`)

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `altitud` | `double` | Metros sobre el nivel del mar |
| `velocidadKmh` | `double` | Velocidad de desplazamiento (default 30 km/h) |

### `tiempoEstimadoMinutos(double distanciaMetros) const`

```
INPUT:  distanciaMetros (double) — distancia a recorrer en metros
OUTPUT: double — tiempo estimado en minutos

PASO A PASO:
1. Si velocidad <= 0, devolver 0.0
2. distanciaKm = distanciaMetros / 1000.0
3. horas = distanciaKm / velocidadKmh
4. minutos = horas * 60.0
5. Devolver minutos

EJEMPLO:
  distancia = 1500 m, velocidad = 30 km/h
  distanciaKm = 1500 / 1000 = 1.5 km
  horas = 1.5 / 30 = 0.05 h
  minutos = 0.05 * 60 = 3.0 min
```

## 4.3 Clase `Usuario` (abstracta)

**Métodos virtuales puros:**
- `getEncabezado() const = 0` — Cada tipo de usuario muestra un encabezado diferente
- `validarCodigo(int codigo) const = 0` — Cada tipo valida su código de forma distinta

## 4.4 Clase `Estudiante` (hereda de `Usuario`)

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `codigoEstudiantil` | `int` | Código único de identificación |
| `facultad` | `string` | Facultad (ej. "Ingeniería") |
| `programa` | `string` | Programa académico (ej. "Sistemas") |
| `semestre` | `int` | Semestre actual |

## 4.5 Clase `Conductor` (hereda de `Usuario`)

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `codigoOperador` | `int` | Código del operador (para login) |
| `experiencia` | `string` | Años de experiencia |
| `turno` | `string` | "Mañana" o "Tarde" |
| `busAsignado` | `int` | ID del bus que conduce (FK a `Bus.idBus`) |

## 4.6 Clase `Administrador` (hereda de `Usuario`)

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `codigoAdmin` | `int` | Código de administrador |

## 4.7 Clase `Bus`

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `idBus` | `int` | Identificador único |
| `placa` | `string` | Placa (ej. "VST-001") |
| `capacidadMaxima` | `int` | Asientos totales |
| `capacidadActual` | `int` | Pasajeros actuales |
| `estado` | `bool` | true = activo |
| `ubicacion` | `UbicacionGPS*` | **Puntero dinámico** a ubicación GPS |
| `indiceParadaActual` | `int` | Índice de la última parada visitada (-1 = no iniciada) |
| `idRutaAsignada` | `int` | ID de la ruta que el bus cubre |

### `setUbicacion(double lat, double lon, double alt, double vel)`

```
PASO A PASO:
1. delete ubicacion;  // Libera la ubicación anterior (si existe)
2. ubicacion = new UbicacionGPS(lat, lon, alt, vel);  // Crea nueva en el heap
```

### `simularMovimiento(double latDest, double lonDest)`

```
PASO A PASO:
1. Si ubicacion es nullptr, salir
2. Calcular delta (5% de la distancia restante):
   deltaLat = (latDest - ubicacion->getLatitud()) * 0.05
   deltaLon = (lonDest - ubicacion->getLongitud()) * 0.05
3. Actualizar posición:
   ubicacion->setLatitud(ubicacion->getLatitud() + deltaLat)
   ubicacion->setLongitud(ubicacion->getLongitud() + deltaLon)

¿POR QUÉ 5%?
Cada llamada avanza el 5% del camino restante.
Nunca llega exactamente al destino (asintóticamente), pero tras ~20 llamadas
está prácticamente encima.
```

### `operator<(const Bus& otro) const`
```cpp
return (capacidadMaxima - capacidadActual) > (otro.capacidadMaxima - otro.capacidadActual);
```
Ordena buses por **mayor espacio disponible** (invertido intencionalmente).

## 4.8 Clase `Parada`

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `idParada` | `int` | Identificador único |
| `nombre` | `string` | Nombre descriptivo |
| `ubicacion` | `Coordenada` | **Objeto directo** (no puntero) con lat/lon |
| `altitud` | `double` | Metros sobre el nivel del mar |
| `estado` | `bool` | true = operativa |
| `ordenEnRuta` | `int` | Posición en la secuencia de la ruta |

## 4.9 Clase `Incidente`

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `idIncidente` | `int` | Identificador único |
| `descripcion` | `string` | Descripción del incidente |
| `tipo` | `string` | "Mecanico", "Accidente", "Otro" |
| `estado` | `string` | "Abierto" o "Cerrado" |

## 4.10 Clase `HorarioRuta`

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `salidasBarrio` | `vector<string>` | Horas de salida del barrio a la universidad |
| `salidasUnillanos` | `vector<string>` | Horas de salida de la universidad al barrio |

## 4.11 Clase `Ruta` (abstracta)

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `idRuta` | `int` | Identificador único |
| `nombre` | `string` | Nombre (ej. "Unillanos - Amarilo") |
| `origen` | `string` | Lugar de origen |
| `destino` | `string` | Destino ("Universidad de los Llanos") |
| `estado` | `bool` | true = activa |
| `tipo` | `string` | "RutaBarrio" o "RutaCentro" |
| `puntoSalida` | `string` | Dirección exacta del punto de encuentro |
| `horario` | `HorarioRuta` | **Composición** — objeto HorarioRuta incrustado |
| `idsParadas` | `vector<int>` | IDs de las paradas (no objetos completos) |

## 4.12 Clase `RutaBarrio` (hereda de `Ruta`)
- `getInfoAdicional()`: `"Tipo: Ruta de Barrio | Trayecto: X -> Y"`

## 4.13 Clase `RutaCentro` (hereda de `Ruta`)
- Atributo extra: `zonaCentro` (string)
- `getInfoAdicional()`: `"Tipo: Ruta Centro | Zona: Z | X -> Y"`

## 4.14 Clase `GestorArchivos`

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `rutaData` | `string` | Ruta al directorio de JSONs (ej. "data") |

**Métodos de carga:** `cargarBuses()`, `cargarParadas()`, `cargarIncidentes()`, `cargarRutas()`, `cargarUsuarios()`, `cargarHorariosPorDefecto()`
**Métodos de guardado:** `guardarBuses()`, `guardarIncidentes()`, `guardarUsuarios()`, `guardarRutas()`

## 4.15 Clase `SistemaTransporte`

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `gestor` | `GestorArchivos` | Objeto de persistencia |
| `buses` | `vector<Bus>` | Flota de buses (objetos directos) |
| `paradas` | `vector<Parada>` | Paradas disponibles (objetos directos) |
| `incidentes` | `vector<Incidente>` | Incidentes (objetos directos) |
| `rutas` | `vector<Ruta*>` | **Punteros** a rutas (polimórficas) |
| `usuarios` | `vector<Usuario*>` | **Punteros** a usuarios (polimórficos) |
| `usuarioActual` | `Usuario*` | **Puntero** al usuario con sesión activa |

---

# Sección 5: Análisis Línea por Línea (Bloques Lógicos)

## 5.1 Métodos Getter/Setter — El Principio de Encapsulamiento

**¿Qué son?** Métodos pequeños que permiten **leer** (get) o **modificar** (set) atributos privados.

**Ejemplo completo — los getters/setters de `Coordenada`:**
```cpp
double Coordenada::getLatitud()  const { return latitud; }
double Coordenada::getLongitud() const { return longitud; }
void Coordenada::setLatitud(double lat)  { latitud  = lat; }
void Coordenada::setLongitud(double lon) { longitud = lon; }
```

**¿Por qué `const` al final de `getLatitud() const`?** El `const` promete que el método NO modifica ningún atributo del objeto. Si accidentalmente escribiéramos `latitud = 10;` dentro, el compilador daría un error.

## 5.2 Análisis completo de `mostrarProximidadBus()`

```cpp
void SistemaTransporte::mostrarProximidadBus(Bus* bus, Ruta* ruta,
                                              const std::map<int, std::string>& horasLlegada) {
    if (!bus || !ruta || !bus->getUbicacion()) return;  // L1

    int idxActual = bus->getIndiceParadaActual();         // L2
    const auto& ids = ruta->getIdsParadas();              // L3

    for (size_t i = 0; i < ids.size(); ++i) {             // L4
        int idP = ids[i];                                  // L5
        auto it = std::find_if(paradas.begin(), paradas.end(),  // L6
            [idP](const Parada& p) { return p.getIdParada() == idP; });
        if (it == paradas.end()) continue;                // L7

        auto itHora = horasLlegada.find(idP);             // L8

        if ((int)i <= idxActual && itHora != horasLlegada.end()) {  // L9
            // Parada YA VISITADA: muestra hora de llegada
            std::cout << "  * Parada (" << idP << ") " << it->getNombre()
                      << "\n    -> 0 m | 0.0 min | Llego: " << itHora->second << "\n";
        } else {
            // Parada PENDIENTE: calcula distancia y tiempo
            double metros = it->distanciaA(
                bus->getUbicacion()->getLatitud(),
                bus->getUbicacion()->getLongitud());
            double minutos = bus->tiempoHastaParada(
                it->getUbicacion().getLatitud(),
                it->getUbicacion().getLongitud());
            std::cout << "    Parada (" << idP << ") " << it->getNombre()
                      << "\n    -> " << std::fixed << std::setprecision(0)
                      << metros << " m | "
                      << std::setprecision(1) << minutos << " min\n";
        }
    }
}
```

**Explicación línea por línea:**
- **L1** — `if (!bus || !ruta || !bus->getUbicacion()) return;`: **Guard clause**: si alguno es `nullptr`, salimos inmediatamente para evitar acceder a memoria inválida.
- **L3** — `const auto& ids = ruta->getIdsParadas();`: Referencia constante al vector de IDs. Evita copiar y promete no modificar.
- **L4** — Usa índice en lugar de range-based porque necesita comparar `i` con `idxActual`.
- **L6** — `std::find_if` con lambda: busca la parada por ID en el vector global.
- **L8** — `horasLlegada.find(idP)`: busca si la parada ya tiene hora de llegada registrada.
- **L9** — `(int)i <= idxActual`: si el índice actual <= índice de parada visitada, la parada YA FUE VISITADA.

## 5.3 Análisis de `autenticarUsuario()` — Mapa y búsqueda

```cpp
static const std::map<int, std::pair<std::string, std::string>> datos = {
    {1, {"Estudiante", "codigo estudiantil"}},
    {2, {"Conductor", "codigo de operador"}},
    {3, {"Administrador", "codigo de administrador"}}
};

auto it = std::find_if(usuarios.begin(), usuarios.end(),
    [tipo, codigo](Usuario* u) {
        return u->getTipo() == datos.at(tipo).first && u->validarCodigo(codigo);
    });
```

**Explicación:**
1. `std::map<int, pair<string,string>>`: Diccionario que mapea opción de menú -> (tipo string, label string)
2. `datos.at(tipo).first`: Obtiene el tipo de usuario ("Estudiante", etc.)
3. La lambda busca un usuario que cumpla **ambas** condiciones: tipo coincidente Y código válido

## 5.4 Análisis de `incidentesDeBus()`

```cpp
std::vector<Incidente*> SistemaTransporte::incidentesDeBus(const std::string& placa) {
    std::vector<Incidente*> resultado;
    for (auto& inc : incidentes)
        if (inc.getDescripcion().find(placa) != std::string::npos)
            resultado.push_back(&inc);
    return resultado;
}
```

**`std::string::find(subcadena)`:** Busca una subcadena dentro del string:
- Si la encuentra: devuelve la **posición** (índice) donde comienza
- Si NO la encuentra: devuelve `std::string::npos`

## 5.5 Análisis de `buscarRutaPorBus()` (versión mejorada)

```cpp
Ruta* SistemaTransporte::buscarRutaPorBus(const std::string& placa) {
    auto it = std::find_if(buses.begin(), buses.end(),
        [&placa](const Bus& b) { return b.getPlaca() == placa; });

    if (it != buses.end() && !rutas.empty()) {
        int idRuta = it->getIdRutaAsignada();
        auto itRuta = std::find_if(rutas.begin(), rutas.end(),
            [idRuta](Ruta* r) { return r->getIdRuta() == idRuta; });
        if (itRuta != rutas.end()) return *itRuta;
    }
    return (!rutas.empty() ? rutas[0] : nullptr);
}
```

**Mejora:** Ahora usa `getIdRutaAsignada()` del bus para obtener el ID de ruta correspondiente, y busca esa ruta específica. Ya no asume `rutas[idBus - 1]`.

---

# Sección 6: Guía de Sintaxis del Lenguaje (C++)

## 6.1 `#include` — Directiva del preprocesador

**¿Qué es?** Una instrucción que se ejecuta ANTES de la compilación. Copia todo el contenido de un archivo aquí.

**Sintaxis:**
```cpp
#include <iostream>    // Ángulos: busca en directorios del sistema
#include "Bus.h"       // Comillas: busca en directorio del proyecto
```

## 6.2 `class` — Definición de clases

**¿Qué es?** Una **plantilla** para crear objetos. Define atributos (datos) y métodos (funciones).

**Analogía:** La clase es el **plano** de una casa. El objeto es la **casa real** construida a partir del plano.

## 6.3 `public:` / `private:` — Especificadores de acceso

| Especificador | Misma clase | Clases hijas | Fuera |
|---------------|:------:|:------:|:-----:|
| `private` | Sí | No | No |
| `protected` | Sí | Sí | No |
| `public` | Sí | Sí | Sí |

## 6.4 `const` — Inmutabilidad

**Usos:**
1. **Métodos constantes** — no modifican el objeto: `double getLatitud() const;`
2. **Parámetros constantes** — el argumento no se modifica: `void setNombre(const std::string& nom);`
3. **Variables constantes** — no pueden reasignarse: `static const double PI = 3.14159;`

## 6.5 `virtual` y `override` — Polimorfismo

- **`virtual`** en la clase base: "este método puede ser redefinido en las hijas"
- **`virtual ... = 0`**: método **puramente virtual** (sin implementación, obligatorio implementarlo)
- **`override`** en la clase hija: "confirmo que estoy redefiniendo un método virtual"

## 6.6 `auto` — Deducción automática de tipos (C++11)

```cpp
auto it = std::find_if(...);
// El compilador deduce que 'it' es de tipo vector<Parada>::iterator
```

## 6.7 Referencias (`&`) y Punteros (`*`)

### Referencias — Alias de una variable
```cpp
int original = 42;
int& ref = original;   // <- ref es OTRO NOMBRE para original
ref = 100;
cout << original;      // <- Imprime 100
```

### Punteros — Direcciones de memoria
```cpp
int valor = 42;
int* p = &valor;    // <- p guarda la DIRECCIÓN de valor
*p = 100;           // <- *p accede al contenido
```

| Característica | Referencia (&) | Puntero (*) |
|---------------|:------:|:------:|
| Puede ser `nullptr` | No | Sí |
| Puede reasignarse | No | Sí |
| Sintaxis de acceso | Como variable normal | `*` para desreferenciar |

## 6.8 `new` y `delete` — Gestión manual de memoria

- **`new`**: Reserva memoria en el **heap**: `UbicacionGPS* ubi = new UbicacionGPS(lat, lon, alt, vel);`
- **`delete`**: Libera esa memoria: `delete ubi;`

## 6.9 `this` — Puntero al objeto actual

```cpp
void Bus::setIdBus(int id) {
    this->idBus = id;  // <- this->idBus = atributo, id = parámetro
}
```

## 6.10 `explicit` — Constructor explícito

Evita conversiones implícitas:
```cpp
explicit GestorArchivos(const std::string& dirData);
GestorArchivos g = "data";  // <- ERROR
GestorArchivos g("data");   // <- OK
```

## 6.11 Lambdas (C++11) — Funciones anónimas

**Sintaxis:** `[captura](parámetros) { cuerpo }`

```cpp
auto it = std::find_if(paradas.begin(), paradas.end(),
    [idP](const Parada& p) { return p.getIdParada() == idP; });
```

- `[idP]`: captura la variable `idP` del contexto exterior
- `(const Parada& p)`: recibe una parada por referencia
- `{ return p.getIdParada() == idP; }`: devuelve true si coincide

## 6.12 Operador `:` (dos puntos)

**Usos:**
1. **Herencia**: `class Estudiante : public Usuario { ... };`
2. **Lista de inicializadores**: `Bus::Bus(...) : idBus(id), placa(plc) { }`
3. **Acceso a miembro por puntero**: `it->getNombre()` equivale a `(*it).getNombre()`

## 6.13 `#pragma once`

Directiva que garantiza que un archivo de cabecera se incluya **solo una vez** durante la compilación.

## 6.14 `static_cast` — Conversión en tiempo de compilación

```cpp
const Estudiante* e = static_cast<const Estudiante*>(u);
```
Convierte un puntero `Usuario*` a `Estudiante*` (downcasting). Es seguro cuando se ha verificado el tipo antes.

---

# Sección 7: Guía de la STL y Librerías Estándar

## 7.1 `std::string` — `<string>`

- `length()`, `empty()`: O(1)
- `find(subcadena)`: O(n) — devuelve posición o `string::npos`
- Concatenación (`+`): O(n)

## 7.2 `std::vector<T>` — `<vector>`

**Complejidad:**
| Operación | Complejidad |
|-----------|:------:|
| `v[i]` | O(1) |
| `push_back` | O(1) amortizado |
| `find_if` | O(n) |
| `begin()`, `end()`, `size()` | O(1) |

## 7.3 Algoritmos de la STL — `<algorithm>`

### `std::find_if`
Busca el PRIMER elemento que cumple una condición:
```cpp
auto it = find_if(inicio, fin, predicado);
```

### `std::count_if`
Cuenta cuántos elementos cumplen una condición:
```cpp
int abiertos = count_if(incidentes.begin(), incidentes.end(),
    [](const Incidente& i) { return i.getEstado() == "Abierto"; });
```

### `std::any_of`
Devuelve `true` si AL MENOS UNO cumple:
```cpp
bool hayAbiertos = any_of(inc.begin(), inc.end(),
    [](Incidente* i) { return i->getEstado() == "Abierto"; });
```

## 7.4 `std::map<K,V>` — `<map>`

**Diccionario** ordenado por clave (árbol binario balanceado). Complejidad: O(log n).

## 7.5 `std::cin`, `std::cout`, `std::cerr` — `<iostream>`

```cpp
cout << "Texto";
cin >> variable;
cerr << "Error";
cin.ignore(100, '\n');
getline(cin, str);
```

## 7.6 `std::ostringstream` — `<sstream>`

Permite formatear texto como si fuera `cout` pero guardándolo en un string:
```cpp
ostringstream ss;
ss << setw(2) << setfill('0') << hora;
return ss.str();
```

## 7.7 Manipuladores de formato — `<iomanip>`

| Manipulador | Efecto |
|-------------|--------|
| `setw(n)` | Ancho mínimo de n caracteres |
| `setfill(c)` | Relleno con carácter c |
| `fixed` | Notación de punto fijo |
| `setprecision(n)` | n decimales |

## 7.8 `std::ifstream`, `std::ofstream` — `<fstream>`

```cpp
ifstream archivo("data/buses.json");
if (!archivo.is_open()) return;
ofstream archivo("data/buses.json");
archivo << j.dump(4);
```

## 7.9 Funciones matemáticas — `<cmath>`

| Función | Propósito |
|---------|-----------|
| `sin(x)` | Seno de x (radianes) |
| `cos(x)` | Coseno de x (radianes) |
| `sqrt(x)` | Raíz cuadrada |
| `atan2(y, x)` | Arco tangente de y/x |

---

# Sección 8: APIs Externas y Dependencias

## 8.1 nlohmann/json — `include/json.hpp`

**¿Qué es?** Librería "header-only" (solo cabecera, no necesita compilación) para manejar JSON en C++.

**Operaciones:**

| Operación | Código | Ejemplo |
|-----------|--------|---------|
| Parsear | `json::parse(archivo)` | `json j = json::parse(archivo);` |
| Acceder a arreglo | `j["buses"]` | `for (auto& b : j["buses"])` |
| Extraer valor | `.get<T>()` | `b["idBus"].get<int>()` |
| Valor con default | `.value(clave, default)` | `r.value("zonaCentro", "")` |
| Verificar existencia | `.contains(clave)` | `r.contains("horarios")` |
| Verificar tipo | `.is_array()`, `.is_null()` | `r["puntoSalida"].is_null()` |
| Crear arreglo | `json::array()` | `j["buses"] = json::array()` |
| Serializar | `.dump(indentación)` | `archivo << j.dump(4)` |

## 8.2 Windows Console API — `<windows.h>`

| Función | Propósito | Uso |
|---------|-----------|-----|
| `GetStdHandle(STD_OUTPUT_HANDLE)` | Obtiene handle de consola | `limpiarPantalla()`, `setColor()` |
| `SetConsoleTextAttribute(h, color)` | Cambia color de texto/fondo | `setColor()` |
| `SetConsoleTitle("...")` | Cambia título de ventana | `iniciar()` |
| `SetConsoleOutputCP(65001)` | Activa UTF-8 para salida | `iniciar()` |
| `SetConsoleCP(65001)` | UTF-8 para entrada | `iniciar()` |
| `GetConsoleScreenBufferInfo(h, &info)` | Obtiene tamaño de pantalla | `limpiarPantalla()` |
| `FillConsoleOutputCharacter(h, c, n, coord, &esc)` | Llena con caracteres | `limpiarPantalla()` |
| `FillConsoleOutputAttribute(h, attr, n, coord, &esc)` | Resetea colores | `limpiarPantalla()` |
| `SetConsoleCursorPosition(h, coord)` | Mueve cursor | `limpiarPantalla()` |
| `GetLocalTime(&st)` | Obtiene hora/fecha local | `horaActual()`, `fechaActual()` |
| `Sleep(ms)` | Pausa el programa | En todos los paneles |

## 8.3 Constantes de Color de Consola

```
Bit:  3         2         1         0
      |         |         |         |
      INTENSITY BLUE      GREEN     RED
```

| Constante | Valor Binario |
|-----------|:------:|
| `FOREGROUND_RED` | `0100` |
| `FOREGROUND_GREEN` | `0010` |
| `FOREGROUND_BLUE` | `0001` |
| `FOREGROUND_INTENSITY` | `1000` |

**Combinaciones de colores:**

| Expresión | Binario | Color |
|-----------|:------:|-------|
| `RED \| GREEN \| BLUE` | `0111` | Blanco (gris claro) |
| `GREEN \| INTENSITY` | `1010` | Verde brillante |
| `RED \| INTENSITY` | `1100` | Rojo brillante |
| `BLUE \| GREEN \| INTENSITY` | `1011` | Cyan brillante |

## 8.4 CMake — Sistema de Compilación

CMake es un **meta-sistema de compilación**: no compila directamente, sino que genera archivos de proyecto para el compilador nativo (Make, Ninja, etc.).

---

# Sección 9: Desglose de Instrucciones Complejas

## 9.1 `setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY)`

**Operador `|` (OR a nivel de bits):** Combina los bits activando cada color:
```
FOREGROUND_BLUE          = 0001
FOREGROUND_GREEN         = 0010
FOREGROUND_INTENSITY     = 1000
                           ---- OR (|)
Resultado:                 1011  = cyan brillante (azul + verde + intensidad)
```

## 9.2 `limpiarPantalla()` — Paso a paso completo

```cpp
static void limpiarPantalla() {
    COORD coord = {0, 0};
    DWORD celdas, escritas;
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(h, &info);
    celdas = info.dwSize.X * info.dwSize.Y;
    FillConsoleOutputCharacter(h, ' ', celdas, coord, &escritas);
    FillConsoleOutputAttribute(h, info.wAttributes, celdas, coord, &escritas);
    SetConsoleCursorPosition(h, coord);
}
```

**Paso a paso:**
1. `GetConsoleScreenBufferInfo`: Obtiene dimensiones (ej. 80x25).
2. `celdas = 80 x 25 = 2000`: total de posiciones en pantalla.
3. `FillConsoleOutputCharacter`: Escribe 2000 espacios empezando desde (0,0) -> borra todo.
4. `FillConsoleOutputAttribute`: Restaura los colores originales.
5. `SetConsoleCursorPosition(h, {0,0})`: Mueve cursor a la esquina superior izquierda.

## 9.3 Fórmula Haversine Desglosada con valores reales

Supongamos dos paradas: (4.14527, -73.652037) y (4.14335, -73.650037)

**Paso 1: Convertir diferencias a radianes**
```
dLat = (4.14335 - 4.14527) x PI / 180 = -0.0000335 rad
dLon = (-73.650037 - (-73.652037)) x PI / 180 = 0.0000349 rad
```

**Paso 2: Calcular 'a'**
```
sin(dLat/2)² = sin(-0.00001675)² ≈ 2.806 x 10^-10
cos(lat1 x PI/180) = cos(0.07233) ≈ 0.99738
cos(lat2 x PI/180) = cos(0.07230) ≈ 0.99738
sin(dLon/2)² = sin(0.00001745)² ≈ 3.045 x 10^-10
a = 2.806e-10 + 0.99738 x 0.99738 x 3.045e-10 ≈ 5.84 x 10^-10
```

**Paso 3: Calcular 'c'**
```
c = 2 x atan2(sqrt(5.84e-10), sqrt(1 - 5.84e-10))
c ≈ 2 x atan2(0.0000242, 0.99999...) ≈ 0.0000484 rad
```

**Paso 4: Calcular distancia**
```
d = 6,371,000 x 0.0000484 ≈ 308 metros
```

## 9.4 `inc.getDescripcion().find(placa) != std::string::npos`

**`std::string::find(str)`:** Busca la subcadena `str` dentro del string:
- Si la encuentra: devuelve la posición (0, 1, 2, ...)
- Si NO la encuentra: devuelve `std::string::npos`
- `npos` = 18446744073709551615 (el número más grande de `size_t`)

## 9.5 Cálculo de `tiempoHastaParada`

```cpp
double Bus::tiempoHastaParada(double latParada, double lonParada) const {
    if (!ubicacion) return -1.0;
    Coordenada destino(latParada, lonParada);
    double dist = ubicacion->distanciaHacia(destino);
    return ubicacion->tiempoEstimadoMinutos(dist);
}
```

**Ejemplo numérico:** Bus en (4.14527, -73.652037), parada en (4.14335, -73.650037):
1. Distancia ≈ 308 metros
2. Velocidad = 30 km/h
3. Tiempo = (308 / 1000 / 30) x 60 = **0.616 minutos** ≈ 37 segundos

## 9.6 Simulación de Movimiento (5% de interpolación)

```cpp
void Bus::simularMovimiento(double latDest, double lonDest) {
    if (!ubicacion) return;
    double deltaLat = (latDest - ubicacion->getLatitud())  * 0.05;
    double deltaLon = (lonDest - ubicacion->getLongitud()) * 0.05;
    ubicacion->setLatitud(ubicacion->getLatitud()   + deltaLat);
    ubicacion->setLongitud(ubicacion->getLongitud() + deltaLon);
}
```

Cada llamada avanza el 5% de la distancia restante -> interpolación exponencial (ease-out). Al principio se mueve rápido, luego se desacelera al acercarse al destino.

## 9.7 Operador < Invertido del Bus

```cpp
bool Bus::operator<(const Bus& otro) const {
    return (capacidadMaxima - capacidadActual) > (otro.capacidadMaxima - otro.capacidadActual);
}
```

Usa `>` (mayor que) en lugar de `<`: un bus con MÁS asientos disponibles se considera "menor". Así, al ordenar ascendentemente con `<`, los buses con más espacio libre aparecen primero.

## 9.8 `buses.emplace_back(id, placa, capMax, capAct, est)` vs `push_back`

```cpp
// push_back: crea objeto temporal, lo copia al vector, destruye el temporal
lista.push_back(Bus(id, placa, capMax, capAct, est));

// emplace_back: construye directamente en el vector (más eficiente)
lista.emplace_back(id, placa, capMax, capAct, est);
```

---

# Sección 10: Flujo de Ejecución Completo (Call Graph)

## 10.1 Desde `main()` hasta el cierre

```
LLAMADA 1: main()
  |
  +-- Crea SistemaTransporte sistema("data")
  |     +-- gestor("data"), usuarioActual = nullptr
  |
  +-- sistema.iniciar()
        |
        +-- 1. CARGA DE DATOS:
        |     +-- gestor.cargarBuses()     -> buses.json -> vector<Bus> (3 buses)
        |     +-- gestor.cargarParadas()   -> paradas.json -> vector<Parada> (67 paradas)
        |     +-- gestor.cargarIncidentes() -> incidentes.json -> vector<Incidente> (3 inc.)
        |     +-- gestor.cargarRutas()     -> rutas.json -> vector<Ruta*>
        |     |     +-- cargarHorariosPorDefecto() -> horarios.json
        |     |     +-- por cada ruta: new RutaBarrio() o new RutaCentro()
        |     +-- gestor.cargarUsuarios()  -> usuarios.json -> vector<Usuario*>
        |           +-- new Estudiante() / new Conductor() / new Administrador()
        |
        +-- 2. CONFIGURACIÓN DE CONSOLA:
        |     +-- SetConsoleTitle("Sistema de Gestion de Transporte - Unillanos")
        |     +-- SetConsoleOutputCP(65001)  // UTF-8
        |     +-- SetConsoleCP(65001)
        |
        +-- 3. BUCLE PRINCIPAL (while true):
              |
              +-- usuarioActual = nullptr
              +-- mostrarMenuPrincipal()
              |     +-- limpiarPantalla()
              |     +-- imprimirEncabezado()
              |     |     +-- imprimirSeparador()
              |     |     +-- horaActual() -> GetLocalTime() -> ostringstream
              |     |     +-- fechaActual() -> GetLocalTime() -> ostringstream
              |     |     +-- setColor() -> SetConsoleTextAttribute()
              |     +-- cout << menu [1] [2] [3] [4]
              |     +-- leerInt() -> cin >> v
              |
              +-- if (op == 4) -> break
              +-- if (op < 1 || op > 3) -> continue
              +-- autenticarUsuario(op)
              |     +-- cout << "Ingrese su ..."
              |     +-- leerInt() -> codigo
              |     +-- find_if(usuarios, lambda)
              |     |     +-- u->getTipo() == tipoEsperado
              |     |     +-- u->validarCodigo(codigo) [polimórfico]
              |     +-- if (encontrado) return puntero
              |     +-- else -> error -> Sleep(2000) -> nullptr
              |
              +-- if (op == 1) -> panelEstudiante()
              |     +-- while (true):
              |           +-- Mostrar lista de rutas
              |           +-- Usuario elige ruta (o 0 = volver)
              |           +-- Buscar ruta por ID: find_if(rutas, ...)
              |           +-- Mostrar detalles: paradas, horarios
              |           +-- if (Reservar == 'S'):
              |           |     +-- buscarBusPorId(idRuta)
              |           |     +-- if (!bus) buscar cualquier bus con espacio
              |           |     +-- if (bus && espacio) incrementar capacidad
              |           |     +-- gestor.guardarBuses(buses)
              |           +-- Sleep/Enter para continuar
              |           +-- break (cuando elige Volver)
              |
              +-- if (op == 2) -> panelConductor()
              |     +-- Conductor* cond = static_cast<Conductor*>(usuarioActual)
              |     +-- Bus* bus = buscarBusPorId(cond->getBusAsignado())
              |     +-- Ruta* ruta = buscarRutaPorBus(bus->getPlaca())
              |     +-- map<int,string> horasLlegada
              |     +-- while (true):
              |           +-- Mostrar info bus, ruta, horarios
              |           +-- mostrarProximidadBus(bus, ruta, horasLlegada)
              |           +-- Op [1] Llegada a parada:
              |           |     +-- idxActual++
              |           |     +-- idSig = ids[idxActual]
              |           |     +-- find_if(paradas, idSig)
              |           |     +-- bus->setUbicacion(lat, lon, alt)
              |           |     +-- horasLlegada[idSig] = horaActual()
              |           |     +-- gestor.guardarBuses(buses)
              |           |     +-- if (fin de ruta): cambiar a siguiente ruta
              |           +-- Op [2] Reportar incidente:
              |           |     +-- cin >> descripcion >> tipo
              |           |     +-- incidentes.emplace_back(id, desc, tipo, "Abierto")
              |           |     +-- gestor.guardarIncidentes(incidentes)
              |           +-- Op [3] Finalizar turno -> break
              |
              +-- if (op == 3) -> panelAdministrador()
                    +-- while (true):
                          +-- count_if(incidentes, estado=="Abierto")
                          +-- Op [1] gestionarRutas():
                          |     +-- Mostrar rutas + estado
                          |     +-- r->setEstado(!r->getEstado()) [toggle]
                          |     +-- gestor.guardarRutas(rutas)
                          +-- Op [2] gestionarUsuarios():
                          |     +-- Mostrar lista (solo lectura)
                          +-- Op [3] gestionarBuses():
                          |     +-- Mostrar flota
                          |     +-- if (cap >= 0 && cap <= max) setCapacidadActual(cap)
                          |     +-- gestor.guardarBuses(buses)
                          +-- Op [4] gestionarHorarios():
                          |     +-- Elegir ruta + tipo (Barrio/Unillanos)
                          |     +-- agregarSalida...()
                          |     +-- gestor.guardarRutas(rutas)
                          +-- Op [5] gestionarIncidentes():
                          |     +-- Mostrar incidentes (rojo=abierto, verde=cerrado)
                          |     +-- inc.cerrar()
                          |     +-- gestor.guardarIncidentes(incidentes)
                          +-- Op [6] Volver -> break

  FIN DEL PROGRAMA (opción 4)
    +-- Se destruye SistemaTransporte (sale de ámbito en main)
          +-- ~SistemaTransporte()
                +-- for (Ruta* r : rutas) delete r;
                +-- for (Usuario* u : usuarios) delete u;
```

## 10.2 Resumen del ciclo de vida de objetos

| Objeto | ¿Cuándo se crea? | ¿Dónde vive? | ¿Cuándo se destruye? |
|--------|------------------|:------:|----------------------|
| `SistemaTransporte` | En `main()`: `SistemaTransporte sistema(...)` | Stack | Al salir de `main()` |
| `Buses` | `GestorArchivos::cargarBuses()` con `emplace_back` | En `vector<Bus>` | Cuando `SistemaTransporte` se destruye |
| `Paradas` | `GestorArchivos::cargarParadas()` con `emplace_back` | En `vector<Parada>` | Cuando `SistemaTransporte` se destruye |
| `Incidentes` | `GestorArchivos::cargarIncidentes()` o `panelConductor()` | En `vector<Incidente>` | Cuando `SistemaTransporte` se destruye |
| `Rutas*` | `GestorArchivos::cargarRutas()` con `new` | Heap | `~SistemaTransporte()` con `delete` |
| `Usuarios*` | `GestorArchivos::cargarUsuarios()` con `new` | Heap | `~SistemaTransporte()` con `delete` |
| `UbicacionGPS*` | `Bus::setUbicacion()` con `new` | Heap | `~Bus()` con `delete` |

---

# Sección 11: Conceptos Teóricos y Buenas Prácticas

## 11.1 Programación Orientada a Objetos (POO)

**Definición:** Paradigma de programación que organiza el código en **objetos** que combinan **datos** (atributos) y **comportamiento** (métodos).

**Pilares de la POO detectados en el proyecto:**

| Pilar | Definición | Ejemplo en el proyecto |
|-------|-----------|----------------------|
| **Abstracción** | Modelar solo los detalles relevantes del mundo real | `Coordenada` solo tiene latitud/longitud, no otros detalles geográficos |
| **Encapsulamiento** | Ocultar datos internos y controlar el acceso | Todos los atributos son `private`, se accede mediante getters/setters |
| **Herencia** | Crear clases nuevas basadas en clases existentes | `Estudiante : Usuario`, `UbicacionGPS : Coordenada`, `RutaBarrio : Ruta` |
| **Polimorfismo** | Tratar objetos de diferentes clases de manera uniforme | `vector<Ruta*>` con `getInfoAdicional()` diferente para cada tipo |

## 11.2 Herencia

**Definición:** Mecanismo que permite que una clase (hija) herede atributos y métodos de otra clase (base).

**Sintaxis:**
```cpp
class Hija : public Base {
    // Hija tiene todo lo de Base + lo que agregue aquí
};
```

**Jerarquía del proyecto:**
```
Coordenada          Usuario (abstracto)     Ruta (abstracto)
    ^                    ^    ^    ^              ^    ^
UbicacionGPS     Estudiante Conductor Admin   RutaBarrio RutaCentro
```

## 11.3 Polimorfismo con `virtual`

**Definición:** Capacidad de que un mismo método se comporte de manera diferente según el tipo real del objeto.

**¿Cómo funciona en memoria?** Cada objeto con métodos virtuales contiene un **puntero oculto** a la **vtable** (tabla de funciones virtuales) de su clase. Al llamar a un método virtual:
1. Sigue el puntero a la vtable
2. Busca la función correcta en la tabla
3. Ejecuta la implementación correspondiente al tipo real

```cpp
void mostrarEncabezado(Usuario* u) {
    cout << u->getEncabezado();  // Polimorfismo: método correcto según tipo real
}
```

## 11.4 Gestión de Memoria Manual vs. Automática

### Memoria Automática (Stack)
Variables locales creadas sin `new`. Se destruyen automáticamente al salir de su ámbito.

### Memoria Manual (Heap)
Objetos creados con `new`. **Deben** destruirse con `delete`.

### RAII (Resource Acquisition Is Initialization)
Patrón donde los recursos se adquieren en el constructor y se liberan en el destructor. Ej: `std::ifstream`, `std::vector`.

## 11.5 Composición vs. Herencia

**Composición:** Una clase contiene objetos de otra clase como atributos.
```cpp
class Parada {
    Coordenada ubicacion;  // <- Coordenada es PARTE de Parada
};
```

**Herencia:** Una clase ES un tipo de otra clase.
```cpp
class UbicacionGPS : public Coordenada {  // <- UbicacionGPS ES una Coordenada
};
```

**Regla práctica:**
- Usa **herencia** cuando sea una relación "ES UN" (Estudiante ES UN Usuario)
- Usa **composición** cuando sea una relación "TIENE UN" (Parada TIENE UNA Coordenada)

## 11.6 Serialización

**Definición:** Proceso de convertir objetos en memoria a un formato que pueda almacenarse (archivo) y luego reconstruirse.

**En el proyecto:**
- **Serialización:** `GestorArchivos::guardarBuses()` convierte cada `Bus` a JSON
- **Deserialización:** `GestorArchivos::cargarBuses()` convierte JSON a objetos `Bus`

## 11.7 Paso por Referencia vs. Paso por Valor

```cpp
void porValor(std::string s);       // <- Se COPIA todo el string (lento, O(n))
void porReferencia(std::string& s); // <- Se pasa solo una referencia (rápido, O(1))
void porRefConst(const std::string& s); // <- Referencia + promesa de no modificar
```

## 11.8 Stack vs Heap

```
+-----------------------------+
|         STACK               |  <- Rápido, automático, limitado (~1-8 MB)
|  (variables locales)        |
|  int x = 5;                 |
|  Coordenada c(4.1, -73.6);  |
+-----------------------------+
|         HEAP                |  <- Más lento, manual, casi ilimitado
|  (memoria dinámica)         |
|  new UbicacionGPS(...)      |
|  new RutaBarrio(...)        |
+-----------------------------+
```

## 11.9 Operador `|` (OR a nivel de bits)

Combina los bits de dos números para formar una máscara de color:
```
FOREGROUND_BLUE      = 0001
FOREGROUND_GREEN     = 0010
FOREGROUND_INTENSITY = 1000
                        ---- OR
Resultado:              1011  (= cyan brillante)
```

---

# Sección 12: Resumen y Ruta de Estudio

## 12.1 Integración de todas las piezas

El sistema **SistemaTransporte** funciona como un **reloj suizo** donde cada engranaje tiene su función:

1. **`main.cpp`** da la señal de arranque creando el objeto `SistemaTransporte`
2. **`SistemaTransporte::iniciar()`** carga todos los datos mediante `GestorArchivos`
3. **`GestorArchivos`** lee los archivos JSON usando la librería **nlohmann/json** y construye objetos de las clases del modelo (`Bus`, `Ruta`, `Usuario`, etc.)
4. El **bucle principal** del menú ofrece las opciones de perfil
5. **`autenticarUsuario()`** busca en el vector de `Usuario*` usando polimorfismo (cada tipo valida su código a su manera)
6. Dependiendo del perfil, se ejecuta uno de los tres paneles
7. Cada cambio se guarda inmediatamente en JSON mediante `GestorArchivos::guardar*()`
8. Al salir, el **destructor** `~SistemaTransporte()` libera toda la memoria dinámica

## 12.2 Ruta de Estudio Recomendada

Para un estudiante que quiere entender el código sin frustrarse, recomiendo leer los archivos en este orden:

### Primera semana — Fundamentos

| Paso | Archivo | Conceptos a aprender |
|------|---------|---------------------|
| 1 | `main.cpp` | Punto de entrada, `try-catch`, creación de objetos |
| 2 | `Coordenada.h` + `.cpp` | Clase simple, `double`, getters/setters, `const`, fórmula matemática |
| 3 | `Parada.h` + `.cpp` | Composición, `bool`, operador `<` |
| 4 | `Incidente.h` + `.cpp` | Clase con estado, método `cerrar()` |
| 5 | `HorarioRuta.h` + `.cpp` | `std::vector<std::string>`, `push_back`, bucles, formateo |

### Segunda semana — Herencia y Polimorfismo

| Paso | Archivo | Conceptos a aprender |
|------|---------|---------------------|
| 6 | `Usuario.h` + `.cpp` | Clase abstracta, `virtual ... = 0`, destructor virtual |
| 7 | `Estudiante.h` + `.cpp` | Herencia, `override`, `validarCodigo()` |
| 8 | `Conductor.h` + `.cpp` | Herencia con más atributos |
| 9 | `Administrador.h` + `.cpp` | Herencia simple |
| 10 | `UbicacionGPS.h` + `.cpp` | Herencia de `Coordenada`, `new/delete` |
| 11 | `Ruta.h` + `.cpp` | Clase abstracta, `vector<int>`, operadores `<` y `==` |
| 12 | `RutaBarrio.h` + `.cpp` | Herencia simple de Ruta |
| 13 | `RutaCentro.h` + `.cpp` | Herencia con atributo extra |

### Tercera semana — Lógica de Negocio

| Paso | Archivo | Conceptos a aprender |
|------|---------|---------------------|
| 14 | `Bus.h` + `.cpp` | Puntero dinámico (`UbicacionGPS*`), destructor, `simularMovimiento()`, operador `<` |
| 15 | `GestorArchivos.h` + `.cpp` | `ifstream`/`ofstream`, `nlohmann/json`, `emplace_back`, `new`, `static_cast` |

### Cuarta semana — Orquestación

| Paso | Archivo | Conceptos a aprender |
|------|---------|---------------------|
| 16 | `SistemaTransporte.h` + `.cpp` | `windows.h`, colores, `GetLocalTime`, `limpiarPantalla`, `autenticarUsuario` |
| 17 | `SistemaTransporte.cpp` (paneles) | `panelEstudiante()`, `panelConductor()`, `panelAdministrador()` |
| 18 | `SistemaTransporte.cpp` (completo) | Flujo completo, `std::find_if`, `std::count_if`, `std::any_of`, lambdas |

## 12.3 Ejercicios de estudio sugeridos

1. **Traza manual**: Sigue el flujo cuando un Estudiante con código 160005557 inicia sesión y reserva un asiento. ¿Qué métodos se llaman y en qué orden?
2. **Modificación sencilla**: Agrega un nuevo tipo de usuario "Invitado" que herede de `Usuario`. ¿Qué archivos necesitas modificar?
3. **Depuración**: Si `gestor.cargarUsuarios()` no puede abrir `usuarios.json`, ¿qué ocurre? ¿Dónde se maneja ese error?
4. **Cálculo**: Si un bus está en lat=4.14, lon=-73.65 y una parada en lat=4.12, lon=-73.63, ¿cuál es la distancia aproximada? (Pista: 1 grado ≈ 111 km en el ecuador)
5. **Extensión**: ¿Cómo agregarías un nuevo método en `GestorArchivos` para guardar también las paradas? ¿Qué firma tendría?

---

# Sección 13: Análisis Focalizado: GestorArchivos.cpp

> **Enfoque:** Análisis ultra-detallado del archivo de persistencia. Cada línea, cada operador, cada estructura de control se explica.

## 13.1 Cabeceras y Alias

```cpp
#include "GestorArchivos.h"     // <- Declaración de la clase (necesario siempre)
#include <fstream>              // <- ifstream (leer) y ofstream (escribir)
#include <iostream>             // <- cerr (para mensajes de error)
#include "../include/json.hpp"  // <- nlohmann/json (parsear y generar JSON)

using json = nlohmann::json;    // <- Alias: ahora "json" en vez de "nlohmann::json"
```

**`using json = nlohmann::json;`**: Crea un **alias** (apodo). `nlohmann` es el **namespace** del autor (Niels Lohmann). `using json = nlohmann::json;` significa: "De ahora en adelante, cuando diga 'json', me refiero a 'nlohmann::json'".

## 13.2 Constructor

```cpp
GestorArchivos::GestorArchivos(const std::string& dirData) : rutaData(dirData) {}
```

**INPUT:** `dirData` — ruta al directorio de datos (ej. "data").  
**OUTPUT:** Ninguno (inicializa el objeto).

La **lista de inicializadores** (`: rutaData(dirData)`) asigna el valor del parámetro al atributo. Es equivalente a `this->rutaData = dirData;` pero más eficiente porque inicializa directamente en lugar de crear un string vacío y luego asignar.

## 13.3 `cargarBuses()`

```cpp
std::vector<Bus> GestorArchivos::cargarBuses() const {
    std::vector<Bus> lista;                           // <- Crea un vector vacío de buses
    std::ifstream archivo(rutaData + "/buses.json");  // <- Abre el archivo para lectura
    if (!archivo.is_open()) return lista;             // <- Si no se pudo abrir, devuelve vacío

    json j = json::parse(archivo);                    // <- Convierte el texto JSON a objeto manipulable
    for (const auto& b : j["buses"]) {                // <- Itera sobre cada bus en el arreglo JSON
        Bus busObj(
            b["idBus"].get<int>(),                    // <- Extrae idBus como entero
            b["placa"].get<std::string>(),            // <- Extrae placa como string
            b["capacidadMaxima"].get<int>(),
            b["capacidadActual"].get<int>(),
            b["estado"].get<bool>()
        );
        // Nuevos campos con valor por defecto
        busObj.setIndiceParadaActual(b.value("indiceParadaActual", -1));
        busObj.setIdRutaAsignada(b.value("idRutaAsignada", b["idBus"].get<int>()));
        lista.push_back(busObj);                      // <- Agrega el bus al vector (COPIA)
    }
    return lista;                                     // <- Devuelve el vector lleno
}
```

**Análisis línea por línea:**

- **`std::vector<Bus> lista;`** — Objeto vector vacío en el stack.
- **`std::ifstream archivo(rutaData + "/buses.json");`** — Input File Stream. Si el archivo no existe, `archivo` queda en estado de error.
- **`if (!archivo.is_open()) return lista;`** — `!` invierte: si NO se abrió, devuelve vector vacío. El sistema funciona con datos vacíos en lugar de fallar.
- **`json j = json::parse(archivo);`** — Lee TODO el archivo y construye un árbol JSON en memoria.
- **`for (const auto& b : j["buses"])`** — Range-based for loop. `const auto&` = referencia constante (evita copiar cada objeto JSON).
- **`b["idBus"].get<int>()`** — `.get<int>()` convierte el valor JSON a tipo `int` de C++.
- **`b.value("indiceParadaActual", -1)`** — `.value(clave, default)`: si la clave existe, devuelve su valor; si no, devuelve `-1`. Crucial para compatibilidad con versiones anteriores del JSON.

## 13.4 `cargarParadas()`

```cpp
std::vector<Parada> GestorArchivos::cargarParadas() const {
    std::vector<Parada> lista;
    std::ifstream archivo(rutaData + "/paradas.json");
    if (!archivo.is_open()) return lista;

    json j = json::parse(archivo);
    for (const auto& p : j) {  // <- paradas.json es un ARRAY DIRECTO (sin clave "paradas")
        lista.emplace_back(     // <- emplace_back construye DIRECTAMENTE dentro del vector
            p["idParada"].get<int>(),
            p["nombre"].get<std::string>(),
            p["ubicacion"]["latitud"].get<double>(),
            p["ubicacion"]["longitud"].get<double>(),
            p["ubicacion"]["altitud"].get<double>(),
            p["estado"].get<bool>(),
            p["ordenEnRuta"].get<int>()
        );
    }
    return lista;
}
```

**Diferencia clave con `cargarBuses()`:** `paradas.json` es un **array directo** `[ ... ]`, se itera sobre `j` directamente. `buses.json` tiene estructura `{ "buses": [ ... ] }`, se itera sobre `j["buses"]`.

**`emplace_back` vs `push_back`:**
```cpp
// push_back: 1) construye un objeto temporal, 2) lo COPIA al vector, 3) destruye el temporal
lista.push_back(Parada(id, nombre, ...));

// emplace_back: 1) construye el objeto DIRECTAMENTE en la memoria del vector (más eficiente)
lista.emplace_back(id, nombre, ...);
```

## 13.5 `cargarHorariosPorDefecto()`

```cpp
HorarioRuta GestorArchivos::cargarHorariosPorDefecto() const {
    HorarioRuta global;                                 // <- Objeto vacío (sin horarios)
    std::ifstream archivo(rutaData + "/horarios.json");
    if (!archivo.is_open()) return global;

    json j = json::parse(archivo);
    if (j.contains("configuracionHorarios")) {
        const auto& conf = j["configuracionHorarios"];
        if (conf.contains("salidasBarrio") && conf["salidasBarrio"].is_array()) {
            for (const auto& h : conf["salidasBarrio"])
                global.agregarSalidaBarrio(h.get<std::string>());
        }
        if (conf.contains("salidasUnillanos") && conf["salidasUnillanos"].is_array()) {
            for (const auto& h : conf["salidasUnillanos"])
                global.agregarSalidaUnillanos(h.get<std::string>());
        }
    }
    return global;
}
```

**`j.contains("configuracionHorarios")`:** Verifica si la clave existe en el objeto JSON, evitando errores si el archivo tiene otro formato. **`conf["salidasBarrio"].is_array()`:** Verifica que el valor sea un arreglo antes de iterar (doble validación).

## 13.6 `cargarRutas()` — El método más complejo

```cpp
std::vector<Ruta*> GestorArchivos::cargarRutas() const {
    std::vector<Ruta*> lista;
    std::ifstream archivo(rutaData + "/rutas.json");
    if (!archivo.is_open()) return lista;

    HorarioRuta horarioGlobal = cargarHorariosPorDefecto();

    json j = json::parse(archivo);
    for (const auto& r : j["rutas"]) {
        std::string tipo    = r["tipo"].get<std::string>();
        std::string pSalida = (r.contains("puntoSalida") && !r["puntoSalida"].is_null())
                              ? r["puntoSalida"].get<std::string>() : "";
        std::string zona    = r.value("zonaCentro", "");

        Ruta* ruta = nullptr;
        if (tipo == "RutaCentro") {
            ruta = new RutaCentro(id, nom, orig, dest, est, pSalida, zona);
        } else {
            ruta = new RutaBarrio(id, nom, orig, dest, est, pSalida);
        }

        for (int idP : r["paradas"])
            ruta->agregarParada(idP);

        if (r.contains("horarios") && r["horarios"].is_object() && !r["horarios"].empty()) {
            // Carga horarios específicos de la ruta
        } else {
            ruta->getHorario() = horarioGlobal;  // <- Asigna horarios por defecto
        }

        lista.push_back(ruta);  // <- Guarda el PUNTERO
    }
    return lista;
}
```

**¿Por qué `std::vector<Ruta*>` en vez de `std::vector<Ruta>`?** Porque `Ruta` es **abstracta** (método virtual puro `= 0`). No se pueden crear objetos `Ruta`, solo de sus hijas `RutaBarrio` y `RutaCentro`.

**`new RutaCentro(...)` y `new RutaBarrio(...)`:** Reserva memoria en el **heap**. Los objetos vivirán hasta que `~SistemaTransporte()` llame a `delete`.

**Carga de horarios — lógica condicional:**
```cpp
if (r.contains("horarios") && r["horarios"].is_object() && !r["horarios"].empty()) {
    // Usa horarios propios
} else {
    ruta->getHorario() = horarioGlobal;  // <- Asignación de objeto completo
}
```
- `contains`: ¿existe la clave?
- `is_object()`: ¿es un objeto JSON?
- `!empty()`: ¿tiene al menos un campo?

## 13.7 `cargarUsuarios()` — Polimorfismo en acción

```cpp
for (const auto& u : j["usuarios"]) {
    std::string tipo = u["tipo"].get<std::string>();

    if (tipo == "Estudiante") {
        lista.push_back(new Estudiante(id, nom, tel, cor, codEst, fac, prog, sem));
    } else if (tipo == "Administrador") {
        lista.push_back(new Administrador(id, nom, tel, cor, codAdmin));
    } else if (tipo == "Conductor") {
        lista.push_back(new Conductor(id, nom, tel, cor, codOp, exp, turno, busId));
    }
}
```

**Upcasting automático:** `new Estudiante(...)` devuelve `Estudiante*`, que se convierte automáticamente a `Usuario*`. El vector `vector<Usuario*>` contiene punteros de diferentes tipos, todos tratados como `Usuario*`.

## 13.8 Métodos de Guardado — Patrón común

Todos los `guardar*()` siguen el mismo patrón:

```cpp
void GestorArchivos::guardarIncidentes(const std::vector<Incidente>& incidentes) const {
    json j;                              // 1. Crear objeto JSON vacío
    j["incidentes"] = json::array();     // 2. Crear arreglo vacío
    for (const Incidente& inc : incidentes) {
        j["incidentes"].push_back({      // 3. Agregar objeto al arreglo
            {"descripcion", inc.getDescripcion()},
            {"estado",      inc.getEstado()},
            {"idIncidente", inc.getIdIncidente()},
            {"tipo",        inc.getTipo()}
        });
    }
    std::ofstream archivo(rutaData + "/incidentes.json");  // 4. Abrir para escritura
    if (archivo.is_open())
        archivo << j.dump(4);            // 5. Escribir JSON con indentación de 4 espacios
}
```

**`j.dump(4)`:** Serializa el objeto JSON a string con 4 espacios de indentación por nivel.

**`guardarUsuarios()` — con `static_cast`:**
```cpp
if (u->getTipo() == "Estudiante") {
    const Estudiante* e = static_cast<const Estudiante*>(u);  // <- Downcasting
    obj["codigoEstudiantil"] = e->getCodigoEstudiantil();
    // ...
}
```

---

# Sección 14: Análisis Focalizado: SistemaTransporte.cpp

> **Enfoque:** Análisis ultra-detallado del archivo orquestador (560 líneas). Cada línea explicada.

## 14.1 Cabeceras

```cpp
#include "SistemaTransporte.h"    // <- La propia clase
#include "Conductor.h"            // <- Para static_cast<Conductor*> en panelConductor()
#include "Estudiante.h"           // <- Para panelEstudiante() (uso implícito)
#include "Administrador.h"        // <- Para panelAdministrador()
#include "RutaCentro.h"           // <- Para static_cast<RutaCentro*> en guardar rutas
#include <windows.h>              // <- API de Windows: colores, consola, hora
#include <ctime>                  // <- Funciones de tiempo
#include <iostream>               // <- cout, cin, cerr
#include <sstream>                // <- ostringstream (formatear hora/fecha)
#include <iomanip>                // <- setw, setfill, setprecision
#include <map>                    // <- std::map (en autenticarUsuario)
#include <algorithm>              // <- find_if, count_if, any_of
```

## 14.2 Funciones Auxiliares Estáticas

```cpp
static const int ANCHO = 80;       // Ancho estándar de la consola
```

**`static` aquí:** Las siguientes funciones solo son visibles dentro de este archivo. Es una forma de hacerlas "privadas" del archivo.

### `limpiarPantalla()`

```cpp
static void limpiarPantalla() {
    COORD coord = {0, 0};                           // X=0, Y=0 (esquina sup. izq.)
    DWORD celdas, escritas;                          // Variables para resultados
    CONSOLE_SCREEN_BUFFER_INFO info;                 // Estructura para info de consola
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);      // Handle de la consola

    GetConsoleScreenBufferInfo(h, &info);             // Lee dimensiones
    celdas = info.dwSize.X * info.dwSize.Y;          // Total de caracteres
    FillConsoleOutputCharacter(h, ' ', celdas, coord, &escritas);  // Llena con espacios
    FillConsoleOutputAttribute(h, info.wAttributes, celdas, coord, &escritas); // Resetea colores
    SetConsoleCursorPosition(h, coord);              // Cursor a (0,0)
}
```

- `COORD coord = {0, 0}`: Inicialización **aggregate** de estructura de Windows. `{0, 0}` -> `coord.X = 0, coord.Y = 0`.
- `HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE)`: Obtiene un "mango" de la salida estándar.
- `&info`: Se pasa la **dirección** de `info`. La función llena sus campos.
- **¿Por qué no `system("cls")`?** `system("cls")` crea un proceso hijo (más lento y potencialmente inseguro). La API de Windows es más eficiente.

### `setColor(WORD c)`

```cpp
static void setColor(WORD c) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
```

**Wrapper** (envoltorio) para acortar la llamada. En vez de escribir `SetConsoleTextAttribute(GetStdHandle(...), FOREGROUND_GREEN | ...)`, escribimos `setColor(FOREGROUND_GREEN | ...)`.

### `leerInt()`

```cpp
static int leerInt() {
    int v;
    if (!(std::cin >> v)) {        // <- Si la entrada falla (ej. letras en vez de número)
        std::cin.clear();           // <- Limpia el estado de error de cin
        std::cin.ignore(10000, '\n'); // <- Descarta hasta 10000 chars o nueva línea
        return -1;                  // <- Devuelve -1 como indicador de error
    }
    std::cin.ignore(10000, '\n');   // <- Descarta el resto de la línea
    return v;
}
```

**`!(std::cin >> v)`**: El operador `!` verifica si el flujo está en estado de error (equivale a `cin.fail()`).
**`std::cin.clear()`**: Limpia la bandera de error. Sin esto, todas las lecturas futuras fallarían.
**`std::cin.ignore(10000, '\n')`**: Descarta hasta 10000 caracteres o hasta el salto de línea. Limpia el buffer.

## 14.3 Constructor y Destructor

```cpp
SistemaTransporte::SistemaTransporte(const std::string& dirData)
    : gestor(dirData), usuarioActual(nullptr) {}

SistemaTransporte::~SistemaTransporte() {
    for (Ruta* r : rutas) delete r;       // <- Libera cada ruta del heap
    for (Usuario* u : usuarios) delete u; // <- Libera cada usuario del heap
}
```

**¿Por qué `delete` y no `delete[]`?** Cada elemento fue creado con `new` individual (no `new[]`).

**¿Qué pasaría sin este destructor?** Los 12+ objetos `Ruta*` y 7+ objetos `Usuario*` quedarían en el heap para siempre = **fuga de memoria** (memory leak).

## 14.4 `imprimirEncabezado()`

```cpp
void SistemaTransporte::imprimirEncabezado() const {
    imprimirSeparador();
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "[ " << horaActual() << " - " << fechaActual() << " ]";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    std::cout << (usuarioActual ? " | " + usuarioActual->getEncabezado()
                                : " | SISTEMA DE GESTION DE TRANSPORTE") << '\n';
    imprimirSeparador();
}
```

**Operador ternario:** `(usuarioActual ? ... : ...)` — si no es `nullptr`, muestra el encabezado personalizado (polimórfico). Si es `nullptr`, muestra el título genérico.

## 14.5 `horaActual()`

```cpp
std::string SistemaTransporte::horaActual() const {
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::ostringstream ss;
    ss << std::setw(2) << std::setfill('0') << st.wHour << ':'
       << std::setw(2) << std::setfill('0') << st.wMinute << ':'
       << std::setw(2) << std::setfill('0') << st.wSecond;
    return ss.str();
}
```

**Ejemplo:** Si son las 8:5:3 -> sin formato sería "8:5:3". Con `setw(2)` y `setfill('0')` -> `"08:05:03"`.

## 14.6 `autenticarUsuario(int tipo)`

```cpp
Usuario* SistemaTransporte::autenticarUsuario(int tipo) {
    static const std::map<int, std::pair<std::string, std::string>> datos = {
        {1, {"Estudiante", "codigo estudiantil"}},
        {2, {"Conductor", "codigo de operador"}},
        {3, {"Administrador", "codigo de administrador"}}
    };

    std::cout << "\nIngrese su " << datos.at(tipo).second << ": ";
    int codigo = leerInt();

    auto it = std::find_if(usuarios.begin(), usuarios.end(),
        [tipo, codigo](Usuario* u) {
            return u->getTipo() == datos.at(tipo).first && u->validarCodigo(codigo);
        });

    if (it != usuarios.end()) return *it;

    setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    std::cout << "\n  [!] Codigo no valido.\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    Sleep(2000);
    return nullptr;
}
```

**`static const std::map<int, pair<string,string>>`:** `static` -> se crea UNA SOLA VEZ y se reusa. `const` -> no se puede modificar.

**`datos.at(tipo).second`:** `.at(tipo)` devuelve el `pair`. `.second` = label ("codigo estudiantil", etc.). `.first` = tipo ("Estudiante", etc.).

**La lambda:** Verifica **dos condiciones** con `&&`:
1. El tipo del usuario coincide con el esperado
2. El código ingresado es válido para ese usuario (llamada polimórfica a `validarCodigo`)

## 14.7 `panelEstudiante()` — Reserva de asientos

```cpp
void SistemaTransporte::panelEstudiante() {
    while (true) {
        // Mostrar directorio de rutas
        for (Ruta* r : rutas)
            std::cout << "[ " << std::setw(2) << r->getIdRuta() << "] "
                      << r->getNombre() << " (" << r->getTipo() << ")\n";
        std::cout << "\n[ 0] Volver\n\n> Ruta: ";
        int idRuta = leerInt();
        if (idRuta == 0) break;

        // Buscar ruta por ID
        auto it = std::find_if(rutas.begin(), rutas.end(),
            [idRuta](Ruta* r) { return r->getIdRuta() == idRuta; });
        if (it == rutas.end()) { Sleep(1500); continue; }

        Ruta* rutaEncontrada = *it;
        // Mostrar detalles: paradas, horarios...

        // Reservar asiento?
        if (toupper(abordar) == 'S') {
            Bus* bus = buscarBusPorId(rutaEncontrada->getIdRuta());
            if (!bus && !buses.empty()) {
                // Buscar cualquier bus con espacio
                auto itBus = std::find_if(buses.begin(), buses.end(),
                    [](const Bus& b) {
                        return b.getCapacidadActual() < b.getCapacidadMaxima() && b.getEstado();
                    });
                if (itBus != buses.end()) bus = &*itBus;
            }
            if (bus && bus->getCapacidadActual() < bus->getCapacidadMaxima()) {
                bus->setCapacidadActual(bus->getCapacidadActual() + 1);
                gestor.guardarBuses(buses);  // <- Persistencia inmediata
            }
        }
    }
}
```

**Búsqueda de bus en dos pasos:**
1. `buscarBusPorId(rutaEncontrada->getIdRuta())` — busca el bus cuyo ID coincida con el ID de la ruta
2. Si no encuentra: busca **cualquier bus** activo con espacio disponible

## 14.8 `panelConductor()` — GPS, llegadas e incidentes

```cpp
void SistemaTransporte::panelConductor() {
    Conductor* cond = static_cast<Conductor*>(usuarioActual);
    Bus* bus = buscarBusPorId(cond->getBusAsignado());
    Ruta* ruta = bus ? buscarRutaPorBus(bus->getPlaca()) : nullptr;
    std::map<int, std::string> horasLlegada; // idParada -> hora de llegada

    while (true) {
        // Mostrar info del bus, ruta, horarios
        // Mostrar proximidad a paradas (visitadas vs pendientes)

        int op = leerInt();

        if (op == 1) {  // LLEGADA A PARADA
            int idxActual = bus->getIndiceParadaActual();
            if (idxActual + 1 < (int)ruta->getIdsParadas().size()) {
                // Aún hay paradas por visitar
                idxActual++;
                int idSig = ruta->getIdsParadas()[idxActual];
                auto it = std::find_if(paradas.begin(), paradas.end(),
                    [idSig](const Parada& p) { return p.getIdParada() == idSig; });
                if (it != paradas.end()) {
                    bus->setUbicacion(it->getUbicacion().getLatitud(),
                                      it->getUbicacion().getLongitud(), it->getAltitud());
                    bus->setIndiceParadaActual(idxActual);
                    horasLlegada[idSig] = horaActual();
                    gestor.guardarBuses(buses);
                }
            } else {
                // FIN DE RUTA: cambia a la siguiente ruta
                bus->setIndiceParadaActual(-1);
                // Calcular siguiente ruta
                int idRutaActual = bus->getIdRutaAsignada();
                auto itRuta = std::find_if(rutas.begin(), rutas.end(),
                    [idRutaActual](Ruta* r) { return r->getIdRuta() == idRutaActual; });
                int nextIndex = 0;
                if (itRuta != rutas.end()) {
                    nextIndex = std::distance(rutas.begin(), itRuta) + 1;
                    if (nextIndex >= (int)rutas.size()) nextIndex = 0;
                }
                if (!rutas.empty()) {
                    bus->setIdRutaAsignada(rutas[nextIndex]->getIdRuta());
                    ruta = rutas[nextIndex];
                }
                horasLlegada.clear();
                gestor.guardarBuses(buses);
            }
        } else if (op == 2) {  // REPORTAR INCIDENTE
            std::string desc, tipo;
            std::cout << "\nDescripcion: "; std::getline(std::cin, desc);
            std::cout << "Tipo (Mecanico/Accidente/Otro): "; std::getline(std::cin, tipo);
            incidentes.emplace_back(incidentes.size() + 1, desc, tipo, "Abierto");
            gestor.guardarIncidentes(incidentes);
        } else if (op == 3) {  // FINALIZAR TURNO
            break;
        }
    }
}
```

**`static_cast<Conductor*>(usuarioActual)`:** Convierte `Usuario*` a `Conductor*`. Es seguro porque SOLO se entra a `panelConductor()` si el usuario es Conductor.

**Lógica de Llegada a Parada (Opción 1):**
- **Caso A (quedan paradas):** Incrementa índice, busca la parada, fija el GPS exactamente en la parada con `setUbicacion()`, registra hora de llegada.
- **Caso B (fin de ruta):** Reinicia índice a -1, calcula siguiente ruta usando `std::distance()` para obtener el índice actual y sumar 1 (vuelve a la primera si llega al final).

**Lógica de Incidente (Opción 2):**
```cpp
incidentes.emplace_back(incidentes.size() + 1, desc, tipo, "Abierto");
```
El nuevo ID es el tamaño actual + 1 (IDs autoincrementales).

## 14.9 `panelAdministrador()` y sub-métodos

```cpp
void SistemaTransporte::panelAdministrador() {
    while (true) {
        // Mostrar estado global
        int abiertos = std::count_if(incidentes.begin(), incidentes.end(),
            [](const Incidente& i) { return i.getEstado() == "Abierto"; });
        // Menu: [1] Rutas [2] Usuarios [3] Buses [4] Horarios [5] Incidentes [6] Volver
    }
}
```

**`std::count_if`:** CUENTA cuántos incidentes tienen estado "Abierto". Recorre TODO el vector.

### `gestionarRutas()` — Toggle booleano
```cpp
r->setEstado(!r->getEstado());  // <- Invierte: true->false, false->true
```

### `gestionarBuses()` — Validación de capacidad
```cpp
if (cap >= 0 && cap <= bus->getCapacidadMaxima()) {
    bus->setCapacidadActual(cap);
    gestor.guardarBuses(buses);
}
```

### `gestionarIncidentes()` — Color dinámico
```cpp
setColor(inc.getEstado() == "Abierto" ? FOREGROUND_RED | FOREGROUND_INTENSITY
                                      : FOREGROUND_GREEN | FOREGROUND_INTENSITY);
```
Abiertos en **rojo brillante**, cerrados en **verde brillante**.

## 14.10 `iniciar()` — El punto de partida

```cpp
void SistemaTransporte::iniciar() {
    // FASE 1: CARGA DE DATOS
    buses = gestor.cargarBuses();
    paradas = gestor.cargarParadas();
    incidentes = gestor.cargarIncidentes();
    rutas = gestor.cargarRutas();
    usuarios = gestor.cargarUsuarios();

    // FASE 2: CONFIGURACIÓN DE CONSOLA
    SetConsoleTitle("Sistema de Gestion de Transporte - Unillanos");
    SetConsoleOutputCP(65001);  // UTF-8 salida
    SetConsoleCP(65001);        // UTF-8 entrada

    // FASE 3: BUCLE PRINCIPAL
    while (true) {
        usuarioActual = nullptr;
        int op = mostrarMenuPrincipal();
        if (op == 4) { Sleep(1000); break; }
        if (op < 1 || op > 3) continue;
        usuarioActual = autenticarUsuario(op);
        if (!usuarioActual) continue;
        if (op == 1) panelEstudiante();
        else if (op == 2) panelConductor();
        else panelAdministrador();
    }
}
```

**Fase 2 — Consola:**
- `SetConsoleTitle("...")`: Cambia el título de la ventana
- `SetConsoleOutputCP(65001)`: 65001 = CP_UTF8. Sin esto, tildes y ñ se ven como símbolos extraños
- `SetConsoleCP(65001)`: Lo mismo para entrada del teclado

**Fase 3 — Bucle:**
- `usuarioActual = nullptr`: Reinicia la sesión al volver al menú
- `autenticarUsuario(op)`: Si falla, vuelve al menú
- Cada panel tiene su propio bucle interno que termina al elegir "Volver" o "Finalizar turno"

---

# Sección 15: Vulnerabilidades y Debilidades del Diseño

## 15.1 Copia superficial (shallow copy) en Bus

Cuando `GestorArchivos::cargarBuses()` hace `lista.push_back(busObj)`, se crea una **copia** de `busObj`. Pero `Bus` contiene un `UbicacionGPS*` (puntero). La copia copia el **puntero**, no el objeto apuntado:

```cpp
Bus busA;
busA.setUbicacion(4.14, -73.65, 463, 30);  // new UbicacionGPS en 0x1000

Bus busB = busA;  // <- Copia: busB.ubicacion también apunta a 0x1000

// busA se destruye: ~Bus() -> delete ubicacion (0x1000)
// busB se destruye: ~Bus() -> delete ubicacion (0x1000) -> ¡DOBLE DELETE!
```

**¿Por qué no falla en la práctica?** Porque los buses se copian al cargar y nunca se destruyen los originales (los temporales del bucle se destruyen, pero como `ubicacion` es `nullptr` porque no se llamó a `setUbicacion()`, el `delete nullptr` es seguro). El problema se manifiesta si se llama a `setUbicacion()` en un bus temporal.

## 15.2 Uso de `new`/`delete` sin `unique_ptr`

Los vectores `rutas` y `usuarios` almacenan punteros **crudos**. Si ocurre una excepción entre el `new` y el `push_back`, el objeto se pierde (fuga). La responsabilidad de llamar a `delete` recae en `~SistemaTransporte()`, lo que es frágil.

**Alternativa más segura:** `std::vector<std::unique_ptr<Ruta>>` — los punteros se liberan automáticamente cuando el vector se destruye o cuando se lanza una excepción.

## 15.3 Dependencia de Windows

`<windows.h>` hace que el programa **solo funcione en Windows**. No es portable a Linux o macOS. Para hacerlo portable, habría que usar una librería como `ncurses` o un sistema de UI multiplataforma.

---

## Mapa de llamadas completo de SistemaTransporte.cpp

```
SistemaTransporte::iniciar()
  +-- gestor.cargarBuses()              ->  GestorArchivos.cpp
  +-- gestor.cargarParadas()            ->  GestorArchivos.cpp
  +-- gestor.cargarIncidentes()         ->  GestorArchivos.cpp
  +-- gestor.cargarRutas()              ->  GestorArchivos.cpp
  |     +-- cargarHorariosPorDefecto()  ->  GestorArchivos.cpp
  +-- gestor.cargarUsuarios()           ->  GestorArchivos.cpp
  +-- SetConsoleTitle/OutputCP/SetCP()  ->  windows.h
  +-- while(true):
        +-- mostrarMenuPrincipal()
        |     +-- limpiarPantalla()
        |     +-- imprimirEncabezado()
        |     +-- horaActual() -> GetLocalTime()
        |     +-- leerInt()
        +-- autenticarUsuario(tipo)
        |     +-- find_if(usuarios, lambda con validarCodigo())
        +-- panelEstudiante()
        |     +-- find_if(rutas, ...)
        |     +-- find_if(paradas, ...)
        |     +-- buscarBusPorId() -> find_if(buses, ...)
        |     +-- gestor.guardarBuses()
        +-- panelConductor()
        |     +-- static_cast<Conductor*>
        |     +-- buscarBusPorId() + buscarRutaPorBus()
        |     +-- incidentesDeBus() -> string::find()
        |     +-- any_of()
        |     +-- mostrarProximidadBus()
        |     |     +-- find_if(paradas, ...)
        |     |     +-- distanciaHacia() + tiempoHastaParada()
        |     +-- [1] setUbicacion() + guardarBuses()
        |     +-- [2] emplace_back() + guardarIncidentes()
        +-- panelAdministrador()
              +-- count_if()
              +-- gestionarRutas() -> toggle + guardarRutas()
              +-- gestionarBuses() -> guardarBuses()
              +-- gestionarHorarios() -> guardarRutas()
              +-- gestionarIncidentes() -> cerrar() + guardarIncidentes()
```

---

> **Fin del documento maestro.**  
> *Creado con propósitos educativos. Proyecto original: Universidad de los Llanos, Villavicencio, Meta, Colombia.*  
> *C++17 | Windows Console API | nlohmann/json | CMake*
