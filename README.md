# Sistema de Gestión de Rutas de Transporte Universitario
**Universidad de los Llanos – Villavicencio, Meta**

---

## Descripción
Sistema en C++ para gestionar las rutas de transporte universitario entre distintos barrios de la ciudad y el campus de Unillanos. Incluye paneles diferenciados para Estudiantes, Conductores y Administradores, con interfaz de consola construida con `windows.h`.

---

## Estructura del Proyecto
```
SistemaTransporte/
├── include/          # Cabeceras (.h) de todas las clases
├── src/              # Implementaciones (.cpp)
├── data/             # Archivos JSON de datos persistentes
├── main.cpp          # Punto de entrada
├── Makefile          # Compilacion con MinGW (Windows)
├── CMakeLists.txt    # Compilacion con CMake
└── README.md
```

---

## Compilación (Windows con MinGW)
```
mingw32-make
```
O con CMake:
```
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```
El ejecutable `SistemaTransporte.exe` quedará en la raíz (o en `build/`).  
La carpeta `data/` debe estar junto al ejecutable.

---

## Credenciales de prueba (tomadas de usuarios.json)
| Perfil        | Nombre            | Codigo   |
|---------------|-------------------|----------|
| Estudiante    | Pedro Rodríguez   | 2024001  |
| Conductor     | Carlos García     | 101      |
| Conductor     | Juan Pérez        | 102      |
| Administrador | María López       | 1        |

---

## Funcionalidades principales
- **Estudiante**: Visor paginado de rutas y paradas (Enter / ESC).
- **Conductor**: Panel con bus asignado, incidentes, horarios y proximidad GPS en tiempo real.
- **Administrador: Gestión de rutas, usuarios, flota, horarios e incidentes; cambios reflejados en los JSON inmediatamente.
- **Tiempo real**: Hora y fecha obtenidas con `GetLocalTime` de Windows.h; avanzan en cada refresco de pantalla.
- **GPS**: Cálculo de distancia (Haversine) y tiempo estimado de llegada del bus a cada parada.


Aquí está el análisis COMPLETO y detallADO del proyecto **Sistema de Gestión de Rutas de Transporte Universitario**:

---

## VISIÓN GENERAL

```
SistemaTransporte/           ← Proyecto C++17 para Universidad de los Llanos, Villavicencio
├── include/    (15 .h)     ← Cabeceras de clases
├── src/        (15 .cpp)   ← Implementaciones
├── data/       (6 .json)   ← Datos persistentes (buses, rutas, paradas, etc.)
├── main.cpp                ← Punto de entrada
├── CMakeLists.txt          ← Sistema de construcción
└── .gitignore
```

---

## 1. `include/Coordenada.h` + `src/Coordenada.cpp`

**Propósito:** Representar un punto geográfico (latitud/longitud). Es la **clase base de UbicacionGPS** y se usa por **composición** dentro de `Parada`.

```cpp
class Coordenada {
private:
    double latitud;
    double longitud;
public:
    Coordenada();                          // Constructor por defecto (0,0)
    Coordenada(double lat, double lon);    // Constructor paramétrico
    // Getters/Setters...
    double distanciaHacia(const Coordenada& otra) const;  // ← FÓRMULA DE HAVERSINE
};
```

**`distanciaHacia()`** implementa la **fórmula del Haversine**:
```cpp
double dLat = (otra.latitud - latitud) * PI / 180.0;
double dLon = (otra.longitud - longitud) * PI / 180.0;
double a = sin(dLat/2)^2 + cos(lat) * cos(otra.lat) * sin(dLon/2)^2;
double c = 2 * atan2(√a, √(1-a));
return RADIO_TIERRA * c;  // 6371000 metros
```

**¿Dónde se usa Coordenada?**
- **Composición en `Parada`**: `Parada::ubicacion` (tipo `Coordenada`) → almacena dónde está cada parada
- **Herencia en `UbicacionGPS`**: `UbicacionGPS : public Coordenada` → añade altitud y velocidad
- **En `Bus::tiempoHastaParada()`**: crea una `Coordenada destino(lat, lon)` para medir distancia
- **En `Parada::distanciaA()`**: crea `Coordenada otro(lat, lon)` y llama a `ubicacion.distanciaHacia(otro)`

```cpp
// Ejemplo concreto en SistemaTransporte.cpp:96-97 → mostrarProximidadBus()
double metros = it->distanciaA(bus->getUbicacion()->getLatitud(),
                                bus->getUbicacion()->getLongitud());
double dist = bus->tiempoHastaParada(it->getUbicacion().getLatitud(),
                                      it->getUbicacion().getLongitud());
```

**Librerías usadas:** `<cmath>` → `std::sin`, `std::cos`, `std::atan2`, `std::sqrt`

---

## 2. `include/UbicacionGPS.h` + `src/UbicacionGPS.cpp`

**Propósito:** Extiende `Coordenada` agregando altitud y velocidad para cálculos de tiempo de viaje.

```cpp
class UbicacionGPS : public Coordenada {  // ← HEREDA DE Coordenada
private:
    double altitud;
    double velocidadKmh;        // 30 km/h por defecto
public:
    double tiempoEstimadoMinutos(double distanciaMetros) const;
};
```

**`tiempoEstimadoMinutos()`**: `(distanciaKm / velocidadKmh) * 60.0` → simple regla de 3.

**¿Dónde se usa UbicacionGPS?**
- **Como puntero dinámico en `Bus`**: `UbicacionGPS* ubicacion` → se crea con `new` en `setUbicacion()` y se destruye en el destructor de Bus
- **En `Bus::simularMovimiento()`**: mueve el bus un 5% hacia una coordenada destino
- **En `Bus::tiempoHastaParada()`**: calcula minutos hasta una parada
- **En `SistemaTransporte::mostrarProximidadBus()`**: muestra distancia y tiempo a cada parada

```cpp
// Ejemplo en Bus.cpp:61-63 → simularMovimiento()
double deltaLat = (latDest - ubicacion->getLatitud()) * 0.05;   // 5% del camino
ubicacion->setLatitud(ubicacion->getLatitud() + deltaLat);       // Avanza
```

---

## 3. `include/Usuario.h` + `src/Usuario.cpp`

**Propósito:** Clase **abstracta base** para los 3 roles del sistema. Define la interfaz polimórfica.

```cpp
class Usuario {
private:
    int idUsuario;
    std::string nombre, telefono, correo, tipo;  // tipo = "Estudiante"|"Conductor"|"Administrador"
public:
    virtual ~Usuario() {}                         // Destructor virtual para polimorfismo
    virtual std::string getEncabezado() const = 0;  // ← PURO: cada rol muestra info diferente
    virtual bool validarCodigo(int codigo) const = 0; // ← PURO: cada rol valida su código
};
```

**¿Dónde se usa Usuario?**
- **Como base polimórfica**: `std::vector<Usuario*> usuarios` en SistemaTransporte
- **En `autenticarUsuario()`**: itera sobre `usuarios` y llama a `u->validarCodigo(codigo)` y `u->getTipo()`
- **En `imprimirEncabezado()`**: `usuarioActual->getEncabezado()` muestra el encabezado personalizado
- **En `gestionarUsuarios()`**: itera mostrando `u->getNombre()` y `u->getTipo()`

---

## 4. `include/Estudiante.h` + `src/Estudiante.cpp`

