<!-- source: README.md @ sha256:2dd2136139ab -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 03. Tu primer programa

Tiempo: unas 2 horas. Requisitos previos: [01. Representación de datos](../01-data-representation/README.es.md), [02. La CPU y sus registros](../02-cpu-and-registers/README.es.md)

## Por qué es importante

Una llamada al sistema es la única forma en que un programa sin bibliotecas puede hacer algo visible: imprimir, leer un archivo, terminar. Cuando puedes emitir llamadas al sistema desde ensamblador, puedes escribir programas reales y puedes leer esas mismas llamadas en cualquier binario que analices. Quienes hacen ingeniería inversa empiezan muchas investigaciones preguntando qué llamadas al sistema hace un programa.

## Conceptos

### Anatomía de una sentencia

Cada línea de código fuente NASM tiene las mismas partes opcionales:

```text
label:    mnemonic   operands        ; comment
```

- Una **etiqueta** (label) da nombre a una dirección. `_start:` y `message:` son etiquetas.
- Un **mnemónico** da nombre a una instrucción, como `mov` o `syscall`.
- Los **operandos** son los valores con los que trabaja la instrucción, separados por comas. El destino va primero: `mov eax, 1` pone 1 en `eax`.
- Un **comentario** empieza con `;` y llega hasta el final de la línea. El ensamblador lo ignora.

Directivas como `section`, `global`, `db` y `equ` parecen similares, pero son órdenes para el ensamblador, no para el procesador. Por sí solas no producen código máquina.

### Secciones

Un programa se almacena en secciones que tienen propósitos y permisos distintos:

| Sección | Contiene | Permisos | Ocupa espacio en el archivo |
|---|---|---|---|
| `.text` | Código | lectura, ejecución | sí |
| `.rodata` | Constantes | lectura | sí |
| `.data` | Variables inicializadas | lectura, escritura | sí |
| `.bss` | Variables que empiezan en cero | lectura, escritura | no |

`.bss` no ocupa espacio en el archivo porque solo contiene un tamaño; el cargador (loader) proporciona la memoria puesta a cero. Puedes ver estas propiedades en la tabla de secciones del programa de ejemplo:

```text
$ readelf -S -W build/03_data_bss
  [ 1] .text    PROGBITS  0000000000401000 001000 000039 00  AX  0   0 16
  [ 2] .rodata  PROGBITS  0000000000402000 002000 000001 00   A  0   0  4
  [ 3] .data    PROGBITS  0000000000403004 002004 000001 00  WA  0   0  4
  [ 4] .bss     NOBITS    0000000000403008 002005 000008 00  WA  0   0  4
```

La columna `Flg` muestra `A` (asignada), `X` (ejecutable) y `W` (escribible). `PROGBITS` significa que la sección tiene bytes en el archivo; `NOBITS` significa que no los tiene.

### Definir datos

| Directiva | Significado |
|---|---|
| `db`, `dw`, `dd`, `dq` | Colocan bytes, palabras (2 bytes), palabras dobles (4) o palabras cuádruples (8) en el archivo |
| `resb`, `resw`, `resd`, `resq` | Reservan espacio sin colocar bytes (se usan en `.bss`) |
| `name equ expression` | Definen una constante; no se usa memoria |
| `$` | La dirección de la posición actual |

El patrón `message_len equ $ - message`, escrito justo después de una cadena, hace que el ensamblador calcule la longitud de la cadena. Úsalo en lugar de contar a mano. Las cadenas en ensamblador son bytes normales. No tienen terminador a menos que lo añadas, así que el `10` (un salto de línea) es solo otro byte que colocas tú.

### El punto de entrada

El programa empieza en la etiqueta `_start`. La directiva `global _start` hace visible la etiqueta para el enlazador, que registra su dirección como punto de entrada.

### Ensamblar y enlazar

Dos herramientas convierten el código fuente en un programa:

```bash
nasm -f elf64 prog.asm -o prog.o      # assemble: source to object file
ld prog.o -o prog                     # link: object file to executable
```

