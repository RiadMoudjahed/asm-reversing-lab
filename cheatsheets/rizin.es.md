<!-- source: rizin.md @ sha256:ae965ed6a7c5 -->
[English](rizin.md) | [العربية](rizin.ar.md) | [Deutsch](rizin.de.md) | [Español](rizin.es.md) | [Français](rizin.fr.md)

# Rizin

Un desensamblador y marco de análisis de línea de órdenes. Lección: [11. La caja de herramientas del ingeniero inverso](../modules/11-toolkit/README.es.md). Las órdenes de abajo se ejecutaron en la imagen del laboratorio.

## Arranque

| Línea de órdenes | Efecto |
|---|---|
| `rizin prog` | Abrir de forma interactiva |
| `rizin -A prog` | Abrir y analizarlo todo |
| `rizin -q -e scr.color=0 -A -c "CMDS" prog` | Ejecutar órdenes separadas por `;`, imprimir texto plano y salir |
| `rizin -d prog` | Abrir bajo el depurador |

## Patrón de las órdenes

Las órdenes son palabras cortas formadas por una letra de familia y un verbo. Añade `?` para ver la ayuda de un prefijo: `a?`, `p?`, `i?`. Añade `@ ADDR` o `@ sym.name` para ejecutar una orden en otro lugar, y `~TEXT` para conservar solo las líneas que contienen `TEXT`.

## Análisis

| Orden | Significado |
|---|---|
| `aaa` | Analizarlo todo (la opción `-A` lo hace al abrir) |
| `afl` | Listar funciones con dirección, número de bloques y tamaño |
| `afvl @ sym.check` | Listar los argumentos y las variables locales de una función |
| `axt @ sym.check` | Quién hace referencia a esta dirección (llamadores) |
| `afn NAME @ ADDR` | Renombrar una función |

## Impresión

| Orden | Significado |
|---|---|
| `pdf @ sym.check` | Desensamblar una función entera |
| `pd 3 @ ADDR` | Desensamblar 3 instrucciones |
| `px 16 @ 0x402038` | Volcado hexadecimal de 16 bytes |
| `ps ascii @ ADDR` | Imprimir una cadena ASCII terminada en cero (el `ps` simple adivina la codificación y puede imprimir basura) |
| `s ADDR` | Mover la posición actual |

## Información

| Orden | Significado |
|---|---|
| `iI` | Resumen del binario: formato, bits, protecciones |
| `iS` | Secciones con desplazamientos, direcciones y permisos |
| `is` | Símbolos (`is~check` filtra) |
| `ii` | Importaciones |
| `iz` | Cadenas en las secciones de datos |
| `izz` | Cadenas en todo el archivo |
| `ie` | Puntos de entrada (programa, init, fini) |

## Consejos sobre la salida

- `scr.color=0` quita los códigos de color cuando copias la salida a un archivo.
- `-A` tarda en archivos grandes. Para un vistazo rápido, abre sin ella y ejecuta `af` en una dirección.
- Rizin nombra las cosas con prefijos: `sym.` funciones, `obj.` datos, `sym.imp.` importaciones, `str.` cadenas.
