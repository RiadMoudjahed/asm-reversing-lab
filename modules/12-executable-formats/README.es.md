<!-- source: README.md @ sha256:ea5aa64cd376 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 12. Formatos ejecutables: ELF y PE

Tiempo: aproximadamente 3 horas. Requisitos previos: [11. La caja de herramientas del ingeniero inverso](../11-toolkit/README.es.md)

## Por qué importa

Todo ejecutable es un archivo que le dice al cargador qué colocar en memoria, dónde, con qué permisos, qué bibliotecas traer y por dónde empezar. Si sabes leer esas cabeceras puedes identificar un binario de un vistazo, encontrar su código y sus datos sin un desensamblador, entender importaciones y exportaciones y detectar un archivo construido o modificado de forma inusual. Linux usa ELF y Windows usa PE; este módulo cubre ambos y termina escribiendo un ejecutable completo a mano.

## Conceptos

### Dos vistas de un archivo

Ambos formatos describen el mismo archivo de dos maneras.

- La **vista de carga** indica cómo mapear el archivo en memoria: qué rangos de bytes, en qué direcciones, con qué permisos. El cargador usa solo esta vista.
- La **vista de enlace** divide el archivo en partes con nombre (secciones) para el enlazador, el depurador y el ingeniero inverso.

Mantén separados dos tipos de dirección. Un **desplazamiento en el archivo** cuenta bytes desde el inicio del archivo. Una **dirección virtual** es donde estarán esos bytes en memoria. Difieren porque el cargador alinea las partes a límites de página y porque algunas partes, como los datos sin inicializar, ocupan memoria pero no espacio en el archivo. Convertir entre ambas es una tarea diaria.

### ELF

Un archivo ELF empieza con la **cabecera ELF**, seguida de la tabla de cabeceras de programa, los datos de las secciones y la tabla de cabeceras de sección.

#### La cabecera ELF

Salida real del programa de ejemplo construido a partir de `examples/12_sample.c`, ejecutado desde `modules/12-executable-formats/examples`:

```bash
make
readelf -h build/12_sample
```

```text
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x401050
  Start of program headers:          64 (bytes into file)
  Start of section headers:          14008 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         13
  Size of section headers:           64 (bytes)
  Number of section headers:         30
```

| Desplazamiento | Tamaño | Campo | Significado |
|---|---|---|---|
| 0x00 | 4 | número mágico | `7f 45 4c 46`, los bytes `.ELF` |
| 0x04 | 1 | clase | 1 para 32 bits, 2 para 64 bits |
| 0x05 | 1 | datos | 1 para little-endian, 2 para big-endian |
| 0x10 | 2 | tipo | 2 ejecutable, 3 objeto compartido (también ejecutables independientes de la posición) |
| 0x12 | 2 | máquina | 0x3e para x86-64, 3 para x86 |
| 0x18 | 8 | entrada | dirección virtual de la primera instrucción |
| 0x20 | 8 | desplazamiento de las cabeceras de programa | desplazamiento en el archivo de la tabla de segmentos |
| 0x28 | 8 | desplazamiento de las cabeceras de sección | desplazamiento en el archivo de la tabla de secciones |
| 0x36 | 2 | tamaño de una entrada de cabecera de programa | 56 en archivos de 64 bits |
| 0x38 | 2 | número de cabeceras de programa | número de segmentos |
| 0x3a | 2 | tamaño de una entrada de cabecera de sección | 64 en archivos de 64 bits |
| 0x3c | 2 | número de cabeceras de sección | número de secciones |

Los desplazamientos corresponden al formato de 64 bits. La cabecera de 32 bits es más pequeña porque las direcciones y los desplazamientos ocupan 4 bytes.

#### Cabeceras de programa: la vista de carga

```bash
readelf -l -W build/12_sample
```

```text
Program Headers:
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  PHDR           0x000040 0x0000000000400040 0x0000000000400040 0x0002d8 0x0002d8 R   0x8
  INTERP         0x000318 0x0000000000400318 0x0000000000400318 0x00001c 0x00001c R   0x1
      [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]
  LOAD           0x000000 0x0000000000400000 0x0000000000400000 0x000548 0x000548 R   0x1000
  LOAD           0x001000 0x0000000000401000 0x0000000000401000 0x000179 0x000179 R E 0x1000
  LOAD           0x002000 0x0000000000402000 0x0000000000402000 0x0000c4 0x0000c4 R   0x1000
  LOAD           0x002df8 0x0000000000403df8 0x0000000000403df8 0x00022c 0x000230 RW  0x1000
  DYNAMIC        0x002e08 0x0000000000403e08 0x0000000000403e08 0x0001d0 0x0001d0 RW  0x8
  GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x10
  GNU_RELRO      0x002df8 0x0000000000403df8 0x0000000000403df8 0x000208 0x000208 R   0x1
```

