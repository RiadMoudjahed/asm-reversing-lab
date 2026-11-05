<!-- source: WRITEUP.md @ sha256:de2d238ddb27 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Análisis resuelto: ejercicio 20. Proyecto final: Tiny VM

El flag es `ARL{tinyvm_20_41_24}`: la máquina tiene 20 códigos de operación válidos, `call` es el byte de código de operación 0x29 (41 en decimal) y el programa de ejemplo tiene 24 instrucciones.

## Parte 1: Reverse

### Orientación

`tinyvm` no tiene símbolos y es pequeño. `strings` muestra el mensaje de uso y los dos diagnósticos (`halt after %zu steps`, `fault %u at offset %u`), y nada más legible. El programa lee su argumento como un archivo, comprueba los cuatro bytes `TVM1` del principio y después ejecuta un bucle. El bucle es el intérprete.

### El despacho

El bucle lee un byte del código y hace dos consultas de tablas antes de saltar:

```text
  401305: movzx  eax,BYTE PTR [rcx]            ; the opcode byte
  401308: movzx  eax,BYTE PTR [rax+0x402160]   ; first table: 256 bytes
  40130f: test   al,al
  401311: je     401403                        ; 0: not an instruction
  401317: movzx  edx,al
  40131a: movzx  edx,BYTE PTR [rdx+0x402130]   ; second table: one byte per instruction
  401321: add    edx,esi                       ; position + length
  401325: cmp    rbx,rsi                       ; against the length of the code
  401328: jb     4013fc                        ; the instruction does not fit
  40132e: mov    DWORD PTR [rsp+0x20],edx      ; new program counter
  401332: cmp    al,0x14
  401334: ja     4012d9
  401336: movzx  eax,al
  401339: jmp    QWORD PTR [rax*8+0x402080]    ; third table: 21 code addresses
```

Tres tablas, entonces:

- La tabla en 0x402160 tiene 256 bytes y una entrada distinta de cero en exactamente 20 posiciones. Esas posiciones son los códigos de operación válidos, así que `N` es 20. La entrada es un índice pequeño de 1 a 20.
- La tabla en 0x402130 se indexa con ese índice y da la longitud de la instrucción: 4 para una instrucción, 3 para cuatro, 1 para dos y 2 para las otras trece.
- La tabla en 0x402080 contiene 21 direcciones de código, una por índice (el índice 0 vuelve al principio del bucle). La comparación con 0x14 es la comprobación de límite.

El contador de programa se guarda en `[rsp+0x20]` y se actualiza antes de que se ejecute el manejador: esa es la convención de los manejadores de la parte 2. El estado es una estructura en la pila: los ocho registros desde `[rsp]`, luego el contador de programa (0x20), el indicador de cero (0x24), el indicador de acarreo (0x28), el código de fallo (0x2c), el indicador de detenido (0x30), la profundidad de la pila de llamadas (0x34), las dieciséis direcciones de retorno (0x38) y los 256 bytes de memoria (0x58).

### Los manejadores

Cada una de las veinte direcciones es el código de una instrucción. Leerlas da el comportamiento; tres ejemplos:

```text
; index 3
  401448: movzx  edx,BYTE PTR [rcx+0x1]        ; the byte after the opcode
  40144c: mov    ecx,edx
  40144e: shr    cl,0x4
  401451: and    ecx,0x7                       ; first register: high nibble, modulo 8
  401454: mov    eax,DWORD PTR [rsp+rcx*4]
  401457: and    edx,0x7                       ; second register: low nibble, modulo 8
  40145a: mov    edx,DWORD PTR [rsp+rdx*4]
  40145d: lea    esi,[rax+rdx*1]
  401460: mov    DWORD PTR [rsp+rcx*4],esi     ; r[a] = r[a] + r[b]
  401463: mov    eax,eax
  401465: mov    edx,edx
  401467: add    rax,rdx                       ; the sum again, in 64 bits
  40146a: shr    rax,0x20
  40146e: mov    DWORD PTR [rsp+0x28],eax      ; carry flag = carry out
  401472: test   esi,esi
  401474: sete   al
  401477: movzx  eax,al
  40147a: mov    DWORD PTR [rsp+0x24],eax      ; zero flag = result is 0
  40147e: jmp    4012d9
```

Esto es `add`: fija ambos indicadores. Los manejadores de `sub`, `xor` y los demás se leen del mismo modo. Los detalles están en el manejador de desplazamiento:

```text
; index 8
  401550: movzx  eax,BYTE PTR [rcx+0x1]
  401554: mov    esi,eax
  401556: and    esi,0xf                       ; the count: the whole low nibble
  401559: mov    edi,eax
  40155b: and    edi,0xf
  40155e: je     4012d9                        ; count 0: nothing changes, flags included
  401564: shr    al,0x4
  401567: and    eax,0x7                       ; the register
  40156a: mov    edx,DWORD PTR [rsp+rax*4]
  40156d: mov    ecx,0x20
  401572: sub    ecx,edi                       ; 32 - count
  401574: mov    edi,edx
  401576: shr    edi,cl                        ; value >> (32 - count)
  401578: mov    ecx,edi
  40157a: and    ecx,0x1
  40157d: mov    DWORD PTR [rsp+0x28],ecx      ; carry flag = last bit shifted out at the top
  401581: mov    ecx,esi
  401583: shl    edx,cl
  401585: mov    DWORD PTR [rsp+rax*4],edx
  401588: test   edx,edx
  40158a: sete   al
  40158d: movzx  eax,al
  401590: mov    DWORD PTR [rsp+0x24],eax      ; zero flag
  401594: jmp    4012d9
```

El contador es el nibble bajo completo (0 a 15), mientras que el registro usa tres bits del nibble alto. Un contador de cero retorna antes de tocar ningún indicador. `shr` es la imagen especular, con el indicador de acarreo tomado del bit `count - 1`.

```text
; index 14 (call)
  401666: mov    eax,DWORD PTR [rsp+0x34]      ; depth
  40166a: cmp    eax,0x10
  40166d: je     401693                        ; full: fault 3
  40166f: lea    esi,[rax+0x1]
  401672: mov    DWORD PTR [rsp+0x34],esi      ; depth + 1
  401676: mov    eax,eax
  401678: mov    WORD PTR [rsp+rax*2+0x38],dx  ; push the program counter (already advanced)
  40167d: movzx  eax,BYTE PTR [rcx+0x2]
  401681: shl    eax,0x8
  401684: movzx  edx,BYTE PTR [rcx+0x1]
  401688: or     eax,edx                       ; target: byte 1 + 256 * byte 2 (low byte first)
  40168a: mov    DWORD PTR [rsp+0x20],eax
  40168e: jmp    4012d9
  401693: mov    DWORD PTR [rsp+0x2c],0x3      ; fault code 3
  40169b: jmp    4016c4                        ; then halt
```

### El conjunto de instrucciones

Juntando los veinte manejadores se obtiene la especificación (los nombres son los del ejercicio):

| Código | Nombre | Longitud | Efecto |
|---|---|---|---|
| 0x5b | `movi` | 4 | `r[a] = number` (el número de 16 bits está en los bytes 2 y 3, byte bajo primero); indicadores sin cambios |
| 0xc2 | `mov` | 2 | `r[a] = r[b]`; indicadores sin cambios |
| 0x17 | `add` | 2 | `r[a] += r[b]`; `cf` = acarreo de salida, `zf` = el resultado es 0 |
| 0x8e | `sub` | 2 | `r[a] -= r[b]`; `cf` = a < b sin signo, `zf` = el resultado es 0 |
| 0x34 | `mul` | 2 | `r[a] *= r[b]` (32 bits bajos); `cf` = 0, `zf` = el resultado es 0 |
| 0xe9 | `xor` | 2 | `r[a] ^= r[b]`; `cf` = 0, `zf` = el resultado es 0 |
| 0x61 | `and` | 2 | `r[a] &= r[b]`; `cf` = 0, `zf` = el resultado es 0 |
| 0xa5 | `shl` | 2 | desplazamiento a la izquierda según el nibble bajo; contador 0 no cambia nada; `cf` = último bit que sale por arriba, `zf` = el resultado es 0 |
| 0x0d | `shr` | 2 | desplazamiento a la derecha según el nibble bajo; contador 0 no cambia nada; `cf` = último bit que sale por abajo, `zf` = el resultado es 0 |
| 0x7a | `cmp` | 2 | indicadores como en `sub`, no se guarda nada |
| 0xf3 | `jmp` | 3 | `pc` = el destino de 16 bits de los bytes 1 y 2 |
| 0x42 | `jz` | 3 | salta si `zf` es 1 |
| 0xb8 | `jnz` | 3 | salta si `zf` es 0 |
| 0x29 | `call` | 3 | guarda `pc` (ya avanzado) y salta; fallo 3 y parada si hay 16 direcciones en la pila |
| 0xd6 | `ret` | 1 | recupera `pc`; fallo 2 y parada si la pila está vacía |
| 0x93 | `ldb` | 2 | `r[a] = mem[r[b] & 255]` |
| 0x6c | `stb` | 2 | `mem[r[a] & 255] = r[b] & 255` |
| 0xfc | `out` | 2 | imprime `r[a]` como número decimal sin signo y un salto de línea |
| 0x05 | `outc` | 2 | imprime el carácter `r[a] & 255` |
| 0xa0 | `halt` | 1 | se detiene |