```cpp
class Estudiante : public Usuario {
private:
    int codigoEstudiantil;    // ← código único para login
    std::string facultad, programa;
    int semestre;
    bool carnetActivo;
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

---

## 5. `include/Conductor.h` + `src/Conductor.cpp`

```cpp
class Conductor : public Usuario {
private:
    int codigoOperador;      // ← código para login
    std::string experiencia, turno;  // turno = "Mañana"|"Tarde"
    int busAsignado;         // ← id del bus que conduce (FK a Bus.idBus)
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
- **En `panelConductor()`**: `Conductor* cond = static_cast<Conductor*>(usuarioActual)` → casting down para obtener `getBusAsignado()`, que se usa para buscar el bus: `Bus* bus = buscarBusPorId(cond->getBusAsignado())`

```cpp
// SistemaTransporte.cpp:222-225
Conductor* cond = static_cast<Conductor*>(usuarioActual);
Bus* bus = buscarBusPorId(cond->getBusAsignado());  // ← busca el bus que conduce
Ruta* ruta = bus ? buscarRutaPorBus(bus->getPlaca()) : nullptr;
```

---

## 6. `include/Administrador.h` + `src/Administrador.cpp`

```cpp
class Administrador : public Usuario {
private:
    int codigoAdmin;     // ← código para login
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

---

## 7. `include/Bus.h` + `src/Bus.cpp`

```cpp
class Bus {
private:
    int idBus;
    std::string placa;
    int capacidadMaxima, capacidadActual;
    bool estado;                 // true=activo, false=inactivo
    UbicacionGPS* ubicacion;     // ← PUNTERO DINÁMICO (se crea con new)
public:
    ~Bus();                      // delete ubicacion;
    void setUbicacion(double lat, double lon, double alt, double vel);
    void simularMovimiento(double latDest, double lonDest);
    double tiempoHastaParada(double latParada, double lonParada) const;
    bool operator<(const Bus& otro) const;  // Ordena por espacio disponible
};
```

**Detalles clave:**
- `ubicacion` es un **puntero crudo** (raw pointer) → `new UbicacionGPS(lat, lon, alt, vel)` en `setUbicacion()`
- `~Bus()` hace `delete ubicacion` → libera memoria
- `simularMovimiento()` mueve 5% hacia el destino en cada llamada
- `operator<` ordena buses por **mayor espacio libre** (capacidadMax - capacidadActual)

```cpp
// Bus.cpp:84-86 → operator<
return (capacidadMaxima - capacidadActual) > (otro.capacidadMaxima - otro.capacidadActual);
// Mayor espacio disponible primero
```

**¿Dónde se usa Bus?**
- **En `panelEstudiante()`**: se busca un bus con espacio disponible y se incrementa `capacidadActual`
- **En `panelConductor()`**: se muestra info del bus, se simula llegada a parada (`bus->simularMovimiento()`), se reportan incidentes asociados a su placa
- **En `gestionarBuses()`**: admin modifica capacidad
- **En `mostrarProximidadBus()`**: calcula distancia y tiempo a cada parada

---

## 8. `include/Parada.h` + `src/Parada.cpp`

```cpp
class Parada {
private:
    int idParada;
    std::string nombre;
    Coordenada ubicacion;           // ← COMPOSICIÓN: usa Coordenada
    double altitud;
    bool estado;
    int ordenEnRuta;               // ← posición en la ruta
public:
    double distanciaA(double lat, double lon) const;  // distancia desde esta parada hasta un punto
    bool operator<(const Parada& otra) const;          // ordena por ordenEnRuta
};
```

**¿Dónde se usa Parada?**
- **En `panelEstudiante()`**: muestra las paradas de cada ruta
- **En `panelConductor()`**: `simularMovimiento()` hacia la primera parada; `mostrarProximidadBus()` recorre `ruta->getIdsParadas()`, busca cada `Parada` por ID y calcula distancia

```cpp
// SistemaTransporte.cpp:92-103 → mostrarProximidadBus()
for (int idP : ruta->getIdsParadas()) {
    auto it = std::find_if(paradas.begin(), paradas.end(),
        [idP](const Parada& p) { return p.getIdParada() == idP; });
    double metros = it->distanciaA(bus->getUbicacion()->getLatitud(),
                                    bus->getUbicacion()->getLongitud());
    double dist = bus->tiempoHastaParada(it->getUbicacion().getLatitud(),
                                          it->getUbicacion().getLongitud());
}
```

**Los datos JSON** contienen **67 paradas** reales de Villavicencio con coordenadas geográficas precisas (desde "Primax La Coralina" hasta "Segundo puente peatonal Samán de Rivera").

---

## 9. `include/Incidente.h` + `src/Incidente.cpp`

```cpp
class Incidente {
private:
    int idIncidente;
    std::string descripcion, tipo, estado;  // estado = "Abierto"|"Cerrado"
public:
    void cerrar();  // estado = "Cerrado"
};
```

**¿Dónde se usa Incidente?**
- **En `panelConductor()`**: el conductor reporta incidentes → `incidentes.emplace_back(id, desc, tipo, "Abierto")` y `gestor.guardarIncidentes()`
- **En `panelAdministrador()` → `gestionarIncidentes()`**: lista todos los incidentes y permite cerrarlos (`inc.cerrar()`)
- **En `incidentesDeBus()`**: filtra incidentes cuya descripción contiene la placa del bus
- **En `panelConductor()`**: `std::any_of()` revisa si hay incidentes abiertos para mostrar "ALERTA" o "OPERATIVO"

```cpp
// SistemaTransporte.cpp:242-245
auto inc = incidentesDeBus(bus->getPlaca());
bool abierto = std::any_of(inc.begin(), inc.end(),
    [](Incidente* i) { return i->getEstado() == "Abierto"; });
// Muestra "ALERTA" en rojo o "OPERATIVO" en verde
```

---

## 10. `include/HorarioRuta.h` + `src/HorarioRuta.cpp`

```cpp
class HorarioRuta {
private:
    std::vector<std::string> salidasBarrio;     // horas "HH:MM:SS"
    std::vector<std::string> salidasUnillanos;
public:
    void agregarSalidaBarrio/Unillanos(const std::string& hora);
    std::string formatearBarrio/Unillanos() const;  // "05:10 | 06:10 | ..."
};
```

**¿Dónde se usa HorarioRuta?**
- **Por composición en `Ruta`**: `HorarioRuta horario`
- **En `GestorArchivos::cargarRutas()`**: si la ruta JSON tiene `horarios` propios los usa; si no, usa los horarios globales de `horarios.json` (cargados por `cargarHorariosPorDefecto()`)
- **En `panelEstudiante()`**: muestra `rutaEncontrada->getHorario().formatearBarrio()`
- **En `panelConductor()`**: muestra horarios
- **En `gestionarHorarios()`**: admin agrega nuevas horas con `agregarSalidaBarrio/Unillanos()`

---

## 11. `include/Ruta.h` + `src/Ruta.cpp`

```cpp
class Ruta {
private:
    int idRuta;
    std::string nombre, origen, destino;
    bool estado;
    std::string tipo;           // "RutaBarrio"|"RutaCentro"
    std::string puntoSalida;    // dirección exacta de salida
    HorarioRuta horario;        // ← COMPOSICIÓN
    std::vector<int> idsParadas;  // ← IDs de paradas (no objetos completos)
public:
    virtual ~Ruta() {}
    virtual std::string getInfoAdicional() const = 0;  // ← PURO: cada subclase da info distinta
    bool operator<(const Ruta& otra) const;   // por idRuta
    bool operator==(const Ruta& otra) const;  // por idRuta
};
```

**¿Dónde se usa Ruta?**
- **Como base polimórfica**: `std::vector<Ruta*> rutas` en SistemaTransporte
- **En todos los paneles**: se itera sobre rutas para mostrar info
- **En `buscarRutaPorBus()`**: asume que el bus i está en la ruta i (índice)

---

## 12. `include/RutaBarrio.h` + `src/RutaBarrio.cpp`

```cpp
class RutaBarrio : public Ruta {
public:
    RutaBarrio(...) : Ruta(id, nom, orig, dest, est, "RutaBarrio", pSalida) {}
    std::string getInfoAdicional() const override {
        return "Tipo: Ruta de Barrio | Trayecto: " + getOrigen() + " -> " + getDestino();
    }
};
```

**10 rutas de barrio** en datos: Amarilo, La Grama, Maracos, Postobón, Covisán, Villacentro, Terminal, Reliquia, Viva, Montecarlo, Porfía.

---

## 13. `include/RutaCentro.h` + `src/RutaCentro.cpp`

```cpp
class RutaCentro : public Ruta {
private:
    std::string zonaCentro;   // "Centro"
public:
    RutaCentro(...) : Ruta(id, nom, orig, dest, est, "RutaCentro", pSalida), zonaCentro(zona) {}
    std::string getInfoAdicional() const override {
        return "Tipo: Ruta Centro | Zona: " + zonaCentro + " | " + getOrigen() + " -> " + getDestino();
    }
};
```

**2 rutas de centro**: Parque (id 4) — la única ruta Centro con zona "Centro".

---

## 14. `include/GestorArchivos.h` + `src/GestorArchivos.cpp`

**Propósito:** CRUD completo sobre archivos JSON. Es el **puente de persistencia**.

```cpp
class GestorArchivos {
private:
    std::string rutaData;     // "data" (directorio de JSONs)
public:
    std::vector<Bus> cargarBuses() const;           // → buses.json
    std::vector<Parada> cargarParadas() const;       // → paradas.json
    std::vector<Incidente> cargarIncidentes() const; // → incidentes.json
    std::vector<Ruta*> cargarRutas() const;          // → rutas.json
    std::vector<Usuario*> cargarUsuarios() const;    // → usuarios.json
    HorarioRuta cargarHorariosPorDefecto() const;    // → horarios.json
    void guardarIncidentes(const std::vector<Incidente>&);
    void guardarBuses(const std::vector<Bus>&);
    void guardarUsuarios(const std::vector<Usuario*>&);
    void guardarRutas(const std::vector<Ruta*>&);
};
```

**Librería nlohmann/json (`json.hpp`):** 26,076 líneas, v3.12.0

Uso específico en el proyecto:

| Operación | Código | Ejemplo en GestorArchivos.cpp |
|-----------|--------|-------------------------------|
| **Parsear archivo** | `json j = json::parse(archivo)` | Línea 16: `json j = json::parse(archivo);` |
| **Acceder a arreglo** | `j["buses"]` | Línea 18: `for (const auto& b : j["buses"])` |
| **Extraer valor** | `b["idBus"].get<int>()` | Línea 19: `b["idBus"].get<int>()` |
| **Valor con default** | `r.value("zonaCentro", "")` | Línea 70: `std::string zona = r.value("zonaCentro", "");` |
| **Verificar existencia** | `r.contains("horarios")` | Línea 80: `if (r.contains("horarios") && r["horarios"].is_object() && !r["horarios"].empty())` |
| **Crear arreglo vacío** | `j["buses"] = json::array()` | Línea 131: `j["buses"] = json::array();` |
| **Agregar objeto** | `j["buses"].push_back({...})` | Línea 133: `j["buses"].push_back({{"idBus", b.getIdBus()}, ...})` |
| **Serializar** | `archivo << j.dump(4)` | Línea 137: `archivo << j.dump(4)` |
| **Verificar tipo** | `r["puntoSalida"].is_null()` | Línea 63: `!r["puntoSalida"].is_null()` |

**Flujo de carga de rutas**: `cargarRutas()` → `cargarHorariosPorDefecto()` → para cada ruta, si tiene `horarios` propios los usa, si no, asigna `horarioGlobal`.

---

## 15. `include/SistemaTransporte.h` + `src/SistemaTransporte.cpp`

**Propósito:** Orquestador principal. 482 líneas, el archivo más grande.

```cpp
class SistemaTransporte {
private:
    GestorArchivos gestor;
    std::vector<Bus> buses;
    std::vector<Parada> paradas;
    std::vector<Incidente> incidentes;
    std::vector<Ruta*> rutas;       // ← polimórficos
    std::vector<Usuario*> usuarios; // ← polimórficos
    Usuario* usuarioActual;         // ← sesión activa
    // + métodos privados...
public:
    SistemaTransporte(const std::string& dirData);
    ~SistemaTransporte();  // delete rutas[i], delete usuarios[i]
    void iniciar();        // entry point
};
```

### **Librería `windows.h`**

| Función | Dónde se usa | Qué hace |
|---------|-------------|----------|
| `GetStdHandle(STD_OUTPUT_HANDLE)` | limpiarPantalla(), setColor() | Obtiene el handle de la consola |
| `SetConsoleTextAttribute(h, color)` | setColor() | Cambia color de texto |
| `SetConsoleTitle("...")` | iniciar() (línea 463) | Pone título a la ventana |
| `SetConsoleOutputCP(65001)` | iniciar() (línea 464) | Activa UTF-8 |
| `GetConsoleScreenBufferInfo(h, &info)` | limpiarPantalla() | Obtiene tamaño de pantalla |
| `FillConsoleOutputCharacter(h, ' ', ...)` | limpiarPantalla() | Llena pantalla con espacios |
| `FillConsoleOutputAttribute(h, attr, ...)` | limpiarPantalla() | Resetea atributos de color |
| `SetConsoleCursorPosition(h, coord)` | limpiarPantalla() | Mueve cursor a (0,0) |
| `GetLocalTime(&st)` | horaActual(), fechaActual() | Obtiene hora/fecha del sistema |
| `Sleep(ms)` | En todos los paneles | Pausa la ejecución |

**Colores usados:**
- `FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY` → cian brillante (separadores, títulos de sección)
- `FOREGROUND_GREEN | FOREGROUND_INTENSITY` → verde brillante (menús, éxito)
- `FOREGROUND_RED | FOREGROUND_INTENSITY` → rojo brillante (errores, alertas)
- `FOREGROUND_BLUE | FOREGROUND_INTENSITY` → azul brillante (subtítulos)
- `FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY` → cian (vehículo, ruta, opciones)
- `FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE` → gris claro (texto normal, reset)

### **Métodos clave de SistemaTransporte:**

#### `iniciar()` (línea 459)
```cpp
buses = gestor.cargarBuses();      // Carga JSON → vectores
paradas = gestor.cargarParadas();
incidentes = gestor.cargarIncidentes();
rutas = gestor.cargarRutas();
usuarios = gestor.cargarUsuarios();
SetConsoleTitle("Sistema de Gestion de Transporte - Unillanos");
SetConsoleOutputCP(65001);  // UTF-8

while (true) {               // ← Loop principal infinito
    usuarioActual = nullptr;
    int op = mostrarMenuPrincipal();  // 1=Est, 2=Cond, 3=Admin, 4=Salir
    if (op == 4) break;
    usuarioActual = autenticarUsuario(op);  // busca por código
    if (!usuarioActual) continue;           // si no existe, reintenta
    if (op == 1) panelEstudiante();
    else if (op == 2) panelConductor();
    else panelAdministrador();
}
```

#### `autenticarUsuario()` (línea 119)
Usa un `std::map<int, pair<string,string>>` para mapear opción de menú → tipo de usuario y label. Busca con `std::find_if` un usuario que coincida en tipo y código.

#### `panelEstudiante()` (línea 142)
- Muestra listado de rutas
- Usuario selecciona una ruta → ve paradas y horarios
- Opción de **reservar asiento**:
  1. Busca bus por ID de ruta: `buscarBusPorId(ruta.idRuta)`
  2. Si no hay, busca cualquier bus con espacio: `find_if(capacidadActual < capacidadMaxima && estado)`
  3. Si encuentra: `bus->setCapacidadActual(bus->getCapacidadActual() + 1)`
  4. Guarda cambios: `gestor.guardarBuses(buses)`

#### `panelConductor()` (línea 222)
- Muestra info del vehículo + alerta si hay incidentes abiertos
- Muestra ruta asignada, horarios, **proximidad del bus a cada parada** (distancia en metros y minutos)
- **Opción 1: Llegada a parada** → `bus->simularMovimiento(latParada, lonParada)` + guarda
- **Opción 2: Reportar incidente** → `incidentes.emplace_back(..., "Abierto")` + guarda
- **Opción 3: Finalizar turno** → break

#### `panelAdministrador()` (línea 296)
- Estado global: número de rutas, paradas, buses, incidentes abiertos
- **Gestión de Rutas**: toggle activo/inactivo
- **Gestión de Usuarios**: solo muestra (read-only)
- **Gestión de Buses**: modifica capacidad actual
- **Gestión de Horarios**: agrega salidas a rutas
- **Gestión de Incidentes**: cierra incidentes

---

## DIAGRAMA DE INTERACCIÓN COMPLETO

```
main.cpp
  └→ SistemaTransporte::iniciar()
       ├→ GestorArchivos::cargarBuses()     →   buses.json   → vector<Bus>
       ├→ GestorArchivos::cargarParadas()   → paradas.json   → vector<Parada>
       ├→ GestorArchivos::cargarIncidentes()→ incidentes.json → vector<Incidente>
       ├→ GestorArchivos::cargarRutas()     →   rutas.json   → vector<Ruta*>
       │   ├→ cargo HorarioRuta global de horarios.json
       │   └→ por cada ruta: new RutaBarrio() o new RutaCentro()
       └→ GestorArchivos::cargarUsuarios() → usuarios.json  → vector<Usuario*>
            ├→ new Estudiante(...)
            ├→ new Conductor(...)
            └→ new Administrador(...)
       └→ Loop: menu → autenticar → panel
            ├→ panelEstudiante()
            │   └→ mostrar rutas → seleccionar → ver paradas/horarios → reservar asiento
            └→ panelConductor()
            │   └→ buscar bus asignado → mostrar ruta → proximidad GPS → simular llegada → reportar incidente
            └→ panelAdministrador()
                └→ gestionar rutas/usuarios/buses/horarios/incidentes → guardar cambios
```

---

## DATOS DEL ARCHIVO `usuarios.json`

| ID | Nombre | Rol | Código Login | Bus |
|----|--------|-----|-------------|-----|
| 1001 | Carlos García | Conductor | **101** | Bus 1 (VST-001) |
| 1002 | Juan Pérez | Conductor | **102** | Bus 2 (VST-002) |
| 5001 | María López | Admin | **1** | — |
| 6001 | Breiner Montaña | Estudiante | 160005557 | — |
| 6002 | Danna Martinez | Estudiante | 160005552 | — |
| 6003 | Harold Padilla | Estudiante | 160005567 | — |

---

## RESUMEN DE FUNCIONAMIENTO DEL SISTEMA

1. Al ejecutar `SistemaTransporte.exe`, se cargan **6 archivos JSON** en memoria
2. Aparece un menú de consola con 4 opciones: Estudiante, Conductor, Administrador, Salir
3. Cada perfil se autentica con un **código numérico** (validación polimórfica)
4. **Estudiante**: navega rutas, ve paradas reales de Villavicencio con coordenadas GPS, horarios, y puede reservar asiento en buses con capacidad disponible
5. **Conductor**: ve su bus asignado, alertas de incidentes, horarios, distancia en **tiempo real** a cada parada (usando Haversine + velocidad de 30 km/h), simula movimiento hacia paradas, reporta incidentes
6. **Administrador**: gestiona estado de rutas (activa/inactiva), ve usuarios, modifica capacidad de buses, agrega horarios, cierra incidentes
7. **Persistencia**: cualquier cambio (reserva, incidente, horario) se guarda inmediatamente al JSON correspondiente
8. Al salir, el destructor `~SistemaTransporte()` libera la memoria de los punteros polimórficos (`delete` en cada `Ruta*` y `Usuario*`)




# Documentación Técnica y Educativa del Proyecto SistemaTransporte

> **Universidad de los Llanos — Villavicencio, Meta, Colombia**  
> **Lenguaje:** C++17  
> **Propósito:** Sistema de gestión de rutas de transporte universitario  
> **Perfil del lector:** Estudiante sin conocimientos previos del proyecto

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
 │   main.cpp → SistemaTransporte.cpp (menús, colores, input)   │
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
    │       gestor.cargarBuses()       →   buses[]
    │       gestor.cargarParadas()     →   paradas[]
    │       gestor.cargarIncidentes()  →   incidentes[]
    │       gestor.cargarRutas()       →   rutas[]
    │       gestor.cargarUsuarios()    →   usuarios[]
    │
    ├── 2. Configurar consola:
    │       SetConsoleTitle("...")
    │       SetConsoleOutputCP(65001)   ← UTF-8
    │
    └── 3. BUCLE PRINCIPAL (while true):
            ├── mostrarMenuPrincipal()
            │     └── [1] Estudiante  [2] Conductor  [3] Administrador  [4] Salir
            ├── autenticarUsuario(opción)
            │     └── Pide código → busca en usuarios[] → si no existe: error
            └── Según el perfil:
                  ├── panelEstudiante()
                  ├── panelConductor()
                  └── panelAdministrador()

  FIN (opción 4)
    └── ~SistemaTransporte()
          └── delete cada Ruta*
          └── delete cada Usuario*
```

---

# Sección 2: Estructura de Carpetas

## 2.1 Árbol del proyecto

```
SistemaTransporte/
├── .gitignore               ← Indica a Git qué archivos ignorar
├── CMakeLists.txt           ← Instrucciones de compilación con CMake
├── README.md                ← Documentación breve
├── main.cpp                 ← Punto de entrada del programa
├── include/                 ← CABECERAS (.h) — declaraciones de clases
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
│   └── json.hpp              ← Librería nlohmann/json (26.000 líneas, terceros)
├── src/                      ← IMPLEMENTACIONES (.cpp) — código de cada método
│   ├── Coordenada.cpp        ──── etc (14 archivos, uno por cada .h)
│   └── SistemaTransporte.cpp
└── data/                     ← DATOS PERSISTENTES (JSON)
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
| `main.cpp` | Punto de entrada | No hay `main` → no se genera ejecutable |
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
- `<iostream>` → `std::cerr` (salida de error estándar)
- `<stdexcept>` → `std::exception` (clase base de excepciones)
- `"SistemaTransporte.h"` → clase `SistemaTransporte`

**Impacto si se elimina:** No hay punto de entrada. El compilador genera error: `main` no definida.

**Explicación detallada:**
- `int main()`: Todo programa C++ necesita UNA función `main`. Devuelve `0` si todo salió bien, otro valor si hubo error.
- `try { ... } catch (...) { ... }`: Manejo de excepciones. Si algo inesperado ocurre dentro del `try`, el flujo salta al `catch`.
- `SistemaTransporte sistema("data")`: Crea un objeto `sistema` de la clase `SistemaTransporte`, pasando `"data"` como ruta de los archivos JSON.
- `sistema.iniciar()`: Arranca el menú principal del sistema.
- `return 0`: Indica al sistema operativo que el programa terminó correctamente.

## 3.2 Archivos `include/*.h` y `src/*.cpp` — Resumen

| Archivo | Propósito | Dependencias clave | ¿Qué falla si se elimina? |
|---------|-----------|--------------------|---------------------------|
| `Coordenada` | Punto geográfico (lat/lon) con Haversine | `<cmath>` | `UbicacionGPS` no hereda, `Parada` no tiene ubicación |
| `UbicacionGPS` | GPS con altitud, velocidad y tiempo estimado | `Coordenada.h` | `Bus` no tiene ubicación GPS |
| `Usuario` | Clase base abstracta para 3 roles | `<string>` | `Estudiante`, `Conductor`, `Administrador` no heredan |
| `Estudiante` | Estudiante con código, facultad, programa | `Usuario.h` | Perfil estudiante no existe |
| `Conductor` | Conductor con bus asignado y turno | `Usuario.h` | Perfil conductor no existe |
| `Administrador` | Administrador con código | `Usuario.h` | Perfil administrador no existe |
| `Bus` | Bus con placa, capacidad y ubicación GPS | `UbicacionGPS.h`, `<string>` | No hay flota, no hay reservas ni simulación |
| `Parada` | Parada con nombre, coordenadas y orden | `Coordenada.h`, `<string>` | Rutas sin paradas |
| `Incidente` | Incidente con tipo y estado | `<string>` | No se pueden reportar incidentes |
| `HorarioRuta` | Horarios de salida (barrio y unillanos) | `<vector>`, `<string>` | Rutas sin horarios |
| `Ruta` | Clase base abstracta para rutas | `HorarioRuta.h`, `Parada.h` | `RutaBarrio` y `RutaCentro` no heredan |
| `RutaBarrio` | Ruta de barrio (hereda de Ruta) | `Ruta.h` | Rutas tipo barrio no existen |
| `RutaCentro` | Ruta de centro con zona | `Ruta.h` | Rutas tipo centro no existen |
| `GestorArchivos` | Persistencia JSON (carga/guarda todo) | Todos los `.h`, `<fstream>`, `json.hpp` | No se cargan ni guardan datos |
| `SistemaTransporte` | Orquestador principal | Todos los `.h`, `<windows.h>` | El programa no arranca |

---

# Sección 4: Análisis de Clases y Estructuras

## 4.1 Clase `Coordenada`

**Representación lógica:** Un punto en la superficie terrestre definido por latitud y longitud.

### Atributos

| Atributo | Tipo | Significado | Ciclo de vida | Quién lo consulta | Quién lo modifica |
|----------|------|-------------|---------------|-------------------|-------------------|
| `latitud` | `double` | Coordenada norte-sur (-90 a 90) | Toda la vida del objeto | `getLatitud()`, `distanciaHacia()` | Constructor, `setLatitud()` |
| `longitud` | `double` | Coordenada este-oeste (-180 a 180) | Toda la vida del objeto | `getLongitud()`, `distanciaHacia()` | Constructor, `setLongitud()` |

### Métodos

#### `Coordenada()` (constructor por defecto)

```
INPUT:  Ninguno
OUTPUT: Ninguno (inicializa el objeto)

PASO A PASO:
1. Asigna 0.0 al atributo latitud
2. Asigna 0.0 al atributo longitud

EFICIENCIA: O(1) — constante
```

#### `Coordenada(double lat, double lon)` (constructor parametrizado)

```
INPUT:  lat (double), lon (double)
OUTPUT: Ninguno

PASO A PASO:
1. Asigna 'lat' al atributo 'latitud'
2. Asigna 'lon' al atributo 'longitud'

EFICIENCIA: O(1)
```

#### `distanciaHacia(const Coordenada& otra) const`

**Descripción:** Calcula la distancia en **metros** entre este punto y otro usando la **fórmula de Haversine**. Es el método más importante de la clase.

```
INPUT:  otra (referencia constante a Coordenada) — punto de destino
OUTPUT: double — distancia en metros

PASO A PASO:
1. Convertir diferencia de latitud a radianes:
   dLat = (otra.latitud - this.latitud) * (π / 180)
2. Convertir diferencia de longitud a radianes:
   dLon = (otra.longitud - this.longitud) * (π / 180)
3. Calcular 'a' usando la fórmula de Haversine:
   a = sen²(dLat/2) + cos(latitud en rad) * cos(otra.latitud en rad) * sen²(dLon/2)
4. Calcular distancia angular 'c':
   c = 2 * atan2(√a, √(1-a))
5. Distancia final = RADIO_TIERRA * c  (6.371.000 metros)

VARIABLES LOCALES:
- dLat (double): diferencia de latitud en radianes
- dLon (double): diferencia de longitud en radianes
- a (double): resultado intermedio
- c (double): distancia angular

EFICIENCIA: O(1) — siempre la misma cantidad de cálculos

¿POR QUÉ HAVERSINE Y NO PITÁGORAS?
La Tierra es curva, no plana. Un grado de longitud NO mide lo mismo
en el ecuador que en los polos. Haversine corrige la curvatura terrestre.
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

**Explicación línea por línea:**
1. `double dLat = (otra.latitud - latitud) * PI / 180.0;` — Diferencia de latitud en radianes
2. `double dLon = (otra.longitud - longitud) * PI / 180.0;` — Diferencia de longitud en radianes
3. `std::sin(dLat / 2) * std::sin(dLat / 2)` — sen²(dLat/2)
4. `std::cos(latitud * PI / 180.0) * std::cos(otra.latitud * PI / 180.0)` — producto de cosenos
5. `std::sin(dLon / 2) * std::sin(dLon / 2)` — sen²(dLon/2)
6. `std::atan2(std::sqrt(a), std::sqrt(1.0 - a))` — arco tangente para obtener distancia angular
7. `RADIO_TIERRA * c` — distancia en metros

## 4.2 Clase `UbicacionGPS` (hereda de `Coordenada`)

**Representación lógica:** Ubicación GPS que extiende `Coordenada` con altitud y velocidad para cálculos de tiempo de viaje.

### Atributos adicionales

| Atributo | Tipo | Significado | Ciclo de vida |
|----------|------|-------------|---------------|
| `altitud` | `double` | Metros sobre el nivel del mar | Toda la vida del objeto |
| `velocidadKmh` | `double` | Velocidad de desplazamiento (default 30 km/h) | Toda la vida del objeto |

### Métodos

#### `tiempoEstimadoMinutos(double distanciaMetros) const`

```
INPUT:  distanciaMetros (double) — distancia a recorrer en metros
OUTPUT: double — tiempo estimado en minutos

PASO A PASO:
1. Si velocidad <= 0, devolver 0.0 (no se puede dividir por cero)
2. distanciaKm = distanciaMetros / 1000.0
3. horas = distanciaKm / velocidadKmh
4. minutos = horas * 60.0
5. Devolver minutos

EFICIENCIA: O(1)

EJEMPLO:
  distancia = 1500 m, velocidad = 30 km/h
  distanciaKm = 1500 / 1000 = 1.5 km
  horas = 1.5 / 30 = 0.05 h
  minutos = 0.05 * 60 = 3.0 min
```

## 4.3 Clase `Usuario` (abstracta)

**Representación lógica:** Cualquier persona que usa el sistema. Es **abstracta** porque tiene métodos virtuales puros (`= 0`). No se pueden crear objetos `Usuario` directamente; solo sirve como plantilla.

### Métodos virtuales puros

#### `getEncabezado() const = 0`

```
INPUT:  Ninguno
OUTPUT: string — línea descriptiva del usuario

¿POR QUÉ ES VIRTUAL PURO?
Cada tipo de usuario muestra un encabezado diferente:
- Estudiante: "Usuario: Pedro Rodríguez (Estudiante - Sistemas)"
- Conductor: "Usuario: Carlos García (Conductor - Turno: Mañana)"
- Administrador: "Usuario: María López (Administrador)"

La clase Usuario no sabe qué tipo es, así que obliga a las hijas a implementarlo.
```

#### `validarCodigo(int codigo) const = 0`

```
INPUT:  codigo (int)
OUTPUT: bool — true si coincide

¿POR QUÉ ES VIRTUAL PURO?
- Estudiante: compara con código estudiantil
- Conductor: compara con código de operador
- Administrador: compara con código admin
```

## 4.4 Clase `Estudiante` (hereda de `Usuario`)

**Representación lógica:** Un estudiante universitario que usa el transporte.

### Atributos adicionales

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `codigoEstudiantil` | `int` | Código único de identificación |
| `facultad` | `string` | Facultad (ej. "Ingeniería") |
| `programa` | `string` | Programa académico (ej. "Sistemas") |
| `semestre` | `int` | Semestre actual |
| `carnetActivo` | `bool` | Si el carné está vigente |

### Métodos override

#### `getEncabezado() const`
```cpp
return "Usuario: " + getNombre() + " (Estudiante - " + programa + ")";
```

#### `validarCodigo(int c) const`
```cpp
return c == codigoEstudiantil;
```

## 4.5 Clase `Conductor` (hereda de `Usuario`)

**Representación lógica:** Un conductor del sistema de transporte.

### Atributos adicionales

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `codigoOperador` | `int` | Código del operador (para login) |
| `experiencia` | `string` | Años de experiencia (ej. "12 años") |
| `turno` | `string` | "Mañana" o "Tarde" |
| `busAsignado` | `int` | ID del bus que conduce (FK a `Bus.idBus`) |

### Métodos override

#### `getEncabezado() const`
```cpp
return "Usuario: " + getNombre() + " (Conductor - Turno: " + turno + ")";
```

#### `validarCodigo(int c) const`
```cpp
return c == codigoOperador;
```

## 4.6 Clase `Administrador` (hereda de `Usuario`)

**Representación lógica:** El administrador del sistema.

### Atributos adicionales

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `codigoAdmin` | `int` | Código de administrador |

### Métodos override

#### `getEncabezado() const`
```cpp
return "Usuario: " + getNombre() + " (Administrador)";
```

#### `validarCodigo(int c) const`
```cpp
return c == codigoAdmin;
```

## 4.7 Clase `Bus`

**Representación lógica:** Un bus de la flota de transporte universitario.

### Atributos

| Atributo | Tipo | Significado | Ciclo de vida |
|----------|------|-------------|---------------|
| `idBus` | `int` | Identificador único | Toda la vida del objeto |
| `placa` | `string` | Placa (ej. "VST-001") | Toda la vida del objeto |
| `capacidadMaxima` | `int` | Asientos totales | Toda la vida del objeto |
| `capacidadActual` | `int` | Pasajeros actuales | Se modifica al reservar |
| `estado` | `bool` | true = activo | Se modifica por admin |
| `ubicacion` | `UbicacionGPS*` | **Puntero dinámico** a ubicación actual | Se crea con `new` en `setUbicacion()`, se destruye en `~Bus()` |

### Métodos

#### `setUbicacion(double lat, double lon, double alt, double vel)`

```
INPUT:  lat, lon, alt, vel (doubles)
OUTPUT: Ninguno

PASO A PASO:
1. delete ubicacion;  // Libera la ubicación anterior (si existe)
2. ubicacion = new UbicacionGPS(lat, lon, alt, vel);  // Crea nueva en el heap
```

#### `simularMovimiento(double latDest, double lonDest)`

```
INPUT:  latDest, lonDest (double) — coordenadas destino
OUTPUT: Ninguno

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

#### `tiempoHastaParada(double latParada, double lonParada) const`

```
INPUT:  latParada, lonParada (double)
OUTPUT: double — minutos estimados (o -1 si no hay ubicación)

PASO A PASO:
1. Si ubicacion es nullptr, devolver -1
2. Crear Coordenada destino(latParada, lonParada)
3. dist = ubicacion->distanciaHacia(destino)  // Haversine
4. minutos = ubicacion->tiempoEstimadoMinutos(dist)
5. Devolver minutos
```

#### `operator<(const Bus& otro) const`

```cpp
return (capacidadMaxima - capacidadActual) > (otro.capacidadMaxima - otro.capacidadActual);
```

Ordena buses por **mayor espacio disponible** (los que tienen más asientos libres aparecen primero). El operador `>` está deliberadamente invertido para que, al ordenar ascendentemente con `<`, los buses con más espacio queden al principio.

## 4.8 Clase `Parada`

**Representación lógica:** Una parada física en el recorrido.

### Atributos

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `idParada` | `int` | Identificador único |
| `nombre` | `string` | Nombre descriptivo |
| `ubicacion` | `Coordenada` | **Objeto directo** (no puntero) con lat/lon |
| `altitud` | `double` | Metros sobre el nivel del mar |
| `estado` | `bool` | true = operativa |
| `ordenEnRuta` | `int` | Posición en la secuencia de la ruta |

### Métodos

#### `distanciaA(double lat, double lon) const`

```
INPUT:  lat, lon (double)
OUTPUT: double — distancia desde esta parada hasta ese punto

PASO A PASO:
1. Crear Coordenada otro(lat, lon)
2. Devolver ubicacion.distanciaHacia(otro)  // Haversine
```

#### `operator<(const Parada& otra) const`

```cpp
return ordenEnRuta < otra.ordenEnRuta;
```

Ordena paradas por su orden en la ruta (de la primera a la última).

## 4.9 Clase `Incidente`

**Representación lógica:** Un problema reportado en el sistema.

### Atributos

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `idIncidente` | `int` | Identificador único |
| `descripcion` | `string` | Descripción del incidente |
| `tipo` | `string` | "Mecanico", "Accidente", "Otro" |
| `estado` | `string` | "Abierto" o "Cerrado" |

### Métodos

#### `cerrar()`

```
INPUT:  Ninguno
OUTPUT: Ninguno

PASO A PASO:
1. estado = "Cerrado"
```

## 4.10 Clase `HorarioRuta`

**Representación lógica:** Colección de horarios de salida de una ruta.

### Atributos

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `salidasBarrio` | `vector<string>` | Horas de salida del barrio a la universidad |
| `salidasUnillanos` | `vector<string>` | Horas de salida de la universidad al barrio |

### Métodos

#### `formatearBarrio() const` / `formatearUnillanos() const`

```
INPUT:  Ninguno
OUTPUT: string — horas separadas por " | "

PASO A PASO:
1. Crear string vacío 'resultado'
2. Para cada hora en el vector:
   a. Si no es la primera, agregar " | "
   b. Agregar la hora
3. Si resultado está vacío, devolver "Sin horarios asignados"
4. Devolver resultado

EJEMPLO:
Vector: ["05:10", "06:10", "07:10"]
Resultado: "05:10 | 06:10 | 07:10"

EFICIENCIA: O(n)
```

## 4.11 Clase `Ruta` (abstracta)

**Representación lógica:** Una ruta de transporte desde un origen hasta la universidad.

### Atributos

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

**Nota:** `idsParadas` guarda solo los **IDs** (enteros), no los objetos `Parada`. Para obtener la Parada completa, se busca en el vector global del sistema.

### Métodos

#### `agregarParada(int idParada)`
```cpp
void Ruta::agregarParada(int idParada) { idsParadas.push_back(idParada); }
```

#### `operator<(const Ruta& otra) const`
```cpp
return idRuta < otra.idRuta;
```

#### `operator==(const Ruta& otra) const`
```cpp
return idRuta == otra.idRuta;
```

## 4.12 Clase `RutaBarrio` (hereda de `Ruta`)

**Representación lógica:** Ruta que conecta un barrio con la universidad.

### Métodos

#### `getInfoAdicional() const`
```cpp
return "Tipo: Ruta de Barrio | Trayecto: " + getOrigen() + " -> " + getDestino();
```

## 4.13 Clase `RutaCentro` (hereda de `Ruta`)

**Representación lógica:** Ruta que pasa por el centro de la ciudad.

### Atributos adicionales

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `zonaCentro` | `string` | Zona del centro (ej. "Centro") |

### Métodos

#### `getInfoAdicional() const`
```cpp
return "Tipo: Ruta Centro | Zona: " + zonaCentro + " | " + getOrigen() + " -> " + getDestino();
```

## 4.14 Clase `GestorArchivos`

**Representación lógica:** El "bibliotecario" del sistema. Lee y escribe archivos JSON para persistencia.

### Atributos

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `rutaData` | `string` | Ruta al directorio de JSONs (ej. "data") |

### Métodos principales

#### `cargarBuses() const`

```
INPUT:  Ninguno
OUTPUT: vector<Bus>

PASO A PASO:
1. Abrir "data/buses.json"
2. Si no se puede abrir, devolver vector vacío
3. json j = json::parse(archivo)
4. Para cada objeto en j["buses"]:
   a. Extraer idBus, placa, capacidadMaxima, capacidadActual, estado
   b. lista.emplace_back(id, placa, capMax, capAct, est)
5. Devolver lista

EFICIENCIA: O(n) donde n = número de buses
```

#### `cargarRutas() const` — el método más complejo

```
INPUT:  Ninguno
OUTPUT: vector<Ruta*> (punteros polimórficos)

PASO A PASO:
1. Abrir "data/rutas.json". Si no se puede, devolver vector vacío
2. Cargar horarios por defecto desde horarios.json
3. Para cada ruta en j["rutas"]:
   a. Leer tipo de ruta
   b. Según el tipo:
      - "RutaCentro" → new RutaCentro(...)
      - Otro → new RutaBarrio(...)
   c. Agregar paradas: ruta->agregarParada(id)
   d. Si la ruta tiene horarios propios en JSON:
      - Cargar salidas específicas
   e. Si NO tiene horarios propios:
      - Asignar horarios por defecto
   f. lista.push_back(ruta)
4. Devolver lista

NOTA DE MEMORIA:
Los objetos se crean con new en el heap.
~SistemaTransporte() debe llamar a delete para cada uno.
```

#### Métodos de guardado

Todos los métodos `guardar*()` siguen el mismo patrón:
1. Crear objeto JSON vacío
2. Recorrer la lista de objetos C++
3. Extraer atributos y construir JSON equivalente
4. Escribir `archivo << j.dump(indentación)`

## 4.15 Clase `SistemaTransporte`

**Representación lógica:** El "director de orquesta". Coordina todo el sistema.

### Atributos

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `gestor` | `GestorArchivos` | Objeto de persistencia |
| `buses` | `vector<Bus>` | Flota de buses (objetos directos) |
| `paradas` | `vector<Parada>` | Paradas disponibles (objetos directos) |
| `incidentes` | `vector<Incidente>` | Incidentes (objetos directos) |
| `rutas` | `vector<Ruta*>` | **Punteros** a rutas (polimórficas) |
| `usuarios` | `vector<Usuario*>` | **Punteros** a usuarios (polimórficos) |
| `usuarioActual` | `Usuario*` | **Puntero** al usuario con sesión activa |

### Métodos principales

#### `iniciar()`

```
INPUT:  Ninguno
OUTPUT: Ninguno (todo el programa vive aquí)

PASO A PASO:
1. Cargar datos: buses, paradas, incidentes, rutas, usuarios
2. Configurar consola: título, UTF-8
3. BUCLE INFINITO (while true):
   a. usuarioActual = nullptr
   b. mostrarMenuPrincipal() → opción
   c. Opción 4 (Salir): break
   d. Opción inválida (<1 o >3): continue
   e. autenticarUsuario(opción):
      - Si nullptr (error): continue
   f. Según opción:
      - 1: panelEstudiante()
      - 2: panelConductor()
      - 3: panelAdministrador()
```

#### `autenticarUsuario(int tipo)`

```
INPUT:  tipo (int) — 1=Estudiante, 2=Conductor, 3=Administrador
OUTPUT: Usuario* — puntero al usuario o nullptr

PASO A PASO:
1. Mapa: {1→"Estudiante", 2→"Conductor", 3→"Administrador"}
2. Pedir código al usuario
3. std::find_if: buscar en usuarios[] que coincida tipo Y código
4. Si se encuentra: devolver puntero
5. Si no: mostrar error, Sleep(2000), devolver nullptr
```

#### `panelEstudiante()`

```
BUCLE INFINITO hasta que el usuario elija "Volver":
1. Mostrar lista de rutas
2. Usuario selecciona una ruta (o 0 para volver)
3. Si la ruta no existe: mostrar error, continuar
4. Mostrar detalles: nombre, punto de salida, paradas, horarios
5. Preguntar "Reservar asiento? (S/N)"
6. Si SÍ:
   a. Buscar bus (por ID de ruta o cualquier bus con espacio)
   b. Si hay espacio: incrementar capacidadActual, guardar cambios
   c. Si no: mostrar "Bus lleno"
7. Esperar Enter
```

#### `panelConductor()`

```
1. Obtener Conductor y su bus asignado + ruta
2. BUCLE INFINITO hasta "Finalizar turno":
   a. Mostrar info del bus (placa, capacidad, alerta)
   b. Mostrar ruta (nombre, origen, destino, horarios)
   c. Mostrar proximidad a paradas (distancia en m y tiempo en min)
   d. Opciones:
      [1] Llegada a parada → simularMovimiento() hacia primera parada
      [2] Reportar incidente → crear Incidente("Abierto"), guardar
      [3] Finalizar turno → break
```

#### `panelAdministrador()`

```
BUCLE INFINITO hasta "Volver":
1. Mostrar estado global (rutas, paradas, buses, incidentes)
2. Opciones:
   [1] Gestionar Rutas → toggle estado activo/inactivo
   [2] Gestionar Usuarios → solo lectura
   [3] Gestionar Buses → modificar capacidad actual
   [4] Gestionar Horarios → agregar salidas a ruta
   [5] Gestionar Incidentes → cerrar incidentes
   [6] Volver → break
```


---

# Documentación Técnica y Educativa — Parte 2

---

# Sección 5: Análisis Línea por Línea (o Bloques Lógicos)

## 5.1 Métodos Getter/Setter — El Principio de Encapsulamiento

**¿Qué son?** Métodos pequeños que permiten **leer** (get) o **modificar** (set) atributos privados.

**Ejemplo completo — los getters/setters de `Coordenada`:**

```cpp
double Coordenada::getLatitud()  const { return latitud; }
double Coordenada::getLongitud() const { return longitud; }
void Coordenada::setLatitud(double lat)  { latitud  = lat; }
void Coordenada::setLongitud(double lon) { longitud = lon; }
```

**¿Qué ocurre en memoria cuando se llama a `getLatitud()`?**

```
Antes de la llamada:       Durante la llamada:           Después:
  ┌──────────────┐          ┌──────────────┐             ┌──────────────┐
  │  latitud     │          │  latitud     │             │  latitud     │
  │  = 4.14527   │ ─────►  │  = 4.14527   │ ──► copia ► │  = 4.14527   │
  └──────────────┘          └──────────────┘   devuelve  └──────────────┘
```

**¿Qué ocurre en memoria cuando se llama a `setLatitud(4.2)`?**

```
Antes:                       Durante:                   Después:
  ┌──────────────┐          ┌──────────────┐           ┌──────────────┐
  │  latitud     │          │  latitud     │           │  latitud     │
  │  = 4.14527   │ ─────►  │  ← 4.2       │ ────►     │  = 4.2       │
  └──────────────┘          └──────────────┘           └──────────────┘
  El método toma el parámetro y lo asigna al atributo
```

**¿Por qué `const` al final de `getLatitud() const`?**

El `const` promete que el método NO modifica ningún atributo del objeto. Si accidentalmente escribiéramos `latitud = 10;` dentro, el compilador daría un error. Es una **garantía** para quien llama al método.

**Lista completa de getters/setters del proyecto:**

| Clase | Getters | Setters |
|-------|---------|---------|
| `Coordenada` | `getLatitud()`, `getLongitud()` | `setLatitud()`, `setLongitud()` |
| `UbicacionGPS` | `getAltitud()`, `getVelocidadKmh()` | `setAltitud()`, `setVelocidadKmh()` |
| `Usuario` | `getIdUsuario()`, `getNombre()`, `getTelefono()`, `getCorreo()`, `getTipo()` | `setIdUsuario()`, `setNombre()`, `setTelefono()`, `setCorreo()`, `setTipo()` |
| `Estudiante` | `getCodigoEstudiantil()`, `getFacultad()`, `getPrograma()`, `getSemestre()`, `isCarnetActivo()` | `setCodigoEstudiantil()`, `setFacultad()`, `setPrograma()`, `setSemestre()`, `setCarnetActivo()` |
| `Conductor` | `getCodigoOperador()`, `getExperiencia()`, `getTurno()`, `getBusAsignado()` | `setCodigoOperador()`, `setExperiencia()`, `setTurno()`, `setBusAsignado()` |
| `Administrador` | `getCodigoAdmin()` | `setCodigoAdmin()` |
| `Bus` | `getIdBus()`, `getPlaca()`, `getCapacidadMaxima()`, `getCapacidadActual()`, `getEstado()`, `getUbicacion()` | `setIdBus()`, `setPlaca()`, `setCapacidadMaxima()`, `setCapacidadActual()`, `setEstado()`, `setUbicacion()` |
| `Parada` | `getIdParada()`, `getNombre()`, `getUbicacion()`, `getAltitud()`, `getEstado()`, `getOrdenEnRuta()` | `setIdParada()`, `setNombre()`, `setUbicacion()`, `setAltitud()`, `setEstado()`, `setOrdenEnRuta()` |
| `Incidente` | `getIdIncidente()`, `getDescripcion()`, `getTipo()`, `getEstado()` | `setIdIncidente()`, `setDescripcion()`, `setTipo()`, `setEstado()` |
| `Ruta` | `getIdRuta()`, `getNombre()`, `getOrigen()`, `getDestino()`, `getEstado()`, `getTipo()`, `getPuntoSalida()`, `getHorario()`, `getIdsParadas()` | `setIdRuta()`, `setNombre()`, `setOrigen()`, `setDestino()`, `setEstado()`, `setTipo()`, `setPuntoSalida()`, `setIdsParadas()` |
| `RutaCentro` | `getZonaCentro()` | `setZonaCentro()` |

## 5.2 Constructor por defecto vs. Constructor parametrizado

Tomemos `Bus` como ejemplo:

### Constructor por defecto

```cpp
Bus::Bus() : idBus(0), capacidadMaxima(0), capacidadActual(0), estado(false), ubicacion(nullptr) {}
```

Inicializa todos los atributos con valores "vacío":
- `idBus = 0` — sin ID asignado
- `capacidadMaxima = 0` — sin capacidad
- `capacidadActual = 0` — sin pasajeros
- `estado = false` — inactivo
- `ubicacion = nullptr` — sin ubicación GPS

**¿Cuándo se usa?** Cuando se crea un bus sin especificar datos: `Bus busTemporal;`

### Constructor parametrizado

```cpp
Bus::Bus(int id, const std::string& plc, int capMax, int capAct, bool est)
    : idBus(id), placa(plc), capacidadMaxima(capMax), capacidadActual(capAct),
      estado(est), ubicacion(nullptr) {}
```

**Sintaxis de lista de inicializadores** (la parte después de `:`). Es más eficiente que asignar dentro del cuerpo porque los atributos se crean directamente con el valor en lugar de crearse vacíos y luego reasignarse.

**¿Cuándo se usa?** Con datos conocidos: `Bus bus1(1, "VST-001", 50, 0, true);`

## 5.3 El destructor de Bus y la gestión de memoria

```cpp
Bus::~Bus() { delete ubicacion; }
```

**¿Qué hace?** Cuando un objeto `Bus` se destruye, libera la memoria del puntero `ubicacion` que se creó con `new UbicacionGPS(...)`.

**¿Por qué es necesario?** Cada `new` debe tener su `delete`. Si no se llamara a `delete`, la memoria de `UbicacionGPS` quedaría ocupada para siempre (**fuga de memoria** o **memory leak**).

```cpp
void ejemplo() {
    Bus bus;
    bus.setUbicacion(4.14, -73.65, 463, 30);  // new UbicacionGPS en el heap
    // ...
}  // ← bus sale de ámbito. Sin destructor, la memoria GPS queda inaccesible → FUGA
```

## 5.4 El destructor de SistemaTransporte

```cpp
SistemaTransporte::~SistemaTransporte() {
    for (Ruta* r : rutas) delete r;
    for (Usuario* u : usuarios) delete u;
}
```

Libera la memoria de todos los objetos creados con `new` por `GestorArchivos::cargarRutas()` y `GestorArchivos::cargarUsuarios()`.

## 5.5 Análisis completo de `mostrarProximidadBus()` — el método más rico

```cpp
void SistemaTransporte::mostrarProximidadBus(Bus* bus, Ruta* ruta) {
    if (!bus || !ruta || !bus->getUbicacion()) return;          // L1

    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);           // L3
    std::cout << "\n[ PROXIMIDAD EN RUTA (30 km/h) ]\n";        // L4
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // L5

    int mostradas = 0;                                           // L7
    for (int idP : ruta->getIdsParadas()) {                      // L8
        auto it = std::find_if(paradas.begin(), paradas.end(),    // L9
            [idP](const Parada& p) { return p.getIdParada() == idP; });
        if (it != paradas.end()) {                                // L10
            double metros = it->distanciaA(                       // L11
                bus->getUbicacion()->getLatitud(),
                bus->getUbicacion()->getLongitud());
            double dist = bus->tiempoHastaParada(                 // L12
                it->getUbicacion().getLatitud(),
                it->getUbicacion().getLongitud());
            std::cout << "  Parada (" << idP << ") " << it->getNombre()  // L13
                      << "\n    -> " << std::fixed << std::setprecision(0)
                      << metros << " m | "
                      << std::setprecision(1) << dist << " min\n";
            if (++mostradas >= 4) break;                          // L14
        }
    }
}
```

**Explicación línea por línea:**

**L1** `if (!bus || !ruta || !bus->getUbicacion()) return;`
**Guard clause**: si alguno es `nullptr`, salimos inmediatamente para evitar acceder a memoria inválida (crash).

- `!bus` es equivalente a `bus == nullptr`
- `!ruta` es equivalente a `ruta == nullptr`
- `!bus->getUbicacion()` verifica si el bus tiene ubicación GPS asignada

**L3-L5** Cambio de colores para resaltar el título.

**L7** `int mostradas = 0;`
Variable contador para limitar la salida a 4 paradas (evitar saturar la pantalla).

**L8** `for (int idP : ruta->getIdsParadas())`
**Range-based for loop** (C++11). Itera sobre cada ID de parada de la ruta. Equivalente a:
```cpp
for (size_t i = 0; i < ruta->getIdsParadas().size(); i++) {
    int idP = ruta->getIdsParadas()[i];
    ...
}
```

**L9** `auto it = std::find_if(paradas.begin(), paradas.end(), [idP](const Parada& p) { return p.getIdParada() == idP; });`
Busca la parada con ID = `idP` en el vector global `paradas`.

- `paradas.begin()`: iterador al primer elemento
- `paradas.end()`: iterador más allá del último (no existe)
- `[idP](const Parada& p) { ... }`: **lambda** — función anónima que captura `idP` y recibe una `Parada`. Devuelve `true` si el ID coincide
- `auto it`: el tipo se deduce automáticamente; es `std::vector<Parada>::iterator`

**L10** `if (it != paradas.end())`
Si `it` NO es igual a `end()`, significa que **se encontró** la parada.

**L11** `double metros = it->distanciaA(bus->getUbicacion()->getLatitud(), bus->getUbicacion()->getLongitud());`
Calcula distancia desde el bus hasta la parada.

- `it->distanciaA(...)` = `(*it).distanciaA(...)` — accede al objeto Parada a través del iterador
- `bus->getUbicacion()` devuelve `UbicacionGPS*` (puntero)
- `->getLatitud()` llama al método del objeto apuntado

**L12** `double dist = bus->tiempoHastaParada(it->getUbicacion().getLatitud(), it->getUbicacion().getLongitud());`
Calcula el tiempo estimado hasta la parada.

- `it->getUbicacion()` devuelve `Coordenada` (objeto directo, no puntero)
- `.getLatitud()` llama al método del objeto directo

**L13** Muestra con formato:
- `std::fixed`: notación de punto fijo (no científica)
- `std::setprecision(0)`: 0 decimales para metros (ej. "1234 m")
- `std::setprecision(1)`: 1 decimal para minutos (ej. "3.5 min")

**L14** `if (++mostradas >= 4) break;`
Incrementa el contador. Si ya mostramos 4 paradas, salimos del bucle con `break`.

## 5.6 Análisis de `panelConductor()` — opción 1: Llegada a parada

```cpp
if (op == 1 && bus && ruta && !ruta->getIdsParadas().empty()) {
    int idSig = ruta->getIdsParadas()[0];  // ← Toma la PRIMERA parada
    auto it = std::find_if(paradas.begin(), paradas.end(), [idSig](const Parada& p) { return p.getIdParada() == idSig; });
    if (it != paradas.end()) {
        bus->simularMovimiento(it->getUbicacion().getLatitud(), it->getUbicacion().getLongitud());
        gestor.guardarBuses(buses);
    }
    Sleep(1500);
}
```

**Explicación:**
1. `idSig = ruta->getIdsParadas()[0]`: Toma el ID de la **primera** parada de la ruta
2. Busca la parada por ID en el vector global
3. `bus->simularMovimiento(lat, lon)`: Mueve el bus un 5% hacia esa parada
4. `gestor.guardarBuses(buses)`: Persiste la nueva posición
5. `Sleep(1500)`: Pausa de 1.5 segundos para que el usuario vea el mensaje

## 5.7 Análisis de `gestionarRutas()` — Toggle de estado

```cpp
r->setEstado(!r->getEstado());  // ← Invierte el valor booleano
```

**`!`** es el operador **NOT lógico**:
- `!true` = `false`
- `!false` = `true`

Esto se llama **toggle** (alternar). Es una forma compacta de cambiar un estado sin usar `if`.

## 5.8 Análisis de `autenticarUsuario()` — Mapa y búsqueda

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
1. `std::map<int, pair<string,string>>`: Diccionario que mapea opción de menú → (tipo string, label string)
2. `datos.at(tipo).first`: Obtiene el tipo de usuario ("Estudiante", etc.)
3. `datos.at(tipo).second`: Obtiene el label ("codigo estudiantil", etc.)
4. La lambda busca un usuario que cumpla **ambas** condiciones:
   - `getTipo() == tipoEsperado` (coincide el tipo)
   - `validarCodigo(codigo)` (coincide el código)

## 5.9 Análisis de `incidentesDeBus()`

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
- Si NO la encuentra: devuelve `std::string::npos` (un número muy grande)

**`&inc`:** Dirección del incidente (puntero). No se copia el objeto, solo se guarda la dirección.

**Ejemplo:** Si descripción = "Falla mecanica en motor del bus VST-001" y placa = "VST-001", `find()` devuelve 35 (posicion donde comienza "VST-001").

## 5.10 Análisis de `buscarRutaPorBus()`

```cpp
Ruta* SistemaTransporte::buscarRutaPorBus(const std::string& placa) {
    auto it = std::find_if(buses.begin(), buses.end(),
        [&placa](const Bus& b) { return b.getPlaca() == placa; });
    return (it != buses.end() && !rutas.empty()) ? rutas[it->getIdBus() - 1]
           : (!rutas.empty() ? rutas[0] : nullptr);
}
```

**Operador ternario:** `condición ? valor_si_true : valor_si_false`

Es equivalente a:
```cpp
if (condición) {
    return valor_si_true;
} else {
    return valor_si_false;
}
```

**Simplificación:** Asume que el bus con ID = 1 corresponde a la ruta en el índice 0 del vector, bus ID = 2 → ruta índice 1, etc.

---

# Sección 6: Guía de Sintaxis del Lenguaje (C++)

Esta sección explica TODOS los elementos de C++ del proyecto asumiendo **cero conocimiento**.

## 6.1 `#include` — Directiva del preprocesador

**¿Qué es?** Una instrucción que se ejecuta ANTES de la compilación. Copia todo el contenido de un archivo aquí.

**Sintaxis:**
```cpp
#include <iostream>    // Ángulos: busca en directorios del sistema
#include "Bus.h"       // Comillas: busca en directorio del proyecto
```

**¿Qué ocurre al compilar?**
```
main.cpp            Preprocesador                Lo que se compila
  #include <iostream>  ───────►  iostream (todo el contenido)
  #include "Bus.h"     ───────►  Bus.h (todo el contenido)
  int main() { ... }           int main() { ... }
```

## 6.2 `class` — Definición de clases

**¿Qué es?** Una **plantilla** para crear objetos. Define atributos (datos) y métodos (funciones).

```cpp
class Casa {                   // ← Así se define una clase
private:
    int numHabitaciones;       // ← Atributo (dato)
public:
    Casa(int n) : numHabitaciones(n) {}  // ← Constructor
    int getHabitaciones() const { return numHabitaciones; }  // ← Método
};

Casa miCasa(3);    // ← Se crea un objeto
```

**Analogía:** La clase es el **plano** de una casa. El objeto es la **casa real** construida a partir del plano.

## 6.3 `public:` / `private:` — Especificadores de acceso

| Especificador | Misma clase | Clases hijas | Fuera |
|---------------|-------------|-------------|-------|
| `private` | ✅ Sí | ❌ No | ❌ No |
| `protected` | ✅ Sí | ✅ Sí | ❌ No |
| `public` | ✅ Sí | ✅ Sí | ✅ Sí |

**Encapsulamiento:** Esconder los detalles internos para mantener los datos consistentes:

```cpp
class CuentaBancaria {
private:
    double saldo;
public:
    void depositar(double cant) {
        if (cant > 0) saldo += cant;  // ← Validación antes de modificar
    }
};

CuentaBancaria c;
c.saldo = -1000;  // ← ERROR: saldo es privado
c.depositar(500); // ← OK
```

## 6.4 `const` — Inmutabilidad

**Usos:**
1. **Métodos constantes** — no modifican el objeto:
   ```cpp
   double getLatitud() const;  // ← Promete no cambiar nada
   ```
2. **Parámetros constantes** — el argumento no se modifica:
   ```cpp
   void setNombre(const std::string& nom);
   ```
3. **Variables constantes** — no pueden reasignarse:
   ```cpp
   static const double PI = 3.14159;
   ```

## 6.5 `virtual` y `override` — Polimorfismo

- **`virtual`** en la clase base: "este método puede ser redefinido en las hijas"
- **`virtual ... = 0`**: método **puramente virtual** (sin implementación, obligatorio implementarlo)
- **`override`** en la clase hija: "confirmo que estoy redefiniendo un método virtual"

```cpp
class Usuario {
public:
    virtual std::string getEncabezado() const = 0;  // ← Puro
};

class Estudiante : public Usuario {
public:
    std::string getEncabezado() const override {    // ← override
        return "Usuario: " + getNombre() + " (Estudiante)";
    }
};
```

**¿Qué permite el polimorfismo?** Tratar objetos de diferentes clases de manera uniforme:

```cpp
vector<Usuario*> usuarios;
usuarios.push_back(new Estudiante(...));
usuarios.push_back(new Conductor(...));

for (Usuario* u : usuarios)
    cout << u->getEncabezado() << '\n';  // ← Cada uno muestra su propio texto
```

## 6.6 `if`, `else`, `else if` — Condicionales

```cpp
if (condición) {
    // Código si la condición es true
} else if (otra condición) {
    // Código si la primera es false y esta es true
} else {
    // Código si ninguna es true
}
```

En C++, cualquier valor distinto de cero se considera `true`.

## 6.7 `for` y `while` — Bucles

### `for` tradicional
```cpp
for (int i = 0; i < 10; i++) { /* se ejecuta 10 veces */ }
```

### `for` basado en rango (C++11)
```cpp
for (Ruta* r : rutas) { /* una vez por cada ruta */ }
```

### `while`
```cpp
while (true) {             // ← Bucle infinito
    if (op == 4) break;    // ← Salida con break
}
```

## 6.8 `auto` — Deducción automática de tipos (C++11)

```cpp
auto it = std::find_if(...);
// El compilador deduce que 'it' es de tipo vector<Parada>::iterator
```

## 6.9 Referencias (`&`) y Punteros (`*`)

### Referencias — Alias de una variable
```cpp
int original = 42;
int& ref = original;   // ← ref es OTRO NOMBRE para original
ref = 100;
cout << original;      // ← Imprime 100
```

**Uso en parámetros**: evita copiar objetos grandes:
```cpp
void setNombre(const std::string& nom) {  // ← Pasa referencia, no copia
    nombre = nom;
}
```

### Punteros — Direcciones de memoria
```cpp
int valor = 42;
int* p = &valor;    // ← p guarda la DIRECCIÓN de valor
*p = 100;           // ← *p accede al contenido
cout << valor;      // ← Imprime 100
```

**Diferencia clave:**

| Característica | Referencia (&) | Puntero (*) |
|---------------|----------------|-------------|
| Puede ser `nullptr` | ❌ No | ✅ Sí |
| Puede reasignarse | ❌ No | ✅ Sí |
| Sintaxis de acceso | Como variable normal | `*` para desreferenciar |

## 6.10 `new` y `delete` — Gestión manual de memoria

- **`new`**: Reserva memoria en el **heap**:
  ```cpp
  UbicacionGPS* ubi = new UbicacionGPS(lat, lon, alt, vel);
  ```
- **`delete`**: Libera esa memoria:
  ```cpp
  delete ubi;
  ```

**Memoria del programa:**
```
┌──────────────────────────┐
│         STACK (pila)     │ ← Variables locales (se libera sola)
├──────────────────────────┤
│         HEAP (montón)    │ ← new/delete (hay que liberarla manualmente)
├──────────────────────────┤
│     CÓDIGO + DATOS GLOB. │ ← Instrucciones y variables globales
└──────────────────────────┘
```

## 6.11 `this` — Puntero al objeto actual

```cpp
void Bus::setIdBus(int id) {
    this->idBus = id;  // ← this->idBus = atributo, id = parámetro
}
```

## 6.12 `explicit` — Constructor explícito

Evita conversiones implícitas:
```cpp
explicit GestorArchivos(const std::string& dirData);
GestorArchivos g = "data";  // ← ERROR: no se puede convertir implícitamente
GestorArchivos g("data");   // ← OK
```

## 6.13 Lambdas (C++11) — Funciones anónimas

**Sintaxis:** `[captura](parámetros) { cuerpo }`

```cpp
auto it = std::find_if(paradas.begin(), paradas.end(),
    [idP](const Parada& p) { return p.getIdParada() == idP; });
