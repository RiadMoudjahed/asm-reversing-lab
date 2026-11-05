<!-- source: ghidra.md @ sha256:2b16cc1b1f48 -->
[English](ghidra.md) | [العربية](ghidra.ar.md) | [Deutsch](ghidra.de.md) | [Español](ghidra.es.md) | [Français](ghidra.fr.md)

# Ghidra

Una suite gráfica de ingeniería inversa con desensamblador y descompilador. Se ejecuta en tu anfitrión, no en la imagen del laboratorio. Lección: [11. La caja de herramientas del ingeniero inverso](../modules/11-toolkit/README.es.md). Los atajos son los predeterminados; pueden diferir entre versiones y configuraciones, y los menús muestran los actuales.

## Flujo de trabajo

1. Crea un proyecto e importa el binario desde `exercises/NN-name/bin/`. Acepta el formato y el lenguaje detectados.
2. Abre el archivo en el navegador de código y ejecuta el análisis predeterminado.
3. Busca el punto de entrada, `main`, las cadenas y las importaciones.
4. Lee el listado y el descompilador uno junto al otro.
5. Renombra, cambia tipos y comenta a medida que aprendes. Cada cambio facilita leer la siguiente función.
6. Guarda el proyecto antes de cerrarlo.

## Ventanas del navegador de código

| Ventana | Uso |
|---|---|
| Listado | Desensamblado con direcciones, bytes, etiquetas y comentarios |
| Descompilador | C aproximado de la función bajo el cursor |
| Árbol de símbolos | Funciones, etiquetas, importaciones, exportaciones |
| Cadenas definidas | Cadenas encontradas en el archivo; doble clic para saltar |
| Grafo de funciones | Bloques básicos y saltos de una función |
| Gestor de tipos de datos | Estructuras, enumeraciones y typedefs integrados y propios |

## Teclas predeterminadas

| Tecla | Acción |
|---|---|
| `G` | Ir a una dirección o a una etiqueta |
| `L` | Renombrar la etiqueta, función o variable bajo el cursor |
| `Ctrl+L` | Cambiar el tipo de una variable en el descompilador |
| `;` | Añadir un comentario |
| `X` | Mostrar las referencias al elemento bajo el cursor |
| `D` | Desensamblar desde el cursor |
| `F` | Crear una función en el cursor |
| `T` | Fijar el tipo de datos del elemento en el listado |

## Recuperar estructuras

1. En el descompilador, busca accesos repetidos como `*(int *)(p + 0x10)` sobre un mismo puntero. Son los campos de una estructura.
2. En el gestor de tipos de datos, crea una estructura con un campo en cada desplazamiento que hayas encontrado y un tamaño que coincida con la reserva de memoria.
3. Cambia el tipo de la variable puntero a un puntero a esa estructura (`Ctrl+L`).
4. El descompilador escribe ahora `p->key` en lugar del desplazamiento. Renombra los campos cuando su significado se aclare.

## Cuando el descompilador parece equivocado

- Compáralo con las instrucciones. Los tipos, las operaciones con o sin signo y la anchura de un campo pueden adivinarse mal.
- Comprueba la convención de llamada de la función. Una incorrecta oculta o inventa argumentos.
- Comprueba que los límites de las funciones son correctos. Una `F` omitida o un byte de datos tratado como código produce disparates.
- Usa el listado como referencia de la verdad.

## Anfitrión y laboratorio

Ghidra lee archivos con las rutas de tu anfitrión, por ejemplo `~/asm-reversing-lab/exercises/01-signal-flare/bin/`, no `/lab/...`. Usa la imagen del laboratorio para `objdump`, `rizin` y `gdb`.
