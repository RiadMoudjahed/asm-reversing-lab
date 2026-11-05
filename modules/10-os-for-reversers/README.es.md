<!-- source: README.md @ sha256:4d47465582de -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 10. Fundamentos de sistemas operativos para ingenieros inversos

Tiempo: aproximadamente 2 horas. Requisitos previos: [09. Qué es la ingeniería inversa](../09-what-is-re/README.es.md)

## Por qué importa

Un programa nunca se ejecuta solo. El sistema operativo le da memoria, decide qué puede tocar, carga su código y realiza toda acción que sale del proceso: abrir un archivo, enviar un paquete, crear un hilo. Cuando lees un desensamblado, muchas llamadas no son la lógica propia del programa sino peticiones al sistema. Conocer las capas te permite distinguir ambas cosas, encontrar rápido el código interesante y entender por qué un depurador muestra las direcciones que muestra.

## Conceptos

### Procesos e hilos

Un **proceso** es un programa en ejecución con su propio espacio de direcciones virtual, sus propios archivos abiertos y su propia identidad. Un **hilo** es un flujo de ejecución dentro de un proceso. Todos los hilos de un proceso comparten su memoria y sus archivos abiertos, pero cada uno tiene sus propios registros y su propia pila. Por eso la pila que estudiaste en el módulo 08 pertenece a un hilo y no a un proceso.

### Memoria virtual

Cada proceso ve un espacio de direcciones privado. Una dirección que usa tu código es una **dirección virtual**. El procesador y el núcleo la traducen a una dirección física mediante tablas de páginas. Dos procesos pueden usar la misma dirección virtual para datos distintos, y un proceso no puede nombrar la memoria de otro.

La memoria se gestiona en **páginas**, normalmente de 4096 bytes. Cada página tiene permisos: lectura, escritura, ejecución. El procesador los hace cumplir: escribir en una página de solo lectura o ejecutar código desde una página no ejecutable provoca un fallo, y el núcleo normalmente termina el proceso con una señal (`SIGSEGV` en Linux).

Los sistemas modernos siguen una regla que suele escribirse **W xor X**: una página es escribible o ejecutable, no ambas cosas, salvo que un programa lo pida. El código compilado está en páginas ejecutables de solo lectura; los datos están en páginas escribibles que no son ejecutables. Para un ingeniero inverso esto es útil: los permisos de una región indican lo que contiene.

La **aleatorización de la disposición del espacio de direcciones** (address space layout randomization, ASLR) coloca la pila, el montón y las bibliotecas en direcciones distintas en cada ejecución. Las direcciones de tus notas de una ejecución no coincidirán con la siguiente. Trabaja con desplazamientos desde el inicio de un módulo y usa nombres en lugar de direcciones absolutas.

### Qué contiene un proceso

| Región | Contenido | Permisos habituales |
|---|---|---|
| Código del programa | Las instrucciones del ejecutable | lectura, ejecución |
| Constantes | Cadenas y tablas que nunca cambian | lectura |
| Datos | Globales inicializadas y puestas a cero | lectura, escritura |
| Montón | Memoria solicitada en tiempo de ejecución (`malloc`) | lectura, escritura |
| Bibliotecas | El código y los datos de cada biblioteca compartida | como arriba, por biblioteca |
| Pila | Una por hilo: marcos, locales, direcciones de retorno | lectura, escritura |

### Cómo se carga un programa

1. Un proceso padre pide al núcleo que ejecute un archivo (en Linux la llamada al sistema es `execve`).
2. El núcleo lee las cabeceras del archivo, mapea el código y los datos del archivo en un nuevo espacio de direcciones y crea la pila con los argumentos y el entorno.
3. Si el programa usa bibliotecas compartidas, el núcleo mapea además un **cargador dinámico** y lo inicia primero. El cargador mapea cada biblioteca necesaria, resuelve las referencias del programa a funciones de biblioteca y luego salta al punto de entrada del programa.
4. El punto de entrada ejecuta código de arranque, que llama a `main`.

Qué campos del archivo gobiernan cada paso es el tema del módulo 12. Por ahora recuerda que la primera instrucción que se ejecuta no es `main` y que la dirección de una función de biblioteca solo se conoce después de que el cargador se haya ejecutado.

### Modo usuario y modo núcleo

El procesador funciona en al menos dos niveles de privilegio. Los programas se ejecutan en **modo usuario** y no pueden tocar el hardware ni la memoria de otros procesos. El núcleo se ejecuta en **modo núcleo** con acceso total. Un programa pide al núcleo que actúe en su nombre con una **llamada al sistema**: una instrucción especial (`syscall` en código Linux de 64 bits, `int 0x80` en la interfaz de 32 bits del módulo 03) que cambia a modo núcleo en un punto de entrada fijo, ejecuta el servicio pedido y regresa.

### Llamadas al sistema frente a funciones de biblioteca

Los programas rara vez emiten llamadas al sistema ellos mismos. Llaman a funciones de una biblioteca, y la biblioteca emite las llamadas al sistema.

```text
your code  ->  puts()      ->  write()  ->  syscall instruction  ->  kernel
              C library        wrapper
```

En Linux la biblioteca C (libc) proporciona envoltorios finos sobre las llamadas al sistema (`write`, `open`, `read`) y servicios mayores construidos sobre ellas (`printf`, `malloc`, `fopen`). Esto da a los ingenieros inversos dos puntos de observación:

- `ltrace` muestra las llamadas a bibliotecas compartidas.
- `strace` muestra las llamadas al sistema que realmente llegan al núcleo.

Las dos vistas del mismo programa difieren: un `puts` se convierte en un `write`, mientras que un `printf` con búfer puede no producir ninguna llamada al sistema hasta que se vacía el búfer.

### Capas en Windows