```

- `[idP]`: captura la variable `idP` del contexto exterior
- `(const Parada& p)`: recibe una parada por referencia
- `{ return p.getIdParada() == idP; }`: devuelve true si coincide

## 6.14 Operador `:` (dos puntos)

**Usos:**
1. **Herencia**: `class Estudiante : public Usuario { ... };`
2. **Lista de inicializadores**: `Bus::Bus(...) : idBus(id), placa(plc) { }`
3. **Acceso a miembro por puntero**: `it->getNombre()` equivale a `(*it).getNombre()`

## 6.15 `static` — Miembros estáticos

Dentro de un archivo `.cpp`, `static` limita el alcance al archivo actual:

```cpp
// Solo visible en Coordenada.cpp
static const double PI = 3.141592653589793;
```

## 6.16 `std::` — El namespace estándar

Todo lo que pertenece a la biblioteca estándar de C++ está dentro de `std`:

```cpp
std::cout      std::vector<int>      std::string
```

## 6.17 Operador `|` (OR a nivel de bits)

Combina los bits de dos números:
```cpp
FOREGROUND_BLUE      = 0001
FOREGROUND_GREEN     = 0010
FOREGROUND_INTENSITY = 1000
                        ──── OR
Resultado:              1011  (= cyan brillante)
```

---

# Sección 7: Guía de la STL y Librerías Estándar

## 7.1 `std::string` — `<string>`

**Funcionamiento:** Secuencia de caracteres con memoria dinámica.
- `length()`, `empty()`: O(1)
- `find(subcadena)`: O(n) — devuelve posición o `string::npos`
- Concatenación (`+`): O(n)

**Usos en el proyecto:**
```cpp
desc.find(placa)       // Buscar placa dentro de descripción
puntoSalida.empty()    // Verificar si está vacío
```

## 7.2 `std::vector<T>` — `<vector>`

**Funcionamiento:** Arreglo dinámico contiguo en el heap.

```
[0] [1] [2] [3] [4] [_] [_] [_]
 ↑                       ↑
 size = 5               capacity = 8
```

Cuando `size == capacity` y se agrega un elemento, se redimensiona al **doble**.

**Complejidad:**
- `v[i]`: O(1)
- `push_back`: O(1) **amortizado**
- `find_if`: O(n)
- `begin()`, `end()`, `size()`: O(1)

**Declaraciones en el proyecto:**
```cpp
vector<Bus> buses;
vector<Parada> paradas;
vector<Ruta*> rutas;        // ← Punteros para polimorfismo
vector<Usuario*> usuarios;  // ← Punteros para polimorfismo
vector<string> salidasBarrio;
vector<int> idsParadas;
```

## 7.3 Algoritmos de la STL — `<algorithm>`

### `std::find_if`

Busca el PRIMER elemento que cumple una condición:
```cpp
auto it = find_if(inicio, fin, predicado);
// Devuelve iterador al elemento o 'fin' si no lo encuentra
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

**Diccionario** ordenado por clave (árbol binario balanceado).

**Complejidad:** O(log n) en inserción y búsqueda.

**Uso en el proyecto:**
```cpp
map<int, pair<string, string>> datos = {
    {1, {"Estudiante", "codigo estudiantil"}},
};
datos.at(1).first  // → "Estudiante"
```

## 7.5 `std::cin`, `std::cout`, `std::cerr` — `<iostream>`

```cpp
cout << "Texto";        // Salida a pantalla
cin >> variable;        // Entrada desde teclado
cerr << "Error";        // Salida de error
cin.ignore(100, '\n');  // Ignorar hasta 100 chars o nueva línea
getline(cin, str);      // Leer línea completa (con espacios)
```

## 7.6 `std::ostringstream` — `<sstream>`