El listado está abreviado a las entradas que importan aquí. Cada entrada `LOAD` le dice al cargador que mapee `FileSiz` bytes desde el desplazamiento `Offset` del archivo en la dirección `VirtAddr`, con los permisos de `Flg`, y que amplíe el mapeo con ceros hasta `MemSiz`. Hay cuatro: una parte de solo lectura con las cabeceras, una parte que se puede leer y ejecutar (el código), una parte de solo lectura (constantes) y una parte de lectura y escritura (datos). En la última `MemSiz` es 4 bytes mayor que `FileSiz`, y eso es el `.bss`: memoria que vale cero al inicio y no necesita espacio en el archivo.

| Tipo | Significado |
|---|---|
| `LOAD` | Un rango que se mapea en memoria |
| `INTERP` | La ruta del cargador dinámico que se inicia primero |
| `DYNAMIC` | La tabla que lee el cargador dinámico: bibliotecas necesarias, tablas de símbolos, reubicaciones |
| `GNU_STACK` | Permisos de la pila; aquí lectura y escritura sin ejecución |
| `GNU_RELRO` | Un rango que pasa a solo lectura una vez aplicadas las reubicaciones |

#### Secciones: la vista de enlace

```bash
readelf -S -W build/12_sample
```

```text
  [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
  [12] .init             PROGBITS        0000000000401000 001000 00001b 00  AX  0   0  4
  [13] .plt              PROGBITS        0000000000401020 001020 000030 10  AX  0   0 16
  [14] .text             PROGBITS        0000000000401050 001050 00011c 00  AX  0   0 16
  [15] .fini             PROGBITS        000000000040116c 00116c 00000d 00  AX  0   0  4
  [16] .rodata           PROGBITS        0000000000402000 002000 00000e 00   A  0   0  4
  [21] .dynamic          DYNAMIC         0000000000403e08 002e08 0001d0 10  WA  7   0  8
  [22] .got              PROGBITS        0000000000403fd8 002fd8 000010 08  WA  0   0  8
  [23] .got.plt          PROGBITS        0000000000403fe8 002fe8 000028 08  WA  0   0  8
  [24] .data             PROGBITS        0000000000404010 003010 000014 00  WA  0   0  8
  [25] .bss              NOBITS          0000000000404024 003024 000004 00  WA  0   0  1
```

Este listado también está abreviado. Las secciones caben dentro de los segmentos: `.init`, `.plt`, `.text` y `.fini` están en el `LOAD` ejecutable; `.rodata` en el de solo lectura; las secciones escribibles forman el último. Las secciones importan a las herramientas; el cargador las ignora, por lo que un archivo puede ejecutarse con la tabla de secciones eliminada o dañada.

| Sección | Contenido |
|---|---|
| `.text` | El código del programa |
| `.rodata` | Constantes y literales de cadena |
| `.data` | Datos escribibles inicializados |
| `.bss` | Datos inicializados a cero, sin espacio en el archivo |
| `.plt`, `.got`, `.got.plt` | Código de enlace para llamar a funciones de bibliotecas compartidas |
| `.dynsym`, `.dynstr` | Símbolos importados y exportados mediante el cargador dinámico |
| `.symtab`, `.strtab` | La tabla de símbolos completa, presente salvo que el archivo esté sin símbolos (stripped) |
| `.dynamic` | La tabla dinámica |

#### Importaciones: PLT, GOT y reubicaciones

El programa llama a `puts` y `printf` de la biblioteca C, cuyas direcciones se desconocen hasta el momento de la carga. Por eso el enlazador genera para cada una un trozo de la **tabla de enlace de procedimientos** (procedure linkage table, PLT) y una entrada en la **tabla de desplazamientos global** (global offset table, GOT) por la que salta el trozo.

```bash
readelf -r build/12_sample
```

