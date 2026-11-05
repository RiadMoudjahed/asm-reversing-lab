<!-- source: README.md @ sha256:26989c484734 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 09. Qué es la ingeniería inversa

Tiempo: aproximadamente 1 hora. Requisitos previos: [08. La pila, los procedimientos y las convenciones de llamada](../08-stack-and-calls/README.es.md)

## Por qué importa

Los módulos 00 a 08 te enseñaron a escribir ensamblador. La ingeniería inversa (reverse engineering) es la misma habilidad usada en sentido contrario: partes de un programa terminado y averiguas qué hace y cómo. Antes de las herramientas y formatos de los módulos siguientes, necesitas una definición clara, un conjunto de usos legítimos, los límites que debes respetar y una forma repetible de trabajar que evite que te pierdas en un binario.

## Conceptos

### Definición

**La ingeniería inversa** consiste en recuperar el diseño, el comportamiento o los datos de un producto a partir del propio producto terminado, sin los documentos de diseño originales. En el caso del software, el producto suele ser un archivo ejecutable, una biblioteca o una imagen de firmware, y el resultado es conocimiento: la descripción de un formato de archivo, la lógica de una función, la lista de acciones que realiza un programa.

No es lo mismo que volver a escribir el programa. El objetivo es comprender. Reescribir es a veces un paso posterior, como en el ejercicio 08 y el ejercicio 09, donde reimplementas funciones que antes entendiste.

### Usos legítimos

| Uso | Ejemplo |
|---|---|
| Interoperabilidad | Documentar un formato de archivo sin documentar para que otra herramienta pueda leerlo |
| Auditoría de seguridad | Revisar un componente de código cerrado en busca de errores de seguridad de memoria |
| Análisis de malware | Averiguar qué hace una muestra hostil para que los defensores puedan detectarla y eliminarla |
| Recuperar conocimiento perdido | Entender una herramienta cuyo código fuente se ha perdido |
| Depuración | Averiguar por qué ocurre un fallo en código que no puedes recompilar con símbolos |
| Aprendizaje | Ver qué hace un compilador con tu propio código |

### Límites

Este curso enseña análisis y comprensión. No enseña, y sus ejercicios nunca piden:

- desarrollar exploits, shellcode o malware;
- eludir protecciones contra copia, licencias o gestión de derechos digitales;
- cualquier actividad contra sistemas o software que no tengas derecho a examinar.

Antes de analizar un programa que no es tuyo, comprueba tres cosas: quién es su propietario, qué dice la licencia o el contrato sobre el análisis y qué dice la ley de tu país. La interoperabilidad y la investigación de seguridad tienen protecciones legales específicas en algunos lugares y ninguna en otros. Las normas difieren entre países y cambian con el tiempo, por lo que este curso no ofrece asesoramiento jurídico; cuando la situación no esté clara, consulta primero a una persona cualificada. Todos los binarios de este repositorio se construyen a partir de código fuente incluido en el repositorio, así que trabajar en los ejercicios siempre está permitido. Consulta [ETHICS.md](../../ETHICS.es.md).

### Dos niveles de ingeniería inversa

La ingeniería inversa **a nivel de sistema** observa cómo se comporta un programa desde fuera: qué archivos abre, qué conexiones de red hace, qué llamadas al sistema emite, qué escribe en disco. El programa se trata como una caja negra y se observa.

La ingeniería inversa **a nivel de código** lee las instrucciones del programa para saber exactamente cómo calcula. Es más lenta y más precisa. Las investigaciones reales alternan ambas: la vista exterior indica dónde mirar y la vista del código explica por qué.

### Análisis estático y dinámico

| | Análisis estático | Análisis dinámico |
|---|---|---|
| Qué haces | Leer el archivo sin ejecutarlo | Ejecutar el programa y observarlo |
| Herramientas | `file`, `strings`, `readelf`, `objdump`, un desensamblador, un descompilador | Un depurador, `strace`, `ltrace` |
| Ventajas | Seguro; ve todo el código, incluso las rutas que nunca se ejecutan | Muestra valores reales; desempaqueta datos que el propio programa decodifica |
| Desventajas | No ve los valores calculados en tiempo de ejecución; el código ofuscado es difícil de leer | Solo ve las rutas que realmente se ejecutan; ejecutar código desconocido exige un entorno seguro |

Ninguno es mejor. Un buen flujo de trabajo usa el análisis estático para formular una pregunta y el dinámico para responderla, y vuelve al estático con los nuevos datos.

### Un flujo de trabajo repetible