Permite formatear texto como si fuera `cout` pero guardándolo en un string:
```cpp
ostringstream ss;
ss << setw(2) << setfill('0') << hora;
return ss.str();  // → "08" (no "8")
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
ifstream archivo("data/buses.json");   // Leer
if (!archivo.is_open()) return;        // Verificar

ofstream archivo("data/buses.json");   // Escribir
archivo << j.dump(4);                  // JSON con indentación
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

## 8.1 CMake — `CMakeLists.txt`

**¿Qué es?** Sistema de construcción que genera archivos para compilar. No compila directamente, sino que **genera las instrucciones**.

**Análisis línea por línea:**

```cmake
cmake_minimum_required(VERSION 3.16)   # Versión mínima de CMake
project(SistemaTransporte CXX)          # Nombre del proyecto, lenguaje C++
set(CMAKE_CXX_STANDARD 17)              # Usar C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Obligatorio
include_directories(include)            # -Iinclude (buscar .h aquí)
set(SOURCES main.cpp src/*.cpp ...)     # Lista de archivos fuente
add_executable(SistemaTransporte ${SOURCES})  # Crear ejecutable
# Copiar data/ al directorio del ejecutable:
add_custom_command(TARGET SistemaTransporte POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/data"
    "$<TARGET_FILE_DIR:SistemaTransporte>/data")
```

## 8.2 nlohmann/json — `include/json.hpp`

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

## 8.3 Windows Console API — `<windows.h>`

| Función | Propósito | Uso |
|---------|-----------|-----|
| `GetStdHandle(STD_OUTPUT_HANDLE)` | Obtiene handle de consola | `limpiarPantalla()`, `setColor()` |
| `SetConsoleTextAttribute(h, color)` | Cambia color de texto/fondo | `setColor()` |
| `SetConsoleTitle("...")` | Cambia título de ventana | `iniciar()` |
| `SetConsoleOutputCP(65001)` | Activa UTF-8 | `iniciar()` |
| `SetConsoleCP(65001)` | UTF-8 para entrada | `iniciar()` |
| `GetConsoleScreenBufferInfo(h, &info)` | Obtiene tamaño de pantalla | `limpiarPantalla()` |
| `FillConsoleOutputCharacter(h, c, n, coord, &esc)` | Llena con caracteres | `limpiarPantalla()` |
| `FillConsoleOutputAttribute(h, attr, n, coord, &esc)` | Resetea colores | `limpiarPantalla()` |
| `SetConsoleCursorPosition(h, coord)` | Mueve cursor | `limpiarPantalla()` |
| `GetLocalTime(&st)` | Obtiene hora/fecha local | `horaActual()`, `fechaActual()` |
| `Sleep(ms)` | Pausa el programa | En todos los paneles |


---

# Documentación Técnica y Educativa — Parte 3

---

# Sección 9: Desglose de Instrucciones Complejas

## 9.1 `setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY)`

**¿Qué hace?** Cambia el color del texto a **cyan brillante**.

**¿Cómo funciona?**

`SetConsoleTextAttribute` recibe un número de 16 bits. Los bits 0-3 controlan el color del texto:

```
Bits: 3     2     1     0
      I     R     G     B     ← I = Intensidad, R = Rojo, G = Verde, B = Azul
```

**Valores de las constantes:**

| Constante | Binario | Hexadecimal |
|-----------|---------|-------------|
| `FOREGROUND_BLUE` | `0001` | `0x0001` |
| `FOREGROUND_GREEN` | `0010` | `0x0002` |
| `FOREGROUND_RED` | `0100` | `0x0004` |
| `FOREGROUND_INTENSITY` | `1000` | `0x0008` |

**Operador `|` (OR a nivel de bits):** Combina los bits activando cada color:
```
FOREGROUND_BLUE          = 0001
FOREGROUND_GREEN         = 0010
FOREGROUND_INTENSITY     = 1000
                           ──── OR (|) — se activan los bits 0, 1 y 3
Resultado:                 1011  = 0x000B = cyan brillante (azul + verde + intensidad)
```

**Combinaciones de colores:**

| Combinación | Binario | Color |
|-------------|---------|-------|
| `BLUE` | `0001` | Azul |
| `GREEN` | `0010` | Verde |
| `RED` | `0100` | Rojo |
| `BLUE \| GREEN` | `0011` | Cyan |
| `RED \| GREEN` | `0110` | Amarillo |
| `RED \| GREEN \| BLUE` | `0111` | Blanco (gris claro) |
| `GREEN \| INTENSITY` | `1010` | Verde brillante |
| `RED \| INTENSITY` | `1100` | Rojo brillante |
| `BLUE \| INTENSITY` | `1001` | Azul brillante |
| `BLUE \| GREEN \| INTENSITY` | `1011` | Cyan brillante |

## 9.2 `limpiarPantalla()` — Paso a paso completo

```cpp
static void limpiarPantalla() {
    COORD coord = {0, 0};                           // Posición (0,0) = esquina superior
    DWORD celdas, escritas;
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);     // Handle de la consola

    GetConsoleScreenBufferInfo(h, &info);            // 1. Obtener tamaño de pantalla
    celdas = info.dwSize.X * info.dwSize.Y;          // 2. Total de celdas (ancho × alto)
    FillConsoleOutputCharacter(h, ' ', celdas, coord, &escritas);  // 3. Llenar con espacios
    FillConsoleOutputAttribute(h, info.wAttributes, celdas, coord, &escritas);  // 4. Resetear colores
    SetConsoleCursorPosition(h, coord);              // 5. Mover cursor a (0,0)
}
```

**Paso a paso:**
1. `GetConsoleScreenBufferInfo`: Obtiene dimensiones. Si la consola es 80×25, `info.dwSize.X = 80`, `info.dwSize.Y = 25`.
2. `celdas = 80 × 25 = 2000`: hay 2000 posiciones de carácter en pantalla.
3. `FillConsoleOutputCharacter(h, ' ', 2000, {0,0}, &escritas)`: Escribe 2000 espacios empezando desde (0,0). Esto borra todo.
4. `FillConsoleOutputAttribute`: Restaura los colores originales de todas las celdas.
5. `SetConsoleCursorPosition(h, {0,0})`: Mueve el cursor a la esquina superior izquierda.

## 9.3 `COORD coord = {0, 0}`

`COORD` es una estructura de Windows:
```cpp
typedef struct _COORD {
    SHORT X;  // Columna (0 = izquierda)
    SHORT Y;  // Fila (0 = arriba)
} COORD;
```

`{0, 0}` = esquina superior izquierda.

## 9.4 `inc.getDescripcion().find(placa) != std::string::npos`

**`std::string::find(str)`:** Busca la subcadena `str` dentro del string:
- Si la encuentra: devuelve la posición (0, 1, 2, ...)
- Si NO la encuentra: devuelve `std::string::npos`

`std::string::npos` es una constante estática que vale el número más grande que puede almacenar `size_t` (normalmente 18446744073709551615 en 64 bits).

**Ejemplo paso a paso:**
```cpp
std::string desc = "Falla mecanica en motor del bus VST-001";
//                    01234567890123456789012345678901234567890
//                    0         1         2         3
desc.find("VST-001");         // → 35 (posición donde comienza "VST-001")
desc.find("ABC-999");         // → std::string::npos (no encontrado)
```

## 9.5 Operador ternario `?:`

```cpp
condición ? expresión_si_true : expresión_si_false;
```

Es una forma compacta de `if-else` que **devuelve un valor**:

```cpp
// En lugar de:
int x;
if (a > b) x = a;
else x = b;

// Se puede escribir como:
int x = (a > b) ? a : b;
```

## 9.6 `!r->getEstado()` — Toggle booleano

```cpp
r->setEstado(!r->getEstado());
```

**`!`:** Operador NOT lógico. Invierte el valor:
- `!true` → `false`
- `!false` → `true`

## 9.7 `for (int idP : ruta->getIdsParadas())` — Range-based for loop

Equivalente a:
```cpp
const std::vector<int>& ids = ruta->getIdsParadas();
for (size_t i = 0; i < ids.size(); i++) {
    int idP = ids[i];
    // ... cuerpo del bucle ...
}
```

**Ventajas:** Más legible, no hay riesgo de errores con índices, funciona con cualquier contenedor que tenga `begin()` y `end()`.

## 9.8 `auto it = std::find_if(...)` — Deducción de tipo

`auto` le dice al compilador: "deduce el tipo por mí". El tipo de `it` será `std::vector<Parada>::iterator` porque `std::find_if` sobre un `vector<Parada>` devuelve ese tipo.

```cpp
// Sin auto (escritura manual):
std::vector<Parada>::iterator it = std::find_if(paradas.begin(), paradas.end(), ...);

// Con auto (el compilador deduce):
auto it = std::find_if(paradas.begin(), paradas.end(), ...);
```

## 9.9 `buses.emplace_back(id, placa, capMax, capAct, est)`

`emplace_back` construye el objeto **directamente** dentro del vector, evitando una copia innecesaria:

```cpp
// push_back: crea el objeto temporal y luego lo copia
lista.push_back(Bus(id, placa, capMax, capAct, est));

// emplace_back: construye directamente en el vector (más eficiente)
lista.emplace_back(id, placa, capMax, capAct, est);
```

## 9.10 `gestor.guardarBuses(buses)` — Persistencia inmediata

Cada vez que se modifica un dato importante, se guarda inmediatamente en el JSON correspondiente. Esto asegura que aunque el programa se cierre inesperadamente, los datos no se pierden.

---

# Sección 10: Flujo de Ejecución Completo (Call Graph)

## 10.1 Desde `main()` hasta el cierre

```
LLAMADA 1: main()
  │
  └── Crea SistemaTransporte sistema("data")
  │     └── Constructor: gestor("data"), usuarioActual = nullptr
  │
  └── sistema.iniciar()
        │
        ├── 1. Carga de datos desde JSON:
        │     ├── gestor.cargarBuses()
        │     │     └── Abre data/buses.json → json::parse → vector<Bus>
        │     ├── gestor.cargarParadas()
        │     │     └── Abre data/paradas.json → json::parse → vector<Parada>
        │     ├── gestor.cargarIncidentes()
        │     │     └── Abre data/incidentes.json → json::parse → vector<Incidente>
        │     ├── gestor.cargarRutas()
        │     │     ├── gestor.cargarHorariosPorDefecto() → horarios.json
        │     │     └── Abre data/rutas.json → para cada ruta:
        │     │           ├── Si tipo="RutaCentro" → new RutaCentro(...)
        │     │           ├── Si no → new RutaBarrio(...)
        │     │           ├── ruta->agregarParada(id) para cada parada
        │     │           └── Asigna horarios (propios o globales)
        │     └── gestor.cargarUsuarios()
        │           └── Abre data/usuarios.json → para cada usuario:
        │                 ├── "Estudiante" → new Estudiante(...)
        │                 ├── "Conductor" → new Conductor(...)
        │                 └── "Administrador" → new Administrador(...)
        │
        ├── 2. Configuración de consola:
        │     ├── SetConsoleTitle("Sistema de Gestion...")
        │     ├── SetConsoleOutputCP(65001)   // UTF-8
        │     └── SetConsoleCP(65001)
        │
        └── 3. BUCLE PRINCIPAL (while true):
              │
              ├── mostrarMenuPrincipal()
              │     ├── limpiarPantalla()
              │     │     ├── GetStdHandle(STD_OUTPUT_HANDLE)
              │     │     ├── GetConsoleScreenBufferInfo()
              │     │     ├── FillConsoleOutputCharacter()  // Borra
              │     │     ├── FillConsoleOutputAttribute()  // Resetea colores
              │     │     └── SetConsoleCursorPosition()    // Cursor a (0,0)
              │     ├── imprimirEncabezado()
              │     │     ├── horaActual() → GetLocalTime() → ostringstream
              │     │     ├── fechaActual() → GetLocalTime() → ostringstream
              │     │     └── setColor() → SetConsoleTextAttribute()
              │     ├── std::cout << menú [1] [2] [3] [4]
              │     └── leerInt() → std::cin >> v
              │
              ├── ¿Opción == 4 (Salir)? → break (termina el bucle)
              │
              ├── ¿Opción < 1 o > 3? → continue (repetir menú)
              │
              ├── autenticarUsuario(op)
              │     ├── std::cout << "Ingrese su ..."
              │     ├── leerInt() → codigo
              │     ├── std::find_if(usuarios, lambda):
              │     │     └── Para cada usuario: validarCodigo(codigo)
              │     │           ├── Estudiante: c == codigoEstudiantil
              │     │           ├── Conductor: c == codigoOperador
              │     │           └── Admin: c == codigoAdmin
              │     ├── ¿Encontrado? → devolver puntero
              │     └── ¿No encontrado? → error → Sleep(2000) → nullptr
              │
              ├── ¿Op == 1? → panelEstudiante()
              │     ├── while (true):
              │     │     ├── Mostrar lista de rutas
              │     │     ├── Usuario elige ruta (o 0 = volver)
              │     │     ├── Buscar ruta por ID en vector rutas
              │     │     ├── Mostrar detalles: paradas, horarios
              │     │     ├── ¿Reservar? (S/N):
              │     │     │     ├── Buscar bus disponible (por ID o cualquier bus)
              │     │     │     ├── Incrementar capacidadActual
              │     │     │     └── gestor.guardarBuses(buses)
              │     │     └── Sleep/Enter para continuar
              │     └── break (cuando elige Volver)
              │
              ├── ¿Op == 2? → panelConductor()
              │     ├── Obtener Conductor* (static_cast)
              │     ├── Buscar bus: buscarBusPorId(cond->getBusAsignado())
              │     ├── Buscar ruta: buscarRutaPorBus(bus->getPlaca())
              │     ├── while (true):
              │     │     ├── Mostrar info bus, ruta, horarios
              │     │     ├── Mostrar proximidad (distancia + tiempo a paradas)
              │     │     ├── Opciones:
              │     │     │     [1] Llegada a parada:
              │     │     │     │     ├── Toma primera parada de la ruta
              │     │     │     │     ├── bus->simularMovimiento(lat, lon)
              │     │     │     │     └── gestor.guardarBuses(buses)
              │     │     │     [2] Reportar incidente:
              │     │     │     │     ├── Pedir descripción y tipo
              │     │     │     │     ├── incidentes.emplace_back(id, desc, tipo, "Abierto")
              │     │     │     │     └── gestor.guardarIncidentes(incidentes)
              │     │     │     [3] Finalizar turno → break
              │     │     └── Sleep(1500)
              │     └── break (cuando elige Finalizar turno)
              │
              └── ¿Op == 3? → panelAdministrador()
                    ├── while (true):
                    │     ├── Mostrar estado global
                    │     ├── Opciones:
                    │     │     [1] gestionarRutas():
                    │     │     │     ├── Mostrar rutas + estado
                    │     │     │     ├── Elegir ID → toggle ruta->setEstado(!estado)
                    │     │     │     └── gestor.guardarRutas(rutas)
                    │     │     [2] gestionarUsuarios():
                    │     │     │     └── Mostrar lista (solo lectura)
                    │     │     [3] gestionarBuses():
                    │     │     │     ├── Mostrar flota
                    │     │     │     ├── Elegir ID + nueva capacidad
                    │     │     │     └── gestor.guardarBuses(buses)
                    │     │     [4] gestionarHorarios():
                    │     │     │     ├── Elegir ruta
                    │     │     │     ├── Elegir tipo (Barrio/Unillanos)
                    │     │     │     ├── Ingresar hora
                    │     │     │     └── gestor.guardarRutas(rutas)
                    │     │     [5] gestionarIncidentes():
                    │     │     │     ├── Mostrar incidentes
                    │     │     │     ├── Elegir ID → inc.cerrar()
                    │     │     │     └── gestor.guardarIncidentes(incidentes)
                    │     │     [6] Volver → break
                    │     └── Sleep(1500)
                    └── break (cuando elige Volver)

  FIN DEL PROGRAMA (opción 4 en menú principal)
    │
    └── main() retorna a sistema operativo
    └── Se destruye SistemaTransporte (sale de ámbito en main)
          └── ~SistemaTransporte()
                ├── for (Ruta* r : rutas) delete r;   // Libera todas las rutas
                └── for (Usuario* u : usuarios) delete u;  // Libera todos los usuarios
```

## 10.2 Resumen del ciclo de vida de objetos

| Objeto | ¿Cuándo se crea? | ¿Dónde vive? | ¿Cuándo se destruye? |
|--------|------------------|-------------|----------------------|
| `SistemaTransporte` | En `main()`: `SistemaTransporte sistema(...)` | Stack (automática) | Al salir de `main()` |
| `Buses` | `GestorArchivos::cargarBuses()` con `emplace_back` | Dentro del vector `buses` | Cuando `SistemaTransporte` se destruye |
| `Paradas` | `GestorArchivos::cargarParadas()` con `emplace_back` | Dentro del vector `paradas` | Cuando `SistemaTransporte` se destruye |
| `Incidentes` | `GestorArchivos::cargarIncidentes()` o `panelConductor()` | Dentro del vector `incidentes` | Cuando `SistemaTransporte` se destruye |
| `Rutas*` | `GestorArchivos::cargarRutas()` con `new` | Heap (montón) | `~SistemaTransporte()` con `delete` |
| `Usuarios*` | `GestorArchivos::cargarUsuarios()` con `new` | Heap (montón) | `~SistemaTransporte()` con `delete` |
| `UbicacionGPS*` | `Bus::setUbicacion()` con `new` | Heap (montón) | `~Bus()` con `delete` |

---

# Sección 11: Conceptos Teóricos y Buenas Prácticas

## 11.1 Programación Orientada a Objetos (POO)

**Definición:** Paradigma de programación que organiza el código en **objetos** que combinan **datos** (atributos) y **comportamiento** (métodos).

**Analogía:** Un carro real tiene atributos (color, marca, velocidad) y comportamientos (acelerar, frenar, girar). En POO, modelamos el carro como una clase con atributos y métodos.

**Pilares de la POO detectados en el proyecto:**

| Pilar | Definición | Ejemplo en el proyecto |
|-------|-----------|----------------------|
| **Abstracción** | Modelar solo los detalles relevantes del mundo real | `Coordenada` solo tiene latitud/longitud, no otros detalles geográficos irrelevantes |
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

**Ejemplo en el proyecto:**
```
Coordenada          Usuario (abstracto)     Ruta (abstracto)
    ↑                    ↑    ↑    ↑              ↑    ↑
UbicacionGPS     Estudiante Conductor Admin   RutaBarrio RutaCentro
```

**¿Qué se hereda?**
- Todos los miembros `public` y `protected`
- Los miembros `private` existen pero no son accesibles directamente

## 11.3 Polimorfismo con `virtual`

**Definición:** Capacidad de que un mismo método se comporte de manera diferente según el tipo real del objeto.

**¿Cómo funciona en memoria?**
```cpp
class Usuario {
public:
    virtual std::string getEncabezado() const = 0;
    // El compilador crea una TABLA VIRTUAL (vtable) para esta clase
};
```

Cada objeto que tiene métodos virtuales contiene un **puntero oculto** a la **vtable** (tabla de funciones virtuales) de su clase. Cuando se llama a un método virtual, el programa:
1. Sigue el puntero a la vtable
2. Busca la función correcta en la tabla
3. Ejecuta la implementación correspondiente al tipo real del objeto

**Esto permite que:**
```cpp
void mostrarEncabezado(Usuario* u) {
    cout << u->getEncabezado();  // ← Sin saber el tipo concreto, llama al método correcto
}

mostrarEncabezado(new Estudiante(...));   // → "Usuario: X (Estudiante - Sistemas)"
mostrarEncabezado(new Conductor(...));    // → "Usuario: X (Conductor - Turno: Mañana)"
mostrarEncabezado(new Administrador(...)); // → "Usuario: X (Administrador)"
```

## 11.4 Gestión de Memoria Manual vs. Automática

### Memoria Automática (Stack)
Variables locales creadas sin `new`. Se destruyen automáticamente al salir de su ámbito.
```cpp
void func() {
    Bus bus;  // ← Creado en el stack
}  // ← Se destruye automáticamente al llegar aquí
```

### Memoria Manual (Heap)
Objetos creados con `new`. **Deben** destruirse con `delete`.
```cpp
void func() {
    Bus* bus = new Bus(1, "VST-001", 50, 0, true);  // ← Heap
    // ... usar bus ...
    delete bus;  // ← Obligatorio o hay fuga de memoria
}
```

### Problema: Fuga de Memoria (Memory Leak)
Ocurre cuando se reserva memoria con `new` pero nunca se llama a `delete`:
```cpp
void func() {
    int* p = new int[1000];  // ← Reserva 4000 bytes en el heap
}  // ← p se destruye, pero los 4000 bytes siguen ocupados e inaccesibles
```

**En el proyecto:**
- `Bus::ubicacion` (UbicacionGPS*) se crea en `setUbicacion()` y se destruye en `~Bus()`
- `rutas` (vector<Ruta*>) se crean en `cargarRutas()` y se destruyen en `~SistemaTransporte()`
- `usuarios` (vector<Usuario*>) se crean en `cargarUsuarios()` y se destruyen en `~SistemaTransporte()`

## 11.5 Composición vs. Herencia

**Composición:** Una clase contiene objetos de otra clase como atributos.
```cpp
class Parada {
    Coordenada ubicacion;  // ← Coordenada es PARTE de Parada
};
```

**Herencia:** Una clase ES un tipo de otra clase.
```cpp
class UbicacionGPS : public Coordenada {  // ← UbicacionGPS ES una Coordenada
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

**Formato intermedio:** JSON (JavaScript Object Notation) — texto legible y estructurado.

## 11.7 Manejo de Excepciones

**Definición:** Mecanismo para manejar errores de forma controlada.

**Sintaxis:**
```cpp
try {
    // Código que puede fallar
} catch (const std::exception& ex) {
    // Código que maneja el error
    std::cerr << "Error: " << ex.what() << '\n';
}
```

**En el proyecto:** `main.cpp` envuelve todo en un `try-catch` para capturar errores imprevistos (como no poder abrir archivos).

## 11.8 Paso por Referencia vs. Paso por Valor

```cpp
void porValor(std::string s);       // ← Se COPIA todo el string (lento)
void porReferencia(std::string& s); // ← Se pasa solo una referencia (rápido)
void porRefConst(const std::string& s); // ← Referencia + promesa de no modificar
```

**¿Por qué usar `const std::string&` en lugar de `std::string`?**
- `std::string` copia: O(n) donde n = longitud del string
- `const std::string&` referencia: O(1) — solo se pasa una dirección

---

# Sección 12: Resumen y Ruta de Estudio (Roadmap)

## 12.1 Integración de todas las piezas

El sistema **SistemaTransporte** funciona como un **reloj suizo** donde cada engranaje tiene su función:

1. **`main.cpp`** da la señal de arranque creando el objeto `SistemaTransporte`
2. **`SistemaTransporte::iniciar()`** carga todos los datos mediante `GestorArchivos`
3. **`GestorArchivos`** lee los archivos JSON usando la librería **nlohmann/json** y construye objetos de las clases del modelo (`Bus`, `Ruta`, `Usuario`, etc.)
4. El **bucle principal** del menú ofrece las opciones de perfil
5. **`autenticarUsuario()`** busca en el vector de `Usuario*` usando polimorfismo (cada tipo valida su código a su manera)
6. Dependiendo del perfil, se ejecuta uno de los tres paneles:
   - **Estudiante**: navega por `Ruta*` y `Parada` (con `Coordenada`), reserva en `Bus`
   - **Conductor**: usa `UbicacionGPS` (en `Bus`) para simular movimiento y calcular distancias con la fórmula de **Haversine**
   - **Administrador**: modifica estados y persiste con `GestorArchivos`
7. Cada cambio se guarda inmediatamente en JSON mediante `GestorArchivos::guardar*()`
8. Al salir, el **destructor** `~SistemaTransporte()` libera toda la memoria dinámica

## 12.2 Ruta de Estudio Recomendada

Para un estudiante que quiere entender el código sin frustrarse, recomiendo leer los archivos en este orden:

### 🥇 Primera semana — Fundamentos

| Paso | Archivo | Conceptos a aprender |
|------|---------|---------------------|
| **1** | `main.cpp` | Punto de entrada, `try-catch`, creación de objetos |
| **2** | `Coordenada.h` + `.cpp` | Clase simple, `double`, getters/setters, `const`, fórmula matemática |
| **3** | `Parada.h` + `.cpp` | Composición (una clase que contiene otra), `bool`, operador `<` |
| **4** | `Incidente.h` + `.cpp` | Clase con estado, método `cerrar()` |
| **5** | `HorarioRuta.h` + `.cpp` | `std::vector<std::string>`, `push_back`, bucles, formateo |

### 🥈 Segunda semana — Herencia y Polimorfismo

| Paso | Archivo | Conceptos a aprender |
|------|---------|---------------------|
| **6** | `Usuario.h` + `.cpp` | Clase abstracta, `virtual ... = 0`, destructor virtual |
| **7** | `Estudiante.h` + `.cpp` | Herencia, `override`, `validarCodigo()` |
| **8** | `Conductor.h` + `.cpp` | Herencia con más atributos |
| **9** | `Administrador.h` + `.cpp` | Herencia simple |
| **10** | `UbicacionGPS.h` + `.cpp` | Herencia de `Coordenada`, `new/delete` |
| **11** | `Ruta.h` + `.cpp` | Clase abstracta, `vector<int>`, operadores `<` y `==` |
| **12** | `RutaBarrio.h` + `.cpp` | Herencia simple de Ruta |
| **13** | `RutaCentro.h` + `.cpp` | Herencia con atributo extra |

### 🥉 Tercera semana — Lógica de Negocio

| Paso | Archivo | Conceptos a aprender |
|------|---------|---------------------|
| **14** | `Bus.h` + `.cpp` | Puntero dinámico (`UbicacionGPS*`), destructor, `simularMovimiento()`, operador `<` |
| **15** | `GestorArchivos.h` + `.cpp` | `ifstream`/`ofstream`, `nlohmann/json`, `emplace_back`, `new`, `static_cast` |

### 🎯 Cuarta semana — Orquestación

| Paso | Archivo | Conceptos a aprender |
|------|---------|---------------------|
| **16** | `SistemaTransporte.h` + `.cpp` (primera mitad) | `windows.h`, colores, `GetLocalTime`, `limpiarPantalla`, `autenticarUsuario` |
| **17** | `SistemaTransporte.cpp` (paneles) | `panelEstudiante()`, `panelConductor()`, `panelAdministrador()` |
| **18** | `SistemaTransporte.cpp` (completo) | Flujo completo, `std::find_if`, `std::count_if`, `std::any_of`, lambdas |

### 📁 Datos

| Paso | Archivo | Conceptos a aprender |
|------|---------|---------------------|
| **19** | `data/*.json` | Formato JSON, persistencia |
| **20** | `CMakeLists.txt` | Sistema de construcción, compilación |

## 12.3 Ejercicios de estudio sugeridos

1. **Traza manual**: Sigue el flujo cuando un Estudiante con código 160005557 inicia sesión y reserva un asiento en el bus VST-001. ¿Qué métodos se llaman y en qué orden?

2. **Modificación sencilla**: Agrega un nuevo tipo de usuario llamado "Invitado" que herede de `Usuario`. El invitado solo puede ver las rutas (no reservar). ¿Qué archivos necesitas modificar?

3. **Depuración**: Si `gestor.cargarUsuarios()` no puede abrir `usuarios.json`, ¿qué ocurre? ¿Dónde se maneja ese error?

4. **Cálculo**: Si un bus está en lat=4.14, lon=-73.65 y una parada está en lat=4.12, lon=-73.63, ¿cuál es la distancia aproximada según Haversine? (Pista: 1 grado ≈ 111km en el ecuador)

5. **Extensión**: ¿Cómo agregarías un nuevo método en `GestorArchivos` para guardar también las paradas? ¿Qué firma tendría?

## 12.4 Palabras finales

El proyecto **SistemaTransporte** es un excelente ejemplo de cómo se construye una aplicación real en C++ aplicando:

- **Programación Orientada a Objetos** (herencia, polimorfismo, encapsulamiento, composición)
- **Gestión de memoria** (stack vs. heap, `new`/`delete`)
- **Persistencia de datos** (serialización JSON con nlohmann)
- **Algoritmos de la STL** (`find_if`, `count_if`, `any_of`)
- **Estructuras de datos** (`vector`, `map`, `string`)
- **Interfaz de usuario** (API de consola de Windows)
- **Arquitectura en capas** (UI → Lógica → Datos)
- **Buenas prácticas** (destructores para liberar memoria, `const` para inmutabilidad, encapsulamiento)

Cada línea de código tiene un propósito claro y está organizada siguiendo principios de ingeniería de software. Al estudiar este proyecto, no solo aprendes C++, sino también **cómo pensar y diseñar software** de manera estructurada y profesional.

---

> **Fin del documento.**  
> *Creado con propósitos educativos. Proyecto original: Universidad de los Llanos, Villavicencio, Meta, Colombia.*



# Documentación Técnica y Educativa Completa — SistemaTransporte (C++)

> **Autor:** Generada automáticamente a partir del análisis exhaustivo del código fuente.
> **Lenguaje:** C++17 · **Plataforma:** Windows · **Compilación:** CMake ≥ 3.16

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

---

# Sección 1: Visión General y Arquitectura

## 1.1 ¿Qué es SistemaTransporte?

**SistemaTransporte** es una aplicación de consola desarrollada en **C++17** para la plataforma **Windows** que simula un sistema de gestión de transporte universitario para la **Universidad de los Llanos (Unillanos)**, ubicada en Villavicencio, Colombia.

El programa permite a tres tipos de usuarios (estudiantes, conductores y administradores) interactuar con el sistema de rutas de buses que conectan diferentes barrios y zonas del centro de la ciudad con la universidad.

## 1.2 ¿Qué problema resuelve?

En muchas universidades, la gestión del transporte estudiantil se realiza de forma manual o descoordinada. Este software resuelve:

1. **Para los estudiantes:** Consultar rutas, ver paradas, conocer horarios y reservar asientos en los buses.
2. **Para los conductores:** Conocer su ruta asignada, registrar llegadas a paradas, reportar incidentes y ver el estado de su bus.
3. **Para los administradores:** Gestionar rutas, usuarios, buses, horarios e incidentes desde un panel centralizado.

## 1.3 ¿Para qué fue creado?

Fue creado como un **proyecto académico** de Programación Orientada a Objetos (POO) para demostrar:

- Uso de **herencia** y **polimorfismo** en C++.
- Gestión de datos con **serialización/deserialización JSON**.
- Separación de responsabilidades mediante **múltiples clases**.
- Interacción con la **Windows API** para la interfaz de consola.
- Uso de la **fórmula Haversine** para calcular distancias geográficas reales.

## 1.4 Diagrama de Arquitectura

El sistema sigue una arquitectura de **tres capas** (similar al patrón MVC):

```
┌─────────────────────────────────────────────────────────────────────────┐
│                       CAPA DE PRESENTACIÓN (UI)                        │
│                                                                         │
│   main.cpp  ──►  SistemaTransporte::iniciar()                          │
│                  ├── mostrarMenuPrincipal()        ← Consola Windows   │
│                  ├── panelEstudiante()             ← setColor()        │
│                  ├── panelConductor()              ← limpiarPantalla() │
│                  └── panelAdministrador()          ← leerInt()         │
│                      ├── gestionarRutas()                              │
│                      ├── gestionarUsuarios()                           │
│                      ├── gestionarBuses()                              │
│                      ├── gestionarHorarios()                           │
│                      └── gestionarIncidentes()                         │
├─────────────────────────────────────────────────────────────────────────┤
│                    CAPA DE LÓGICA DE NEGOCIO                           │
│                                                                         │
│   Coordenada ─► UbicacionGPS ─► Bus                                   │
│   HorarioRuta ──┐                                                      │
│   Parada ───────┤                                                      │
│   Incidente     ├── Ruta (abstracta) ──► RutaBarrio                   │
│                 │                    └──► RutaCentro                    │
│   Usuario (abstracta) ──► Estudiante                                   │
│                       ├──► Conductor                                   │
│                       └──► Administrador                               │
├─────────────────────────────────────────────────────────────────────────┤
│                    CAPA DE PERSISTENCIA                                 │
│                                                                         │
│   GestorArchivos                                                        │
│   ├── cargarBuses()          ◄─── buses.json                           │
│   ├── cargarParadas()        ◄─── paradas.json                         │
│   ├── cargarIncidentes()     ◄─── incidentes.json                      │
│   ├── cargarRutas()          ◄─── rutas.json                           │
│   ├── cargarUsuarios()       ◄─── usuarios.json                        │
│   ├── cargarHorariosPorDefecto() ◄─── horarios.json                    │
│   ├── guardarBuses()         ───► buses.json                           │
│   ├── guardarIncidentes()    ───► incidentes.json                      │
│   ├── guardarUsuarios()      ───► usuarios.json                        │
│   └── guardarRutas()         ───► rutas.json                           │
├─────────────────────────────────────────────────────────────────────────┤
│                    CAPA DE DATOS (Archivos JSON)                        │
│                                                                         │
│   data/                                                                 │
│   ├── buses.json         (3 buses)                                     │
│   ├── paradas.json       (67 paradas con GPS)                          │
│   ├── incidentes.json    (3 incidentes)                                │
│   ├── rutas.json         (12 rutas)                                    │
│   ├── usuarios.json      (6 usuarios)                                  │
│   └── horarios.json      (configuración global)                        │
└─────────────────────────────────────────────────────────────────────────┘
```

## 1.5 Flujo General de Ejecución

```
main()
  └─► SistemaTransporte sistema("data")     // Constructor: crea GestorArchivos
      └─► sistema.iniciar()
          ├─► cargarBuses()                  // Lee buses.json → vector<Bus>
          ├─► cargarParadas()                // Lee paradas.json → vector<Parada>
          ├─► cargarIncidentes()             // Lee incidentes.json → vector<Incidente>
          ├─► cargarRutas()                  // Lee rutas.json → vector<Ruta*>
          ├─► cargarUsuarios()               // Lee usuarios.json → vector<Usuario*>
          ├─► SetConsoleTitle(...)            // Configura título de ventana
          ├─► SetConsoleOutputCP(65001)       // Habilita UTF-8
          └─► BUCLE PRINCIPAL (while true)
              ├─► mostrarMenuPrincipal()     // Muestra opciones 1-4
              ├─► autenticarUsuario(op)      // Pide código y busca en usuarios
              ├─► panelEstudiante()          // Si op == 1
              ├─► panelConductor()           // Si op == 2
              ├─► panelAdministrador()       // Si op == 3
              └─► break (si op == 4)         // Salir
```

---

# Sección 2: Estructura de Carpetas

## 2.1 Vista General del Proyecto

```
SistemaTransporte/
├── CMakeLists.txt              ← Archivo de configuración de CMake
├── main.cpp                    ← Punto de entrada del programa
├── documentacion_tecnica.md    ← Este documento
│
├── include/                    ← Directorio de archivos de cabecera (.h)
│   ├── json.hpp                ← Librería nlohmann/json (single-header)
│   ├── Coordenada.h
│   ├── UbicacionGPS.h
│   ├── HorarioRuta.h
│   ├── Parada.h
│   ├── Incidente.h
│   ├── Ruta.h
│   ├── RutaBarrio.h
│   ├── RutaCentro.h
│   ├── Bus.h
│   ├── Usuario.h
│   ├── Estudiante.h
│   ├── Conductor.h
│   ├── Administrador.h
│   ├── GestorArchivos.h
│   └── SistemaTransporte.h
│
├── src/                        ← Directorio de implementaciones (.cpp)
│   ├── Coordenada.cpp
│   ├── UbicacionGPS.cpp
│   ├── HorarioRuta.cpp
│   ├── Parada.cpp
│   ├── Incidente.cpp
│   ├── Ruta.cpp
│   ├── RutaBarrio.cpp
│   ├── RutaCentro.cpp
│   ├── Bus.cpp
│   ├── Usuario.cpp
│   ├── Estudiante.cpp
│   ├── Conductor.cpp
│   ├── Administrador.cpp
│   ├── GestorArchivos.cpp
│   └── SistemaTransporte.cpp
│
└── data/                       ← Directorio de archivos de datos JSON
    ├── buses.json
    ├── paradas.json
    ├── incidentes.json
    ├── rutas.json
    ├── usuarios.json
    └── horarios.json
```

## 2.2 Explicación de Cada Directorio

### `include/` — Archivos de Cabecera (Headers)

**¿Qué es un archivo de cabecera?** Es un archivo con extensión `.h` que contiene las **declaraciones** de clases, funciones y constantes. Piensa en él como un "contrato" o "índice": dice QUÉ existe (nombres de clases, firmas de funciones, tipos de atributos) pero NO dice CÓMO funciona.

**¿Por qué es importante?** En C++, cuando un archivo `.cpp` quiere usar una clase definida en otro lugar, necesita saber su "forma" (sus atributos, sus métodos). El header proporciona esa información mediante `#include`.

**Responsabilidad:** Definir la **interfaz pública y privada** de cada clase sin revelar los detalles de implementación.

### `src/` — Archivos de Implementación

**¿Qué es un archivo de implementación?** Es un archivo con extensión `.cpp` que contiene el **cuerpo** (código ejecutable) de los métodos declarados en los headers. Aquí es donde se escribe la lógica real.

**Responsabilidad:** Implementar el comportamiento concreto de cada método: qué cálculos hace, qué decisiones toma, qué retorna.

### `data/` — Archivos de Datos JSON

**¿Qué es JSON?** JSON (JavaScript Object Notation) es un formato de texto estándar para representar datos estructurados. Es legible tanto por humanos como por programas.

**Responsabilidad:** Almacenar el estado persistente del sistema: buses, paradas, rutas, usuarios e incidentes. Cuando el programa se cierra y se vuelve a abrir, los datos se recuperan de estos archivos.

## 2.3 ¿Por Qué Se Separan Headers de Implementaciones en C++?

Esta separación es una **convención fundamental** del lenguaje C++ y tiene razones técnicas y prácticas:

1. **Compilación separada:** El compilador puede compilar cada `.cpp` independientemente. Si cambias la implementación de `Bus.cpp`, solo se recompila ese archivo, no todo el proyecto.

2. **Ocultación de la implementación:** Puedes distribuir los `.h` (interfaz) sin revelar los `.cpp` (código interno). Esto es como dar el manual de instrucciones de un electrodoméstico sin revelar los planos de ingeniería.

3. **Reducción de dependencias:** Si `SistemaTransporte.cpp` necesita usar la clase `Bus`, solo incluye `Bus.h`. No necesita saber cómo `Bus` implementa sus métodos.

4. **Evitar definiciones múltiples:** Si el código de un método estuviera en el `.h` y dos archivos `.cpp` incluyeran ese `.h`, el compilador encontraría el mismo método definido dos veces, lo cual es un error. Ponerlo en el `.cpp` evita esto.

**Analogía:** Imagina un restaurante. El menú (header) te dice qué platos hay disponibles y qué ingredientes tienen. La receta (implementación) está en la cocina y el cliente no la ve. Puedes cambiar la receta sin cambiar el menú, siempre que el plato siga siendo el mismo.

---

# Sección 3: Explicación Archivo por Archivo

## 3.1 `main.cpp`

- **Ubicación:** `SistemaTransporte/main.cpp`
- **Propósito:** Es el **punto de entrada** del programa. Todo programa C++ comienza su ejecución en la función `main()`.

### Dependencias

```cpp
#include <iostream>              // Para std::cerr (imprimir errores)
#include <stdexcept>             // Para std::exception (capturar excepciones)
#include "SistemaTransporte.h"   // Para usar la clase SistemaTransporte
```

- `<iostream>` se necesita porque en caso de error se imprime un mensaje usando `std::cerr`.
- `<stdexcept>` proporciona la clase base `std::exception` para capturar errores.
- `"SistemaTransporte.h"` incluye la declaración de la clase principal.

### ¿Qué contiene?

Solo la función `main()` con un bloque `try-catch`:
- Crea un objeto `SistemaTransporte` pasándole la ruta `"data"` donde están los JSON.
- Llama a `sistema.iniciar()` que carga datos y arranca el bucle de menús.
- Si ocurre cualquier excepción, la captura e imprime el mensaje de error.

### ¿Qué fallaría si se elimina?

**Todo.** Sin `main.cpp` el compilador no puede crear un ejecutable porque no existe punto de entrada.

---

## 3.2 `CMakeLists.txt`

- **Ubicación:** `SistemaTransporte/CMakeLists.txt`
- **Propósito:** Archivo de configuración de **CMake**, el sistema de compilación. Le dice al compilador qué archivos compilar, qué estándar usar y cómo organizar el proyecto.

### ¿Qué contiene?

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

### ¿Qué fallaría si se elimina?

No se podría compilar el proyecto con CMake. Habría que compilar manualmente con comandos del compilador, lo cual es tedioso y propenso a errores.

---

## 3.3 `include/Coordenada.h` y `src/Coordenada.cpp`

- **Propósito:** Define la clase `Coordenada` que representa un **punto geográfico** con latitud y longitud.
- **Dependencias del header:** Ninguna (solo `#pragma once`).
- **Dependencias de la implementación:** `"Coordenada.h"`, `<cmath>` (funciones matemáticas).

### ¿Qué contiene?

- Dos atributos privados: `latitud` (double) y `longitud` (double).
- Dos constructores (vacío y con parámetros).
- Getters y setters.
- El método `distanciaHacia()` que implementa la **fórmula Haversine** para calcular distancia entre dos puntos geográficos en metros.

### ¿Qué fallaría si se elimina?

`UbicacionGPS`, `Parada`, `Bus` y cualquier cálculo de distancias dejaría de funcionar, ya que todos dependen de `Coordenada`.

---

## 3.4 `include/UbicacionGPS.h` y `src/UbicacionGPS.cpp`

- **Propósito:** Extiende `Coordenada` con **altitud** y **velocidad** para simular una posición GPS en tiempo real.
- **Dependencias:** `"Coordenada.h"` (hereda de ella).

### ¿Qué contiene?

- Dos atributos adicionales: `altitud` (double) y `velocidadKmh` (double, por defecto 30 km/h).
- Método `tiempoEstimadoMinutos()` que convierte distancia en metros a tiempo en minutos usando la velocidad actual.

### ¿Qué fallaría si se elimina?

La clase `Bus` no podría compilar porque su atributo `UbicacionGPS* ubicacion` depende de esta clase. Todo el cálculo de tiempos de llegada dejaría de funcionar.

---

## 3.5 `include/HorarioRuta.h` y `src/HorarioRuta.cpp`

- **Propósito:** Almacena los horarios de salida desde el barrio y desde Unillanos.
- **Dependencias:** `<string>`, `<vector>`.

### ¿Qué contiene?

- Dos vectores de strings: `salidasBarrio` y `salidasUnillanos` (cada uno con horas como "06:10:00").
- Métodos para agregar horarios individualmente.
- Métodos `formatearBarrio()` y `formatearUnillanos()` que concatenan los horarios separados por `" | "` para mostrarlos en pantalla.

### ¿Qué fallaría si se elimina?

La clase `Ruta` no compilaría porque contiene un objeto `HorarioRuta horario` como atributo.

---

## 3.6 `include/Parada.h` y `src/Parada.cpp`

- **Propósito:** Representa una **parada física** dentro de una ruta de bus.
- **Dependencias:** `<string>`, `"Coordenada.h"`.

### ¿Qué contiene?

- Atributos: `idParada`, `nombre`, `ubicacion` (Coordenada), `altitud`, `estado` (activa o no), `ordenEnRuta`.
- Método `distanciaA(lat, lon)` que calcula la distancia desde la parada a un punto dado.
- Operador `<` para comparar paradas por su orden en la ruta (permite ordenar vectores de paradas).

### ¿Qué fallaría si se elimina?

Todas las funcionalidades de rutas y visualización de paradas dejarían de funcionar. `Ruta` almacena IDs de paradas y `SistemaTransporte` busca paradas constantemente.

---

## 3.7 `include/Incidente.h` y `src/Incidente.cpp`

- **Propósito:** Registra un incidente reportado en el sistema (mecánico, accidente, etc.).
- **Dependencias:** `<string>`.

### ¿Qué contiene?

- Atributos: `idIncidente`, `descripcion`, `tipo` ("Mecánico", "Accidente"), `estado` ("Abierto" o "Cerrado").
- Método `cerrar()` que cambia el estado a "Cerrado".

### ¿Qué fallaría si se elimina?

Los conductores no podrían reportar problemas y los administradores no podrían gestionar incidentes.

---

## 3.8 `include/Ruta.h` y `src/Ruta.cpp`

- **Propósito:** Clase **base abstracta** para todos los tipos de ruta del sistema.
- **Dependencias:** `<string>`, `<vector>`, `"HorarioRuta.h"`, `"Parada.h"`.

### ¿Qué contiene?

- Atributos: `idRuta`, `nombre`, `origen`, `destino`, `estado`, `tipo`, `puntoSalida`, `horario` (HorarioRuta), `idsParadas` (vector de IDs de paradas).
- Método virtual puro `getInfoAdicional() = 0` que obliga a las subclases a implementar información específica.
- Operadores `<` y `==` para comparar rutas por ID.
- Destructor virtual `virtual ~Ruta() {}` necesario para polimorfismo seguro.

### ¿Qué fallaría si se elimina?

**Toda la lógica de rutas colapsa.** `RutaBarrio`, `RutaCentro`, `GestorArchivos` y `SistemaTransporte` dependen de `Ruta`.

---

## 3.9 `include/RutaBarrio.h` y `src/RutaBarrio.cpp`

- **Propósito:** Ruta que opera entre un **barrio** de la ciudad y la universidad. Es la subclase más simple de `Ruta`.
- **Dependencias:** `"Ruta.h"`.

### ¿Qué contiene?

- Constructor que fija automáticamente el tipo como `"RutaBarrio"`.
- Implementación de `getInfoAdicional()` que retorna un texto como `"Tipo: Ruta de Barrio | Trayecto: Barrio Amarilo -> Universidad de los Llanos"`.

### ¿Qué fallaría si se elimina?

Las 11 rutas de barrio del sistema no podrían crearse. El cargador de rutas en `GestorArchivos` fallaría al intentar crear objetos `RutaBarrio`.

---

## 3.10 `include/RutaCentro.h` y `src/RutaCentro.cpp`

- **Propósito:** Ruta que opera desde la **zona centro** de la ciudad hacia la universidad. Agrega el atributo `zonaCentro`.
- **Dependencias:** `"Ruta.h"`.

### ¿Qué contiene?

- Atributo adicional: `zonaCentro` (string) — la zona del centro cubierta.
- Constructor que fija el tipo como `"RutaCentro"`.
- `getInfoAdicional()` incluye la zona del centro en el texto.

### ¿Qué fallaría si se elimina?

La ruta 4 ("Unillanos - Parque"), que es de tipo `RutaCentro`, no podría cargarse.

---

## 3.11 `include/Bus.h` y `src/Bus.cpp`

- **Propósito:** Representa un **bus de la flota** con su ubicación GPS en tiempo real.
- **Dependencias:** `<string>`, `"UbicacionGPS.h"`, `<cstdlib>`.

### ¿Qué contiene?

- Atributos: `idBus`, `placa`, `capacidadMaxima`, `capacidadActual`, `estado`, `ubicacion` (puntero a `UbicacionGPS`), `indiceParadaActual`, `idRutaAsignada`.
- **Destructor** que libera la memoria del puntero `ubicacion` con `delete`.
- `setUbicacion()` que crea un nuevo objeto `UbicacionGPS` en el heap.
- `simularMovimiento()` que mueve el GPS del bus un 5% hacia el destino.
- `tiempoHastaParada()` que calcula tiempo estimado usando distancia y velocidad.
- Operador `<` que compara buses por capacidad disponible (orden inverso).

### ¿Qué fallaría si se elimina?

No habría buses en el sistema. Los conductores no tendrían vehículo asignado y los estudiantes no podrían reservar asientos.

---

## 3.12 `include/Usuario.h` y `src/Usuario.cpp`

- **Propósito:** Clase **base abstracta** para todos los tipos de usuario del sistema.
- **Dependencias:** `<string>`.

### ¿Qué contiene?

- Atributos comunes: `idUsuario`, `nombre`, `telefono`, `correo`, `tipo`.
- Dos métodos virtuales puros:
  - `getEncabezado() = 0` — devuelve texto de bienvenida del usuario.
  - `validarCodigo(int) = 0` — verifica si un código de acceso es correcto.
- Destructor virtual para polimorfismo seguro.

### ¿Qué fallaría si se elimina?

`Estudiante`, `Conductor` y `Administrador` no compilarían. Todo el sistema de autenticación dejaría de existir.

---

## 3.13 `include/Estudiante.h` y `src/Estudiante.cpp`

- **Propósito:** Representa a un **estudiante universitario** con datos académicos.
- **Dependencias:** `"Usuario.h"`.

### ¿Qué contiene?

- Atributos adicionales: `codigoEstudiantil`, `facultad`, `programa`, `semestre`.
- `getEncabezado()` retorna `"Usuario: Nombre (Estudiante - Programa)"`.
- `validarCodigo()` compara el código ingresado con `codigoEstudiantil`.

---

## 3.14 `include/Conductor.h` y `src/Conductor.cpp`

- **Propósito:** Representa a un **conductor** con su turno y bus asignado.
- **Dependencias:** `"Usuario.h"`.

### ¿Qué contiene?

- Atributos adicionales: `codigoOperador`, `experiencia`, `turno`, `busAsignado` (idBus).
- `getEncabezado()` retorna `"Usuario: Nombre (Conductor - Turno: Mañana)"`.
- `validarCodigo()` compara con `codigoOperador`.

---

## 3.15 `include/Administrador.h` y `src/Administrador.cpp`

- **Propósito:** Representa al **administrador** del sistema con acceso total.
- **Dependencias:** `"Usuario.h"`.

### ¿Qué contiene?

- Atributo adicional: `codigoAdmin`.
- `getEncabezado()` retorna `"Usuario: Nombre (Administrador)"`.
- `validarCodigo()` compara con `codigoAdmin`.

---

## 3.16 `include/GestorArchivos.h` y `src/GestorArchivos.cpp`

- **Propósito:** Gestiona la **lectura y escritura** de todos los archivos JSON. Es la capa de persistencia.
- **Dependencias:** Incluye TODOS los headers de las clases de negocio, `<fstream>`, `<iostream>`, y `"json.hpp"`.

### ¿Qué contiene?

- Un atributo `rutaData` (string) con la ruta al directorio de datos.
- **Métodos de carga (lectura):**
  - `cargarBuses()` → lee `buses.json`
  - `cargarParadas()` → lee `paradas.json`
  - `cargarIncidentes()` → lee `incidentes.json`
  - `cargarRutas()` → lee `rutas.json` (con polimorfismo)
  - `cargarUsuarios()` → lee `usuarios.json` (con polimorfismo)
  - `cargarHorariosPorDefecto()` → lee `horarios.json`
- **Métodos de guardado (escritura):**
  - `guardarBuses()`, `guardarIncidentes()`, `guardarUsuarios()`, `guardarRutas()`

### ¿Qué fallaría si se elimina?

El sistema no podría leer ni escribir datos. Sería un programa vacío sin información.

---

## 3.17 `include/SistemaTransporte.h` y `src/SistemaTransporte.cpp`

- **Propósito:** **Clase principal** que orquesta todos los módulos. Es el "cerebro" del programa.
- **Dependencias:** Incluye todos los headers necesarios: `GestorArchivos.h`, `Bus.h`, `Parada.h`, `Incidente.h`, `Ruta.h`, `Usuario.h`, `<vector>`, `<string>`, `<map>`, `<windows.h>`, `<ctime>`, `<iostream>`, `<sstream>`, `<iomanip>`, `<algorithm>`.

### ¿Qué contiene?

- Todos los vectores de datos: `buses`, `paradas`, `incidentes`, `rutas`, `usuarios`.
- Puntero `usuarioActual` al usuario autenticado.
- Funciones auxiliares estáticas: `limpiarPantalla()`, `setColor()`, `leerInt()`.
- Métodos de navegación: `mostrarMenuPrincipal()`, `autenticarUsuario()`.
- Paneles por rol: `panelEstudiante()`, `panelConductor()`, `panelAdministrador()`.
- Sub-menús del administrador: `gestionarRutas()`, `gestionarUsuarios()`, `gestionarBuses()`, `gestionarHorarios()`, `gestionarIncidentes()`.
- Métodos auxiliares: `buscarBusPorId()`, `buscarRutaPorBus()`, `incidentesDeBus()`, `horaActual()`, `fechaActual()`, `mostrarProximidadBus()`.

---

## 3.18 Archivos JSON de Datos

### `data/buses.json`
Contiene un array `"buses"` con 3 buses. Cada bus tiene: `idBus`, `placa`, `capacidadMaxima`, `capacidadActual`, `estado`.

### `data/paradas.json`
Contiene un **array raíz** (sin clave envolvente) con 67 paradas reales de Villavicencio. Cada parada tiene: `idParada`, `nombre`, `ubicacion` (con `latitud`, `longitud`, `altitud`), `estado`, `ordenEnRuta`.

### `data/incidentes.json`
Contiene un array `"incidentes"` con 3 incidentes de ejemplo: fallas mecánicas y accidentes.

### `data/rutas.json`
Contiene un array `"rutas"` con 12 rutas. Cada ruta tiene: `idRuta`, `nombre`, `origen`, `destino`, `estado`, `tipo`, `puntoSalida`, `paradas` (array de IDs), `horarios` (puede estar vacío).

### `data/usuarios.json`
Contiene un array `"usuarios"` con 6 usuarios: 2 conductores, 1 administrador y 3 estudiantes. Cada uno tiene campos comunes y campos específicos según su tipo.

### `data/horarios.json`
Contiene la configuración global de horarios por defecto: horarios de salida desde el barrio y desde Unillanos que se aplican a las rutas que no tengan horarios propios.

---

# Sección 4: Análisis de Clases y Estructuras

## 4.0 Patrón de Encapsulamiento: Getters y Setters

Antes de detallar cada clase, expliquemos el patrón de **getters y setters** que se repite en todo el proyecto.

### ¿Qué es el encapsulamiento?

El **encapsulamiento** es uno de los cuatro pilares de la Programación Orientada a Objetos. Consiste en:

1. Declarar los atributos de la clase como **privados** (`private`), para que nadie externo pueda acceder directamente a ellos.
2. Proporcionar **métodos públicos** (`public`) para leer (**getter**) y modificar (**setter**) esos atributos de forma controlada.

### ¿Por qué no acceder directamente a los atributos?

Si los atributos fueran públicos, cualquier parte del código podría modificarlos sin control. Ejemplo:

```cpp
// SIN encapsulamiento (PELIGROSO):
bus.capacidadActual = -500;  // ¡Valor absurdo y no hay forma de prevenirlo!

// CON encapsulamiento (SEGURO):
bus.setCapacidadActual(-500); // El setter PODRÍA validar y rechazar el valor
```

### Ejemplo Detallado: Getter y Setter de `Coordenada`

**Header (Coordenada.h):**
```cpp
class Coordenada {
private:
    double latitud;     // Solo accesible dentro de la clase
public:
    double getLatitud() const;   // Getter: lee el valor
    void setLatitud(double lat); // Setter: escribe el valor
};
```

**Implementación (Coordenada.cpp):**
```cpp
double Coordenada::getLatitud() const { return latitud; }
void Coordenada::setLatitud(double lat) { latitud = lat; }
```

### Desglose del getter:

- `double` → tipo de retorno (devuelve un número decimal).
- `Coordenada::` → indica que este método pertenece a la clase `Coordenada`.
- `getLatitud()` → nombre del método (convención: "get" + nombre del atributo).
- `const` → promete que este método NO modifica ningún atributo del objeto.
- `{ return latitud; }` → cuerpo: simplemente devuelve el valor almacenado.

### Desglose del setter:

- `void` → no retorna nada.
- `setLatitud(double lat)` → recibe un parámetro `lat` de tipo `double`.
- `{ latitud = lat; }` → asigna el valor recibido al atributo privado.

**Para el resto de clases, los getters y setters siguen exactamente este mismo patrón. No se repetirá la explicación completa para cada uno; simplemente se listarán.**

---

## 4.1 Clase `Coordenada`

### Descripción General
Representa un **punto geográfico** en la superficie terrestre definido por su latitud y longitud. Es la clase más básica del sistema y sirve como fundamento para `UbicacionGPS` y `Parada`.

### Tabla de Atributos

| Atributo | Tipo | Significado | Ciclo de vida | Quién lo lee | Quién lo escribe |
|----------|------|-------------|---------------|-------------|-----------------|
| `latitud` | `double` | Posición norte-sur en grados decimales | Toda la vida del objeto | `getLatitud()`, `distanciaHacia()` | Constructor, `setLatitud()` |
| `longitud` | `double` | Posición este-oeste en grados decimales | Toda la vida del objeto | `getLongitud()`, `distanciaHacia()` | Constructor, `setLongitud()` |

### Métodos

#### Constructor por defecto: `Coordenada()`

- **Firma:** `Coordenada::Coordenada()`
- **Parámetros:** Ninguno.
- **Valor de retorno:** N/A (es constructor).
- **Responsabilidad:** Inicializar latitud y longitud en 0.0.
- **Implementación:** Usa lista de inicialización: `: latitud(0.0), longitud(0.0) {}`.
- **Ejemplo de uso:** `Coordenada c;` crea un punto en la coordenada (0, 0).

#### Constructor con parámetros: `Coordenada(double, double)`

- **Firma:** `Coordenada::Coordenada(double lat, double lon)`
- **Parámetros:** `lat` (latitud), `lon` (longitud).
- **Responsabilidad:** Inicializar con valores específicos.
- **Ejemplo de uso:** `Coordenada c(4.14527, -73.652037);`

#### Getters y Setters

Siguen el patrón descrito en la sección 4.0:
- `double getLatitud() const` — Retorna `latitud`.
- `double getLongitud() const` — Retorna `longitud`.
- `void setLatitud(double lat)` — Asigna `latitud = lat`.
- `void setLongitud(double lon)` — Asigna `longitud = lon`.

#### Método `distanciaHacia()`

- **Firma:** `double Coordenada::distanciaHacia(const Coordenada& otra) const`
- **Parámetros:** `otra` — referencia constante a otra `Coordenada`.
- **Valor de retorno:** `double` — distancia en **metros**.
- **Responsabilidad:** Calcula la distancia entre dos puntos geográficos usando la **fórmula Haversine**.
- **Flujo interno (pseudocódigo):**
  ```
  1. Convertir diferencias de latitud y longitud a radianes
  2. Calcular el componente 'a' usando senos y cosenos
  3. Calcular el arco central 'c' con atan2
  4. Multiplicar por el radio de la Tierra (6,371,000 m)
  5. Retornar la distancia
  ```
- **Variables locales:** `dLat`, `dLon`, `a`, `c` (todas `double`).
- **Complejidad:** O(1) — cálculo matemático de tiempo constante.
- **Ejemplo de uso:**
  ```cpp
  Coordenada a(4.14527, -73.652037);
  Coordenada b(4.14335, -73.650037);
  double metros = a.distanciaHacia(b); // Retorna ~275 metros (aproximado)
  ```

---

## 4.2 Clase `UbicacionGPS`

### Descripción General
**Hereda de `Coordenada`** y añade altitud y velocidad para simular la posición GPS de un bus en tiempo real. La relación "es-un" es correcta: una ubicación GPS **es una** coordenada con información extra.

### Diagrama de Herencia
```
    Coordenada
        │
        ▼
   UbicacionGPS
```

### Tabla de Atributos (propios, además de los heredados)

| Atributo | Tipo | Significado | Valor por defecto |
|----------|------|-------------|-------------------|
| `altitud` | `double` | Altura sobre el nivel del mar en metros | 0.0 |
| `velocidadKmh` | `double` | Velocidad actual del bus en km/h | 30.0 |

### Métodos

#### Constructor con parámetros

- **Firma:** `UbicacionGPS(double lat, double lon, double alt, double vel = 30.0)`
- **Nota:** `vel` tiene **valor por defecto** de 30.0, lo que significa que puedes omitir ese argumento.
- **Ejemplo:** `UbicacionGPS gps(4.14, -73.65, 463);` — la velocidad será 30.0 automáticamente.

#### Método `tiempoEstimadoMinutos()`

- **Firma:** `double UbicacionGPS::tiempoEstimadoMinutos(double distanciaMetros) const`
- **Parámetros:** `distanciaMetros` — distancia a recorrer.
- **Valor de retorno:** Tiempo estimado en minutos.
- **Flujo interno:**
  ```
  1. Si velocidadKmh <= 0 → retornar 0.0 (evita división por cero)
  2. Convertir metros a kilómetros: distanciaKm = distanciaMetros / 1000.0
  3. Calcular tiempo: (distanciaKm / velocidadKmh) * 60.0
  4. Retornar resultado
  ```
- **Ejemplo:** Si la distancia es 1500 metros y la velocidad es 30 km/h:
  - distanciaKm = 1.5
  - tiempo = (1.5 / 30) × 60 = 3.0 minutos

---

## 4.3 Clase `HorarioRuta`

### Descripción General
Almacena las listas de horarios de salida para una ruta: horarios desde el barrio hacia la universidad y horarios desde Unillanos hacia el barrio.

### Tabla de Atributos

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `salidasBarrio` | `std::vector<std::string>` | Lista de horas de salida desde el barrio |
| `salidasUnillanos` | `std::vector<std::string>` | Lista de horas de salida desde Unillanos |

### Métodos Especiales

#### `formatearBarrio()` y `formatearUnillanos()`

- **Firma:** `std::string formatearBarrio() const`
- **Responsabilidad:** Concatenar todos los horarios en un solo string separados por `" | "`.
- **Flujo interno:**
  ```
  1. Crear string vacío 'resultado'
  2. Para cada horario en el vector:
     a. Si no es el primero, agregar " | "
     b. Agregar el horario
  3. Si resultado está vacío, retornar "Sin horarios asignados"
  4. Si no, retornar resultado
  ```
- **Ejemplo de salida:** `"05:10:00 | 06:10:00 | 07:10:00 | 08:20:00 | 13:20:00"`

---

## 4.4 Clase `Parada`

### Descripción General
Representa una parada física de bus dentro de una ruta. Contiene nombre, ubicación geográfica, altitud, estado (activa/inactiva) y su posición ordinal dentro de la ruta.

### Tabla de Atributos

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `idParada` | `int` | Identificador único de la parada |
| `nombre` | `std::string` | Nombre descriptivo de la parada |
| `ubicacion` | `Coordenada` | Posición geográfica (latitud, longitud) |
| `altitud` | `double` | Altura sobre el nivel del mar |
| `estado` | `bool` | `true` = activa, `false` = inactiva |
| `ordenEnRuta` | `int` | Posición secuencial dentro de la ruta |

### Métodos Especiales

#### `distanciaA(double lat, double lon)`

- **Firma:** `double Parada::distanciaA(double lat, double lon) const`
- **Responsabilidad:** Calcula la distancia en metros desde esta parada hasta un punto dado.
- **Flujo:** Crea un `Coordenada` temporal con los parámetros y usa `ubicacion.distanciaHacia()`.

#### `operator<(const Parada& otra)`

- **Firma:** `bool Parada::operator<(const Parada& otra) const`
- **Responsabilidad:** Compara paradas por su `ordenEnRuta`. Esto permite usar `std::sort` sobre vectores de paradas.
- **Implementación:** `return ordenEnRuta < otra.ordenEnRuta;`

---

## 4.5 Clase `Incidente`

### Descripción General
Registra un evento problemático ocurrido en el sistema de transporte, como una falla mecánica o un accidente.

### Tabla de Atributos

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `idIncidente` | `int` | Identificador único |
| `descripcion` | `std::string` | Texto descriptivo del problema |
| `tipo` | `std::string` | Categoría: "Mecanico", "Accidente", "Otro" |
| `estado` | `std::string` | "Abierto" o "Cerrado" |

### Método Especial

#### `cerrar()`

- **Firma:** `void Incidente::cerrar()`
- **Responsabilidad:** Marca el incidente como resuelto cambiando `estado` a `"Cerrado"`.
- **Implementación:** `estado = "Cerrado";`

---

## 4.6 Clase `Ruta` (Abstracta)

### Descripción General
Clase base para todos los tipos de ruta. Es **abstracta** porque contiene el método virtual puro `getInfoAdicional() = 0`, lo que significa que no se pueden crear objetos `Ruta` directamente; solo de sus subclases.

### Diagrama de Herencia
```
      Ruta (abstracta)
      ├── RutaBarrio
      └── RutaCentro
```

### Tabla de Atributos

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `idRuta` | `int` | Identificador único |
| `nombre` | `std::string` | Nombre descriptivo (ej: "Unillanos - Amarilo") |
| `origen` | `std::string` | Punto de inicio |
| `destino` | `std::string` | Punto de llegada |
| `estado` | `bool` | `true` = activa |
| `tipo` | `std::string` | "RutaBarrio" o "RutaCentro" |
| `puntoSalida` | `std::string` | Descripción del punto exacto de partida |
| `horario` | `HorarioRuta` | Horarios de salida |
| `idsParadas` | `std::vector<int>` | IDs de las paradas en orden |

### Métodos Especiales

#### `getInfoAdicional()` — Virtual Puro

- **Firma:** `virtual std::string getInfoAdicional() const = 0`
- **El `= 0`** significa que NO tiene implementación aquí. Cada subclase DEBE implementarla.
- **¿Por qué?** Porque la información adicional depende del tipo de ruta.

#### `getHorario()` — Dos Versiones

```cpp
HorarioRuta& getHorario();              // Versión mutable
const HorarioRuta& getHorario() const;  // Versión constante
```

La versión mutable retorna una **referencia** al horario, permitiendo modificarlo directamente. La versión `const` es para cuando solo se necesita leer.

#### `agregarParada(int idParada)`

- **Implementación:** `idsParadas.push_back(idParada);` — agrega el ID al final del vector.

#### Operadores de Comparación

- `operator<` compara por `idRuta` (para ordenar).
- `operator==` compara por `idRuta` (para buscar).

---

## 4.7 Clase `RutaBarrio`

### Descripción General
Hereda de `Ruta`. Representa una ruta desde un barrio de la ciudad hasta Unillanos. No agrega atributos propios; solo implementa `getInfoAdicional()`.

### Método `getInfoAdicional()`

```cpp
std::string RutaBarrio::getInfoAdicional() const {
    return "Tipo: Ruta de Barrio | Trayecto: " + getOrigen() + " -> " + getDestino();
}
```

**Nota:** El constructor fija automáticamente `tipo = "RutaBarrio"` al llamar al constructor base con ese valor.

---

## 4.8 Clase `RutaCentro`

### Descripción General
Hereda de `Ruta`. Agrega el atributo `zonaCentro` que indica qué zona del centro de la ciudad cubre esta ruta.

### Atributo Adicional

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `zonaCentro` | `std::string` | Zona del centro cubierta (ej: "Centro") |

### Método `getInfoAdicional()`

```cpp
std::string RutaCentro::getInfoAdicional() const {
    return "Tipo: Ruta Centro | Zona: " + zonaCentro + " | " + getOrigen() + " -> " + getDestino();
}
```

---

## 4.9 Clase `Bus`

### Descripción General
Representa un bus de la flota de transporte. Es la única clase que gestiona **memoria dinámica** explícita usando `new`/`delete` para el puntero `UbicacionGPS*`.

### Tabla de Atributos

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `idBus` | `int` | Identificador único |
| `placa` | `std::string` | Placa del vehículo (ej: "VST-001") |
| `capacidadMaxima` | `int` | Número máximo de pasajeros |
| `capacidadActual` | `int` | Pasajeros actuales |
| `estado` | `bool` | `true` = activo |
| `ubicacion` | `UbicacionGPS*` | **Puntero** a la ubicación GPS actual |
| `indiceParadaActual` | `int` | Índice de la última parada visitada (-1 = no iniciada) |
| `idRutaAsignada` | `int` | ID de la ruta que el bus cubre |

### Métodos Especiales

#### Destructor `~Bus()`

```cpp
Bus::~Bus() {
    delete ubicacion;
}
```

**¿Por qué?** El atributo `ubicacion` es un puntero que apunta a memoria reservada con `new`. Si no se libera con `delete`, se produce un **memory leak** (fuga de memoria): la memoria queda reservada pero inaccesible.

#### `setUbicacion(double lat, double lon, double alt, double vel)`

```cpp
void Bus::setUbicacion(double lat, double lon, double alt, double vel) {
    delete ubicacion;                                    // Libera la anterior
    ubicacion = new UbicacionGPS(lat, lon, alt, vel);    // Crea una nueva
}
```

**¿Por qué `delete` primero?** Si ya existía una ubicación anterior, se debe liberar esa memoria antes de asignar una nueva. De lo contrario, la memoria antigua se pierde (memory leak).

#### `simularMovimiento(double latDest, double lonDest)`

```cpp
void Bus::simularMovimiento(double latDest, double lonDest) {
    if (!ubicacion) return;
    double deltaLat = (latDest - ubicacion->getLatitud())  * 0.05;
    double deltaLon = (lonDest - ubicacion->getLongitud()) * 0.05;
    ubicacion->setLatitud(ubicacion->getLatitud()   + deltaLat);
    ubicacion->setLongitud(ubicacion->getLongitud() + deltaLon);
}
```

**¿Cómo funciona?** Calcula el 5% de la distancia restante hacia el destino y avanza esa cantidad. Esto produce un movimiento suave que se desacelera al acercarse al destino (similar a una interpolación exponencial).

#### `tiempoHastaParada(double latParada, double lonParada)`

```cpp
double Bus::tiempoHastaParada(double latParada, double lonParada) const {
    if (!ubicacion) return -1.0;
    Coordenada destino(latParada, lonParada);
    double dist = ubicacion->distanciaHacia(destino);
    return ubicacion->tiempoEstimadoMinutos(dist);
}
```

**Flujo:**
1. Si no hay ubicación GPS, retorna -1 (error).
2. Crea una coordenada temporal con la posición de la parada.
3. Calcula la distancia en metros (Haversine).
4. Convierte a tiempo en minutos usando la velocidad del bus.

#### `operator<(const Bus& otro)`

```cpp
bool Bus::operator<(const Bus& otro) const {
    return (capacidadMaxima - capacidadActual) > (otro.capacidadMaxima - otro.capacidadActual);
}
```

**¡Atención!** Este operador está **invertido intencionalmente**: el bus con MAYOR capacidad disponible se considera "menor". Esto es útil para ordenar buses de forma que el que tenga más asientos libres aparezca primero.

---

## 4.10 Clase `Usuario` (Abstracta)

### Descripción General
Clase base abstracta para todos los tipos de usuario. No se pueden crear objetos `Usuario` directamente.

### Diagrama de Herencia
```
      Usuario (abstracta)
      ├── Estudiante
      ├── Conductor
      └── Administrador
```

### Tabla de Atributos

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `idUsuario` | `int` | Identificador único |
| `nombre` | `std::string` | Nombre completo |
| `telefono` | `std::string` | Número de teléfono |
| `correo` | `std::string` | Correo electrónico |
| `tipo` | `std::string` | "Estudiante", "Conductor" o "Administrador" |

### Métodos Virtuales Puros

- `virtual std::string getEncabezado() const = 0` — Texto de bienvenida personalizado.
- `virtual bool validarCodigo(int codigo) const = 0` — Autenticación por código.

---

## 4.11 Clase `Estudiante`

### Atributos Adicionales

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `codigoEstudiantil` | `int` | Código universitario (ej: 160005557) |
| `facultad` | `std::string` | Facultad (ej: "Ingeniería") |
| `programa` | `std::string` | Programa académico (ej: "Sistemas") |
| `semestre` | `int` | Semestre actual |

### Implementaciones Polimórficas

```cpp
std::string Estudiante::getEncabezado() const {
    return "Usuario: " + getNombre() + " (Estudiante - " + programa + ")";
}

bool Estudiante::validarCodigo(int c) const {
    return c == codigoEstudiantil;
}
```

---

## 4.12 Clase `Conductor`

### Atributos Adicionales

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `codigoOperador` | `int` | Código de operador (ej: 101) |
| `experiencia` | `std::string` | Experiencia (ej: "12 años") |
| `turno` | `std::string` | Turno asignado (ej: "Mañana") |
| `busAsignado` | `int` | ID del bus que opera |

---

## 4.13 Clase `Administrador`

### Atributos Adicionales

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `codigoAdmin` | `int` | Código de administrador (ej: 1) |

Es la subclase más simple de `Usuario`.

---

## 4.14 Clase `GestorArchivos`

### Descripción General
Es la **capa de persistencia** del sistema. Se encarga de traducir entre objetos C++ y archivos JSON. Lee archivos JSON para crear objetos y escribe objetos de vuelta a JSON.

### Atributo

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `rutaData` | `std::string` | Ruta al directorio `data/` |

### Método `cargarBuses()`

- **Firma:** `std::vector<Bus> cargarBuses() const`
- **Flujo:**
  1. Abrir `buses.json` con `std::ifstream`.
  2. Si no se abre, retornar vector vacío.
  3. Parsear el JSON con `json::parse(archivo)`.
  4. Iterar sobre `j["buses"]`.
  5. Para cada bus, crear un objeto `Bus` con los datos del JSON.
  6. Configurar campos opcionales (`indiceParadaActual`, `idRutaAsignada`).
  7. Agregar al vector con `push_back`.
  8. Retornar el vector.

### Método `cargarRutas()` — Con Polimorfismo

- **Firma:** `std::vector<Ruta*> cargarRutas() const`
- **Flujo:**
  1. Abrir `rutas.json`.
  2. Cargar horarios globales por defecto.
  3. Para cada ruta en el JSON:
     a. Si `tipo == "RutaCentro"` → crear `new RutaCentro(...)`.
     b. Si no → crear `new RutaBarrio(...)`.
  4. Cargar paradas con `agregarParada()`.
  5. Cargar horarios propios o usar los globales.
  6. Agregar el puntero al vector.
- **Nota importante:** Se usa `new` para crear objetos en el **heap**, lo que permite almacenar punteros polimórficos `Ruta*` que apuntan a subclases diferentes.

### Método `cargarUsuarios()` — Con Polimorfismo

Similar a `cargarRutas()`: según el campo `tipo` del JSON, crea `Estudiante`, `Administrador` o `Conductor` con `new`.

### Método `guardarUsuarios()` — Con `static_cast`

```cpp
if (u->getTipo() == "Estudiante") {
    const Estudiante* e = static_cast<const Estudiante*>(u);
    obj["codigoEstudiantil"] = e->getCodigoEstudiantil();
    // ...
}
```

Aquí se usa `static_cast` para convertir el puntero base `Usuario*` al tipo derivado `Estudiante*` y acceder a sus atributos específicos.

---

## 4.15 Clase `SistemaTransporte`

### Descripción General
**Clase principal** que orquesta todo el sistema. Contiene todos los datos en memoria, gestiona la interfaz de usuario por consola y coordina las operaciones CRUD.

### Tabla de Atributos

| Atributo | Tipo | Significado |
|----------|------|-------------|
| `gestor` | `GestorArchivos` | Instancia del gestor de archivos |
| `buses` | `std::vector<Bus>` | Todos los buses cargados |
| `paradas` | `std::vector<Parada>` | Todas las paradas cargadas |
| `incidentes` | `std::vector<Incidente>` | Todos los incidentes cargados |
| `rutas` | `std::vector<Ruta*>` | Punteros a las rutas (polimórficos) |
| `usuarios` | `std::vector<Usuario*>` | Punteros a los usuarios (polimórficos) |
| `usuarioActual` | `Usuario*` | Puntero al usuario que inició sesión |

### Funciones Auxiliares Estáticas (fuera de la clase)

Estas funciones están definidas como `static` dentro del archivo `.cpp`, lo que las hace **privadas al archivo** (no visibles desde otros archivos):

#### `limpiarPantalla()`

Usa la Windows API para limpiar la consola sin `system("cls")`:
1. Obtiene el handle de la consola.
2. Obtiene el tamaño del buffer.
3. Llena toda la pantalla con espacios.
4. Resetea los atributos de color.
5. Mueve el cursor a la posición (0,0).

#### `setColor(WORD c)`

```cpp
static void setColor(WORD c) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
```

Cambia el color del texto en la consola usando la Windows API.

#### `leerInt()`

Lee un entero de la entrada estándar con manejo de errores:
1. Intenta leer un `int`.
2. Si falla (el usuario escribió texto), limpia el estado de `cin` y descarta la entrada.
3. Retorna -1 en caso de error.

### Método `iniciar()`

Es el método más importante. Su flujo:

1. **Carga todos los datos** desde los archivos JSON.
2. **Configura la consola:** título de ventana y codificación UTF-8.
3. **Bucle principal infinito:**
   a. Resetea `usuarioActual` a `nullptr`.
   b. Muestra el menú principal.
   c. Si elige "Salir" (4), limpia pantalla y sale.
   d. Si elige un rol (1-3), autentica al usuario.
   e. Si la autenticación falla, vuelve al menú.
   f. Si es exitosa, llama al panel correspondiente.

### Método `autenticarUsuario(int tipo)`

- **Firma:** `Usuario* SistemaTransporte::autenticarUsuario(int tipo)`
- **Flujo:**
  1. Usa un `std::map` estático con los datos de cada tipo (nombre del tipo, texto para pedir el código).
  2. Pide al usuario su código.
  3. Busca en el vector `usuarios` usando `std::find_if` con una lambda que verifica tipo y código.
  4. Si encuentra coincidencia, retorna el puntero al usuario.
  5. Si no, muestra error y retorna `nullptr`.

### Método `panelEstudiante()`

Bucle que muestra las rutas activas y permite al estudiante:
1. Seleccionar una ruta por ID.
2. Ver las paradas de la ruta con sus nombres.
3. Ver los horarios de salida.
4. Reservar un asiento en un bus disponible.

### Método `panelConductor()`

Muestra información del bus asignado, la ruta, horarios y permite:
1. **Registrar llegada a parada:** Avanza al siguiente punto de la ruta, actualiza GPS.
2. **Reportar incidente:** Crea un nuevo incidente y lo guarda.
3. **Finalizar turno:** Sale del panel.

### Método `panelAdministrador()`

Muestra estadísticas globales (cantidad de rutas, buses, incidentes abiertos) y un menú con:
1. Gestionar rutas (activar/desactivar).
2. Gestionar usuarios (ver listado).
3. Gestionar buses (cambiar capacidad).
4. Gestionar horarios (agregar horarios a rutas).
5. Gestionar incidentes (cerrar incidentes abiertos).

### Método `mostrarProximidadBus()`

Muestra para cada parada de la ruta:
- Si ya fue visitada: distancia 0 m, tiempo 0 min, hora de llegada registrada.
- Si está pendiente: distancia real en metros y tiempo estimado en minutos desde la posición actual del bus.

### Método `buscarBusPorId(int id)`

Usa `std::find_if` con una lambda para encontrar un bus por su ID en el vector `buses`.

### Método `buscarRutaPorBus(const std::string& placa)`

Busca primero el bus por placa, luego busca la ruta asignada a ese bus por su `idRutaAsignada`.

### Método `incidentesDeBus(const std::string& placa)`

Busca incidentes cuya descripción contenga la placa del bus, usando `std::string::find()`.

---

# Sección 5: Análisis Línea por Línea (Bloques Lógicos)

## 5.1 Bloque: Constantes de Coordenada.cpp

```cpp
static const double PI = 3.14159265358979323846;
static const double RADIO_TIERRA = 6371000.0; // metros
```

### Desglose:

- `static` → la variable solo es visible dentro de este archivo `.cpp`. Si otro archivo define otra variable llamada `PI`, no habrá conflicto.
- `const` → el valor no se puede cambiar después de la inicialización. Si alguien intentara escribir `PI = 5;`, el compilador daría un error.
- `double` → tipo de dato de punto flotante de 64 bits. Puede almacenar hasta 15-17 dígitos significativos. Se necesita esta precisión para coordenadas geográficas.
- `PI = 3.14159265358979323846` → el número π con 20 decimales. Se usa para convertir grados a radianes.
- `RADIO_TIERRA = 6371000.0` → el radio medio de la Tierra en metros. Se usa en la fórmula Haversine.

**¿Qué ocurre en memoria?** Estas constantes se almacenan en la sección de datos de solo lectura del ejecutable. Al ser `static` y `const`, el compilador puede incluso sustituirlas directamente por su valor en el código (optimización).

**¿Qué pasaría si no se inicializaran?** En C++, las variables de tipo `double` sin inicializar contienen **valores basura** (bits aleatorios de lo que había previamente en esa posición de memoria). Los cálculos con la fórmula Haversine producirían resultados completamente incorrectos e impredecibles.

---

## 5.2 Bloque: Fórmula Haversine (Coordenada.cpp)

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

### Desglose línea por línea:

**Línea 1: `double dLat = (otra.latitud - latitud) * PI / 180.0;`**
- `otra.latitud - latitud` → diferencia de latitud en **grados**.
- `* PI / 180.0` → convierte grados a **radianes** (fórmula: radianes = grados × π / 180).
- `dLat` es de tipo `double` porque las coordenadas GPS tienen decimales.

**Línea 2:** Igual pero para la longitud.

**Línea 3-5: Cálculo de `a`**
- `std::sin(dLat / 2) * std::sin(dLat / 2)` → seno al cuadrado de la mitad de la diferencia de latitud.
- `std::cos(latitud * PI / 180.0)` → coseno de la latitud del punto actual (convertida a radianes).
- `std::cos(otra.latitud * PI / 180.0)` → coseno de la latitud del otro punto.
- El resultado `a` es un valor entre 0 y 1 que representa la relación entre el arco y la cuerda entre los dos puntos.

**Línea 6: `double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));`**
- `std::sqrt(a)` → raíz cuadrada de `a`.
- `std::sqrt(1.0 - a)` → raíz cuadrada del complemento.
- `std::atan2(y, x)` → arcotangente de dos argumentos (más precisa que `atan` para ángulos en cualquier cuadrante).
- `c` es el **arco central** en radianes.

**Línea 7: `return RADIO_TIERRA * c;`**
- Multiplica el arco por el radio de la Tierra para obtener la distancia real en metros.

---

## 5.3 Bloque: Constructor del Bus

```cpp
Bus::Bus() : idBus(0), capacidadMaxima(0), capacidadActual(0), 
             estado(false), ubicacion(nullptr), indiceParadaActual(-1), idRutaAsignada(0) {}
```

### Desglose:

- `: idBus(0), ...` → **lista de inicialización**. Es la forma preferida de inicializar atributos en C++.
- `ubicacion(nullptr)` → el puntero se inicializa a `nullptr` (nulo, no apunta a nada). Esto es CRÍTICO porque si se dejara sin inicializar, el puntero contendría una dirección de memoria aleatoria, y cualquier intento de acceder a `ubicacion->algo()` provocaría un **crash** del programa (acceso a memoria no válida, "segmentation fault").
- `indiceParadaActual(-1)` → -1 indica "ruta no iniciada". No es un índice válido de un vector, lo cual sirve como centinela.

**¿Por qué lista de inicialización en lugar de asignación en el cuerpo?**
```cpp
// Malo (asignación):
Bus::Bus() {
    idBus = 0;       // El atributo se crea con basura, LUEGO se asigna 0
}

// Bueno (inicialización):
Bus::Bus() : idBus(0) {}  // El atributo se CREA directamente con 0
```

Para tipos primitivos como `int` la diferencia es mínima, pero para objetos como `std::string` es significativa: la lista de inicialización evita crear un objeto vacío y luego sobrescribirlo.

---

## 5.4 Bloque: limpiarPantalla() (SistemaTransporte.cpp)

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

### Desglose:

- `COORD coord = {0, 0}` → estructura de Windows que representa una posición en la consola (columna 0, fila 0 = esquina superior izquierda).
- `DWORD celdas, escritas` → `DWORD` es un alias de Windows para `unsigned long` (32 bits sin signo). `celdas` almacenará el número total de caracteres de la pantalla; `escritas` almacenará cuántos caracteres se llenaron.
- `HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE)` → obtiene un "mango" (identificador) de la salida de consola.
- `GetConsoleScreenBufferInfo(h, &info)` → obtiene información del buffer de pantalla (tamaño, posición del cursor, atributos).
- `celdas = info.dwSize.X * info.dwSize.Y` → el número total de celdas es ancho × alto.
- `FillConsoleOutputCharacter(h, ' ', celdas, coord, &escritas)` → llena TODA la pantalla con espacios (' '), empezando desde (0,0).
- `FillConsoleOutputAttribute(h, info.wAttributes, celdas, coord, &escritas)` → restaura los atributos de color a los predeterminados.
- `SetConsoleCursorPosition(h, coord)` → mueve el cursor a (0,0).

**¿Por qué no usar `system("cls")`?** Aunque es más simple, `system("cls")` crea un proceso hijo, lo cual es más lento y potencialmente inseguro. La implementación con la Windows API es más eficiente.

---

## 5.5 Bloque: setColor y sus llamadas

```cpp
static void setColor(WORD c) { SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c); }
```

Y sus llamadas típicas:

```cpp
setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
```

### Desglose:

- `WORD` → tipo de Windows, entero de 16 bits sin signo.
- `FOREGROUND_RED`, `FOREGROUND_GREEN`, `FOREGROUND_BLUE`, `FOREGROUND_INTENSITY` → constantes que representan bits de color (ver Sección 9 para desglose binario completo).
- `|` → operador OR bit-a-bit que **combina** los flags de color.

---

## 5.6 Bloque: leerInt() — Lectura Segura de Enteros

```cpp
static int leerInt() {
    int v;
    if (!(std::cin >> v)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        return -1;
    }
    std::cin.ignore(10000, '\n');
    return v;
}
```

### Desglose:

- `int v;` → variable local no inicializada (se inicializará por `cin >> v`).
- `std::cin >> v` → intenta leer un entero del teclado. Si el usuario escribe "hola" en lugar de un número, la lectura **falla** y `cin` entra en estado de error.
- `!(std::cin >> v)` → el operador `!` verifica si la lectura falló.
- `std::cin.clear()` → **resetea** el estado de error de `cin`. Sin esto, todas las lecturas futuras fallarían.
- `std::cin.ignore(10000, '\n')` → **descarta** hasta 10000 caracteres o hasta encontrar un salto de línea. Esto limpia los caracteres residuales en el buffer de entrada.
- `return -1` → valor centinela que indica "entrada inválida".
- El segundo `std::cin.ignore(10000, '\n')` (cuando la lectura es exitosa) descarta el '\n' que queda después del número leído.

**¿Qué pasaría sin este manejo de errores?** Si el usuario escribe texto cuando se espera un número y no se limpia el error, el programa entraría en un **bucle infinito** porque `cin` seguiría en estado de error e ignoraría todas las lecturas futuras.

---

## 5.7 Bloque: Autenticación con Lambda y std::find_if

```cpp
auto it = std::find_if(usuarios.begin(), usuarios.end(), [tipo, codigo](Usuario* u) {
    return u->getTipo() == datos.at(tipo).first && u->validarCodigo(codigo);
});
```

### Desglose:

- `std::find_if` → algoritmo de la STL que busca el primer elemento que cumple una condición.
- `usuarios.begin(), usuarios.end()` → rango de búsqueda (todo el vector).
- `[tipo, codigo]` → **lista de capturas** de la lambda: captura las variables `tipo` y `codigo` por valor (copia).
- `(Usuario* u)` → parámetro de la lambda: cada elemento del vector.
- `u->getTipo() == datos.at(tipo).first` → verifica que el tipo del usuario coincida.
- `u->validarCodigo(codigo)` → llama al método virtual puro, que se despacha polimórficamente al método correcto de la subclase.
- `auto it` → el tipo del iterador se deduce automáticamente (sería `std::vector<Usuario*>::iterator`).

---

## 5.8 Bloques Repetitivos: Getters y Setters Triviales

Los siguientes patrones se repiten en todas las clases y siguen el modelo explicado en la Sección 4.0:

```cpp
// Patrón getter para tipo primitivo
int Clase::getAtributo() const { return atributo; }

// Patrón getter para string
std::string Clase::getAtributo() const { return atributo; }

// Patrón setter para tipo primitivo
void Clase::setAtributo(int val) { atributo = val; }

// Patrón setter para string (por referencia constante)
void Clase::setAtributo(const std::string& val) { atributo = val; }
```

Las clases que siguen este patrón exacto son: `Coordenada`, `UbicacionGPS`, `HorarioRuta`, `Parada`, `Incidente`, `Ruta`, `Bus`, `Usuario`, `Estudiante`, `Conductor`, `Administrador`.

---

# Sección 6: Guía de Sintaxis del Lenguaje (C++)

## 6.1 Estructuras de Control

### `if / else`

**¿Qué es?** Ejecuta código condicionalmente. Si la condición es verdadera, ejecuta un bloque; si es falsa, ejecuta otro (opcionalmente).

**Sintaxis:**
```cpp
if (condicion) {
    // código si verdadero
} else if (otra_condicion) {
    // código si la segunda es verdadera
} else {
    // código si ninguna es verdadera
}
```

**Ejemplos del proyecto:**
```cpp
if (op == 1) panelEstudiante();
else if (op == 2) panelConductor();
else panelAdministrador();
```

### `for`

**¿Qué es?** Ejecuta un bloque de código un número determinado de veces, o itera sobre los elementos de una colección.

**Sintaxis clásica:**
```cpp
for (inicialización; condición; incremento) {
    // cuerpo
}
```

**Range-based for (C++11):**
```cpp
for (tipo elemento : contenedor) {
    // usar elemento
}
```

**Ejemplos del proyecto:**
```cpp
// Clásico con índice
for (size_t i = 0; i < salidasBarrio.size(); ++i) {
    resultado += salidasBarrio[i];
}

// Range-based con referencia
for (Ruta* r : rutas) {
    std::cout << r->getNombre();
}

// Range-based con auto y referencia
for (const auto& b : j["buses"]) {
    // 'b' es una referencia constante a cada elemento JSON
}
```

### `while`

**¿Qué es?** Ejecuta un bloque mientras la condición sea verdadera. Si la condición es falsa al inicio, no se ejecuta nunca.

**Ejemplo del proyecto:**
```cpp
while (true) {  // Bucle infinito, se sale con 'break'
    int op = mostrarMenuPrincipal();
    if (op == 4) break;
    // ...
}
```

### `switch`

**¿Qué es?** Selecciona entre múltiples opciones basándose en el valor de una expresión entera. No se usa explícitamente en este proyecto (se usan cadenas de `if-else`), pero es un constructo importante del lenguaje.

**Sintaxis:**
```cpp
switch (variable) {
    case 1: /* código */ break;
    case 2: /* código */ break;
    default: /* código por defecto */
}
```

---

## 6.2 Clases y Acceso

### `class`

**¿Qué es?** Define un nuevo tipo de dato que agrupa atributos (datos) y métodos (funciones) relacionados. Por defecto, todo es `private`.

**Sintaxis:**
```cpp
class NombreClase {
private:
    int atributo;           // Solo accesible dentro de la clase
public:
    void metodo();          // Accesible desde cualquier lugar
protected:
    int protegido;          // Accesible en esta clase y sus hijas
};
```

### `struct`

**¿Qué es?** Idéntico a `class` pero por defecto todo es `public`. No se usa en este proyecto, pero la estructura `COORD` de Windows es un ejemplo.

### `public` / `private` / `protected`

| Modificador | Accesible desde la propia clase | Accesible desde subclases | Accesible desde afuera |
|-------------|:------:|:------:|:------:|
| `private` | ✅ | ❌ | ❌ |
| `protected` | ✅ | ✅ | ❌ |
| `public` | ✅ | ✅ | ✅ |

**En el proyecto:** Todos los atributos son `private` y los métodos son `public`, siguiendo el principio de encapsulamiento.

---

## 6.3 Modificadores y Palabras Clave

### `const`

**¿Qué es?** Indica que un valor NO puede ser modificado después de su inicialización.

**Usos en el proyecto:**

1. **Variables constantes:**
   ```cpp
   static const double PI = 3.14159265358979323846;
   ```

2. **Métodos constantes:**
   ```cpp
   double getLatitud() const;  // Este método NO modifica el objeto
   ```

3. **Parámetros por referencia constante:**
   ```cpp
   void setNombre(const std::string& nom);  // 'nom' no se modifica
   ```

### `static`

**¿Qué es?** Tiene dos significados según el contexto:

1. **Variables/funciones estáticas en un archivo:** Solo visibles dentro de ese archivo.
   ```cpp
   static const int ANCHO = 80;  // Solo visible en SistemaTransporte.cpp
   static void setColor(WORD c); // Solo invocable desde SistemaTransporte.cpp
   ```

2. **Variables locales estáticas:** Mantienen su valor entre llamadas.
   ```cpp
   static const std::map<int, std::pair<std::string, std::string>> datos = { ... };
   // Se inicializa solo la primera vez
   ```

### `virtual`

**¿Qué es?** Indica que un método puede ser **sobrescrito** (overridden) por una subclase, y que la versión correcta se elegirá en **tiempo de ejecución** según el tipo real del objeto.

**Ejemplo del proyecto:**
```cpp
class Usuario {
    virtual std::string getEncabezado() const = 0;  // Virtual puro
    virtual ~Usuario() {}                            // Destructor virtual
};
```

**¿Por qué virtual?** Sin `virtual`, si tenemos un `Usuario* u = new Estudiante(...)` y llamamos a `u->getEncabezado()`, se llamaría al método de `Usuario` (que no existe porque es puro). Con `virtual`, se llama al método de `Estudiante`.

**¿Por qué destructor virtual?** Si hacemos `delete u` donde `u` es de tipo `Usuario*` pero apunta a un `Estudiante`, sin destructor virtual solo se llamaría al destructor de `Usuario`, sin liberar los recursos de `Estudiante`. Con destructor virtual, se llama al destructor correcto.

### `override`

**¿Qué es?** Indica explícitamente que un método está sobrescribiendo uno virtual de la clase base. El compilador verifica que efectivamente existe un método virtual con esa firma en la clase padre.

```cpp
std::string getEncabezado() const override;  // Sobrescribe el de Usuario
```

**Ventaja:** Si cometes un error en la firma (nombre diferente, tipos diferentes), el compilador te lo dirá en lugar de crear un método nuevo silenciosamente.

### `auto`

**¿Qué es?** Le pide al compilador que **deduzca el tipo** automáticamente basándose en el valor asignado.

```cpp
auto it = std::find_if(buses.begin(), buses.end(), [id](const Bus& b) { ... });
// 'it' es de tipo std::vector<Bus>::iterator, pero no hace falta escribirlo
```

### `explicit`

**¿Qué es?** Previene conversiones implícitas de tipo en constructores. Usado en `GestorArchivos`:

```cpp
explicit GestorArchivos(const std::string& dirData);
```

Sin `explicit`, podría ocurrir una conversión accidental: `GestorArchivos g = "data";` (pasa un string al constructor sin querer). Con `explicit`, debes escribir explícitamente: `GestorArchivos g("data");`.

---

## 6.4 Referencias y Punteros

### Referencias (`&`)

**¿Qué es?** Un **alias** (otro nombre) para una variable existente. Una vez inicializada, siempre se refiere al mismo objeto.

```cpp
void setNombre(const std::string& nom);  // 'nom' es una referencia a un string existente
```

**¿Por qué usar referencias?**
- Evita copiar objetos grandes. Sin `&`, cada vez que llamas a `setNombre("Carlos")`, se copiaría todo el string. Con `&`, solo se pasa la dirección de memoria (mucho más rápido).
- `const &` garantiza además que no se modificará el objeto original.

### Punteros (`*`)

**¿Qué es?** Una variable que almacena la **dirección de memoria** de otro objeto. A diferencia de las referencias, puede ser `nullptr` (no apuntar a nada) y puede cambiar a lo que apunta.

```cpp
UbicacionGPS* ubicacion;  // 'ubicacion' almacena la dirección de un UbicacionGPS
```

**Operaciones con punteros:**
- `*ptr` → **desreferencia**: accede al objeto al que apunta.
- `ptr->metodo()` → accede a un método del objeto apuntado (equivalente a `(*ptr).metodo()`).
- `new Tipo()` → reserva memoria en el heap y retorna un puntero.
- `delete ptr` → libera la memoria apuntada.
- `nullptr` → valor especial que indica "no apunta a nada".

**En el proyecto, los punteros se usan para:**
1. **Polimorfismo:** `std::vector<Ruta*>` almacena punteros a `RutaBarrio` o `RutaCentro`.
2. **Propiedad opcional:** `UbicacionGPS* ubicacion` puede ser `nullptr` si el bus no tiene GPS.
3. **Sesión:** `Usuario* usuarioActual` puede ser `nullptr` si nadie ha iniciado sesión.

---

## 6.5 Lambdas

**¿Qué es?** Una **función anónima** (sin nombre) que se define en el lugar donde se usa. Introducidas en C++11.

**Sintaxis:**
```
[capturas](parámetros) -> tipo_retorno { cuerpo }
```

**Ejemplos del proyecto:**

```cpp
// Lambda que captura 'id' por valor
[id](const Bus& b) { return b.getIdBus() == id; }