```text
Relocation section '.rela.dyn' at offset 0x4e8 contains 2 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000403fd8  000100000006 R_X86_64_GLOB_DAT 0000000000000000 __libc_start_main@GLIBC_2.34 + 0
000000403fe0  000300000006 R_X86_64_GLOB_DAT 0000000000000000 __gmon_start__ + 0

Relocation section '.rela.plt' at offset 0x518 contains 2 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000404000  000200000007 R_X86_64_JUMP_SLO 0000000000000000 puts@GLIBC_2.2.5 + 0
000000404008  000400000007 R_X86_64_JUMP_SLO 0000000000000000 __printf_chk@GLIBC_2.3.4 + 0
```

Una **reubicación** es una instrucción al cargador: "escribe la dirección de este símbolo en esta posición". Las entradas `JUMP_SLOT` indican que las posiciones 0x404000 y 0x404008 recibirán las direcciones de `puts` y `__printf_chk`. En `main` el código llama al trozo, no a la biblioteca:

```bash
objdump -d -M intel --no-show-raw-insn -j .plt build/12_sample
```

```text
0000000000401020 <puts@plt-0x10>:
  401020:    push   QWORD PTR [rip+0x2fca]        # 403ff0 <_GLOBAL_OFFSET_TABLE_+0x8>
  401026:    jmp    QWORD PTR [rip+0x2fcc]        # 403ff8 <_GLOBAL_OFFSET_TABLE_+0x10>
  40102c:    nop    DWORD PTR [rax+0x0]

0000000000401030 <puts@plt>:
  401030:    jmp    QWORD PTR [rip+0x2fca]        # 404000 <puts@GLIBC_2.2.5>
  401036:    push   0x0
  40103b:    jmp    401020 <_init+0x20>
```

Antes de la primera llamada, la posición 0x404000 contiene 0x401036, la dirección del `push` situado justo después del `jmp`. Así, la primera llamada a `puts` cae en `push 0` (el índice de la reubicación) y salta al trozo compartido en 0x401020, que entra en el resolvedor del cargador dinámico. El resolvedor busca `puts`, escribe su dirección en la posición y salta a ella. Cada llamada posterior salta directamente a `puts`. Esto es el **enlace perezoso** (lazy binding). Muchos sistemas construyen en cambio con enlace inmediato, que lo resuelve todo al inicio, así que en otros archivos puedes ver un patrón distinto; los trozos y la tabla son los mismos.

Para un ingeniero inverso la consecuencia es sencilla: una llamada a `foo@plt` es una llamada a la función de biblioteca `foo`, y las tablas `.rela.plt` y `.dynsym` enumeran toda función de biblioteca que el programa puede llamar.

### PE

Un archivo PE (Portable Executable) empieza con una **cabecera DOS** que comienza con las letras `MZ`. Es un resto del formato DOS de 16 bits, y el único campo que aún importa está en el desplazamiento 0x3c: `e_lfanew`, el desplazamiento en el archivo de las **cabeceras NT**. Entre la cabecera DOS y las cabeceras NT suele haber un pequeño programa DOS que imprime "This program cannot be run in DOS mode".

Las cabeceras NT empiezan con la firma `PE\0\0`, seguida de la **cabecera de archivo**, que indica la máquina y el número de secciones, y de la **cabecera opcional**, que es obligatoria en los ejecutables a pesar de su nombre. A continuación viene una **tabla de secciones**.

El programa `examples/12_pe_dump.py` analiza estas estructuras solo con la biblioteca estándar. Un programa de Windows construido con MinGW en la imagen del laboratorio:

```bash
python3 12_pe_dump.py build/12_win.exe
```

```text
e_lfanew (offset of the NT headers): 0x80
machine: 0x8664  sections: 10  optional header size: 240
optional header magic: 0x20b (0x20b means PE32+)
entry point RVA: 0x1410  image base: 0x140000000
section alignment: 0x1000  file alignment: 0x200
import directory: RVA 0x8000, size 0x52c

name      virtual size  RVA       raw size  raw offset  flags
.text     0x00001768    0x001000  0x001800  0x000400    0x60000060
.data     0x00000080    0x003000  0x000200  0x001c00    0xc0000040
.rdata    0x000008a0    0x004000  0x000a00  0x001e00    0x40000040
.pdata    0x00000210    0x005000  0x000400  0x002800    0x40000040
.xdata    0x000001dc    0x006000  0x000200  0x002c00    0x40000040
.bss      0x00000180    0x007000  0x000000  0x000000    0xc0000080
.idata    0x0000052c    0x008000  0x000600  0x002e00    0xc0000040
.CRT      0x00000060    0x009000  0x000200  0x003400    0xc0000040
.tls      0x00000010    0x00a000  0x000200  0x003600    0xc0000040
.reloc    0x00000078    0x00b000  0x000200  0x003800    0x42000040

entry point RVA 0x1410 is at file offset 0x810

imported libraries and functions:
  KERNEL32.dll: DeleteCriticalSection, EnterCriticalSection, GetLastError, InitializeCriticalSection, LeaveCriticalSection, SetUnhandledExceptionFilter ...
  msvcrt.dll: __C_specific_handler, __getmainargs, __initenv, __iob_func, __set_app_type, __setusermatherr ...
```

