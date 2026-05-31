cmake_minimum_required(VERSION 3.16)
project(SistemaTransporte CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Incluye los headers del proyecto
include_directories(include)

# Recopila todas las implementaciones
set(SOURCES
    main.cpp
    src/Coordenada.cpp
    src/UbicacionGPS.cpp
    src/Usuario.cpp
    src/Estudiante.cpp
    src/Administrador.cpp
    src/Conductor.cpp
    src/HorarioRuta.cpp
    src/Bus.cpp
    src/Parada.cpp
    src/Incidente.cpp
    src/Ruta.cpp
    src/RutaBarrio.cpp
    src/RutaCentro.cpp
    src/GestorArchivos.cpp
    src/SistemaTransporte.cpp
)

add_executable(SistemaTransporte ${SOURCES})

# Copia la carpeta data junto al ejecutable al compilar
add_custom_command(TARGET SistemaTransporte POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/data"
    "$<TARGET_FILE_DIR:SistemaTransporte>/data"
)