// Lambda que captura 'placa' por referencia
[&placa](const Bus& b) { return b.getPlaca() == placa; }

// Lambda sin capturas
[](const Bus& b) { return b.getCapacidadActual() < b.getCapacidadMaxima() && b.getEstado(); }

// Lambda que captura múltiples variables
[tipo, codigo](Usuario* u) { return u->getTipo() == datos.at(tipo).first && u->validarCodigo(codigo); }
```

**Lista de capturas:**
- `[x]` → captura `x` **por valor** (copia).
- `[&x]` → captura `x` **por referencia** (alias).
- `[=]` → captura TODAS las variables del ámbito por valor.
- `[&]` → captura TODAS por referencia.
- `[]` → no captura nada.

---

## 6.6 Operadores

### Operadores de Comparación

| Operador | Significado | Ejemplo del proyecto |
|----------|-------------|----------------------|
| `==` | Igual a | `u->getTipo() == "Estudiante"` |
| `!=` | Diferente de | `it != buses.end()` |
| `<` | Menor que | `ordenEnRuta < otra.ordenEnRuta` |
| `>` | Mayor que | Usado en `operator<` de Bus (invertido) |
| `<=` | Menor o igual | `velocidadKmh <= 0.0` |
| `>=` | Mayor o igual | `cap >= 0` |

### Operadores Lógicos

| Operador | Significado | Ejemplo |
|----------|-------------|---------|
| `&&` | AND lógico | `b.getEstado() && b.getCapacidadActual() < b.getCapacidadMaxima()` |
| `\|\|` | OR lógico | No usado explícitamente |
| `!` | NOT lógico | `!ubicacion` (true si es nullptr) |

### Operadores Bit-a-bit

| Operador | Significado | Ejemplo |
|----------|-------------|---------|
| `\|` | OR bit-a-bit | `FOREGROUND_RED \| FOREGROUND_GREEN \| FOREGROUND_BLUE` |
| `&` | AND bit-a-bit | `conf.contains("salidasBarrio") && conf["salidasBarrio"].is_array()` (nota: `&&` es lógico, `&` sería bit-a-bit) |

### Operador de Flecha (`->`)

```cpp
ruta->getNombre()  // Accede a getNombre() del objeto al que apunta 'ruta'
```
Equivale a `(*ruta).getNombre()`.

---

## 6.7 Casting (Conversiones de Tipo)

### `static_cast`

**¿Qué es?** Realiza una conversión de tipo en **tiempo de compilación**. Se usa cuando estás seguro del tipo real del objeto.

**Ejemplo del proyecto:**
```cpp
const Estudiante* e = static_cast<const Estudiante*>(u);
```

Aquí `u` es de tipo `const Usuario*`, pero sabemos (por haber verificado `u->getTipo() == "Estudiante"`) que realmente apunta a un `Estudiante`. El `static_cast` nos permite acceder a los métodos específicos de `Estudiante`.

**Ventaja sobre C-style cast:** Es explícito y más seguro. El compilador verifica que la conversión tiene sentido (por ejemplo, que `Estudiante` hereda de `Usuario`).

**Riesgo:** Si te equivocas y el objeto no es realmente un `Estudiante`, el comportamiento es **indefinido** (el programa podría crashear o dar resultados incorrectos).

### `dynamic_cast`

No se usa en este proyecto, pero es la alternativa segura a `static_cast` para jerarquías polimórficas. Verifica el tipo en **tiempo de ejecución** y retorna `nullptr` si la conversión no es válida.

---

## 6.8 Directivas del Preprocesador

### `#pragma once`

