<!-- source: elf.md @ sha256:1621c3a969bd -->
[English](elf.md) | [العربية](elf.ar.md) | [Deutsch](elf.de.md) | [Español](elf.es.md) | [Français](elf.fr.md)

# ELF

El formato ejecutable de Linux. Lección: [12. Formatos ejecutables: ELF y PE](../modules/12-executable-formats/README.es.md).

## Estructura de un archivo

| Parte | Dónde | Lo usa |
|---|---|---|
| Cabecera ELF | Desplazamiento 0, 64 bytes en un archivo de 64 bits | Todo |
| Tabla de cabeceras de programa | En `e_phoff`, normalmente 64 | El cargador |
| Datos de las secciones | Entre las tablas | Ambos |
| Tabla de cabeceras de sección | En `e_shoff`, normalmente al final | Enlazador, depurador, ingeniero inverso (no hace falta para ejecutar) |

El cargador lee solo las cabeceras de programa. Un archivo sin cabeceras de sección se ejecuta igualmente.

## Campos de la cabecera ELF (64 bits)

| Desplazamiento | Tamaño | Campo | Significado |
|---|---|---|---|
| 0 | 4 | Número mágico de `e_ident` | `7f 45 4c 46` (`\x7fELF`) |
| 4 | 1 | Clase | 1 = 32 bits, 2 = 64 bits |
| 5 | 1 | Datos | 1 = little endian, 2 = big endian |
| 16 | 2 | `e_type` | 2 = EXEC, 3 = DYN (objeto compartido o ejecutable independiente de la posición) |
| 18 | 2 | `e_machine` | 3 = x86, 62 = x86-64 |
| 24 | 8 | `e_entry` | Dirección virtual de la primera instrucción |
| 32 | 8 | `e_phoff` | Desplazamiento en el archivo de la tabla de cabeceras de programa |
| 40 | 8 | `e_shoff` | Desplazamiento en el archivo de la tabla de cabeceras de sección |
| 54 | 2 | `e_phentsize` | Tamaño de una cabecera de programa (56) |
| 56 | 2 | `e_phnum` | Número de cabeceras de programa |
| 58 | 2 | `e_shentsize` | Tamaño de una cabecera de sección (64) |
| 60 | 2 | `e_shnum` | Número de cabeceras de sección |
| 62 | 2 | `e_shstrndx` | Índice de la sección que contiene los nombres de sección |

## Tipos de cabecera de programa

| Tipo | Significado |
|---|---|
| `PT_LOAD` | Un rango del archivo que se mapea: desplazamiento en el archivo, dirección virtual, tamaño en archivo, tamaño en memoria, permisos |
| `PT_INTERP` | Ruta del cargador dinámico |
| `PT_DYNAMIC` | La sección dinámica: bibliotecas necesarias, tablas de reubicación |
| `PT_NOTE` | Notas, como el identificador de compilación |
| `PT_GNU_STACK` | Permisos de la pila |
| `PT_GNU_RELRO` | Rango que pasa a solo lectura tras la reubicación |

Un segmento cuyo tamaño en memoria supera su tamaño en archivo se rellena con ceros más allá de los datos del archivo. Así `.bss` no ocupa espacio en el archivo.

## Secciones habituales

| Sección | Contenido |
|---|---|
| `.text` | Código |
| `.rodata` | Datos de solo lectura, literales de cadena |
| `.data` | Datos inicializados y escribibles |
| `.bss` | Datos inicializados a cero, sin bytes en el archivo |
| `.plt`, `.plt.sec` | Fragmentos que saltan a través de la GOT |
| `.got`, `.got.plt` | Direcciones que rellena el cargador dinámico |
| `.dynsym`, `.dynstr` | Símbolos y nombres necesarios al cargar |
| `.symtab`, `.strtab` | Tabla de símbolos completa; la elimina `strip` |
| `.rela.dyn`, `.rela.plt` | Entradas de reubicación |
| `.init_array`, `.fini_array` | Funciones que se ejecutan antes de `main` y al salir |
| `.eh_frame` | Información para desenrollar la pila |

## Llamadas dinámicas

Una llamada a una función de biblioteca va a un fragmento en la PLT. El fragmento salta a través de una entrada de la GOT. Con enlace diferido, la primera llamada cae en el cargador dinámico, que resuelve el nombre y escribe la dirección real en la entrada de la GOT; las llamadas posteriores van directamente allí. Un binario enlazado con `-z now` lo resuelve todo al arrancar.

## Órdenes

```bash
file prog                    # kind, bits, stripped or not
readelf -h prog              # ELF header
readelf -lW prog             # program headers and the section-to-segment map
readelf -SW prog             # section headers
readelf -sW prog             # symbol tables
readelf -dW prog             # dynamic section (needed libraries)
readelf -rW prog             # relocations
objdump -d -M intel prog     # disassemble code sections
objdump -s -j .rodata prog   # hex dump of one section
nm -D prog                   # dynamic symbols
ldd prog                     # libraries the loader would bring in
strings -a -t x prog         # printable strings with file offsets
```

## Conversiones

- Desplazamiento en el archivo de una dirección dentro de un segmento `PT_LOAD`: `offset = address - p_vaddr + p_offset`.
- Comprueba el segmento, no la sección, cuando no coinciden. El cargador usa segmentos.
- Un ejecutable independiente de la posición (`e_type` 3) se carga en una base aleatoria. Las direcciones del archivo son relativas a esa base.
