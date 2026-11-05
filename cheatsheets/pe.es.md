<!-- source: pe.md @ sha256:83be44b0ddeb -->
[English](pe.md) | [العربية](pe.ar.md) | [Deutsch](pe.de.md) | [Español](pe.es.md) | [Français](pe.fr.md)

# PE

El formato ejecutable de Windows. Lección: [12. Formatos ejecutables: ELF y PE](../modules/12-executable-formats/README.es.md). Los desplazamientos de abajo se comprobaron con un ejecutable MinGW de 64 bits.

## Estructura de un archivo

| Parte | Dónde | Tamaño |
|---|---|---|
| Cabecera DOS | Desplazamiento 0, empieza con `MZ` | 64 bytes; el campo en 0x3c es `e_lfanew` |
| Fragmento DOS | Después de la cabecera DOS | Variable |
| Firma NT | En `e_lfanew` | 4 bytes: `PE\0\0` |
| Cabecera de archivo | Justo después de la firma | 20 bytes |
| Cabecera opcional | Justo después de la cabecera de archivo | 240 bytes en PE32+, 224 en PE32 |
| Tabla de secciones | Justo después de la cabecera opcional | 40 bytes por sección |
| Datos de las secciones | En los desplazamientos en bruto | Alineados a la alineación de archivo |

## Cabecera de archivo

| Desplazamiento | Tamaño | Campo | Notas |
|---|---|---|---|
| 0 | 2 | Máquina | 0x8664 x86-64, 0x14c x86 |
| 2 | 2 | Número de secciones | |
| 16 | 2 | Tamaño de la cabecera opcional | Dónde empieza la tabla de secciones |
| 18 | 2 | Características | 0x2000 marca una DLL |

## Cabecera opcional (desplazamientos desde su inicio)

| Desplazamiento | Tamaño | Campo | Notas |
|---|---|---|---|
| 0 | 2 | Número mágico | 0x10b PE32, 0x20b PE32+ |
| 16 | 4 | Punto de entrada | Una RVA |
| 24 | 8 | Base de la imagen | PE32+. En PE32 la base de la imagen es un campo de 4 bytes en el desplazamiento 28 |
| 32 | 4 | Alineación de sección | En memoria, normalmente 0x1000 |
| 36 | 4 | Alineación de archivo | En el archivo, normalmente 0x200 |
| 56 | 4 | Tamaño de la imagen | Tamaño total en memoria, múltiplo de la alineación de sección |
| 60 | 4 | Tamaño de las cabeceras | Redondeado hacia arriba a la alineación de archivo |
| 108 | 4 | Número de directorios de datos | Normalmente 16 (PE32+) |
| 112 | 8 cada uno | Directorios de datos | Pares `(RVA, size)` (PE32+). En PE32 empiezan en el desplazamiento 96 |

## Directorios de datos

| Índice | Directorio |
|---|---|
| 0 | Exportación |
| 1 | Importación |
| 2 | Recurso |
| 3 | Excepción (`.pdata`, 64 bits) |
| 4 | Certificado (un desplazamiento en el archivo, no una RVA) |
| 5 | Reubicación base |
| 6 | Depuración |
| 9 | Almacenamiento local de hilo |
| 12 | Tabla de direcciones de importación |
| 13 | Importación diferida |

## Cabecera de sección (40 bytes)

| Desplazamiento | Tamaño | Campo |
|---|---|---|
| 0 | 8 | Nombre, rellenado con ceros |
| 8 | 4 | Tamaño virtual |
| 12 | 4 | RVA |
| 16 | 4 | Tamaño en bruto |
| 20 | 4 | Desplazamiento en bruto |
| 36 | 4 | Características |

Bits de las características: 0x20 código, 0x40 datos inicializados, 0x80 datos sin inicializar, 0x20000000 ejecutable, 0x40000000 legible, 0x80000000 escribible.

## Secciones habituales

| Sección | Contenido |
|---|---|
| `.text` | Código |
| `.rdata` | Datos de solo lectura, nombres de importación y a menudo el directorio de importación |
| `.data`, `.bss` | Datos escribibles inicializados y inicializados a cero |
| `.idata` | Directorio de importación y tablas (MinGW) |
| `.edata` | Directorio de exportación |
| `.pdata`, `.xdata` | Datos de desenrollado y de excepciones (64 bits) |
| `.reloc` | Reubicaciones base |
| `.rsrc` | Recursos |
| `.tls` | Plantilla de almacenamiento local de hilo |

## Conversiones

- De RVA a desplazamiento en el archivo: busca la sección con `section RVA <= RVA < section RVA + virtual size` y calcula `offset = RVA - section RVA + section raw offset`.
- Dirección virtual: `image base + RVA`. Con ASLR el cargador puede elegir otra base, y la tabla de reubicaciones corrige las direcciones absolutas.
- El tamaño virtual de una sección puede ser mayor o menor que su tamaño en bruto. La parte posterior a los datos en bruto se rellena con ceros.

## Importaciones

Un array de descriptores, uno por DLL, termina con un descriptor todo a cero. Cada uno apunta al nombre de la DLL, a la tabla de nombres de importación (lo que se pide) y a la tabla de direcciones de importación (donde el cargador escribe las direcciones). Las llamadas del código pasan por la IAT, como `call [rip + disp]`.

## Órdenes

```bash
file prog.exe                                  # kind, bits
x86_64-w64-mingw32-objdump -x prog.exe         # headers, sections, imports
x86_64-w64-mingw32-objdump -d -M intel prog.exe
x86_64-w64-mingw32-objdump -p prog.exe         # import tables and data directories
strings -a -t x prog.exe
python3 modules/12-executable-formats/examples/12_pe_dump.py prog.exe
```