**¿Qué es?** Una directiva que garantiza que un archivo de cabecera se incluya **solo una vez** durante la compilación, evitando errores de definición múltiple.

**Alternativa clásica (include guards):**
```cpp
#ifndef COORDENADA_H
#define COORDENADA_H
// ... contenido del header ...
#endif
```

`#pragma once` es más limpio y es soportado por todos los compiladores modernos.

### `#include`

**¿Qué es?** Copia el contenido de otro archivo en el punto donde aparece la directiva.

**Dos formas:**
- `#include <archivo>` → busca en los directorios del sistema (librerías estándar).
- `#include "archivo"` → busca primero en el directorio del proyecto, luego en los del sistema.

---

## 6.9 Namespace

**¿Qué es?** Un espacio de nombres que agrupa identificadores para evitar conflictos.

```cpp
using json = nlohmann::json;  // Alias: 'json' ahora significa 'nlohmann::json'
```

`std::` es el namespace de la biblioteca estándar de C++. Todos los elementos como `cout`, `string`, `vector`, etc., pertenecen a `std`.

---

# Sección 7: Guía de la STL y Librerías Estándar

## 7.1 `std::vector` — Array Dinámico

**Librería:** `<vector>`

**¿Qué es?** Un contenedor que almacena elementos del mismo tipo en memoria contigua, con tamaño dinámico (crece automáticamente según se necesite).