Windows usa una pila de capas más profunda.

| Capa | Ejemplos | Función |
|---|---|---|
| Programa | El ejecutable | Tu código |
| API de Windows | `kernel32.dll`, `user32.dll`, `advapi32.dll` | La interfaz documentada: `CreateFileW`, `MessageBoxW` |
| API nativa | `ntdll.dll` | Funciones de nivel más bajo llamadas `Nt...` y `Zw...`; cada una es un pequeño trozo que carga un número de servicio y ejecuta `syscall` |
| Núcleo | `ntoskrnl.exe` | Los servicios en sí |

Una llamada a la API de Windows suele atravesar una o más capas antes de llegar al núcleo. Los números de llamada al sistema en Windows cambian entre versiones, por eso los programas llaman a `ntdll` en lugar de emitir la instrucción ellos mismos.

### Manejadores, descriptores y objetos

El núcleo posee los recursos: archivos, procesos, hilos, sockets. Un programa se refiere a ellos con un número pequeño que no puede interpretar por sí solo.

- En Linux el número es un **descriptor de archivo** (0 es la entrada estándar, 1 la salida estándar, 2 el error estándar). Archivos, tuberías y sockets lo usan todos.
- En Windows el número es un **manejador** (handle): un índice en una tabla por proceso de objetos del núcleo. Los manejadores se refieren a archivos, procesos, hilos, eventos, claves del registro y más, y cada tipo de objeto tiene sus propias funciones.

Cuando veas un entero pequeño pasado a una llamada, pregunta qué objeto nombra y de dónde lo obtuvo el programa. Seguir el número hasta su creación suele mostrar lo que hace una función.

## Pruébalo

Construye los ejemplos y ejecútalos desde `modules/10-os-for-reversers/examples`.

```bash
make
build/10_regions
```

El programa informa de qué región contiene una función, una constante, una global, un bloque del montón y una variable local. Salida real (no se imprimen direcciones porque cambian en cada ejecución):

```text
function main          r-xp  /lab/modules/10-os-for-reversers/examples/build/10_regions
function printf        r-xp  /usr/lib/x86_64-linux-gnu/libc.so.6
constant string        r--p  /lab/modules/10-os-for-reversers/examples/build/10_regions
global variable        rw-p  /lab/modules/10-os-for-reversers/examples/build/10_regions
heap block             rw-p  [heap]
local variable         rw-p  [stack]
```

El código es ejecutable y de solo lectura, las constantes son de solo lectura, los datos, el montón y la pila son escribibles y no ejecutables, y `printf` vive en otro archivo: la biblioteca C compartida. La cuarta letra `p` significa privada.

Ahora observa cómo el procesador hace cumplir los permisos:

```bash
build/10_perms
```

```text
page is read-write: wrote "writable"
page is read-only: the write caused a fault (SIGSEGV)
the contents are still "writable"
```

Por último, compara las dos capas:

```bash
strace -e trace=write build/10_layers
ltrace -e puts+fflush+syscall build/10_layers
```

```text
write(1, "through puts()\n", 15through puts()
)        = 15
write(1, "through syscall()\n", 18through syscall()
)     = 18
```

```text
10_layers->puts("through puts()")                = 15
10_layers->fflush(0x7d3f5d4745c0through puts()
)                = 0
10_layers->syscall(1, 1, 0x402004, 18through syscall()
)           = 18
+++ exited (status 0) +++
```

El programa hizo dos escrituras por rutas distintas. `strace` muestra ambas como la misma llamada al sistema `write`. `ltrace` muestra que la primera pasó por `puts` y la segunda por la función `syscall` de la biblioteca. La salida del propio programa aparece dentro de los trazados porque ambas van al terminal. El puntero impreso para `fflush` varía de una ejecución a otra.

Por último, lista las bibliotecas que necesita el programa y comprueba que se mapean y no forman parte del archivo:

```bash
ldd build/10_layers
```

```text
    linux-vdso.so.1 (addr)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (addr)
    /lib64/ld-linux-x86-64.so.2 (addr)
```

Aquí las direcciones se sustituyen por `(addr)` porque difieren en cada ejecución. La última línea es el cargador dinámico, y la primera es una pequeña región que proporciona el núcleo.

## Errores comunes

- **Fijar direcciones a mano.** ASLR mueve bibliotecas, pila y montón en cada ejecución. Anota desplazamientos y nombres.
- **Confundir la función de biblioteca con la llamada al sistema.** `printf` no es una llamada al sistema. Mira qué capa estás leyendo antes de sacar conclusiones.
- **Suponer que una llamada equivale a una llamada al sistema.** El almacenamiento en búfer, los reintentos y los envoltorios cambian el número.
- **Ignorar los permisos.** Una región escribible y ejecutable a la vez es inusual y merece una explicación.
- **Tratar un manejador o un descriptor como significativo por sí solo.** El número es solo un índice; lo que importa es el objeto que hay detrás.

## Resumen

- Un proceso tiene un espacio de direcciones virtual privado formado por páginas con permisos de lectura, escritura y ejecución; los hilos lo comparten y cada uno tiene su propia pila.
- El núcleo carga un programa, normalmente con ayuda de un cargador dinámico, así que la primera instrucción que se ejecuta no es `main`.
- El código en modo usuario llega al núcleo solo mediante llamadas al sistema; las bibliotecas las envuelven.
- `ltrace` muestra llamadas a bibliotecas y `strace` llamadas al sistema; en Windows hay más capas: API de Windows, API nativa, núcleo.
- Los recursos se nombran con descriptores de archivo (Linux) y manejadores (Windows).

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 11, la caja de herramientas del ingeniero inverso.
- Ejercicios que usan este módulo: [Ejercicio 11](../../exercises/11-portable-puzzle/README.es.md).
