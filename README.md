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