Para código de 32 bits, cambia ambos comandos:

```bash
nasm -f elf32 prog.asm -o prog.o
ld -m elf_i386 prog.o -o prog
```

### Llamadas al sistema

Una llamada al sistema pide al núcleo del sistema operativo que haga algo en tu nombre. La convención en Linux depende del modo:

| | 64 bits | 32 bits |
|---|---|---|
| Instrucción | `syscall` | `int 0x80` |
| Número de llamada | `rax` | `eax` |
| Argumentos 1 a 6 | `rdi`, `rsi`, `rdx`, `r10`, `r8`, `r9` | `ebx`, `ecx`, `edx`, `esi`, `edi`, `ebp` |
| Resultado | `rax` | `eax` |

Los números de llamada también difieren entre modos. `write` es 1 en código de 64 bits y 4 en código de 32 bits. `exit` es 60 y 1. La tabla completa está en la [hoja de referencia de llamadas al sistema de Linux](../../cheatsheets/syscalls-linux.es.md).

`write(fd, buffer, length)` envía `length` bytes a partir de `buffer` al descriptor de archivo `fd`. El descriptor 1 es la salida estándar y el 2 la salida de error estándar. Devuelve el número de bytes escritos.

### El programa completo

Este es [03_hello64.asm](examples/03_hello64.asm). Cada línea está comentada porque este es tu primer programa.

```nasm
bits 64
default rel

SYS_WRITE   equ 1               ; system call numbers (64-bit table)
SYS_EXIT    equ 60
STDOUT      equ 1               ; file descriptor of standard output

section .rodata
message:    db "Hello, assembly!", 10       ; 10 is the newline byte
message_len equ $ - message                 ; bytes from 'message' to here

section .text
global _start                   ; make the entry point visible to the linker

_start:
    mov     eax, SYS_WRITE      ; rax = call number
    mov     edi, STDOUT         ; rdi = argument 1: file descriptor
    lea     rsi, [message]      ; rsi = argument 2: address of the text
    mov     edx, message_len    ; rdx = argument 3: number of bytes
    syscall                     ; enter the kernel: write(1, message, len)

    mov     eax, SYS_EXIT       ; rax = call number
    xor     edi, edi            ; rdi = argument 1: exit status 0
    syscall                     ; exit(0)
```

`lea rsi, [message]` carga la dirección de `message`; no lee el texto. El módulo 04 cubre `lea` y el direccionamiento por completo. `xor edi, edi` es la forma habitual de poner un registro a cero.

[03_hello32.asm](examples/03_hello32.asm) hace lo mismo en código de 32 bits con `int 0x80`. [03_data_bss.asm](examples/03_data_bss.asm) muestra las cuatro secciones en un solo programa.

### La perspectiva de la ingeniería inversa

El ensamblador conserva tu estructura, pero no tus nombres. Abre el programa que acabas de compilar:

```text
$ objdump -d -M intel build/03_hello64
0000000000401000 <_start>:
  401000:  mov  eax,0x1
  401005:  mov  edi,0x1
  40100a:  lea  rsi,[rip+0xfef]        # 402000 <message>
  401011:  mov  edx,0x11
  401016:  syscall
  401018:  mov  eax,0x3c
  40101d:  xor  edi,edi
  40101f:  syscall
```

Lo que sobrevivió: las instrucciones y las etiquetas `_start` y `message` (la tabla de símbolos las conserva). Lo que no: las constantes `SYS_WRITE`, `STDOUT` y `message_len`, que se convirtieron en los números 1, 1 y 0x11 (17), y todos los comentarios. Esto es lo que verás en programas sin código fuente: números donde el autor tenía nombres. Reconocer que `mov eax, 1` antes de `syscall` significa `write` es la primera habilidad de ingeniería inversa de este curso.

## Pruébalo

Compila los ejemplos y ejecútalos:

```bash
cd modules/03-first-program/examples
make run
```

Salida esperada:

```text
== 03_hello64
Hello, assembly!
== 03_data_bss
A
== 03_hello32
Hello, assembly!
```

