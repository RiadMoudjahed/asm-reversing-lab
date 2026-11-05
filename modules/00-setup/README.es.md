<!-- source: README.md @ sha256:b48e9369096e -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 00. Preparación del laboratorio

Tiempo: unos 45 minutos. Requisitos previos: solo conocimientos básicos de C y un equipo con Linux, macOS o Windows que pueda ejecutar Docker.

## Por qué es importante

Todos los ejemplos y ejercicios del curso se ejecutan en un único entorno preparado, para que tus resultados coincidan con los que se muestran en las lecciones. Diez minutos de preparación aquí te ahorran horas de "en mi equipo funciona" más adelante.

Al terminar este módulo tendrás un laboratorio operativo, una cadena de herramientas verificada, Ghidra en tu equipo anfitrión y una idea clara de cómo se comprueban los ejercicios.

## Conceptos

### Dos formas de ejecutar el laboratorio

**Docker (recomendado).** La imagen del laboratorio contiene cada herramienta en una versión fija. La carpeta de tu repositorio se monta dentro en `/lab`, de modo que los archivos que editas en tu equipo aparecen en el laboratorio al instante.

```bash
make shell
```

La primera ejecución construye la imagen, que ocupa unos 6,6 GB en disco y descarga muchos paquetes, así que reserva tiempo y una buena conexión. Las siguientes ejecuciones arrancan en segundos. Dentro del laboratorio el indicador de la línea de comandos muestra el usuario `lab` y tienes disponibles los comandos del curso (`make check`, `nasm`, `gdb`).

**Instalación nativa (Ubuntu o Debian).** Si prefieres no usar Docker:

```bash
sudo scripts/install-native.sh
```

Después antepón `NATIVE=1` a los comandos del curso, por ejemplo `make NATIVE=1 check EX=01`. El script instala el mismo ensamblador, los mismos compiladores, el mismo depurador y las mismas bibliotecas de Python que la imagen. No instala Rizin ni Ghidra.

### Qué contiene el laboratorio

| Herramienta | Para qué la usas |
|---|---|
| `nasm` | El ensamblador: convierte código fuente en ensamblador en un archivo objeto |
| `ld` | El enlazador: convierte archivos objeto en un ejecutable |
| `gcc`, `g++` | Compiladores de C y C++, también usados para enlazar ensamblador con arneses de prueba en C |
| `gdb` con GEF | El depurador, con una visualización legible de registros y memoria |
| `objdump`, `readelf` | Desensamblan código e inspeccionan la estructura de los ejecutables |
| `rizin` | Un desensamblador y marco de análisis de línea de comandos |
| `strace`, `ltrace` | Muestran las llamadas al sistema y a bibliotecas que hace un programa |
| `yara` | Búsqueda de patrones, usada en el módulo de análisis de malware |
| `mingw-w64`, `wine` | Compilan y ejecutan programas de Windows en Linux |
| Python 3 | Ejecuta los verificadores de los ejercicios; bibliotecas para leer archivos ELF y PE |

### Ghidra en tu equipo anfitrión

Ghidra es una suite gráfica de ingeniería inversa. Es demasiado grande y demasiado gráfica para la imagen del laboratorio, así que la instalas en tu propio equipo y abres los archivos de los retos desde la carpeta de tu repositorio.

1. Descarga Ghidra desde la página oficial de versiones del proyecto.
2. Instala la versión de JDK que exige el README de esa versión.
3. Descomprime Ghidra, inícialo con su script de arranque y crea un proyecto.
4. Importa los binarios desde `exercises/NN-name/bin/` en la carpeta de tu repositorio.

Empezarás a usar Ghidra en el módulo 11. Instalarlo ahora te permite seguir después sin interrupciones.

### Cómo funcionan los ejercicios y los verificadores

Cada ejercicio vive en `exercises/NN-name/` y tiene dos partes:

- **Parte 1, Reverse.** Analizas el programa de `bin/` y respondes preguntas. Las respuestas producen una bandera (flag) con la forma `ARL{...}`.
- **Parte 2, Assemble.** Editas `starter/solution.asm` para que cumpla una especificación.

