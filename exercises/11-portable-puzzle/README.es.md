<!-- source: README.md @ sha256:306f7c5d94e2 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 11. Rompecabezas portable

Level 3. Tiempo: unas 4 horas. Arquitectura: x86-64 Windows (bajo Wine). Módulos: [08. La pila, los procedimientos y las convenciones de llamada](../../modules/08-stack-and-calls/README.es.md), [10. Fundamentos de sistemas operativos para ingenieros inversos](../../modules/10-os-for-reversers/README.es.md), [12. Formatos ejecutables: ELF y PE](../../modules/12-executable-formats/README.es.md)

## Escenario

Una herramienta de consola de Windows, `puzzle.exe`, llega sin documentación. Un equipo de empaquetado necesita algunos datos de sus cabeceras, y además quiere una pequeña biblioteca auxiliar, escrita en ensamblador para que no tenga dependencias en tiempo de ejecución, que averigüe dónde está en el archivo de disco una dirección dada de una imagen PE cargada. Primero lees tú mismo las cabeceras. Después escribes la biblioteca auxiliar.

## Objetivos

- Analizar las cabeceras, la tabla de secciones y el directorio de importaciones de un archivo PE32+.
- Convertir una dirección virtual relativa (RVA) en un desplazamiento en el archivo.
- Escribir funciones de Windows x64 que respeten la convención de llamada de Microsoft: registros de argumentos, espacio de sombra, alineación de la pila y registros no volátiles.

## Parte 1: Reverse

El binario es `bin/puzzle.exe`. Es un programa de consola que imprime una línea; puedes ejecutarlo bajo Wine.

```bash
wine bin/puzzle.exe
x86_64-w64-mingw32-objdump -p bin/puzzle.exe
python3 ../../modules/12-executable-formats/examples/12_pe_dump.py bin/puzzle.exe
```

Encuentra estos seis valores:

1. `S`: el número de secciones, en decimal.
2. `I`: la base de la imagen, en hexadecimal en minúsculas sin `0x`.
3. `R`: la RVA del punto de entrada, en hexadecimal en minúsculas sin `0x`.
4. `O`: el desplazamiento en el archivo del byte situado en la RVA `0x12348`, en hexadecimal en minúsculas sin `0x`.
5. `D`: el número de bibliotecas del directorio de importaciones, en decimal.
6. `K`: el número de funciones importadas de `KERNEL32.dll`, en decimal.

```text
ARL{portable_<S>_<I>_<R>_<O>_<D>_<K>}
```

Por ejemplo, los valores 5, 140000000, 1410, 7a30, 2 y 9 darían `ARL{portable_5_140000000_1410_7a30_2_9}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para implementar dos funciones en la convención de llamada Microsoft x64.

```c
#include <stdint.h>

uint32_t pe_rva_to_offset(const void *sections, uint32_t count, uint32_t rva);
uint32_t pe_entry_offset(const void *file);

/* Provided by the tests. */
void log_rva(uint32_t rva);
```

- `pe_rva_to_offset` recibe un puntero a una tabla de secciones de `count` entradas y una RVA. Devuelve el desplazamiento en el archivo de esa RVA, o `0xFFFFFFFF` si ninguna sección contiene la RVA en sus datos en bruto. Una sección contiene las RVA desde su dirección virtual hasta, sin incluirla, su dirección virtual más su tamaño de datos en bruto.
- `pe_entry_offset` recibe un puntero al inicio de un archivo PE en memoria. Si el archivo no empieza por `MZ`, o las cabeceras NT no empiezan por `PE\0\0`, devuelve `0xFFFFFFFF`. En otro caso lee la RVA del punto de entrada de la cabecera opcional y llama una vez a `log_rva` con esa RVA. Después devuelve el desplazamiento en el archivo del punto de entrada, calculado con la tabla de secciones que sigue a la cabecera opcional. Si el punto de entrada no está en ninguna sección, devuelve `0xFFFFFFFF`.

Necesitas la disposición de las cabeceras y de la tabla de secciones; en la Parte 1 la encuentras.

## Reglas

- Sigue la convención Microsoft x64: argumentos en `rcx`, `rdx`, `r8` y `r9`; 32 bytes de espacio de sombra en cada llamada; `rsp` múltiplo de 16 en cada `call`.
- Conserva `rbx`, `rbp`, `rdi`, `rsi` y de `r12` a `r15`. Ten en cuenta que `rdi` y `rsi` no son volátiles en Windows.
- `log_rva` destruye todos los registros volátiles y escribe en su espacio de sombra. Guarda lo que necesites tras la llamada en registros no volátiles o en tu propio marco de pila.
- No llames a ninguna otra función.
- Los resultados son de 32 bits y se desbordan como números sin signo.

## Comprueba tu trabajo

```bash
make check EX=11 ARGS='--flag ARL{your_answer_here}'
```

El verificador construye un programa de prueba con el compilador cruzado MinGW, lo enlaza con tu archivo objeto y lo ejecuta bajo Wine. Informa de la Parte 1 y la Parte 2 por separado. Sin `--flag`, solo comprueba la Parte 2. Cada prueba de la Parte 2 imprime un resultado seguido de `ok` o de una lista de lo que salió mal: `align`, `regs` o `calls`.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