**¿Cómo funciona internamente?** Mantiene un array interno con una capacidad reservada. Cuando se agota, reserva un array nuevo (generalmente el doble de tamaño), copia los elementos y libera el anterior.

**Complejidad:**
| Operación | Complejidad |
|-----------|-------------|
| Acceso por índice `v[i]` | O(1) |
| `push_back()` | O(1) amortizado |
| `emplace_back()` | O(1) amortizado |
| `size()` | O(1) |
| `begin()`, `end()` | O(1) |
| Búsqueda lineal | O(n) |

**Usos en el proyecto:**
```cpp
std::vector<Bus> buses;              // Vector de objetos Bus
std::vector<Ruta*> rutas;            // Vector de punteros a Ruta
std::vector<int> idsParadas;         // Vector de enteros
std::vector<std::string> salidasBarrio; // Vector de strings
```

**Métodos usados:**
- `push_back(elem)` → agrega `elem` al final (copia).
- `emplace_back(args...)` → construye el elemento directamente al final (más eficiente).
- `size()` → retorna el número de elementos.
- `empty()` → retorna `true` si no tiene elementos.
- `begin()` / `end()` → iteradores al inicio y al final (para algoritmos y range-for).
- `operator[]` → acceso por índice sin verificación de límites.

---

## 7.2 `std::map` — Diccionario Ordenado

**Librería:** `<map>`

**¿Qué es?** Un contenedor asociativo que almacena pares clave-valor, ordenados por clave. Implementado internamente como un **árbol rojo-negro** (un tipo de árbol binario balanceado).

**Complejidad:**
| Operación | Complejidad |
|-----------|-------------|
| Inserción | O(log n) |
| Búsqueda | O(log n) |
| Eliminación | O(log n) |

**Usos en el proyecto:**

```cpp
// Mapa de opciones de autenticación
static const std::map<int, std::pair<std::string, std::string>> datos = {
    {1, {"Estudiante", "codigo estudiantil"}},
    {2, {"Conductor", "codigo de operador"}},
    {3, {"Administrador", "codigo de administrador"}}
};

// Mapa de horas de llegada por ID de parada
std::map<int, std::string> horasLlegada;  // idParada -> hora
```

**Métodos usados:**
- `at(key)` → retorna el valor asociado a la clave (lanza excepción si no existe).
- `find(key)` → retorna un iterador al elemento (o `end()` si no existe).
- `operator[]` → acceso/inserción por clave.
- `clear()` → elimina todos los elementos.

---

## 7.3 `std::string` — Cadenas de Texto

**Librería:** `<string>`

**¿Qué es?** Clase que encapsula un array de caracteres con manejo automático de memoria.

**Métodos usados en el proyecto:**
- `+` → concatenación: `"Hola " + nombre`.
- `empty()` → verifica si está vacía.
- `find(substr)` → busca una subcadena; retorna `std::string::npos` si no la encuentra.
- `size()` → longitud de la cadena.

**Ejemplo:**
```cpp
if (inc.getDescripcion().find(placa) != std::string::npos)
    // La descripción del incidente contiene la placa del bus
```

---

## 7.4 `std::find_if` — Búsqueda con Predicado

**Librería:** `<algorithm>`

**¿Qué es?** Algoritmo que busca el primer elemento en un rango que cumple una condición (predicado).

**Firma:**
```cpp
template<class InputIt, class UnaryPred>
InputIt find_if(InputIt first, InputIt last, UnaryPred pred);
```

**Complejidad:** O(n) — examina cada elemento hasta encontrar uno que cumpla o llegar al final.

**Usos en el proyecto (MUY frecuente):**
```cpp
// Buscar bus por ID
auto it = std::find_if(buses.begin(), buses.end(), 
    [id](const Bus& b) { return b.getIdBus() == id; });

// Buscar ruta por ID
auto itRuta = std::find_if(rutas.begin(), rutas.end(), 
    [idRuta](Ruta* r) { return r->getIdRuta() == idRuta; });

// Buscar parada por ID
auto it = std::find_if(paradas.begin(), paradas.end(), 
    [idP](const Parada& p) { return p.getIdParada() == idP; });
```

**Retorno:** Un **iterador** al elemento encontrado, o `end()` si no se encontró. Siempre se debe verificar:
```cpp
if (it != buses.end()) {
    // Encontrado: usar *it para acceder al elemento
}
```

---

## 7.5 `std::any_of` — ¿Algún Elemento Cumple?

**Librería:** `<algorithm>`

**¿Qué es?** Retorna `true` si al menos un elemento en el rango cumple la condición.

**Uso en el proyecto:**
```cpp
bool abierto = std::any_of(inc.begin(), inc.end(), 
    [](Incidente* i) { return i->getEstado() == "Abierto"; });
```

**Complejidad:** O(n) en el peor caso (si ninguno cumple). Puede terminar antes si encuentra uno.

---

## 7.6 `std::count_if` — Contar Elementos que Cumplen

**Librería:** `<algorithm>`

**Uso en el proyecto:**
```cpp
int abiertos = std::count_if(incidentes.begin(), incidentes.end(), 
    [](const Incidente& i) { return i.getEstado() == "Abierto"; });
```

**Complejidad:** O(n) — siempre recorre todos los elementos.

---

## 7.7 `std::distance` — Distancia Entre Iteradores

**Librería:** `<iterator>`

**Uso en el proyecto:**
```cpp
nextIndex = std::distance(rutas.begin(), itRuta) + 1;
```

Calcula cuántas posiciones hay entre dos iteradores. Equivale al "índice" del elemento en el vector.

---

## 7.8 Manipuladores de Salida

### `std::fixed` y `std::setprecision`

**Librería:** `<iomanip>`

```cpp
std::cout << std::fixed << std::setprecision(0) << metros << " m";
std::cout << std::setprecision(1) << minutos << " min";
```

- `std::fixed` → usa formato de punto fijo (no notación científica).
- `std::setprecision(n)` → muestra exactamente `n` decimales.

### `std::setw`

**Librería:** `<iomanip>`

```cpp
std::cout << std::setw(2) << std::setfill('0') << st.wHour;
```

- `std::setw(2)` → la siguiente salida ocupará al menos 2 caracteres de ancho.
- `std::setfill('0')` → rellena con '0' en lugar de espacios (para que 9 se muestre como "09").

### `std::setfill`

Establece el carácter de relleno para `setw`. Persiste hasta que se cambie.

---

## 7.9 `std::ostringstream` — Stream en Memoria

**Librería:** `<sstream>`

**¿Qué es?** Un stream que escribe a una cadena en memoria en lugar de a la consola.

```cpp
std::ostringstream ss;
ss << std::setw(2) << std::setfill('0') << st.wHour << ':'
   << std::setw(2) << std::setfill('0') << st.wMinute;
return ss.str();  // Retorna el string construido
```

**Ventaja:** Permite construir strings complejos con formato, usando la misma sintaxis de `cout`.

---

## 7.10 `std::ifstream` y `std::ofstream` — Archivos

**Librería:** `<fstream>`

```cpp
std::ifstream archivo(rutaData + "/buses.json");  // Abre para lectura
std::ofstream archivo(rutaData + "/buses.json");  // Abre para escritura

if (!archivo.is_open()) return;  // Verifica que se abrió correctamente
```

- `ifstream` = **i**nput **f**ile **stream** (lectura).
- `ofstream` = **o**utput **f**ile **stream** (escritura).
- Se cierran automáticamente cuando salen del ámbito (RAII).

---

# Sección 8: APIs Externas y Dependencias

## 8.1 CMake — Sistema de Compilación

CMake es un **meta-sistema de compilación**: no compila directamente, sino que genera archivos de proyecto para el compilador nativo (Visual Studio, Make, Ninja, etc.).

### Análisis línea por línea de CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
```
Exige CMake versión 3.16 o superior. Si tienes una versión inferior, CMake se detendrá con error.

```cmake
project(SistemaTransporte CXX)
```
Define el nombre del proyecto como "SistemaTransporte" y el lenguaje como C++ (`CXX`).

```cmake
set(CMAKE_CXX_STANDARD 17)
```
Usa el estándar C++17, que habilita features como `auto`, `if constexpr`, `std::optional`, etc.

```cmake
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```
Si el compilador no soporta C++17, la configuración falla en lugar de "degradarse" silenciosamente.

```cmake
include_directories(include)
```
Agrega `include/` al path de búsqueda de headers. Así, `#include "Bus.h"` encontrará `include/Bus.h`.

```cmake
set(SOURCES main.cpp src/Coordenada.cpp ...)
```
Define la variable `SOURCES` con la lista de todos los `.cpp` a compilar.

```cmake
add_executable(SistemaTransporte ${SOURCES})
```
Crea un ejecutable llamado "SistemaTransporte" compilando todos los archivos listados.

```cmake
add_custom_command(TARGET SistemaTransporte POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/data"
    "$<TARGET_FILE_DIR:SistemaTransporte>/data"
)
```
Después de compilar (`POST_BUILD`), copia automáticamente la carpeta `data/` junto al ejecutable generado. Esto es necesario porque el programa busca los JSON en el directorio `data/` relativo al ejecutable.

---

## 8.2 nlohmann::json — Librería JSON

**¿Qué es?** Una librería de C++ de encabezado único (**header-only**) que permite trabajar con datos JSON de forma intuitiva.

**Archivo:** `include/json.hpp`

### Funciones Usadas en el Proyecto

#### `json::parse(stream)`

```cpp
json j = json::parse(archivo);
```

Lee todo el contenido del stream (archivo) y lo convierte en un objeto JSON en memoria. Si el contenido no es JSON válido, lanza una excepción.

#### `operator[]`

```cpp
j["buses"]           // Accede al array "buses" del objeto JSON
b["idBus"]           // Accede al campo "idBus" de un objeto bus
p["ubicacion"]["latitud"]  // Acceso anidado
```

Funciona igual que un mapa: accede a un valor por su clave. Si la clave no existe en modo lectura, lanza una excepción.

#### `get<type>()`

```cpp
b["idBus"].get<int>()          // Convierte el valor JSON a int
b["placa"].get<std::string>()  // Convierte a string
b["estado"].get<bool>()        // Convierte a bool
p["ubicacion"]["latitud"].get<double>()  // Convierte a double
```

Convierte un valor JSON al tipo C++ especificado. Si el tipo no coincide (ej: intentas `get<int>()` de un string), lanza una excepción.

#### `value(key, default)`

```cpp
b.value("indiceParadaActual", -1)  // Retorna el valor si existe, sino -1
r.value("zonaCentro", "")          // Retorna "" si no existe
```

Versión segura de `operator[]`: si la clave no existe, retorna el valor por defecto en lugar de lanzar una excepción.

#### `contains(key)`

```cpp
if (j.contains("configuracionHorarios")) { ... }
```

Verifica si una clave existe en el objeto JSON. Retorna `true` o `false`.

#### `is_object()`, `is_array()`, `is_null()`

```cpp
if (r["horarios"].is_object() && !r["horarios"].empty()) { ... }
if (r["puntoSalida"].is_null()) { ... }
```

Verifican el tipo del valor JSON.

#### `dump(indent)`

```cpp
archivo << j.dump(4);  // Escribe JSON con indentación de 4 espacios
archivo << j.dump(2);  // Con indentación de 2 espacios
```

Serializa el objeto JSON a string. El parámetro indica la indentación para formato legible.

#### `json::array()`

```cpp
j["incidentes"] = json::array();
```

Crea un array JSON vacío.

#### `push_back(object)`

```cpp
j["incidentes"].push_back({
    {"descripcion", inc.getDescripcion()},
    {"estado",      inc.getEstado()},
    {"idIncidente", inc.getIdIncidente()},
    {"tipo",        inc.getTipo()}
});
```

Agrega un elemento al array JSON. La sintaxis `{ {clave, valor}, ... }` crea un objeto JSON inline.

---

## 8.3 Windows API (`<windows.h>`)

### `SetConsoleTextAttribute(handle, attributes)`

Cambia el color del texto en la consola.

```cpp
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
```

### `GetStdHandle(STD_OUTPUT_HANDLE)`

Obtiene el **handle** (identificador) de la salida estándar de la consola. Es como un "ticket" que identifica la consola para funciones posteriores.

### `SetConsoleOutputCP(65001)` y `SetConsoleCP(65001)`

Configura la consola para usar la **página de códigos 65001** (UTF-8), lo que permite mostrar caracteres especiales como tildes (á, é, í) y ñ.

### `SetConsoleTitle("texto")`

Establece el texto de la barra de título de la ventana de consola.

### `GetLocalTime(&SYSTEMTIME)`

Obtiene la hora y fecha actual del sistema operativo. `SYSTEMTIME` es una estructura con campos: `wYear`, `wMonth`, `wDay`, `wHour`, `wMinute`, `wSecond`.

### `Sleep(milisegundos)`

Pausa la ejecución del programa durante el número de milisegundos indicado.

```cpp
Sleep(2000);  // Pausa 2 segundos (para que el usuario lea un mensaje)
```

### `FillConsoleOutputCharacter`, `FillConsoleOutputAttribute`, `SetConsoleCursorPosition`

Funciones de bajo nivel usadas en `limpiarPantalla()` para limpiar la consola sin `system("cls")`.

---

## 8.4 Constantes de Color de Consola

Las constantes de color de la Windows API usan **bits individuales** para representar cada componente:

```
Bit:  3         2         1         0
      |         |         |         |
      INTENSITY BLUE      GREEN     RED
```

| Constante | Valor Decimal | Valor Binario | Color |
|-----------|:---:|:---:|-------|
| `FOREGROUND_RED` | 4 | `0100` | Rojo |
| `FOREGROUND_GREEN` | 2 | `0010` | Verde |
| `FOREGROUND_BLUE` | 1 | `0001` | Azul |
| `FOREGROUND_INTENSITY` | 8 | `1000` | Brillo alto |

Al combinarlos con `|` (OR bit-a-bit):

| Expresión | Binario | Color Resultante |
|-----------|:---:|-------|
| `FOREGROUND_RED \| FOREGROUND_GREEN \| FOREGROUND_BLUE` | `0111` | Blanco (gris claro) |
| `FOREGROUND_GREEN \| FOREGROUND_INTENSITY` | `1010` | Verde brillante |
| `FOREGROUND_RED \| FOREGROUND_INTENSITY` | `1100` | Rojo brillante |
| `FOREGROUND_BLUE \| FOREGROUND_GREEN \| FOREGROUND_INTENSITY` | `1011` | Cian brillante |

---

# Sección 9: Desglose de Instrucciones Complejas

## 9.1 Llamadas con Operadores Bit-a-bit (setColor)

