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