1. **Triaje.** Identifica el archivo sin ejecutarlo: tipo, arquitectura, tamaño, hash, cadenas, funciones importadas, secciones. Lleva minutos y a menudo responde la pregunta o muestra dónde mirar.
2. **Observar.** Ejecuta el programa en un lugar seguro y registra lo que hace desde fuera: salida, llamadas al sistema, archivos.
3. **Formular una pregunta.** Anota una cosa concreta que necesitas saber, por ejemplo "¿dónde se comprueba la entrada?" o "¿cuál es la disposición de este registro?".
4. **Encontrar el código.** Usa cadenas, importaciones y llamadas al sistema como puntos de partida y sigue las referencias cruzadas hasta el código que importa.
5. **Leer y nombrar.** Lee las instrucciones, averigua qué hace cada función y dale un nombre significativo. Los nombres son el principal resultado del trabajo.
6. **Probar la idea.** Comprueba tu comprensión con un depurador, recorriendo el código con entradas elegidas, o reimplementando la función y comparando resultados.
7. **Informar.** Registra lo que encontraste, cómo lo sabes y qué queda por conocer. Separa los hechos de las suposiciones.

Toma notas sobre la marcha. La memoria falla rápido en un binario grande, y una nota como "la función en 0x401136 copia la cabecera en un búfer de 32 bytes" ahorra una hora al día siguiente.

### Un lugar seguro para ejecutar programas

Ejecuta programas desconocidos solo en un entorno que puedas desechar. Para este curso basta el contenedor del laboratorio, porque todos los binarios proceden de este repositorio. Para muestras reales, usa una máquina virtual sin red y con una instantánea a la que volver. Nunca ejecutes una muestra desconocida en la máquina que contiene tus archivos.

## Pruébalo

Haz el triaje del programa más pequeño del curso sin ejecutarlo. Los comandos se ejecutan desde `modules/03-first-program/examples`, después de que `make` haya construido los ejemplos.

```bash
file build/03_hello64
strings -n 4 build/03_hello64 | head -6
readelf -h build/03_hello64 | grep -E "Type|Entry"
```

Salida real:

```text
build/03_hello64: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, with debug_info, not stripped
Hello, assembly!
03_hello64.asm
/lab/modules/03-first-program/examples/
NASM 2.16.01
03_hello64.asm
YYuY0Y/
  Type:                              EXEC (Executable file)
  Entry point address:               0x401000
```

Sin ejecutar nada, sabes que este es un programa Linux de 64 bits para x86-64 sin bibliotecas, que se escribió en ensamblador con NASM (la cadena del productor y el nombre del archivo fuente quedan en el archivo, junto con el directorio de compilación), que no se le quitaron los símbolos y que imprime un saludo. Eso es triaje.

Ahora obsérvalo desde fuera:

```bash
strace -q build/03_hello64
```

```text
execve("build/03_hello64", ["build/03_hello64"], 0x7ffcb16ebed8 /* 15 vars */) = 0
write(1, "Hello, assembly!\n", 17Hello, assembly!
)      = 17
exit(0)                                 = ?
+++ exited with 0 +++
```

Las llamadas al sistema confirman lo que sugerían las cadenas: un `write` de 17 bytes a la salida estándar y luego `exit`. La salida del propio programa se mezcla en el trazado porque ambas van al terminal. Esto es ingeniería inversa a nivel de sistema; leer las instrucciones sería a nivel de código.

Escribe en tres líneas lo que sabes de este archivo y cómo lo sabes. Ese es el hábito que debes formar.

## Errores comunes

- **Empezar en el desensamblador.** Sin triaje lees código sin saber para qué sirve el programa. Dedica los primeros minutos a la vista exterior.
- **Leerlo todo.** Un binario puede tener miles de funciones. Sigue la pregunta, no el orden del archivo.
- **Fiarse de los nombres.** Los nombres de símbolos y las cadenas pueden ser erróneos o engañosos a propósito. Trátalos como pistas que hay que confirmar.
- **Ejecutar código desconocido en tu propia máquina.** Usa un entorno desechable.
- **Mezclar hechos y suposiciones en las notas.** Marca lo que verificaste y lo que supusiste.

## Resumen

- La ingeniería inversa recupera cómo funciona un programa a partir del propio programa; el objetivo es comprender.
- Los usos legítimos incluyen la interoperabilidad, la auditoría de seguridad, el análisis de malware, la recuperación de conocimiento perdido y el aprendizaje. Comprueba la propiedad, la licencia y la ley antes de analizar software que no es tuyo.
- El análisis estático lee el archivo; el dinámico lo ejecuta. El buen trabajo alterna entre ambos.
- Un flujo de triaje, observación, pregunta, búsqueda del código, nombrado, prueba e informe mantiene el trabajo enfocado.
- Ejecuta programas desconocidos solo en un entorno desechable.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 10, fundamentos de sistemas operativos para ingenieros inversos.