```cpp
setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
```

### Desglose paso a paso:

1. `FOREGROUND_GREEN` = 2 = binario `0010`
2. `FOREGROUND_INTENSITY` = 8 = binario `1000`
3. Operación OR bit-a-bit (`|`):

```
    0010    (FOREGROUND_GREEN = 2)
  | 1000    (FOREGROUND_INTENSITY = 8)
  ------
    1010    (resultado = 10)
```

4. El resultado `1010` (decimal 10) se pasa a `SetConsoleTextAttribute`.
5. La consola interpreta: bit 1 (GREEN) encendido + bit 3 (INTENSITY) encendido = **verde brillante**.

### Otro ejemplo:

```cpp
setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
```

```
    0100    (RED = 4)
  | 0010    (GREEN = 2)
  | 0001    (BLUE = 1)
  ------
    0111    (resultado = 7)
```

Resultado: RGB completo sin intensidad = **blanco/gris claro** (color por defecto de la consola).

### Ejemplo con ternario:

```cpp
setColor(abierto ? FOREGROUND_RED | FOREGROUND_INTENSITY : FOREGROUND_GREEN | FOREGROUND_INTENSITY);
```

Si `abierto` es `true`:
- `FOREGROUND_RED | FOREGROUND_INTENSITY` = `0100 | 1000` = `1100` = **rojo brillante**.

Si `abierto` es `false`:
- `FOREGROUND_GREEN | FOREGROUND_INTENSITY` = `0010 | 1000` = `1010` = **verde brillante**.

---

## 9.2 Fórmula Haversine Desglosada

La fórmula Haversine calcula la distancia más corta entre dos puntos sobre una esfera (la Tierra). Es más precisa que una simple diferencia de coordenadas porque tiene en cuenta la curvatura terrestre.

### Fórmula matemática:

```
a = sin²(Δlat/2) + cos(lat₁) × cos(lat₂) × sin²(Δlon/2)
c = 2 × atan2(√a, √(1-a))
d = R × c
```

Donde:
- `Δlat` = diferencia de latitudes en radianes
- `Δlon` = diferencia de longitudes en radianes
- `R` = radio de la Tierra (6,371,000 m)
- `d` = distancia en metros

### Desglose con valores reales:

Supongamos dos paradas: (4.14527, -73.652037) y (4.14335, -73.650037)

**Paso 1: Convertir diferencias a radianes**
```
dLat = (4.14335 - 4.14527) × π / 180 = -0.00192 × 0.01745 = -0.0000335 rad
dLon = (-73.650037 - (-73.652037)) × π / 180 = 0.002 × 0.01745 = 0.0000349 rad
```

**Paso 2: Calcular 'a'**
```
sin(dLat/2)² = sin(-0.00001675)² ≈ 2.806 × 10⁻¹⁰
cos(lat₁ × π/180) = cos(0.07233) ≈ 0.99738
cos(lat₂ × π/180) = cos(0.07230) ≈ 0.99738
sin(dLon/2)² = sin(0.00001745)² ≈ 3.045 × 10⁻¹⁰
a = 2.806×10⁻¹⁰ + 0.99738 × 0.99738 × 3.045×10⁻¹⁰ ≈ 5.84 × 10⁻¹⁰
```

**Paso 3: Calcular 'c'**
```
c = 2 × atan2(√(5.84×10⁻¹⁰), √(1 - 5.84×10⁻¹⁰))
c ≈ 2 × atan2(0.0000242, 0.99999...) ≈ 0.0000484 rad
```

**Paso 4: Calcular distancia**
```
d = 6,371,000 × 0.0000484 ≈ 308 metros
```

---

## 9.3 Expresiones con static_cast

```cpp
const Estudiante* e = static_cast<const Estudiante*>(u);
```

### Desglose:

1. `u` es de tipo `const Usuario*` — un puntero a la clase base.
2. Sabemos que `u->getTipo() == "Estudiante"`, así que **sabemos** que realmente apunta a un `Estudiante`.
3. `static_cast<const Estudiante*>(u)` → le dice al compilador: "trata este puntero como si fuera un puntero a `Estudiante`".
4. `const` se mantiene: el puntero resultante es constante (no permite modificar el objeto).
5. Ahora `e` puede acceder a métodos de `Estudiante` como `getCodigoEstudiantil()`, `getFacultad()`, etc.

### En memoria:

```
                        Memoria del objeto
  u (Usuario*) ──────► ┌─────────────────────┐
                       │  idUsuario           │  ← Parte de Usuario
                       │  nombre              │
                       │  telefono            │
                       │  correo              │
                       │  tipo                │
  e (Estudiante*) ───► │  codigoEstudiantil   │  ← Parte de Estudiante
                       │  facultad            │     (misma dirección que u)
                       │  programa            │
                       │  semestre            │
                       └─────────────────────┘
```

Tanto `u` como `e` apuntan a la **misma dirección** de memoria. La diferencia es que `e` "ve" más atributos porque sabe que el objeto es un `Estudiante`.

---

## 9.4 Expresiones con std::find_if y Lambdas

### Ejemplo complejo: autenticación

```cpp
auto it = std::find_if(usuarios.begin(), usuarios.end(), [tipo, codigo](Usuario* u) {
    return u->getTipo() == datos.at(tipo).first && u->validarCodigo(codigo);
});
```

### Desglose componente por componente:

1. **`std::find_if`** — función de `<algorithm>`. Itera de `begin()` a `end()`.
2. **`usuarios.begin()`** — iterador al primer elemento del vector `usuarios`.
3. **`usuarios.end()`** — iterador "pasado el final" (marca de terminación).
4. **`[tipo, codigo]`** — la lambda captura las variables `tipo` (int) y `codigo` (int) por **valor** (copia). Estas variables son del ámbito de la función `autenticarUsuario`.
5. **`(Usuario* u)`** — cada elemento del vector es un `Usuario*`.
6. **`u->getTipo()`** — llama al getter de tipo (definido en `Usuario`).
7. **`datos.at(tipo).first`** — `datos` es un `std::map`. `.at(tipo)` retorna un `std::pair`. `.first` es el nombre del tipo (ej: "Estudiante").
8. **`u->validarCodigo(codigo)`** — llamada **polimórfica**. Aunque el puntero es `Usuario*`, se ejecuta `Estudiante::validarCodigo`, `Conductor::validarCodigo` o `Administrador::validarCodigo` según el tipo real del objeto.
9. **`&&`** — ambas condiciones deben ser verdaderas.
10. **`auto it`** — el tipo se deduce como `std::vector<Usuario*>::iterator`.

---

## 9.5 Cálculo de tiempoHastaParada

```cpp
double Bus::tiempoHastaParada(double latParada, double lonParada) const {
    if (!ubicacion) return -1.0;
    Coordenada destino(latParada, lonParada);
    double dist = ubicacion->distanciaHacia(destino);
    return ubicacion->tiempoEstimadoMinutos(dist);
}
```

### Desglose:

1. **`if (!ubicacion)`** — El operador `!` aplicado a un puntero retorna `true` si es `nullptr`. Esto es una **verificación de seguridad**: si el bus no tiene GPS, retorna -1.0 como señal de error.

2. **`Coordenada destino(latParada, lonParada)`** — Crea un objeto `Coordenada` temporal en el **stack** (memoria automática). Se destruirá al final de la función.

3. **`ubicacion->distanciaHacia(destino)`** — Aquí ocurre algo interesante:
   - `ubicacion` es de tipo `UbicacionGPS*`.
   - `UbicacionGPS` hereda de `Coordenada`.
   - `distanciaHacia` está definida en `Coordenada`.
   - Funciona porque un `UbicacionGPS` **es un** `Coordenada` (herencia).
   - El método calcula la distancia en metros usando la fórmula Haversine.

4. **`ubicacion->tiempoEstimadoMinutos(dist)`** — Convierte la distancia a tiempo usando:
   ```
   tiempo_minutos = (dist_metros / 1000 / velocidad_kmh) × 60
   ```

### Ejemplo numérico:

Si el bus está en (4.14527, -73.652037) y la parada en (4.14335, -73.650037):
1. Distancia ≈ 308 metros
2. Velocidad = 30 km/h
3. Tiempo = (308 / 1000 / 30) × 60 = (0.308 / 30) × 60 = 0.01027 × 60 = **0.616 minutos** ≈ 37 segundos

---

## 9.6 Simulación de Movimiento del Bus

```cpp
void Bus::simularMovimiento(double latDest, double lonDest) {
    if (!ubicacion) return;
    double deltaLat = (latDest - ubicacion->getLatitud())  * 0.05;
    double deltaLon = (lonDest - ubicacion->getLongitud()) * 0.05;
    ubicacion->setLatitud(ubicacion->getLatitud()   + deltaLat);
    ubicacion->setLongitud(ubicacion->getLongitud() + deltaLon);
}
```

### Desglose:

El factor `0.05` (5%) crea un movimiento de **interpolación exponencial** (ease-out):

- Cada llamada, el bus recorre el 5% de la distancia restante.
- Al principio (lejos), el 5% es grande → se mueve rápido.
- Al final (cerca), el 5% es pequeño → se mueve lento.
- Nunca llega exactamente al destino (asintótico), pero se acerca progresivamente.

Ejemplo si la diferencia de latitud es 0.1:
- Paso 1: avanza 0.005 → queda 0.095
- Paso 2: avanza 0.00475 → queda 0.09025
- Paso 3: avanza 0.004513 → queda 0.08574
- ...continúa acercándose exponencialmente.

---

## 9.7 Operador < Invertido del Bus

```cpp
bool Bus::operator<(const Bus& otro) const {
    return (capacidadMaxima - capacidadActual) > (otro.capacidadMaxima - otro.capacidadActual);
}
```

### Desglose:

- `capacidadMaxima - capacidadActual` = **asientos disponibles** de este bus.
- `otro.capacidadMaxima - otro.capacidadActual` = asientos disponibles del otro bus.
- El operador `>` (mayor que) invierte la lógica: un bus con MÁS asientos disponibles es "menor" en la ordenación.

**¿Por qué?** Si se usa `std::sort` con este operador, los buses con más asientos libres aparecerán primero. Esto facilita asignar pasajeros al bus con más capacidad disponible.

---

# Sección 10: Flujo de Ejecución Completo (Call Graph)

## 10.1 Arranque del Programa

```
┌──────────────────────────────────────────────────────────────────────┐
│                        INICIO: main()                                │
│                                                                      │
│  1. try {                                                            │
│  2.   SistemaTransporte sistema("data");                             │
│       └─► Constructor:                                               │
│           ├─► gestor("data")     // GestorArchivos con ruta "data"   │
│           └─► usuarioActual = nullptr                                │
│                                                                      │
│  3.   sistema.iniciar();                                             │
│       └─► (ver diagrama detallado abajo)                             │
│                                                                      │
│  } catch (std::exception& ex) {                                      │
│      std::cerr << "[ERROR CRITICO] " << ex.what();                   │
│      return 1;                                                       │
│  }                                                                   │
│  return 0;                                                           │
└──────────────────────────────────────────────────────────────────────┘
```

## 10.2 Método iniciar() — Diagrama Detallado

```
SistemaTransporte::iniciar()
│
├─► gestor.cargarBuses()
│   ├─► Abre data/buses.json
│   ├─► json::parse(archivo)
│   ├─► Para cada bus en j["buses"]:
│   │   ├─► Crea Bus(id, placa, capMax, capAct, est)
│   │   ├─► setIndiceParadaActual(...)
│   │   ├─► setIdRutaAsignada(...)
│   │   └─► push_back al vector
│   └─► Retorna vector<Bus> (3 buses)
│
├─► gestor.cargarParadas()
│   ├─► Abre data/paradas.json
│   ├─► json::parse(archivo)
│   ├─► Para cada parada (array raíz):
│   │   └─► emplace_back(id, nombre, lat, lon, alt, estado, orden)
│   └─► Retorna vector<Parada> (67 paradas)
│
├─► gestor.cargarIncidentes()
│   └─► Similar: retorna vector<Incidente> (3 incidentes)
│
├─► gestor.cargarRutas()
│   ├─► gestor.cargarHorariosPorDefecto()  ◄── Lee horarios.json
│   ├─► Abre data/rutas.json
│   ├─► Para cada ruta:
│   │   ├─► if tipo == "RutaCentro" → new RutaCentro(...)
│   │   ├─► else → new RutaBarrio(...)
│   │   ├─► Carga IDs de paradas
│   │   ├─► Carga horarios propios o usa globales
│   │   └─► push_back(puntero)
│   └─► Retorna vector<Ruta*> (12 rutas, 1 RutaCentro + 11 RutaBarrio)
│
├─► gestor.cargarUsuarios()
│   ├─► Abre data/usuarios.json
│   ├─► Para cada usuario:
│   │   ├─► if tipo == "Estudiante" → new Estudiante(...)
│   │   ├─► if tipo == "Administrador" → new Administrador(...)
│   │   └─► if tipo == "Conductor" → new Conductor(...)
│   └─► Retorna vector<Usuario*> (6 usuarios)
│
├─► SetConsoleTitle("Sistema de Gestion de Transporte - Unillanos")
├─► SetConsoleOutputCP(65001)   // UTF-8
├─► SetConsoleCP(65001)         // UTF-8 para entrada
│
└─► BUCLE PRINCIPAL ──────────────────────────────────────────
    │
    ├─► usuarioActual = nullptr
    ├─► op = mostrarMenuPrincipal()
    │   ├─► limpiarPantalla()
    │   ├─► imprimirEncabezado()
    │   │   ├─► imprimirSeparador()  → línea "====..."
    │   │   ├─► horaActual() → "HH:MM:SS"
    │   │   ├─► fechaActual() → "DD/MM/YYYY"
    │   │   └─► imprimirSeparador()
    │   ├─► Muestra opciones 1-4
    │   └─► retorna leerInt()
    │
    ├─► if (op == 4) → mensaje de despedida + break
    │
    ├─► if (op < 1 || op > 3) → continue (vuelve al inicio)
    │
    ├─► limpiarPantalla() + imprimirEncabezado()
    │
    ├─► usuarioActual = autenticarUsuario(op)
    │   ├─► Pide código al usuario
    │   ├─► find_if en usuarios con lambda (tipo + código)
    │   ├─► Si encuentra → retorna puntero al usuario
    │   └─► Si no → mensaje de error + Sleep(2000) + nullptr
    │
    ├─► if (!usuarioActual) → continue
    │
    ├─► if (op == 1) → panelEstudiante()
    │   └─► (bucle con rutas, paradas, horarios, reservar asiento)
    │
    ├─► if (op == 2) → panelConductor()
    │   └─► (bucle con bus, ruta, llegadas, incidentes)
    │
    └─► if (op == 3) → panelAdministrador()
        └─► (bucle con sub-menús de gestión)
```

## 10.3 Destrucción y Liberación de Memoria

Cuando el bucle principal termina (opción 4) y `iniciar()` retorna:

```
~SistemaTransporte()
├─► for (Ruta* r : rutas) delete r;
│   ├─► ~RutaBarrio() o ~RutaCentro()  (destructor virtual)
│   └─► ~Ruta()  (destructor base)
│
├─► for (Usuario* u : usuarios) delete u;
│   ├─► ~Estudiante(), ~Conductor() o ~Administrador()
│   └─► ~Usuario()
│
├─► ~vector<Bus>()  (automático)
│   └─► Para cada Bus:
│       └─► ~Bus()
│           └─► delete ubicacion;  (libera UbicacionGPS del heap)
│
├─► ~vector<Parada>()  (automático)
├─► ~vector<Incidente>()  (automático)
└─► ~GestorArchivos()  (automático, trivial)
```

**Nota importante:** Los vectores de `Bus`, `Parada` e `Incidente` contienen **objetos directos** (no punteros), así que sus destructores se llaman automáticamente. Los vectores de `Ruta*` y `Usuario*` contienen **punteros**, así que requieren `delete` manual (que se hace en el destructor de `SistemaTransporte`).

---

# Sección 11: Conceptos Teóricos y Buenas Prácticas

## 11.1 Programación Orientada a Objetos (POO)

### Encapsulamiento

| Aspecto | Detalle |
|---------|---------|
| **Definición** | Ocultar los detalles internos de un objeto y exponer solo una interfaz pública controlada. |
| **Analogía** | Un carro: tú usas el volante, pedales y palanca (interfaz pública), pero no necesitas saber cómo funciona el motor internamente (implementación privada). |
| **Ejemplo en el proyecto** | Todos los atributos son `private`. Acceso mediante `getX()` y `setX()`. |
| **Errores comunes** | Hacer atributos `public` "para ahorrar tiempo", romper el setter sin validación, no usar `const` en getters. |

### Herencia

| Aspecto | Detalle |
|---------|---------|
| **Definición** | Una clase (hija) adquiere las propiedades y métodos de otra clase (padre). |
| **Analogía** | Un estudiante "es un" usuario. Tiene todo lo de un usuario (nombre, teléfono) más cosas propias (código estudiantil, semestre). |
| **Ejemplo en el proyecto** | `Estudiante : public Usuario`, `UbicacionGPS : public Coordenada`, `RutaBarrio : public Ruta`. |
| **Errores comunes** | Herencia excesiva (más de 3-4 niveles), usar herencia cuando debería ser composición, olvidar el destructor virtual. |

### Polimorfismo

| Aspecto | Detalle |
|---------|---------|
| **Definición** | La capacidad de tratar objetos de diferentes clases de forma uniforme a través de una interfaz común. |
| **Analogía** | Un control remoto universal: presionas "Volumen+" y funciona con cualquier TV, aunque cada TV lo implemente diferente internamente. |
| **Ejemplo en el proyecto** | `vector<Usuario*> usuarios` contiene `Estudiante`, `Conductor` y `Administrador`. Al llamar `u->validarCodigo(c)`, se ejecuta la versión correcta según el tipo real. |
| **Errores comunes** | Olvidar la palabra `virtual`, olvidar `override` en las subclases, no hacer el destructor virtual. |

### Abstracción

| Aspecto | Detalle |
|---------|---------|
| **Definición** | Modelar conceptos del mundo real como clases, capturando solo las características relevantes para el problema. |
| **Analogía** | Un mapa: no incluye cada piedra y hoja, solo calles, edificios y puntos de interés relevantes. |
| **Ejemplo en el proyecto** | La clase `Bus` no modela el motor, la transmisión o el combustible; solo las propiedades relevantes para el transporte: capacidad, placa, ubicación, ruta. |
| **Errores comunes** | Modelar demasiados detalles irrelevantes, clases con demasiadas responsabilidades. |

---

## 11.2 Gestión de Memoria

### Stack vs Heap

```
┌─────────────────────────────┐
│         STACK               │  ← Rápido, automático, limitado
│  (variables locales)        │
│                             │
│  int x = 5;                 │  ← Se destruye al salir del ámbito
│  Coordenada c(4.1, -73.6);  │  ← Se destruye al salir del ámbito
│  std::string s = "hola";    │  ← Se destruye al salir del ámbito
│                             │
├─────────────────────────────┤
│         HEAP                │  ← Más lento, manual, ilimitado*
│  (memoria dinámica)         │
│                             │
│  new UbicacionGPS(...)      │  ← Vive hasta que hagas 'delete'
│  new RutaBarrio(...)        │  ← Vive hasta que hagas 'delete'
│  new Estudiante(...)        │  ← Vive hasta que hagas 'delete'
└─────────────────────────────┘
```

| Característica | Stack | Heap |
|----------------|-------|------|
| Velocidad | Muy rápida | Más lenta |
| Tamaño | Limitado (~1-8 MB) | Casi ilimitado |
| Gestión | Automática | Manual (new/delete) |
| Riesgo | Desbordamiento | Memory leaks |

### RAII (Resource Acquisition Is Initialization)

**¿Qué es?** Un patrón donde los recursos (memoria, archivos, conexiones) se adquieren en el constructor y se liberan en el destructor.

**Ejemplo en el proyecto:**
- `std::ifstream archivo(...)` → abre el archivo al crearse, lo cierra al destruirse.
- `std::vector<Bus>` → reserva memoria al crearse, la libera al destruirse.
- Los objetos en el stack se destruyen automáticamente → RAII perfecto.

### new y delete

```cpp
// new: reserva memoria en el heap y construye el objeto
ubicacion = new UbicacionGPS(lat, lon, alt, vel);

// delete: destruye el objeto y libera la memoria
delete ubicacion;
```

**Regla de oro:** Todo `new` debe tener un `delete` correspondiente. Si no, hay **memory leak**.

**En este proyecto:**
- `Bus::~Bus()` hace `delete ubicacion;`
- `SistemaTransporte::~SistemaTransporte()` hace `delete` de cada `Ruta*` y `Usuario*`.

---

## 11.3 Serialización JSON

| Aspecto | Detalle |
|---------|---------|
| **Definición** | Convertir objetos en memoria a un formato de texto (JSON) que se puede almacenar en un archivo o transmitir por red. La **deserialización** es el proceso inverso. |
| **Analogía** | Serializar es como empacar tu maleta para un viaje (organizar objetos en un formato transportable). Deserializar es como desempacar al llegar. |
| **Ejemplo en el proyecto** | `GestorArchivos::guardarBuses()` convierte `vector<Bus>` a JSON. `GestorArchivos::cargarBuses()` convierte JSON a `vector<Bus>`. |
| **Errores comunes** | Asumir que los campos siempre existen (usar `value()` con default), no manejar archivos que no se pueden abrir, no formatear la salida (`dump(4)`). |

---

## 11.4 Fórmula Haversine

| Aspecto | Detalle |
|---------|---------|
| **Definición** | Fórmula matemática que calcula la distancia más corta entre dos puntos sobre una esfera (círculo máximo o "distancia ortodrómica"). |
| **¿Por qué no Pitágoras?** | Las coordenadas geográficas son ángulos en una esfera, no posiciones en un plano. Pitágoras daría errores crecientes con la distancia. |
| **Precisión** | Exacta para una esfera perfecta. La Tierra es un elipsoide achatado, así que hay un error de ~0.3%, suficiente para este proyecto. |
| **Ejemplo en el proyecto** | `Coordenada::distanciaHacia()` implementa la fórmula completa. |

---

## 11.5 Patrón MVC (Modelo-Vista-Controlador)

Aunque el proyecto no implementa MVC de forma estricta, se puede identificar una separación similar:

| Componente | Equivalente en el proyecto | Responsabilidad |
|------------|---------------------------|-----------------|
| **Modelo** | Clases de negocio (Coordenada, Bus, Ruta, Usuario...) | Representar datos y lógica |
| **Vista** | Funciones de impresión en SistemaTransporte.cpp | Mostrar información al usuario |
| **Controlador** | SistemaTransporte (bucle principal, menús) | Coordinar modelo y vista |

La clase `GestorArchivos` actúa como una **capa de acceso a datos** (DAO), que no es parte del MVC clásico pero es una buena práctica adicional.

---

# Sección 12: Resumen y Ruta de Estudio

## 12.1 Síntesis del Proyecto

SistemaTransporte es un proyecto de complejidad media que demuestra el uso práctico de:

- **15 clases** organizadas en una jerarquía con herencia y polimorfismo.
- **Persistencia JSON** usando nlohmann/json para leer y escribir 6 archivos de datos.
- **Cálculos geográficos** reales con la fórmula Haversine sobre 67 paradas de Villavicencio.
- **Interfaz de consola Windows** con colores y formato, usando la Windows API directamente.
- **Gestión de memoria** con `new`/`delete` para polimorfismo con punteros crudos.
- **Tres roles de usuario** (estudiante, conductor, administrador) con funcionalidades diferentes.
- **CMake** como sistema de compilación portable.

El proyecto contiene aproximadamente **1,800 líneas** de código C++ distribuidas en 32 archivos, con datos reales de 12 rutas de bus universitario.

## 12.2 Orden Recomendado de Lectura

Para un estudiante que quiere entender el proyecto de cero, se recomienda este orden:

### Fase 1: Fundamentos (clases simples)
1. **`Coordenada.h` + `Coordenada.cpp`** — Clase más simple. Introduce getters/setters y la fórmula Haversine.
2. **`UbicacionGPS.h` + `UbicacionGPS.cpp`** — Introduce herencia (primera subclase).
3. **`Incidente.h` + `Incidente.cpp`** — Clase independiente y sencilla.
4. **`HorarioRuta.h` + `HorarioRuta.cpp`** — Uso de vectores y strings.

### Fase 2: Clases de Negocio (complejidad media)
5. **`Parada.h` + `Parada.cpp`** — Composición (tiene un `Coordenada`) y operador `<`.
6. **`Ruta.h` + `Ruta.cpp`** — Clase abstracta, métodos virtuales puros.
7. **`RutaBarrio.h` + `RutaBarrio.cpp`** — Subclase simple de Ruta.
8. **`RutaCentro.h` + `RutaCentro.cpp`** — Subclase con atributo extra.
9. **`Bus.h` + `Bus.cpp`** — Punteros, new/delete, destructor.

### Fase 3: Jerarquía de Usuarios
10. **`Usuario.h` + `Usuario.cpp`** — Clase abstracta con dos métodos puros.
11. **`Estudiante.h` + `Estudiante.cpp`** — Implementa los métodos virtuales.
12. **`Conductor.h` + `Conductor.cpp`** — Similar a Estudiante.
13. **`Administrador.h` + `Administrador.cpp`** — La subclase más simple.

### Fase 4: Persistencia
14. **`GestorArchivos.h` + `GestorArchivos.cpp`** — Lectura/escritura JSON, polimorfismo en acción.

### Fase 5: Orquestación
15. **`SistemaTransporte.h` + `SistemaTransporte.cpp`** — La clase más grande y compleja.
16. **`main.cpp`** — Punto de entrada (sorprendentemente simple).

### Fase 6: Configuración y Datos
17. **`CMakeLists.txt`** — Sistema de compilación.
18. **Archivos JSON** — Datos del sistema.

## 12.3 Consejos de Estudio

1. **No intentes entenderlo todo a la vez.** Empieza por `Coordenada` y avanza progresivamente.

2. **Compila y ejecuta frecuentemente.** Haz pequeños cambios y observa qué pasa. Rompe cosas a propósito para entender los errores.

3. **Dibuja los diagramas de herencia** en papel. Traza las flechas entre clases. Esto te ayudará a visualizar las relaciones.

4. **Pon breakpoints** (puntos de interrupción) en el debugger y ejecuta paso a paso. Observa cómo cambian las variables en memoria.

5. **Modifica los JSON** y observa cómo afecta al programa. Agrega una parada, cambia un horario, crea un nuevo usuario.

6. **Practica el patrón getter/setter** creando tu propia clase simple (por ejemplo, `Persona` con nombre y edad).

7. **Experimenta con los colores** de la consola. Cambia las combinaciones de flags y observa los resultados.

8. **Lee la documentación de nlohmann/json** en https://github.com/nlohmann/json para entender mejor las operaciones JSON.

9. **Comprende la diferencia entre stack y heap.** Dibuja un diagrama de memoria para cada variable del constructor de `Bus`.

10. **Practica con la fórmula Haversine.** Toma dos coordenadas de Google Maps y calcula la distancia a mano. Luego verifica con el programa.

---

> **Nota final:** Este documento cubre EXHAUSTIVAMENTE todo el código fuente del proyecto SistemaTransporte. Cada clase, cada método, cada bloque lógico y cada concepto ha sido explicado con el detalle y la paciencia de un profesor universitario. Si algo no queda claro, relee la sección correspondiente y consulta los ejemplos prácticos proporcionados. ¡El aprendizaje de la programación es un proceso gradual y repetitivo!