Para comprobar tu trabajo:

```bash
make check EX=01 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de cada parte por separado. Nunca muestra la bandera esperada: el repositorio solo guarda un hash con sal (salted hash) de ella. Si una parte falla, lee `HINTS.md` en la carpeta del ejercicio. Las pistas tienen tres niveles; abre un nivel cada vez.

La carpeta `solutions/` contiene respuestas completas. Mantenla cerrada hasta que hayas terminado o lo hayas intentado en serio.

### Mapa del repositorio

```text
modules/     the lessons, each with runnable examples
exercises/   the exercises, each with a binary, a starter file and tests
solutions/   complete solutions (spoilers)
cheatsheets/ one-page references to keep open while you work
glossary/    definitions of the terms used in the course
```

## Pruébalo

Inicia el laboratorio y ejecuta la prueba de humo. Ensambla un programa de cuatro instrucciones, lo enlaza, lo ejecuta y después verifica la cadena de herramientas.

```bash
make shell
cd modules/00-setup/examples
make run
```

Salida esperada:

```text
mkdir -p build
nasm -f elf64 00_exit42.asm -o build/00_exit42.o
ld build/00_exit42.o -o build/00_exit42
./build/00_exit42; echo "exit status: $?"
exit status: 42
bash check_tools.sh
nasm       ok
ld         ok
gcc        ok
g++        ok
make       ok
gdb        ok
objdump    ok
readelf    ok
strace     ok
ltrace     ok
rizin      ok
yara       ok
python3    ok
xxd        ok
file       ok
gcc -m32   ok
```

El programa de `00_exit42.asm` hace una sola cosa: termina con el estado de salida 42. Todavía no necesitas entenderlo; el módulo 03 explica cada línea. Lo importante es que los tres pasos (ensamblar, enlazar, ejecutar) funcionaron, lo que demuestra que la cadena de herramientas está bien.

Abre ahora GDB una vez para confirmar que GEF se cargó:

```bash
gdb -q build/00_exit42
```

La primera línea de la salida debería mencionar `GEF for linux ready`. Escribe `quit` para salir.

## Errores frecuentes

- **`permission denied` al ejecutar Docker.** En Linux tu usuario debe pertenecer al grupo `docker`, o debes ejecutar Docker con permisos de administrador. Consulta la documentación de Docker de tu sistema.
- **`make shell` falla en Windows.** Ejecútalo desde una terminal donde funcionen tanto `make` como `docker`, como una shell de WSL 2.
- **Una herramienta muestra `MISSING` tras una instalación nativa.** Vuelve a ejecutar `scripts/install-native.sh` y lee su salida. Que falte `gcc -m32` significa que no se instalaron los paquetes multilib.
- **Ghidra no encuentra tu archivo.** Ghidra se ejecuta en el anfitrión, así que abre la ruta de tu anfitrión (por ejemplo `~/asm-reversing-lab/exercises/01-signal-flare/bin/`), no `/lab/...`.
- **Editar archivos a la vez dentro del laboratorio y en el anfitrión.** Elige un editor y un lugar. La carpeta del repositorio es compartida, así que ambos ven los mismos archivos, pero dos editores pueden sobrescribirse mutuamente.

## Resumen

- Ejecuta el curso en el laboratorio Docker (`make shell`) o en una instalación nativa de Ubuntu o Debian.
- `make run` en la carpeta `examples` de este módulo demuestra que el ensamblador, el enlazador y las herramientas funcionan.
- Ghidra se instala en tu anfitrión y abre archivos de la carpeta de tu repositorio.
- Cada ejercicio tiene una parte Reverse (una bandera) y una parte Assemble (código que comprueban las pruebas).
- `solutions/` contiene spoilers; usa primero `HINTS.md`.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 01, representación de datos.
- Ejercicios que usan este módulo: [Ejercicio 01](../../exercises/01-signal-flare/README.es.md).
