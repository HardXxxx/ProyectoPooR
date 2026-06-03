# Documentación Focalizada: SistemaTransporte.cpp y GestorArchivos.cpp

> **Enfoque:** Análisis ultra-detallado de los dos archivos más importantes del sistema.  
> **Tono:** Profesor universitario. **Cero suposiciones.** Cada línea, cada operador, cada estructura de control se explica.  
> **Audiencia:** Estudiante que ya leyó la documentación general y quiere dominar la lógica central.

---

## Prólogo: ¿Por qué estos dos archivos?

- **`SistemaTransporte.cpp`** (560 líneas) es el **cerebro** del programa. Orquesta desde que el usuario enciende el sistema hasta que lo apaga. Contiene: menús, autenticación, los 3 paneles (estudiante, conductor, administrador), la simulación GPS, la reserva de asientos, la gestión de incidentes, y la comunicación con la persistencia.
- **`GestorArchivos.cpp`** (300 líneas) es la **memoria persistente** del sistema. Traduce entre objetos C++ vivos en RAM y archivos JSON en disco. Sin este archivo, los datos desaparecerían al cerrar el programa.

---

## PARTE I: `GestorArchivos.cpp` — El Puente entre la Memoria y el Disco

### I.1 Cabeceras y Alias

```cpp
#include "GestorArchivos.h"     // ← Declaración de la clase (necesario siempre)
#include <fstream>              // ← ifstream (leer archivos) y ofstream (escribir archivos)
#include <iostream>             // ← cerr (para mensajes de error)
#include "../include/json.hpp"  // ← nlohmann/json (parsear y generar JSON)

using json = nlohmann::json;    // ← Alias: ahora escribimos "json" en vez de "nlohmann::json"
```

**Explicación de `using json = nlohmann::json`:**

`nlohmann::json` es el nombre completo de la clase principal de la librería. `nlohmann` es el **namespace** del autor (Niels Lohmann). Escribir `nlohmann::json` cada vez que queremos usarla sería tedioso. `using json = nlohmann::json;` crea un **alias** (apodo): a partir de ahora, `json` significa `nlohmann::json`. Es como decir: "De ahora en adelante, cuando diga 'Pepe', me refiero a 'José'".

### I.2 Constructor

```cpp
GestorArchivos::GestorArchivos(const std::string& dirData) : rutaData(dirData) {}
```

**INPUT:** `dirData` — una cadena de texto que contiene la ruta al directorio de datos (ej. "data").  
**OUTPUT:** Ninguno (inicializa el objeto).

**Paso a paso:**
1. La **lista de inicializadores** (`: rutaData(dirData)`) asigna el valor del parámetro `dirData` al atributo `rutaData`. Es equivalente a escribir `this->rutaData = dirData;` dentro del cuerpo, pero más eficiente porque inicializa directamente en lugar de crear un string vacío y luego asignar.

**El operador `:` en el constructor:**
- Después de `)`, los dos puntos `:` inician la **lista de inicializadores**.
- `rutaData(dirData)` llama al **constructor de copia** de `std::string`, que copia el contenido de `dirData` a `rutaData`.

### I.3 `cargarBuses()`

```cpp
std::vector<Bus> GestorArchivos::cargarBuses() const {
    std::vector<Bus> lista;                           // ← Crea un vector vacío de buses
    std::ifstream archivo(rutaData + "/buses.json");  // ← Abre el archivo para lectura
    if (!archivo.is_open()) return lista;             // ← Si no se pudo abrir, devuelve vacío

    json j = json::parse(archivo);                    // ← Convierte el texto JSON a objeto manipulable
    for (const auto& b : j["buses"]) {                // ← Itera sobre cada bus en el arreglo JSON
        Bus busObj(
            b["idBus"].get<int>(),                    // ← Extrae idBus como entero
            b["placa"].get<std::string>(),            // ← Extrae placa como string
            b["capacidadMaxima"].get<int>(),
            b["capacidadActual"].get<int>(),
            b["estado"].get<bool>()
        );
        // Nuevos campos agregados
        busObj.setIndiceParadaActual(b.value("indiceParadaActual", -1));  // ← Si no existe, usa -1
        busObj.setIdRutaAsignada(b.value("idRutaAsignada", b["idBus"].get<int>())); // ← Si no existe, usa idBus
        lista.push_back(busObj);                      // ← Agrega el bus al vector
    }
    return lista;                                     // ← Devuelve el vector lleno
}
```

#### Análisis línea por línea:

**L1: `std::vector<Bus> lista;`**
- Se crea un **objeto vector vacío** en el **stack** (memoria automática).
- `lista` no tiene elementos, su `size()` es 0 y su `capacity()` también es 0.
- Cuando se agreguen buses con `push_back`, el vector irá creciendo dinámicamente en el **heap**.

**L2: `std::ifstream archivo(rutaData + "/buses.json");`**
- `std::ifstream` = **Input File Stream** (flujo de entrada desde archivo).
- El constructor recibe una ruta: `rutaData + "/buses.json"`. Como `rutaData` es "data", la ruta completa es "data/buses.json".
- Si el archivo no existe, `archivo` queda en estado de error.

**L3: `if (!archivo.is_open()) return lista;`**
- `is_open()` devuelve `true` si el archivo se abrió correctamente.
- `!` invierte: si NO se abrió, devolvemos el vector vacío.
- **¿Por qué no lanzar una excepción?** El diseñador eligió que el sistema funcione con datos vacíos en lugar de fallar. Así, si falta un archivo, el programa arranca pero muestra "sin datos".

**L4: `json j = json::parse(archivo);`**
- `json::parse(archivo)` lee TODO el archivo y construye un árbol JSON en memoria.
- ¿Qué es un **árbol JSON**? Es una estructura de datos anidada. Para:
  ```json
  { "buses": [ { "idBus": 1, "placa": "VST-001" } ] }
  ```
  Produce un objeto `j` donde:
  - `j` es un objeto (dict)
  - `j["buses"]` es un arreglo (list)
  - `j["buses"][0]` es un objeto
  - `j["buses"][0]["idBus"]` es el número 1

**L5: `for (const auto& b : j["buses"])`**
- **Range-based for loop** sobre el arreglo `j["buses"]`.
- `const auto& b`: cada elemento `b` es una **referencia constante** a un objeto JSON. `auto` deduce el tipo como `const nlohmann::json&`.
- ¿Por qué `const auto&` en vez de `auto`? `auto` (sin `&`) COPIA cada objeto JSON (lento y memoria). `const auto&` solo pasa una referencia (rápido, sin copia).

**L6-L10: Construcción del Bus**
```cpp
Bus busObj(
    b["idBus"].get<int>(),
    b["placa"].get<std::string>(),
    ...
);
```
- `b["idBus"]` accede al campo "idBus" del objeto JSON `b`.
- `.get<int>()` convierte el valor JSON a tipo `int` de C++.
- Se llama al **constructor parametrizado** de `Bus`: `Bus(int id, string placa, ...)`.

**L11: `busObj.setIndiceParadaActual(b.value("indiceParadaActual", -1));`**
- `b.value("indiceParadaActual", -1)`: Si el campo "indiceParadaActual" existe en el JSON, devuelve su valor. Si NO existe, devuelve `-1` (valor por defecto). Esto es crucial para compatibilidad con versiones anteriores del JSON que no tenían este campo.

