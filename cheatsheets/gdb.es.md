<!-- source: gdb.md @ sha256:5d1d9c061ac4 -->
[English](gdb.md) | [العربية](gdb.ar.md) | [Deutsch](gdb.de.md) | [Español](gdb.es.md) | [Français](gdb.fr.md)

# GDB

El depurador de GNU. Lección: [11. La caja de herramientas del ingeniero inverso](../modules/11-toolkit/README.es.md). Cada orden de abajo se ejecutó en la imagen del laboratorio.

## Arranque

| Línea de órdenes | Efecto |
|---|---|
| `gdb prog` | Cargar un programa |
| `gdb --args prog a b` | Cargarlo con argumentos |
| `gdb -nx -q -batch -ex "CMD" prog` | Ejecutar órdenes sin complementos e imprimir texto plano (para notas y scripts) |
| `gdb -p PID` | Engancharse a un proceso en ejecución |

Dentro de GDB, `run ARGS` inicia el programa y `start` para al principio de `main` cuando hay símbolos. `quit` sale.

## Puntos de interrupción y de observación

| Orden | Significado |
|---|---|
| `break check`, `b *0x401184` | Parar en una función o en una dirección exacta |
| `tbreak check` | Parar una vez y luego borrar el punto de interrupción |
| `info breakpoints` | Listar los puntos de interrupción y sus aciertos |
| `delete N`, `disable N`, `enable N` | Quitar, desactivar o activar el punto de interrupción `N` |
| `condition N EXPR` | Parar en el punto de interrupción `N` solo cuando `EXPR` es verdadera |
| `watch EXPR`, `watch *(int *)$rsp` | Parar cuando cambia un valor |
| `rwatch EXPR`, `awatch EXPR` | Parar en una lectura, o en una lectura o escritura |

Un programa sin símbolos no tiene nombres de función. Usa la dirección (`b *0x401184`) o el punto de entrada (`info files`, luego `b *ADDR`).

## Ejecución

| Orden | Significado |
|---|---|
| `continue` | Reanudar |
| `stepi`, `nexti` | Una instrucción, entrando en las llamadas o saltándolas |
| `step`, `next` | Una línea de código fuente (necesita información de depuración) |
| `finish` | Ejecutar hasta que la función actual retorne |
| `until ADDR` | Ejecutar hasta alcanzar una dirección |
| `call EXPR` | Llamar a una función del programa |
| `set var X = V`, `set $rax = 0` | Cambiar una variable o un registro |
| `jump *ADDR` | Continuar en otra dirección |

## Ver el estado

| Orden | Significado |
|---|---|
| `info registers`, `info registers rdi` | Todos los registros, o los indicados |
| `print EXPR`, `p/x $rsp` | Evaluar; `/x` hexadecimal, `/d` decimal, `/t` binario, `/c` carácter |
| `x/NFU ADDR` | Examinar memoria: `N` unidades, formato `F`, tamaño de unidad `U` |
| `bt` | Pila de llamadas |
| `info frame` | Marco en la parada actual |
| `info proc mappings` | Mapa de memoria del proceso |
| `info functions REGEX` | Funciones que coinciden con un patrón |
| `disassemble check` | Desensamblar una función |
| `display/i $pc` | Mostrar la siguiente instrucción en cada parada |

### Formatos y unidades de `x`

| Letra | Significado |
|---|---|
| `x`, `d`, `u`, `o`, `t` | Hexadecimal, con signo, sin signo, octal, binario |
| `c`, `s` | Caracteres, cadena |
| `i` | Instrucciones |
| `b`, `h`, `w`, `g` | Tamaño de unidad: 1, 2, 4, 8 bytes |

Ejemplos: `x/4gx $rsp` cuatro palabras de 8 bytes en hexadecimal, `x/s $rdi` la cadena en `rdi`, `x/5i $pc` cinco instrucciones desde la actual.

## Ajustes que conviene conocer

| Orden | Significado |
|---|---|
| `set disassembly-flavor intel` | Sintaxis Intel (la imagen del laboratorio la activa) |
| `set args A B` | Argumentos para el siguiente `run` |
| `set disable-randomization on` | Direcciones fijas (el valor por omisión fuera de contenedores) |
| `set logging enabled on` | Copiar la salida a `gdb.txt` |
| `set confirm off` | Sin preguntas de confirmación |

## GEF

La imagen del laboratorio carga GEF, que imprime los registros, la pila y el código tras cada parada. Órdenes útiles:

| Orden | Significado |
|---|---|
| `context` | Redibujar la vista de registros, pila y código |
| `vmmap` | Mapa de memoria con permisos |
| `checksec` | Protecciones del archivo cargado |
| `telescope $rsp 8` | Palabras de la pila con los valores a los que apuntan |
| `search-pattern TEXT` | Buscar bytes en memoria |

Usa `gdb -nx` cuando necesites una salida plana.

## Recordatorios

- La dirección de pila en `rdi` o `rsp` cambia entre ejecuciones. No la copies en tus notas como un hecho.
- Tras `finish`, el resultado está en `rax`.
- Dentro de un contenedor, el aviso "Error disabling address space randomization" es normal.