Ahora observa cómo cambian los registros en el depurador:

```bash
gdb -q build/03_hello64
```

En el indicador `gef>`, escribe estos comandos uno a uno:

```text
break _start
run
stepi 4
info registers rax rdi rdx rip
x/s $rsi
```

GEF imprime un panel grande después de cada parada. Busca las líneas de registros. Tras cuatro comandos `stepi` se han ejecutado las cuatro primeras instrucciones y ves:

```text
rax            0x1                 0x1
rdi            0x1                 0x1
rdx            0x11                0x11
rip            0x401016            0x401016 <_start+22>
0x402000 <message>: "Hello, assembly!\n,"
```

Los registros contienen los tres argumentos y el número de llamada, y `rip` apunta a la instrucción `syscall`. La cadena muestra una `,` sobrante tras el salto de línea: `x/s` imprime hasta encontrar un byte cero y `message` no tiene ninguno, así que lee un byte de lo que venga después. Ahora ejecuta `stepi` una vez más. El texto aparece en tu pantalla y los registros cambian:

- `rax` es ahora `0x11`: el resultado de `write`, los 17 bytes escritos.
- `rcx` contiene `0x401018`, la dirección de la instrucción siguiente, y `r11` contiene `0x302`. La instrucción `syscall` sobrescribe ambos. Nunca guardes un valor en `rcx` ni en `r11` a través de un `syscall`.

Por último, mira el mismo programa desde fuera:

```bash
strace ./build/03_hello64
```

```text
execve("./build/03_hello64", ["./build/03_hello64"], ...) = 0
write(1, "Hello, assembly!\n", 17Hello, assembly!
)      = 17
exit(0)                                 = ?
+++ exited with 0 +++
```

Al ejecutar `strace ./build/03_hello32` se ven las mismas llamadas y se añade la línea `[ Process PID=... runs in 32 bit mode. ]`.

## Errores frecuentes

- **Olvidar `global _start`.** El enlazador avisa con `cannot find entry symbol _start; defaulting to ...` y elige el inicio de `.text`. El programa puede seguir funcionando, por casualidad, y romperse cuando añades código antes de `_start`. Trata la advertencia como un error.
- **Mezclar formatos.** `nasm -f elf32` con un `ld` normal produce un error de enlazado o un programa incorrecto. Usa `-f elf32` con `ld -m elf_i386` y `-f elf64` con `ld` normal.
- **Olvidar `exit`.** Tras tu última instrucción el procesador ejecuta los bytes que siguen y el programa termina con un fallo de segmentación.
- **Usar `int 0x80` en código de 64 bits.** Sigue funcionando en Linux, pero usa la tabla de llamadas de 32 bits y argumentos de 32 bits, así que las direcciones por encima de 4 GiB fallan. En código de 64 bits usa `syscall`.
- **Contar mal una cadena.** El salto de línea que añades con `10` es un byte más. Usa `$ - label`.
- **Suponer que las cadenas acaban en cero.** No es así. Pasa la longitud a `write` y coloca tú un `0` cuando una rutina posterior lo espere.

## Resumen

- Una sentencia es `label: mnemonic operands ; comment`; las directivas se dirigen al ensamblador y por sí solas no producen código.
- El código vive en `.text`, las constantes en `.rodata` y las variables en `.data` y `.bss`.
- Linux de 64 bits usa `syscall` con el número en `rax` y los argumentos en `rdi`, `rsi`, `rdx`; el de 32 bits usa `int 0x80` con `eax`, `ebx`, `ecx`, `edx`.
- Las constantes y los comentarios desaparecen al ensamblar; los números permanecen.
- `$ - label` calcula una longitud; `exit` debe terminar todo programa.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 04, mover datos y modos de direccionamiento.
- Ejercicios que usan este módulo: [Ejercicio 01](../../exercises/01-signal-flare/README.es.md), [Ejercicio 02](../../exercises/02-two-worlds/README.es.md), [Ejercicio 05](../../exercises/05-ticket-counter/README.es.md).
