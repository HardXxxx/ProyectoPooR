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