En toda instrucción con byte de registro, el primer registro es el nibble alto y el segundo el nibble bajo, cada uno tomado módulo 8; los números de 16 bits se guardan con el byte bajo primero. `isa.json` en `solutions/20/` contiene esta tabla en el formato del ejercicio. Los fallos 1 (código de operación no válido), 5 (se salió del final o demasiados pasos) y 6 (una instrucción cortada por el final del código) vienen del bucle del intérprete y no forman parte de los manejadores.

### El programa de ejemplo

Con la tabla, un desensamblador es un script pequeño (`solutions/20/tvdis.py`). El listado de `bin/sample.tvm`:

```text
      0000  5b 00 05 15   movi r0, 5381
      0004  5b 10 00 00   movi r1, 0
      0008  5b 20 06 00   movi r2, 6
      000c  5b 30 01 00   movi r3, 1
L010: 0010  93 41         ldb r4, [r1]
      0012  29 34 00      call L034
      0015  17 13         add r1, r3
      0017  7a 12         cmp r1, r2
      0019  b8 10 00      jnz L010
      001c  fc 00         out r0
      001e  5b 10 00 00   movi r1, 0
L022: 0022  93 41         ldb r4, [r1]
      0024  05 40         outc r4
      0026  17 13         add r1, r3
      0028  7a 12         cmp r1, r2
      002a  b8 22 00      jnz L022
      002d  5b 50 0a 00   movi r5, 10
      0031  05 50         outc r5
      0033  a0            halt
L034: 0034  c2 60         mov r6, r0
      0036  a5 65         shl r6, 5
      0038  17 06         add r0, r6
      003a  e9 04         xor r0, r4
      003c  d6            ret
24 instructions; data segment: b'vmcore'
```

Calcula un hash de los seis bytes de datos con `h = h * 33 xor c` (la subrutina en 0x34 calcula `h + (h << 5)`, que es `h * 33`), imprime el hash y después imprime los bytes como texto. El listado tiene 24 instrucciones, así que `I` es 24, y `call` es 0x29, que son 41. De ahí sale el flag.

## Parte 2: Assemble

### Los manejadores

Los manejadores de referencia están en `solutions/20/solution.asm`. Todos empiezan descomponiendo el byte de registro. Las comprobaciones que importan y los errores que cuestan tiempo:

- **Las máscaras.** Los campos de registro son nibbles, de 0 a 15, tomados módulo 8 (`and 7`). El contador de desplazamiento es el nibble bajo completo (`and 15`) y no se reduce.
- **Qué indicadores.** `mul` borra el indicador de acarreo; `shl` y `shr` fijan el indicador de acarreo con el último bit que sale y no hacen nada en absoluto con un contador de cero. Un manejador que borra o recalcula los indicadores con contador cero falla las pruebas que empiezan con valores aleatorios de indicadores.
- **El acarreo de `shl`.** Es el bit `32 - count` del valor antiguo, que no es el mismo bit que en `shr` (bit `count - 1`).
- **`cmp`.** El indicador de acarreo es una comparación sin signo; una comparación con signo falla cuando el bit alto de alguno de los valores está activado.
- **`call` y `ret`.** La dirección guardada es el contador de programa ya avanzado. Los casos de fallo cambian solo el código de fallo y el indicador de detenido.
- **`ldb` y `stb`.** Solo se usa el byte bajo del registro de dirección, y `stb` guarda solo el byte bajo del valor.
- **Conservar registros.** Los manejadores usan solo registros que una función puede cambiar, así que no hay nada que guardar.

### El programa

El programa de referencia es `solutions/20/program.tvs`. Un bucle guarda el número actual en la dirección que da un contador y avanza el par; una subrutina cuenta hacia atrás desde 12, carga cada byte y lo imprime. El registro que nunca se fija sirve de cero para la prueba final del bucle. Se ejecuta en 165 pasos.

```text
fill:   stb  [r2], r0
        mov  r4, r0
        add  r4, r1
        mov  r0, r1
        mov  r1, r4
        add  r2, r3
        cmp  r2, r5
        jnz  fill
```

## Errores que cuestan tiempo

- Tomar los campos de registro en el orden equivocado o del nibble equivocado. Un `register_a` erróneo lo detecta la comprobación de `isa.json` y el programa, porque el ensamblador coloca los registros según tus notas.
- Leer `sub` con los operandos invertidos. El primer registro es el destino y el operando izquierdo.
- Pasar por alto que algunas instrucciones dejan los indicadores intactos (`movi`, `mov`, `ldb`, `stb`).
- Contar el segmento de datos o la cabecera en el número de instrucciones.
- Escribir un manejador que funciona para los registros 0 a 7 y falla para los nibbles 8 a 15, que dan la vuelta.