Campos clave:

| Campo | Significado |
|---|---|
| Máquina | 0x8664 es x86-64; 0x14c es x86 |
| Número mágico de la cabecera opcional | 0x10b es PE32 (32 bits); 0x20b es PE32+ (64 bits) |
| Base de la imagen | La dirección preferida a la que se carga el archivo (aquí 0x140000000) |
| Punto de entrada | Una RVA: relativa a la base de la imagen |
| Alineación de secciones | Alineación en memoria (0x1000, una página) |
| Alineación en el archivo | Alineación en el archivo (0x200) |
| Directorios de datos | Una tabla de 16 pares `(RVA, size)` que apuntan a importaciones, exportaciones, reubicaciones, recursos y más |

#### RVA y desplazamientos en el archivo

Las direcciones PE dentro del archivo son **direcciones virtuales relativas** (RVA): desplazamientos desde la base de la imagen una vez cargada. Cada sección ocupa un rango en memoria y otro en el archivo. Para convertir una RVA en un desplazamiento en el archivo, busca la sección que la contiene y aplica:

```text
file offset = RVA - section RVA + section raw offset
```

Ejemplo resuelto con la tabla anterior: la RVA del punto de entrada 0x1410 está en `.text` (RVA 0x1000, desplazamiento en bruto 0x400), así que el desplazamiento es 0x1410 - 0x1000 + 0x400 = 0x810. Coincide con la salida del programa. El tamaño virtual de la sección (0x1768) puede superar su tamaño en bruto, y `.bss` tiene tamaño virtual pero no datos en bruto, igual que en ELF.

#### Importaciones

El directorio de importaciones (aquí la sección `.idata`) es una matriz de descriptores, uno por biblioteca, que termina con un descriptor todo a ceros. Cada descriptor apunta al nombre de la biblioteca y a dos matrices paralelas: la **tabla de nombres de importación** (import name table, INT), que lista las funciones deseadas, y la **tabla de direcciones de importación** (import address table, IAT), que el cargador rellena con sus direcciones. Cada entrada de la INT apunta a un `hint` y un nombre. Las llamadas del programa pasan por la IAT, como `call qword ptr [rip + offset]`. Es el equivalente PE de PLT y GOT, con la diferencia de que el cargador rellena toda la IAT antes de que arranque el programa.

#### Otras tablas

El **directorio de exportaciones** lista las funciones que ofrece una DLL. La **tabla de reubicaciones** (`.reloc`) lista los lugares que hay que ajustar cuando la imagen se carga en una dirección distinta de la base preferida. Los recursos, el almacenamiento local de hilo (TLS), los datos de excepciones (`.pdata` y `.xdata` en 64 bits) y el directorio de depuración tienen sus propias entradas en los directorios de datos.

### Comparación

| | ELF | PE |
|---|---|---|
| Número mágico | `7f 45 4c 46` | `MZ`, luego `PE\0\0` en `e_lfanew` |
| Punto de entrada | Dirección virtual | RVA desde la base de la imagen |
| Vista de carga | Cabeceras de programa (segmentos) | Tabla de secciones y la cabecera opcional |
| Importaciones | `.dynsym`, `.rela.plt`, PLT y GOT | Directorio de importaciones, INT e IAT |
| Exportaciones | Entradas de `.dynsym` marcadas como definidas | Directorio de exportaciones |
| Reubicaciones | `.rela.dyn`, `.rela.plt` | `.reloc` |
| Cargador | `ld.so`, indicado en `INTERP` | El cargador de Windows en el núcleo y `ntdll` |

### Escribir un ejecutable a mano

Como el cargador necesita pocos campos, un programa Linux válido puede ser tan pequeño como una cabecera ELF, una cabecera de programa y unas pocas instrucciones. El archivo `examples/12_tiny_elf.asm` se ensambla en un binario plano con `nasm -f bin`, que escribe exactamente los bytes que declaras y nada más.