**L12: `busObj.setIdRutaAsignada(b.value("idRutaAsignada", b["idBus"].get<int>()));`**
- Similar: si no existe "idRutaAsignada", usa el `idBus` como valor por defecto.

**L13: `lista.push_back(busObj);`**
- `push_back` agrega el objeto `busObj` al FINAL del vector.
- **IMPORTANTE:** Esto CREA UNA COPIA de `busObj` dentro del vector. El `busObj` original se destruye al salir de la iteración. Si `Bus` tuviera punteros sin un constructor de copia adecuado, esto causaría problemas (doble delete). En este proyecto, `Bus` tiene un `UbicacionGPS*` que se copia como puntero crudo — esto es una **debilidad** del diseño (ver sección debuilidades al final).

### I.4 `cargarParadas()`

```cpp
std::vector<Parada> GestorArchivos::cargarParadas() const {
    std::vector<Parada> lista;
    std::ifstream archivo(rutaData + "/paradas.json");
    if (!archivo.is_open()) return lista;

    json j = json::parse(archivo);
    for (const auto& p : j) {  // ← paradas.json es un ARRAY DIRECTO (sin clave "paradas")
        lista.emplace_back(     // ← emplace_back construye DIRECTAMENTE dentro del vector
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

**Diferencia clave con `cargarBuses()`:**

- `buses.json` tiene estructura: `{ "buses": [ ... ] }` — se itera sobre `j["buses"]`.
- `paradas.json` tiene estructura: `[ ... ]` — es un **array directo**, se itera sobre `j` directamente.

**`emplace_back` vs `push_back`:**

```cpp
// push_back: 1) construye un objeto temporal, 2) lo COPIA al vector, 3) destruye el temporal
lista.push_back(Parada(id, nombre, lat, lon, alt, est, orden));

// emplace_back: 1) construye el objeto DIRECTAMENTE en la memoria del vector
lista.emplace_back(id, nombre, lat, lon, alt, est, orden);
```

`emplace_back` es más eficiente porque evita la copia intermedia. Recibe los mismos argumentos que el constructor y los pasa directamente.

**Acceso anidado a JSON:**

```cpp
p["ubicacion"]["latitud"].get<double>()
// Primero accede a p["ubicacion"] (objeto anidado)
// Luego accede a ["latitud"] dentro de ese objeto
// Finalmente .get<double>() convierte a double de C++
```

### I.5 `cargarIncidentes()`

```cpp
std::vector<Incidente> GestorArchivos::cargarIncidentes() const {
    std::vector<Incidente> lista;
    std::ifstream archivo(rutaData + "/incidentes.json");
    if (!archivo.is_open()) return lista;

    json j = json::parse(archivo);
    for (const auto& inc : j["incidentes"]) {
        lista.emplace_back(
            inc["idIncidente"].get<int>(),
            inc["descripcion"].get<std::string>(),
            inc["tipo"].get<std::string>(),
            inc["estado"].get<std::string>()
        );
    }
    return lista;
}
```

**Estructura del JSON:**
```json
{ "incidentes": [
    { "idIncidente": 1, "descripcion": "...", "tipo": "Mecanico", "estado": "Abierto" }
] }
```
Es idéntico en patrón a `cargarBuses()`: objeto con un arreglo.

### I.6 `cargarHorariosPorDefecto()`

```cpp
HorarioRuta GestorArchivos::cargarHorariosPorDefecto() const {
    HorarioRuta global;                                 // ← Objeto vacío (sin horarios)
    std::ifstream archivo(rutaData + "/horarios.json");
    if (!archivo.is_open()) return global;              // ← Si no existe, devuelve vacío

    json j = json::parse(archivo);
    if (j.contains("configuracionHorarios")) {          // ← Verifica que la clave exista
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

**`j.contains("configuracionHorarios")`:** Verifica si la clave existe en el objeto JSON, evitando errores si el archivo tiene otro formato.

**`conf["salidasBarrio"].is_array()`:** Verifica que el valor sea un arreglo antes de iterar. Esto es una **doble validación**: primero existe la clave, luego es del tipo correcto.

### I.7 `cargarRutas()` — El método más complejo

```cpp
std::vector<Ruta*> GestorArchivos::cargarRutas() const {
    std::vector<Ruta*> lista;
    std::ifstream archivo(rutaData + "/rutas.json");
    if (!archivo.is_open()) return lista;

    HorarioRuta horarioGlobal = cargarHorariosPorDefecto();  // ← Carga horarios globales

    json j = json::parse(archivo);
    for (const auto& r : j["rutas"]) {
        std::string tipo    = r["tipo"].get<std::string>();
        std::string pSalida = (r.contains("puntoSalida") && !r["puntoSalida"].is_null())
                              ? r["puntoSalida"].get<std::string>() : "";
        std::string zona    = r.value("zonaCentro", "");  // ← Solo RutaCentro tiene esto

        Ruta* ruta = nullptr;
        if (tipo == "RutaCentro") {
            ruta = new RutaCentro(id, nom, orig, dest, est, pSalida, zona);
        } else {
            ruta = new RutaBarrio(id, nom, orig, dest, est, pSalida);
        }

        for (int idP : r["paradas"])                       // ← Agrega IDs de paradas
            ruta->agregarParada(idP);

        if (r.contains("horarios") && r["horarios"].is_object() && !r["horarios"].empty()) {
            // Carga horarios específicos de la ruta
        } else {
            ruta->getHorario() = horarioGlobal;            // ← Usa horarios por defecto
        }

        lista.push_back(ruta);                             // ← Guarda el PUNTERO
    }
    return lista;
}
```

#### Análisis detallado:

**`std::vector<Ruta*>`** — El vector contiene **punteros**, no objetos. ¿Por qué?
- Porque `Ruta` es **abstracta** (tiene `virtual ... = 0`). No se pueden crear objetos `Ruta`, solo de sus hijas `RutaBarrio` y `RutaCentro`.
- Para tener polimorfismo, necesitamos punteros: `Ruta*` puede apuntar a una `RutaBarrio` o a una `RutaCentro`.

**`r.value("zonaCentro", "")`:**
- `json::value(clave, predeterminado)` es un método que busca la clave. Si existe, devuelve su valor. Si NO existe, devuelve el valor predeterminado (`""`).
- Es más seguro que `r["zonaCentro"]` porque no lanza excepción si la clave no existe.

**`new RutaCentro(...)` y `new RutaBarrio(...)`:**
- `new` reserva memoria en el **heap** (montón) y devuelve un puntero.
- Estos objetos vivirán hasta que alguien llame a `delete` sobre ellos.
- En este proyecto, `~SistemaTransporte()` es quien hace `delete` de cada `Ruta*`.

**Carga de horarios — la lógica condicional:**
```cpp
if (r.contains("horarios") && r["horarios"].is_object() && !r["horarios"].empty()) {
    // Usa horarios propios
} else {
    ruta->getHorario() = horarioGlobal;  // ← ASIGNACIÓN DE OBJETO COMPLETO
}
```
- `r.contains("horarios")`: ¿existe la clave "horarios"?
- `r["horarios"].is_object()`: ¿es un objeto JSON (no un arreglo, no un número)?
- `!r["horarios"].empty()`: ¿tiene al menos un campo?
- Si alguna condición falla, se asignan los horarios globales.
- `ruta->getHorario() = horarioGlobal`: `getHorario()` devuelve una **referencia** al `HorarioRuta` dentro de `Ruta`. Luego se usa el **operador de asignación** (`=`) de `HorarioRuta` para copiar todos los horarios.

### I.8 `cargarUsuarios()`

```cpp
std::vector<Usuario*> GestorArchivos::cargarUsuarios() const {
    // ...
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
    return lista;
}
```

**Polimorfismo en acción:**
- El vector `std::vector<Usuario*>` puede contener punteros a `Estudiante`, `Conductor` o `Administrador`.
- Cuando se llama a `u->getEncabezado()` o `u->validarCodigo()`, se ejecuta la versión correspondiente al tipo real (polimorfismo).
- **`new Estudiante(...)`**: crea un objeto `Estudiante` en el heap. Devuelve un `Estudiante*`, que se **convierte automáticamente** a `Usuario*` (upcasting). Esto funciona porque `Estudiante` ES UN `Usuario`.

### I.9 Métodos de Guardado — Patrón común

Todos los `guardar*()` siguen el mismo patrón:

```cpp
void GestorArchivos::guardarIncidentes(const std::vector<Incidente>& incidentes) const {
    json j;                              // 1. Crear objeto JSON vacío
    j["incidentes"] = json::array();     // 2. Crear arreglo vacío bajo la clave "incidentes"
    for (const Incidente& inc : incidentes) {
        j["incidentes"].push_back({      // 3. Agregar objeto al arreglo
            {"descripcion", inc.getDescripcion()},
            {"estado",      inc.getEstado()},
            {"idIncidente", inc.getIdIncidente()},
            {"tipo",        inc.getTipo()}
        });
    }
    std::ofstream archivo(rutaData + "/incidentes.json");  // 4. Abrir archivo para escritura
    if (archivo.is_open())               // 5. Si se pudo abrir...
        archivo << j.dump(4);            // 6. ...escribir JSON con indentación de 4 espacios
}
```

**`j.dump(4)`:** El método `dump(indentación)` convierte el objeto JSON a una cadena de texto con formato legible. `4` significa "4 espacios de indentación por nivel". Sin indentación, sería:
```json
{"incidentes":[{"descripcion":"...","estado":"Abierto",...}]}
```
Con indentación:
```json
{
    "incidentes": [
        {
            "descripcion": "...",
            "estado": "Abierto"
        }
    ]
}
```

**`guardarUsuarios()` — el más complejo porque tiene subtipos:**
```cpp
for (const Usuario* u : usuariosList) {
    json obj = {
        {"idUsuario", u->getIdUsuario()},
        {"nombre",    u->getNombre()},
        // ... campos comunes
    };
    if (u->getTipo() == "Estudiante") {
        const Estudiante* e = static_cast<const Estudiante*>(u);  // ← Downcasting
        obj["codigoEstudiantil"] = e->getCodigoEstudiantil();
        // ... campos específicos
    }
    j["usuarios"].push_back(obj);
}
```

**`static_cast<const Estudiante*>(u)`:** Convierte un puntero `Usuario*` a `Estudiante*`. Esto se llama **downcasting**. Sabe que es seguro porque antes verificó `u->getTipo() == "Estudiante"`.

**`guardarRutas()` — maneja `puntoSalida` como null:**
```cpp
{"puntoSalida", r->getPuntoSalida().empty() ? json(nullptr) : json(r->getPuntoSalida())}
```
- Si `puntoSalida` está vacío, guarda `null` en JSON.
- Si no, guarda el string.
- Esto mantiene consistencia con el JSON original que permite `"puntoSalida": null`.

---

## PARTE II: `SistemaTransporte.cpp` — El Director de Orquesta

### II.1 Cabeceras

```cpp
#include "SistemaTransporte.h"    // ← La propia clase
#include "Conductor.h"            // ← Para hacer static_cast<Conductor*> en panelConductor()
#include "Estudiante.h"           // ← Para panelEstudiante() (uso implícito)
#include "Administrador.h"        // ← Para panelAdministrador() (uso implícito)
#include "RutaCentro.h"           // ← Para static_cast<RutaCentro*> en guardar rutas
#include <windows.h>              // ← API de Windows: colores, consola, hora
#include <ctime>                  // ← Funciones de tiempo (incluido pero no usado directamente)
#include <iostream>               // ← cout, cin, cerr
#include <sstream>                // ← ostringstream (para formatear hora/fecha)
#include <iomanip>                // ← setw, setfill, setprecision (formateo de salida)
#include <map>                    // ← std::map (en autenticarUsuario)
#include <algorithm>              // ← find_if, count_if, any_of
```

### II.2 Constantes y Funciones Auxiliares (estáticas)

```cpp
static const int ANCHO = 80;       // ← Ancho estándar de la consola en caracteres
```

**`static` aquí:** Las siguientes funciones son `static`, lo que significa que tienen **alcance de archivo**. Solo son visibles dentro de `SistemaTransporte.cpp`. No pueden ser llamadas desde otros archivos. Es una forma de hacerlas "privadas" de este archivo.

#### `limpiarPantalla()`

```cpp
static void limpiarPantalla() {
    COORD coord = {0, 0};                           // ← Estructura con X=0, Y=0
    DWORD celdas, escritas;                          // ← Variables para resultados
    CONSOLE_SCREEN_BUFFER_INFO info;                 // ← Estructura para info de consola
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);      // ← Obtiene el manejador de la consola

    GetConsoleScreenBufferInfo(h, &info);             // ← Lee dimensiones de la consola
    celdas = info.dwSize.X * info.dwSize.Y;          // ← Total de caracteres (ancho × alto)
    FillConsoleOutputCharacter(h, ' ', celdas, coord, &escritas);  // ← Escribe espacios
    FillConsoleOutputAttribute(h, info.wAttributes, celdas, coord, &escritas); // ← Resetea colores
    SetConsoleCursorPosition(h, coord);              // ← Mueve cursor a (0,0)
}
```

**`COORD coord = {0, 0}`:** Inicialización **aggregate** de una estructura de Windows:
```cpp
typedef struct _COORD { SHORT X; SHORT Y; } COORD;
// {0, 0} → coord.X = 0, coord.Y = 0 (esquina superior izquierda)
```

**`DWORD`:** Tipo de Windows = `unsigned long` (32 bits sin signo).

**`HANDLE`:** Tipo opaco de Windows que representa un recurso del sistema (en este caso, la consola).

**`GetConsoleScreenBufferInfo(h, &info)`:**
- `h`: el handle de la consola.
- `&info`: la **dirección** de la estructura `info`. La función llena los campos de `info`.
- Después de la llamada: `info.dwSize.X` = ancho en caracteres (ej. 80), `info.dwSize.Y` = alto (ej. 25).

**`FillConsoleOutputCharacter(h, ' ', celdas, coord, &escritas)`:**
- Escribe `celdas` espacios (' ') empezando en la posición `coord`.
- `&escritas`: variable donde se almacena cuántos caracteres se escribieron realmente.

**`FillConsoleOutputAttribute(h, info.wAttributes, celdas, coord, &escritas)`:**
- Restaura los colores originales. `info.wAttributes` se guardó ANTES de modificar nada, así que restaura los colores que tenía la consola al iniciar el programa.

**`SetConsoleCursorPosition(h, coord)`:**
- Mueve el cursor a la posición (0,0). El próximo `std::cout` aparecerá en la primera línea.

#### `setColor(WORD c)`

```cpp
static void setColor(WORD c) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}
```

Un **wrapper** (envoltorio) para hacer más corta la llamada. En vez de escribir cada vez:
```cpp
SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
```
Escribimos:
```cpp
setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
```

#### `leerInt()`

```cpp
static int leerInt() {
    int v;
    if (!(std::cin >> v)) {        // ← Si la entrada falla (ej. el usuario escribe letras)
        std::cin.clear();           // ← Limpia el estado de error de cin
        std::cin.ignore(10000, '\n'); // ← Descarta hasta 10000 caracteres o hasta nueva línea
        return -1;                  // ← Devuelve -1 como indicador de error
    }
    std::cin.ignore(10000, '\n');   // ← Descarta el resto de la línea (incluyendo el \n)
    return v;
}
```

**`std::cin >> v`:** Intenta leer un entero desde el teclado.
- Si el usuario escribe "42", `v = 42`, el flujo queda en estado bueno.
- Si el usuario escribe "hola", la lectura FALLA, `cin` entra en estado de error, `v` NO se modifica.

**`!(std::cin >> v)`:** El `!` verifica si el flujo está en estado de error. Equivalente a `std::cin.fail()`.

**`std::cin.clear()`:** Limpia la bandera de error. Sin esto, todos los intentos posteriores de leer también fallarían.

**`std::cin.ignore(10000, '\n')`:** Descarta caracteres. Sirve para:
1. Limpiar el búfer después de leer un número (el `\n` que el usuario presionó queda en el búfer).
2. Después de un error, descarta toda la línea incorrecta.

### II.3 Constructor y Destructor

```cpp
SistemaTransporte::SistemaTransporte(const std::string& dirData)
    : gestor(dirData), usuarioActual(nullptr) {}
```

**Lista de inicializadores:**
- `gestor(dirData)`: Construye el objeto `GestorArchivos` con `dirData` como ruta. `gestor` es un **atributo por composición** (se construye automáticamente cuando se construye `SistemaTransporte`).
- `usuarioActual(nullptr)`: Inicializa el puntero como `nullptr` (ningún usuario ha iniciado sesión).

```cpp
SistemaTransporte::~SistemaTransporte() {
    for (Ruta* r : rutas) delete r;       // ← Libera cada ruta del heap
    for (Usuario* u : usuarios) delete u; // ← Libera cada usuario del heap
}
```

**¿Por qué `delete` y no `delete[]`?** Porque cada elemento fue creado con `new` individual (no `new[]`). `new RutaCentro(...)` y `new Estudiante(...)` se liberan con `delete`.

**¿Qué pasaría si no existiera este destructor?** Los 12+ objetos `Ruta*` y 7+ objetos `Usuario*` quedarían en el heap para siempre = **fuga de memoria** (memory leak).

### II.4 `imprimirSeparador()` y `imprimirEncabezado()`

```cpp
void SistemaTransporte::imprimirSeparador() const {
    setColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY); // ← Cyan brillante
    std::cout << std::string(ANCHO, '=') << '\n';  // ← 80 signos '='
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);       // ← Gris claro (reset)
}

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

**`std::string(ANCHO, '=')`:** Construye un string de 80 caracteres '='.
- Primer parámetro (80): cantidad de repeticiones.
- Segundo parámetro ('='): carácter a repetir.
- Resultado: `"================================================================================"`

**Operador ternario en `std::cout`:**
```cpp
(usuarioActual ? " | " + usuarioActual->getEncabezado()
               : " | SISTEMA DE GESTION DE TRANSPORTE")
```
- Si `usuarioActual` NO es `nullptr`: muestra el encabezado personalizado.
- Si es `nullptr`: muestra el título genérico.

**Polimorfismo en acción:** `usuarioActual->getEncabezado()` llama al método virtual. Dependiendo del tipo real del objeto (Estudiante, Conductor, Administrador), se ejecuta una implementación diferente.

### II.5 `horaActual()` y `fechaActual()`

```cpp
std::string SistemaTransporte::horaActual() const {
    SYSTEMTIME st;                    // ← Estructura de Windows para fecha/hora
    GetLocalTime(&st);                // ← Llena la estructura con la hora local actual

    std::ostringstream ss;            // ← "String output stream": como cout pero a un string
    ss << std::setw(2) << std::setfill('0') << st.wHour << ':'
       << std::setw(2) << std::setfill('0') << st.wMinute << ':'
       << std::setw(2) << std::setfill('0') << st.wSecond;
    return ss.str();                  // ← Convierte el stream a std::string
}
```

**`SYSTEMTIME`:** Estructura de Windows con campos: `wYear`, `wMonth`, `wDay`, `wHour`, `wMinute`, `wSecond`, `wMilliseconds`.

**`GetLocalTime(&st)`:**
- Función de la API de Windows.
- Llena la estructura `st` con la fecha y hora local actual del sistema.
- `&st` = dirección de `st` (paso por puntero para que la función modifique la variable).

**`std::ostringstream`:** Permite usar `<<` para construir un string, igual que haríamos con `std::cout` pero en memoria.

**`std::setw(2)`:** El siguiente campo ocupará al menos 2 caracteres.
**`std::setfill('0')`:** Los espacios se rellenan con '0'.

**Ejemplo:** Si son las 8:5:3 (8 horas, 5 minutos, 3 segundos):
```
Sin setw/setfill: "8:5:3"
Con setw/setfill: "08:05:03"  ← ¡Formato correcto!
```

**`return ss.str()`:** `str()` devuelve el contenido del `ostringstream` como un `std::string`.

### II.6 `buscarBusPorId()`

```cpp
Bus* SistemaTransporte::buscarBusPorId(int id) {
    auto it = std::find_if(buses.begin(), buses.end(),
        [id](const Bus& b) { return b.getIdBus() == id; });
    return it != buses.end() ? &*it : nullptr;
}
```

**`std::find_if(inicio, fin, predicado)`:**
- Busca el primer elemento entre `inicio` y `fin` que cumpla el predicado.
- `buses.begin()` = iterador al primer bus.
- `buses.end()` = iterador "más allá del último" (marca de final).
- `[id](const Bus& b) { return b.getIdBus() == id; }` = lambda que captura `id` y verifica si el ID del bus coincide.

**`&*it`:**
- `it` es un iterador (similar a un puntero).
- `*it` **desreferencia** el iterador y obtiene una **referencia** al objeto `Bus`.
- `&*it` obtiene la **dirección** de ese objeto → `Bus*`.
- No se devuelve una copia, se devuelve un **puntero al objeto original** dentro del vector.

**`it != buses.end()`:** Si `find_if` no encuentra nada, devuelve `end()`. Entonces devolvemos `nullptr`.

### II.7 `buscarRutaPorBus()` — Versión actualizada

```cpp
Ruta* SistemaTransporte::buscarRutaPorBus(const std::string& placa) {
    auto it = std::find_if(buses.begin(), buses.end(),
        [&placa](const Bus& b) { return b.getPlaca() == placa; });

    if (it != buses.end() && !rutas.empty()) {
        int idRuta = it->getIdRutaAsignada();                           // ← Obtiene ruta asignada al bus
        auto itRuta = std::find_if(rutas.begin(), rutas.end(),
            [idRuta](Ruta* r) { return r->getIdRuta() == idRuta; });   // ← Busca la ruta por ID
        if (itRuta != rutas.end()) return *itRuta;                     // ← Devuelve la ruta encontrada
    }
    return (!rutas.empty() ? rutas[0] : nullptr);                      // ← Fallback a primera ruta
}
```

**Cambio importante:** Ahora usa `getIdRutaAsignada()` del bus para obtener el ID de ruta correspondiente, y luego busca esa ruta específica en el vector de rutas. Ya no asume `rutas[idBus - 1]`. Es una mejora significativa.

**`[&placa]`:** Captura `placa` por **referencia** (evita copiar el string).
**`[idRuta]`:** Captura `idRuta` por **valor** (un int es pequeño, copiar es eficiente).

### II.8 `incidentesDeBus()`

```cpp
std::vector<Incidente*> SistemaTransporte::incidentesDeBus(const std::string& placa) {
    std::vector<Incidente*> resultado;
    for (auto& inc : incidentes)
        if (inc.getDescripcion().find(placa) != std::string::npos)
            resultado.push_back(&inc);
    return resultado;
}
```

**`inc.getDescripcion().find(placa) != std::string::npos`:**
- `find()` busca la subcadena `placa` dentro de la descripción.
- Si la encuentra: devuelve la posición (índice) donde comienza.
- Si NO la encuentra: devuelve `std::string::npos` (constante = 18446744073709551615).

**`resultado.push_back(&inc)`:**
- `&inc` obtiene la **dirección** del incidente dentro del vector `incidentes`.
- No se crea una copia del incidente, solo se guarda un puntero a él.
- Los punteros devueltos son válidos mientras el vector `incidentes` no se redimensione.

### II.9 `mostrarProximidadBus()` — La versión más completa

```cpp
void SistemaTransporte::mostrarProximidadBus(Bus* bus, Ruta* ruta,
                                              const std::map<int, std::string>& horasLlegada) {
    if (!bus || !ruta || !bus->getUbicacion()) return;

    int idxActual = bus->getIndiceParadaActual();
    const auto& ids = ruta->getIdsParadas();

    for (size_t i = 0; i < ids.size(); ++i) {   // ← Itera por ÍNDICE (no range-based)
        int idP = ids[i];
        auto it = std::find_if(paradas.begin(), paradas.end(),
            [idP](const Parada& p) { return p.getIdParada() == idP; });
        if (it == paradas.end()) continue;

        auto itHora = horasLlegada.find(idP);

        if ((int)i <= idxActual && itHora != horasLlegada.end()) {
            // Parada ya visitada: muestra hora de llegada, distancia 0
            std::cout << "  * Parada (" << idP << ") " << it->getNombre()
                      << "\n    -> 0 m | 0.0 min | Llego: " << itHora->second << "\n";
        } else {
            // Parada pendiente: calcula distancia y tiempo
            double metros = it->distanciaA(bus->getUbicacion()->getLatitud(),
                                           bus->getUbicacion()->getLongitud());
            double minutos = bus->tiempoHastaParada(it->getUbicacion().getLatitud(),
                                                    it->getUbicacion().getLongitud());
            std::cout << "    Parada (" << idP << ") " << it->getNombre()
                      << "\n    -> " << std::fixed << std::setprecision(0) << metros << " m | "
                      << std::setprecision(1) << minutos << " min\n";
        }
    }
}
```

**Parámetro nuevo:** `const std::map<int, std::string>& horasLlegada` — mapa que asocia ID de parada → hora de llegada (ej. {1 → "08:15:30", 2 → "08:23:10"}).

**Lógica de paradas visitadas vs. pendientes:**
- `bus->getIndiceParadaActual()`: devuelve el índice de la última parada visitada (ej. 2 = ya pasó las paradas 0, 1, 2).
- `(int)i <= idxActual`: si el índice actual de iteración es menor o igual al índice visitado, la parada YA FUE VISITADA.
- `itHora != horasLlegada.end()`: si la parada está en el mapa de horas de llegada.

Para las paradas **visitadas**: muestra distancia 0, tiempo 0, y la hora de llegada real.
Para las paradas **pendientes**: calcula distancia y tiempo desde la posición actual del bus.

**`for (size_t i = 0; i < ids.size(); ++i)`:** Usa índice en lugar de range-based porque necesita comparar `i` con `idxActual`.

**`const auto& ids = ruta->getIdsParadas();`:**
- `auto` deduce el tipo como `const std::vector<int>&`.
- La referencia evita copiar todo el vector.

**`itHora->second`:** Cuando `itHora` es un iterador de `std::map<int, string>`:
- `itHora->first` = clave (el ID de parada).
- `itHora->second` = valor (la hora de llegada).

### II.10 `mostrarMenuPrincipal()`

```cpp
int SistemaTransporte::mostrarMenuPrincipal() {
    limpiarPantalla();              // ← Borra todo
    imprimirEncabezado();           // ← Muestra [hora - fecha] | SISTEMA DE GESTION...
    setColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    std::cout << "Seleccione su perfil:\n\n"
              << "  [1] Estudiante\n  [2] Conductor\n  [3] Administrador\n"
              << "  [4] Salir\n\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    imprimirSeparador();
    std::cout << "> Opcion: ";
    return leerInt();               // ← Lee la opción del usuario
}
```

**Flujo completo de una iteración:**
1. Se limpia la pantalla (desaparece todo lo anterior).
2. Se imprime el encabezado con hora, fecha y título.
3. Se imprimen las 4 opciones del menú.
4. Se lee la opción del usuario.
5. Se devuelve el número (1-4, o -1 si hubo error de entrada).

### II.11 `autenticarUsuario()` — El guardia de la puerta

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

    if (it != usuarios.end()) return *it;  // ← Usuario encontrado

    setColor(FOREGROUND_RED | FOREGROUND_INTENSITY);
    std::cout << "\n  [!] Codigo no valido.\n";
    setColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    Sleep(2000);  // ← Espera 2 segundos para que el usuario lea el mensaje
    return nullptr;
}
```

**`static const std::map<int, std::pair<string, string>>`:**
- `static`: el mapa se crea UNA SOLA VEZ (en la primera llamada) y se reusa en todas las llamadas posteriores. No se recrea cada vez.
- `const`: el mapa no se puede modificar.
- `map<int, pair<string, string>>`: asocia un entero (opción del menú) con un par (tipo como string, label como string).

**`datos.at(tipo).second`:**
- `at(tipo)`: devuelve el `pair` asociado a la clave `tipo`. Si `tipo` no existe, lanza `std::out_of_range`.
- `.second`: segundo elemento del par (el label: "codigo estudiantil", etc.).
- `.first`: primer elemento del par (el tipo: "Estudiante", etc.).

**La lambda de búsqueda:**
```cpp
[u, codigo](Usuario* u) {
    return u->getTipo() == datos.at(tipo).first && u->validarCodigo(codigo);
}
```
Verifica **dos condiciones** con `&&` (AND lógico):
1. El tipo del usuario coincide con el esperado (ej. "Estudiante").
2. El código ingresado es válido para ese usuario (cada tipo valida distinto).

**`if (it != usuarios.end()) return *it;`:**
- Si `find_if` encontró algo, `it` apunta al elemento. `*it` desreferencia y obtiene `Usuario*`.
- `return *it` devuelve el puntero al usuario autenticado.

### II.12 `panelEstudiante()` — El más interactivo

```cpp
void SistemaTransporte::panelEstudiante() {
    while (true) {                          // ← Bucle infinito del panel
        limpiarPantalla();
        imprimirEncabezado();
        std::cout << "DIRECTORIO DE RUTAS ACTIVAS\n";
        std::cout << std::string(ANCHO, '-') << "\nSeleccione una ruta:\n\n";

        for (Ruta* r : rutas)               // ← Muestra todas las rutas
            std::cout << "[ " << std::setw(2) << r->getIdRuta() << "] " << r->getNombre()
                      << " (" << r->getTipo() << ")\n";
        std::cout << "\n[ 0] Volver\n\n> Ruta: ";

        int idRuta = leerInt();
        if (idRuta == 0) break;             // ← Salir del panel

        auto it = std::find_if(rutas.begin(), rutas.end(),
            [idRuta](Ruta* r) { return r->getIdRuta() == idRuta; });
        if (it == rutas.end()) {
            // Ruta no existe: mensaje de error
            Sleep(1500);
            continue;                       // ← Vuelve al inicio del while
        }

        Ruta* rutaEncontrada = *it;         // ← Puntero a la ruta seleccionada

        // Muestra DETALLES de la ruta:
        // 1. Nombre y punto de salida
        // 2. PARADAS (busca cada Parada por ID en el vector global)
        // 3. HORARIOS (formateados desde el objeto HorarioRuta)

        std::cout << "\n>>> Reservar asiento? (S/N): ";
        char abordar = 'N';
        if (std::cin >> abordar) {
            std::cin.ignore(10000, '\n');
        } else {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }

        if (toupper(abordar) == 'S') {
            // LÓGICA DE RESERVA:
            // 1. Intenta buscar bus por ID de ruta (buscarBusPorId)
            // 2. Si no encuentra, busca CUALQUIER bus con espacio disponible
            Bus* bus = buscarBusPorId(rutaEncontrada->getIdRuta());
            if (!bus && !buses.empty()) {
                auto itBus = std::find_if(buses.begin(), buses.end(),
                    [](const Bus& b) {
                        return b.getCapacidadActual() < b.getCapacidadMaxima() && b.getEstado();
                    });
                if (itBus != buses.end()) bus = &*itBus;
            }

            if (bus && bus->getCapacidadActual() < bus->getCapacidadMaxima()) {
                bus->setCapacidadActual(bus->getCapacidadActual() + 1);
                gestor.guardarBuses(buses);   // ← PERSISTENCIA INMEDIATA
                // Mensaje de éxito
            } else {
                // Mensaje de bus lleno
            }
            std::cout << "\n[Enter para continuar]"; std::cin.get();
        }
    }
}
```

**Búsqueda de bus en dos pasos:**
1. `Bus* bus = buscarBusPorId(rutaEncontrada->getIdRuta())` — busca el bus cuyo ID coincida con el ID de la ruta (convención: ruta 1 = bus 1).
2. Si no encuentra: busca **cualquier bus** activo con espacio disponible usando `std::find_if`.

**`toupper(abordar) == 'S'`:**
- `toupper()` convierte un carácter a mayúscula. Así 's' y 'S' funcionan igual.

**`b.getCapacidadActual() < b.getCapacidadMaxima() && b.getEstado()`:**
- Dos condiciones: que haya espacio libre Y que el bus esté activo.

### II.13 `panelConductor()` — GPS y simulación en tiempo real

```cpp
void SistemaTransporte::panelConductor() {
    Conductor* cond = static_cast<Conductor*>(usuarioActual);  // ← Convierte Usuario* a Conductor*
    Bus* bus = buscarBusPorId(cond->getBusAsignado());         // ← Busca el bus del conductor
    Ruta* ruta = bus ? buscarRutaPorBus(bus->getPlaca()) : nullptr;  // ← Busca la ruta
    std::map<int, std::string> horasLlegada; // ← Mapa: idParada → hora de llegada

    while (true) {
        // --- MUESTRA INFORMACIÓN ---
        // [ VEHICULO ]: placa, capacidad, estado (ALERTA/OPERATIVO)
        // [ RUTA ]: nombre, origen, destino, horarios
        // [ PROXIMIDAD ]: distancia y tiempo a cada parada (visitadas vs pendientes)
        // [ ESTADO DE RUTA ]: última parada visitada

        int op = leerInt();

        if (op == 1) {  // --- LLEGADA A PARADA ---
            int idxActual = bus->getIndiceParadaActual();
            if (idxActual + 1 < (int)ruta->getIdsParadas().size()) {
                // AÚN HAY PARADAS POR VISITAR
                idxActual++;
                int idSig = ruta->getIdsParadas()[idxActual];
                auto it = std::find_if(paradas.begin(), paradas.end(),
                    [idSig](const Parada& p) { return p.getIdParada() == idSig; });
                if (it != paradas.end()) {
                    // Fija el GPS del bus EXACTAMENTE en la parada
                    bus->setUbicacion(it->getUbicacion().getLatitud(),
                                      it->getUbicacion().getLongitud(), it->getAltitud());
                    bus->setIndiceParadaActual(idxActual);
                    std::string horaLlg = horaActual();
                    horasLlegada[idSig] = horaLlg;  // ← Registra hora de llegada
                    gestor.guardarBuses(buses);
                }
            } else {
                // FIN DE RUTA: cambia a la siguiente ruta
                bus->setIndiceParadaActual(-1);  // ← Reinicia a "en origen"

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
                    ruta = rutas[nextIndex];  // ← Cambia la ruta actual
                }
                horasLlegada.clear();  // ← Limpia horas al cambiar de ruta
                gestor.guardarBuses(buses);
            }
        } else if (op == 2) {  // --- REPORTAR INCIDENTE ---
            std::cout << "\nDescripcion: "; std::string desc; std::getline(std::cin, desc);
            std::cout << "Tipo (Mecanico/Accidente/Otro): "; std::string tipo; std::getline(std::cin, tipo);
            incidentes.emplace_back(incidentes.size() + 1, desc, tipo, "Abierto");
            gestor.guardarIncidentes(incidentes);
        } else if (op == 3) {  // --- FINALIZAR TURNO ---
            break;
        }
    }
}
```

**`static_cast<Conductor*>(usuarioActual)`:** Convierte el puntero genérico `Usuario*` a `Conductor*`. Esto es seguro porque `usuarioActual` SOLO puede ser `Conductor*` si entramos a `panelConductor()`. Si fuera otro tipo, el sistema no permitiría la entrada.

**`bus->getBusAsignado()`:** Obtiene el ID del bus que conduce este conductor (ej. 1 para Carlos García).

#### Lógica de Llegada a Parada (Opción 1):

**Caso A: Quedan paradas por visitar:**
1. Obtiene el índice actual de parada (ej. 2 = ya visitó paradas 0, 1, 2).
2. Incrementa: `idxActual++` (ahora apunta a la siguiente parada, índice 3).
3. Obtiene el ID de la parada en esa posición: `ids[idxActual]`.
4. Busca la Parada por ID en el vector global.
5. **Fija el GPS del bus EXACTAMENTE en la parada**: `bus->setUbicacion(lat, lon, alt)`.
6. **Actualiza el índice de parada actual**: `bus->setIndiceParadaActual(idxActual)`.
7. Registra la hora de llegada en el mapa `horasLlegada`.
8. Persiste los cambios a disco.

**Caso B: Se acabaron las paradas (fin de ruta):**
1. Reinicia el índice: `bus->setIndiceParadaActual(-1)` (bus en origen).
2. Calcula la siguiente ruta:
   - Busca la ruta actual por ID.
   - `std::distance(rutas.begin(), itRuta)` calcula el ÍNDICE de la ruta actual en el vector.
   - Suma 1 para pasar a la siguiente ruta.
   - Si llega al final, vuelve a la primera (`nextIndex = 0`).
3. Asigna la nueva ruta al bus: `bus->setIdRutaAsignada(nuevaRuta)`.
4. Actualiza la variable local `ruta` al nuevo puntero.
5. Limpia el mapa de horas de llegada.

#### Lógica de Reportar Incidente (Opción 2):

```cpp
incidentes.emplace_back(incidentes.size() + 1, desc, tipo, "Abierto");
```
- `incidentes.size() + 1`: el nuevo ID es el tamaño actual + 1 (IDs autoincrementales).
- `emplace_back` construye el objeto `Incidente` directamente en el vector.
- Siempre se crea en estado "Abierto".

### II.14 `panelAdministrador()` y sus sub-métodos

#### `panelAdministrador()` — Menú principal del admin

```cpp
void SistemaTransporte::panelAdministrador() {
    while (true) {
        // [ ESTADO GLOBAL ]
        // - Número de rutas, paradas, buses
        // - Lista de placas de buses
        // - Incidentes abiertos (en rojo si > 0, en verde si 0)

        int abiertos = std::count_if(incidentes.begin(), incidentes.end(),
            [](const Incidente& i) { return i.getEstado() == "Abierto"; });

        // Menu: [1] Rutas [2] Usuarios [3] Buses [4] Horarios [5] Incidentes [6] Volver
    }
}
```

**`std::count_if(inicio, fin, predicado)`:**
- Similar a `find_if`, pero CUENTA cuántos elementos cumplen la condición.
- Recorre TODO el vector de incidentes.
- Cuenta aquellos cuyo `estado == "Abierto"`.

#### `gestionarRutas()` — Toggle de estado

```cpp
void SistemaTransporte::gestionarRutas() {
    // Muestra cada ruta con su estado: "  [1] Unillanos - Amarilo | ACTIVA"
    for (Ruta* r : rutas)
        std::cout << "  [" << r->getIdRuta() << "] " << r->getNombre()
                  << " | " << (r->getEstado() ? "ACTIVA" : "INACTIVA") << "\n";

    int id = leerInt();
    if (id > 0) {
        for (Ruta* r : rutas) {
            if (r->getIdRuta() == id) {
                r->setEstado(!r->getEstado());  // ← Toggle: true → false, false → true
                gestor.guardarRutas(rutas);
                break;
            }
        }
    }
}
```

**Operador ternario para el estado:**
```cpp
r->getEstado() ? "ACTIVA" : "INACTIVA"
```
Si `getEstado()` devuelve `true`, imprime "ACTIVA". Si `false`, imprime "INACTIVA".

**`!r->getEstado()`:** Invierte el booleano (toggle):
- `!true` = `false` (activa → inactiva)
- `!false` = `true` (inactiva → activa)

#### `gestionarBuses()` — Modificar capacidad

```cpp
void SistemaTransporte::gestionarBuses() {
    for (Bus& b : buses)
        std::cout << "  [" << b.getIdBus() << "] " << b.getPlaca()
                  << " | " << b.getCapacidadActual() << "/" << b.getCapacidadMaxima()
                  << " | " << (b.getEstado() ? "Activo" : "Inactivo") << "\n";
    // ...
    if (cap >= 0 && cap <= bus->getCapacidadMaxima()) {
        bus->setCapacidadActual(cap);
        gestor.guardarBuses(buses);
    }
}
```

**Validación de capacidad:** `cap >= 0 && cap <= bus->getCapacidadMaxima()`
- La capacidad no puede ser negativa.
- La capacidad no puede exceder la máxima.
- Si la validación falla, no se modifica nada.

#### `gestionarHorarios()` — Agregar salidas

```cpp
void SistemaTransporte::gestionarHorarios() {
    // ...
    std::cout << "Salida [1] Barrio [2] Unillanos: "; int op = leerInt();
    std::cout << "Hora (HH:MM:SS): "; std::string hora; std::getline(std::cin, hora);
    if (op == 1) r->getHorario().agregarSalidaBarrio(hora);
    else r->getHorario().agregarSalidaUnillanos(hora);
    gestor.guardarRutas(rutas);
}
```

**`std::getline(std::cin, hora)`:** Lee una línea completa incluyendo espacios. Se usa en lugar de `std::cin >> hora` porque las horas pueden contener `:` (aunque `>>` también funcionaría aquí, `getline` es más general).

#### `gestionarIncidentes()` — Cerrar incidentes

```cpp
void SistemaTransporte::gestionarIncidentes() {
    for (Incidente& inc : incidentes) {
        setColor(inc.getEstado() == "Abierto" ? FOREGROUND_RED | FOREGROUND_INTENSITY
                                              : FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        std::cout << "  [" << inc.getIdIncidente() << "] [" << inc.getEstado() << "] "
                  << inc.getTipo() << ": " << inc.getDescripcion() << "\n";
    }
    // ...
    if (inc.getIdIncidente() == id) {
        inc.cerrar();         // ← Cambia estado de "Abierto" a "Cerrado"
        gestor.guardarIncidentes(incidentes);
        break;
    }
}
```

**Color dinámico:** Los incidentes abiertos se muestran en **rojo brillante**, los cerrados en **verde brillante**.

### II.15 `iniciar()` — El punto de partida

```cpp
void SistemaTransporte::iniciar() {
    // FASE 1: CARGA DE DATOS
    buses = gestor.cargarBuses();
    paradas = gestor.cargarParadas();
    incidentes = gestor.cargarIncidentes();
    rutas = gestor.cargarRutas();
    usuarios = gestor.cargarUsuarios();

    // FASE 2: CONFIGURACIÓN DE CONSOLA
    SetConsoleTitle("Sistema de Gestion de Transporte - Unillanos");  // ← Título ventana
    SetConsoleOutputCP(65001);  // ← UTF-8 para la salida (tildes, ñ)
    SetConsoleCP(65001);        // ← UTF-8 para la entrada (teclado)

    // FASE 3: BUCLE PRINCIPAL
    while (true) {
        usuarioActual = nullptr;                      // ← "Cerrar sesión"
        int op = mostrarMenuPrincipal();              // ← Mostrar menú y leer opción

        if (op == 4) {                                // ← SALIR
            limpiarPantalla();
            std::cout << "\n  Sistema cerrado. Hasta luego.\n\n";
            Sleep(1000);
            break;                                    // ← Sale del while(true)
        }

        if (op < 1 || op > 3) continue;               // ← Opción inválida, reintentar

        limpiarPantalla();
        imprimirEncabezado();
        usuarioActual = autenticarUsuario(op);        // ← Intentar autenticar

        if (!usuarioActual) continue;                 // ← Falló autenticación, reintentar

        if (op == 1) panelEstudiante();               // ← Panel Estudiante
        else if (op == 2) panelConductor();           // ← Panel Conductor
        else panelAdministrador();                    // ← Panel Administrador
    }
}
```

**Fase 1 — Carga:** Orden importante: `cargarRutas()` necesita `cargarHorariosPorDefecto()` internamente, y todos son independientes entre sí. No importa el orden porque cada carga es autónoma.

**Fase 2 — Consola:**
- `SetConsoleTitle("...")`: Cambia el título de la ventana (arriba a la izquierda).
- `SetConsoleOutputCP(65001)`: 65001 = CP_UTF8. Sin esto, caracteres como "ó", "í", "ñ" se verían como símbolos extraños.
- `SetConsoleCP(65001)`: Lo mismo pero para la entrada del teclado.

**Fase 3 — Bucle:**
- `usuarioActual = nullptr`: Reinicia la sesión. Al volver al menú, el usuario anterior queda "desconectado".
- `autenticarUsuario(op)`: Si devuelve `nullptr`, el código no era válido. Se muestra el menú de nuevo.
- Cada panel (`panelEstudiante`, `panelConductor`, `panelAdministrador`) tiene su propio bucle interno que termina cuando el usuario elige "Volver" o "Finalizar turno".

---

## PARTE III: Vulnerabilidades y Debilidades del Diseño

### III.1 Copia superficial (shallow copy) en Bus

Cuando `GestorArchivos::cargarBuses()` hace `lista.push_back(busObj)`, se crea una **copia** de `busObj`. Pero `Bus` contiene un `UbicacionGPS*` (puntero). La copia copia el **puntero**, no el objeto apuntado. Ambos objetos (original y copia) apuntan a la MISMA dirección de memoria.

```cpp
Bus busA;
busA.setUbicacion(4.14, -73.65, 463, 30);  // new UbicacionGPS en dirección 0x1000

Bus busB = busA;  // ← Copia: busB.ubicacion también apunta a 0x1000
                   //   NO hay un nuevo UbicacionGPS

// Cuando busA se destruye: ~Bus() hace delete ubicacion (0x1000)
// Cuando busB se destruye: ~Bus() hace delete ubicacion (0x1000) → ¡ERROR! ¡Doble delete!
```

**¿Por qué no falla en la práctica?** Porque los buses se copian al cargar y nunca se destruyen los originales (los temporales del bucle se destruyen, pero como `ubicacion` es `nullptr` porque no se llamó a `setUbicacion()`, el `delete nullptr` es seguro). El problema se manifiesta si se llama a `setUbicacion()` en un bus temporal.

### III.2 Uso de `new`/`delete` sin `unique_ptr`

Los vectores `rutas` y `usuarios` almacenan punteros CRUDOS. Si ocurre una excepción entre el `new` y el `push_back`, el objeto se pierde (fuga). Además, la responsabilidad de llamar a `delete` recae en `~SistemaTransporte()`, lo que es frágil.

**Alternativa más segura:** `std::vector<std::unique_ptr<Ruta>>` — los punteros se liberan automáticamente.

### III.3 Dependencia de Windows

`<windows.h>` hace que el programa **solo funcione en Windows**. No es portable a Linux o macOS. Para hacerlo portable, habría que usar una librería como `ncurses` o un sistema de UI multiplataforma.

---

## Apéndice: Mapa de llamadas completo de SistemaTransporte.cpp

```
iniciar()
├── gestor.cargarBuses()              →  GestorArchivos.cpp
├── gestor.cargarParadas()            →  GestorArchivos.cpp
├── gestor.cargarIncidentes()         →  GestorArchivos.cpp
├── gestor.cargarRutas()              →  GestorArchivos.cpp
│     └── gestor.cargarHorariosPorDefecto()  →  GestorArchivos.cpp
├── gestor.cargarUsuarios()           →  GestorArchivos.cpp
├── SetConsoleTitle()                 →  windows.h
├── SetConsoleOutputCP/SetConsoleCP() →  windows.h
└── while(true):
      ├── mostrarMenuPrincipal()
      │     ├── limpiarPantalla()     →  windows.h
      │     ├── imprimirEncabezado()
      │     │     ├── imprimirSeparador()
      │     │     ├── horaActual()    →  GetLocalTime() (windows.h)
      │     │     └── fechaActual()   →  GetLocalTime()
      │     └── leerInt()             →  std::cin
      │
      ├── autenticarUsuario(tipo)
      │     └── std::find_if(usuarios, lambda con validarCodigo())
      │
      ├── panelEstudiante()
      │     ├── std::find_if(rutas, ...)
      │     ├── std::find_if(paradas, ...)  [por cada parada]
      │     ├── buscarBusPorId()
      │     │     └── std::find_if(buses, ...)
      │     ├── gestor.guardarBuses()
      │     └── Sleep()               →  windows.h
      │
      ├── panelConductor()
      │     ├── static_cast<Conductor*>
      │     ├── buscarBusPorId()
      │     ├── buscarRutaPorBus()
      │     │     └── std::find_if(buses, ...)
      │     │     └── std::find_if(rutas, ...)
      │     ├── incidentesDeBus()
      │     │     └── std::string::find()
      │     ├── std::any_of()
      │     ├── mostrarProximidadBus()
      │     │     ├── std::find_if(paradas, ...)  [por cada parada]
      │     │     ├── Coordenada::distanciaHacia()
      │     │     └── Bus::tiempoHastaParada()
      │     ├── [1] Bus::setUbicacion() + gestor.guardarBuses()
      │     ├── [2] incidentes.emplace_back() + gestor.guardarIncidentes()
      │     └── [3] break
      │
      └── panelAdministrador()
            ├── std::count_if()
            ├── gestionarRutas()
            │     └── gestor.guardarRutas()
            ├── gestionarUsuarios()
            ├── gestionarBuses()
            │     ├── buscarBusPorId()
            │     └── gestor.guardarBuses()
            ├── gestionarHorarios()
            │     ├── HorarioRuta::agregarSalida...()
            │     └── gestor.guardarRutas()
            ├── gestionarIncidentes()
            │     ├── Incidente::cerrar()
            │     └── gestor.guardarIncidentes()
            └── [6] break
```

---

> **Fin del análisis focalizado.**  
> Este documento se enfoca exclusivamente en `SistemaTransporte.cpp` y `GestorArchivos.cpp`.  
> Para el análisis completo de todas las demás clases, consultar `DOCUMENTACION_COMPLETA.md`.