```bash
make
build/12_tiny_elf; echo $?
wc -c < build/12_tiny_elf
readelf -h -l -W build/12_tiny_elf | grep -E "Class|Type|Machine|Entry|Number of|LOAD"
```

```text
42
132
  Class:                             ELF64
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Entry point address:               0x400078
  Number of program headers:         1
  Number of section headers:         0
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  LOAD           0x000000 0x0000000000400000 0x0000000000400000 0x000084 0x000084 R E 0x1000
```

El programa completo ocupa 132 bytes: 64 de la cabecera ELF, 56 de la única cabecera de programa y 12 de las instrucciones. La única entrada `LOAD` mapea todo el archivo desde el desplazamiento 0 en la dirección 0x400000 con permiso de lectura y ejecución, de modo que los bytes de cabecera y el código comparten un mapeo, y el punto de entrada 0x400078 está 120 bytes más allá, justo después de las dos cabeceras. No hay secciones ni símbolos; el programa se ejecuta porque el cargador no necesita ni lo uno ni lo otro.

Lee el archivo completo: cada `db`, `dw`, `dd` y `dq` es un campo de las tablas anteriores, con su comentario. Observa que `p_offset` es cero y `p_vaddr` coincide con la dirección de carga; el cargador exige que el desplazamiento y la dirección virtual coincidan módulo el tamaño de página.

Un archivo PE puede escribirse de la misma manera, con una cabecera DOS, cabeceras NT, una tabla de secciones y una sección. Es más largo porque el formato PE tiene más campos obligatorios; el ejercicio 11 te pide leer uno con atención, no escribirlo.

## Pruébalo

1. Ejecuta `readelf -h build/12_tiny_elf` y relaciona cada línea con un `dw`, `dd` o `dq` de `12_tiny_elf.asm`.
2. Cambia el estado de salida en el ensamblador a 7, vuelve a construir y confirma con `echo $?`.
3. Con `readelf -S` sobre `12_sample`, convierte la dirección de `counter` (0x404020, de `objdump -d`) en un desplazamiento en el archivo usando la dirección y el desplazamiento de la sección. Después compruébalo con `xxd -s OFFSET -l 4 build/12_sample`; el valor es 5.
4. Ejecuta `python3 12_pe_dump.py` sobre el PE y convierte a mano la RVA 0x8000 del directorio de importaciones en un desplazamiento en el archivo. Compáralo con la tabla de secciones.
5. Encuentra la cadena `hello` en `12_sample` con `strings -t x` y calcula la dirección virtual a partir de la sección `.rodata`.

## Errores comunes

- **Mezclar desplazamientos en el archivo y direcciones virtuales.** Las herramientas imprimen ambos. Comprueba la cabecera de la columna antes de usar un número.
- **Olvidar que las RVA son relativas a la base de la imagen.** En un PE, suma la base para obtener la dirección que muestra el depurador.
- **Fiarse de la tabla de secciones.** El cargador la ignora. Un archivo con la tabla de secciones ausente o manipulada puede ejecutarse igualmente, y el nombre de una sección no dice qué son los bytes.
- **Leer cabeceras con el ancho de palabra equivocado.** Un ELF o PE de 32 bits usa anchos de campo distintos. Comprueba primero la clase o el número mágico.
- **Suponer que las llamadas a `foo@plt` ejecutan `foo` la primera vez.** La primera llamada pasa por el resolvedor cuando el enlace perezoso está activado.

## Resumen

- Un cargador mapea un archivo con una vista (segmentos ELF, secciones PE con la cabecera opcional), mientras que las herramientas usan otra (secciones).
- ELF tiene una cabecera ELF, cabeceras de programa y cabeceras de sección; PE tiene una cabecera DOS, cabeceras NT y una tabla de secciones.
- Las importaciones llegan a las funciones de biblioteca mediante PLT y GOT en ELF, y mediante el directorio de importaciones y la IAT en PE.
- Convertir una dirección en un desplazamiento en el archivo es restar la dirección de la sección y sumar su desplazamiento en el archivo.
- Un ejecutable válido puede ser muy pequeño, porque el cargador solo necesita los campos de carga.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 13, reconocer estructuras de código.
- Hojas de referencia: [ELF](../../cheatsheets/elf.es.md), [PE](../../cheatsheets/pe.es.md).
- Ejercicios que usan este módulo: [Ejercicio 10](../../exercises/10-header-surgeon/README.es.md), [Ejercicio 11](../../exercises/11-portable-puzzle/README.es.md).
