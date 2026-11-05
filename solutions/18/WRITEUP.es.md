<!-- source: WRITEUP.md @ sha256:936cb9e94e24 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Resolución: ejercicio 18. Informe de caza de errores

La bandera es `ARL{bughunt_16_128_8_64_5}`: el manejador de nombres acepta una longitud de hasta 16 en un búfer de 16 bytes, el manejador de números deja pasar por su comprobación con signo todos los bytes de longitud desde 128, el manejador de etiquetas acepta un valor de etiqueta de hasta 8 para una tabla de 8 entradas, el tamaño del bloque de datos se desborda a 8 bits desde una cuenta de 64, y el tipo de registro 5 libera memoria que se sigue usando.

## Parte 1: Reverse

### Orientación

El binario no tiene símbolos y el código de análisis se expandió dentro de la función principal. El primer paso es encontrar la estructura del programa en lugar de leerlo desde arriba:

- El archivo se lee con un solo `fread` de hasta 4096 bytes en un búfer estático en 0x404080. Tras la llamada, la longitud devuelta no se vuelve a usar: nada compara una posición con el tamaño de los datos. Es la primera pista sobre el recorrido de registros.
- Los registros se despachan mediante una tabla de saltos:

```text
401465:  mov    rbx,rdi
401468:  cmp    al,0x5                       ; type above 5: unknown, error -6
40146a:  ja     40153f
401470:  movzx  eax,al
401473:  jmp    QWORD PTR [rax*8+0x402088]   ; table for the types 1 to 5
```

La tabla en 0x402088 contiene, para los tipos 1 a 5, las direcciones 0x401316, 0x401362, 0x4012e4, 0x4013bc y 0x401438. Cada dirección es el código de un tipo de registro, y cada una es un sumidero que auditar. Probar con archivos de entrada pequeños confirma lo que hacen: el tipo 1 fija el nombre, el 2 suma números, el 3 cuenta etiquetas, el 4 guarda un bloque de datos y el 5 lo descarta.

### El recorrido de registros (todos los tipos)

```text
401440:  add    rbp,0x1                      ; one more record
401444:  movzx  eax,BYTE PTR [rbx]           ; type
401447:  movzx  edx,BYTE PTR [rbx+0x1]       ; length
40144b:  movzx  r8d,dl
40144f:  lea    rsi,[rbx+0x2]                ; payload
401453:  movzx  ecx,dl
401456:  lea    rdi,[rsi+rcx*1]              ; the next record = payload + length
40145a:  mov    r14,rbx
40145d:  test   al,al
40145f:  je     4012b6                       ; type 0 ends the loop
401465:  mov    rbx,rdi
```

El bucle lee dos bytes de cabecera y calcula la posición siguiente, y en ningún momento se compara la posición con el final de los datos. Un archivo cuyo último registro declara una longitud que no cabe hace que el código lea más allá del final del búfer, y un archivo sin registro de tipo 0 hace que lea hasta encontrar uno, esté donde esté. La clase es una lectura fuera de límites (CWE-125).

### Nombres (tipo 1, 0x401316)

```text
401316:  pxor   xmm0,xmm0
40131a:  movaps XMMWORD PTR [rsp+0x80],xmm0  ; the 16-byte local name, cleared
401322:  cmp    r8d,0x10
401326:  ja     401356                       ; length above 16: error -2
401328:  lea    rax,[rsp+0x80]
...
401335:  rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]    ; copy length bytes
40133a:  mov    BYTE PTR [rsp+r8*1+0x80],0x0            ; terminator at name[length]
```

El búfer tiene 16 bytes, el límite es 16 y el terminador se escribe en el índice `length`. Con longitud 16 el terminador es el byte diecisiete, uno más allá del final: un error de uno que escribe un byte cero fuera del búfer (CWE-193, una escritura fuera de límites). Así `N` es 16, y el límite correcto es 15.

### Números (tipo 2, 0x401362)

```text
401362:  cmp    dl,0x20
401365:  jg     4013b0                       ; signed: above 32 is error -3
401367:  lea    rax,[rsp+0x20]               ; a local array of 8 integers (32 bytes)
40136c:  movsx  rcx,dl                       ; the size, sign-extended to 64 bits
401370:  mov    rdi,rax
401373:  rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]
```

La comparación usa el registro de 8 bits `dl` y un salto con signo. Pasan los bytes de longitud de 0 a 32, y también todos los bytes de 128 a 255, que son negativos leídos como números de 8 bits con signo. El `movsx` siguiente extiende un valor negativo a una cuenta gigantesca de 64 bits para `rep movs`. Así `V` es 128. A la comprobación le falta además la prueba de que la longitud sea múltiplo de 4. Es un error de conversión de con signo a sin signo (CWE-195) que lleva a una escritura fuera de límites. La corrección compara la longitud sin signo y comprueba el múltiplo de 4.

### Etiquetas (tipo 3, 0x4012e4)

```text
4012f6:  movzx  eax,BYTE PTR [rdx]           ; the tag byte
4012f9:  cmp    al,0x8
4012fb:  ja     40154b                       ; above 8: error -4
401301:  movzx  eax,al
401304:  add    DWORD PTR [rsp+rax*4],0x1    ; histogram[tag]++
```

El histograma son ocho contadores de 32 bits en la parte baja del marco (puestos a cero por un `rep stos` anterior de 8 palabras dobles). La comprobación deja pasar el valor 8, y el incremento en el índice 8 toca los cuatro bytes posteriores a la tabla: un error de uno con escritura (CWE-193). Así `T` es 8; el límite correcto es 7. En esta compilación el vecino es el array del manejador de números, así que el daño es silencioso, y eso es lo que hace peligroso este tipo de error.

### Bloques de datos (tipo 4, 0x4013bc)

```text
4013bc:  movzx  r13d,BYTE PTR [r14+0x2]      ; count = first payload byte
4013c1:  mov    rdi,r12
4013c4:  call   free                         ; the previous block
4013c9:  lea    edi,[r13*4+0x0]
4013d1:  movzx  edi,dil                      ; the size is truncated to 8 bits
4013d5:  call   malloc
4013da:  mov    r12,rax
4013dd:  movzx  eax,r13b
4013e1:  shl    eax,0x2                      ; the copy size is count * 4 in 32 bits
...                                          ; a copy of that many bytes into the block
```

El tamaño de la reserva es `count * 4` truncado a 8 bits, y el tamaño de la copia es `count * 4` en 32 bits. Coinciden mientras la cuenta sea menor que 64. A partir de una cuenta de 64, el producto llega a 256 y el tamaño de la reserva da la vuelta (64 da `malloc(0)`, 65 da `malloc(4)`), mientras la copia sigue escribiendo 4 veces la cuenta: un desbordamiento de montón (CWE-190 que lleva a CWE-122). Así `B` es 64. El código tampoco compara nunca la longitud del registro con la cuenta, de modo que la copia puede leer más allá de la carga útil (CWE-125). La corrección calcula el tamaño en un tipo ancho y exige que la longitud sea igual a 1 más 4 veces la cuenta.

### Liberación (tipo 5, 0x401438)

```text
401438:  mov    rdi,r12
40143b:  call   free                         ; free the block; r12 keeps its value
401440:  ...                                 ; the loop continues
...
40147a:  movzx  ecx,BYTE PTR [r12+rax*1]     ; at the end: sum the block bytes (r13 of them)
...
401494:  mov    rdi,r12
401497:  call   free                         ; and free it again
```

El registro de tipo 5 libera el bloque y deja sin cambios `r12` y la longitud `r13`. El código posterior al bucle suma los bytes del bloque y lo libera por segunda vez: una lectura tras la liberación (CWE-416) y una doble liberación (CWE-415). Así `U` es 5. (El registro de tipo 4 también libera el bloque antiguo antes de reservar uno nuevo, lo que solo es correcto porque sobrescribe `r12`.) La corrección pone el puntero a nulo y la longitud a 0 tras la liberación.

### Un informe modelo

```text
Title:      The record walk never compares its position with the end of the data
Class:      CWE-125 out-of-bounds read
Location:   main (the parser is inlined), loop at 0x401440 to 0x401465
Severity:   Medium. The length bytes come from the file; only reads are affected.
Cause:      The next record is computed as payload + length and the position is
            never compared with the size of the data returned by fread.
Evidence:   A file with a last record of length 255 and 10 bytes of data makes
            the loop read the stale content of the static buffer.
Impact:     Bytes outside the file are interpreted as records; the output depends
            on memory that is not part of the input and the program can fault.
Fix:        Track the bytes that are left, and refuse a record when fewer than 2
            bytes remain or the payload does not fit.
Test:       Truncations at every position of a valid file; a last record of length
            255 with no data.

Title:      Names: off-by-one in the terminator
Class:      CWE-193 off-by-one (out-of-bounds write of one byte)
Location:   0x401322 (limit) and 0x40133a (terminator)
Severity:   Medium. One byte with a fixed value, written next to a stack buffer.
Cause:      The limit is 16 for a buffer of 16 bytes, and the terminator is written
            at index length.
Evidence:   A name of 16 bytes passes the check; the store at [rsp+r8+0x80] with
            r8 = 16 is outside the 16 bytes at [rsp+0x80].
Impact:     A zero byte is written to whatever follows the buffer.
Fix:        Accept lengths up to 15.
Test:       Names of 0, 15, 16 and 255 bytes.

Title:      Numbers: signed length check
Class:      CWE-195 signed to unsigned conversion (leading to an out-of-bounds write)
Location:   0x401362 to 0x401373
Severity:   High. A length byte of 128 to 255 gives a copy size of about 2^64.
Cause:      The length is tested as a signed 8-bit value and then sign-extended to
            the copy size.
Evidence:   A record with length 128 crashes the program; a debugger stopped at
            0x401370 shows rcx = 0xffffffffffffff80.
Impact:     The copy runs far beyond its 32-byte destination and ends with a fault.
Fix:        Treat the length as unsigned; require a multiple of 4 and at most 32.
Test:       Lengths 0, 4, 32, 33, 36, 127, 128, 255.

Title:      Tags: the table has 8 entries and the check accepts 8
Class:      CWE-193 off-by-one (out-of-bounds write of a counter)
Location:   0x4012f9 and 0x401304
Severity:   Medium. A counter increment on the four bytes after the table.
Cause:      The limit is "above 8" for a table whose last index is 7.
Evidence:   A tag value of 8 passes the check and the increment is at [rsp+0x20].
Impact:     The data that follows the table is changed.
Fix:        Reject tag values above 7.
Test:       Tag values 0, 7, 8, 9, 255.

Title:      Data blocks: the allocation size wraps
Class:      CWE-190 integer overflow (leading to CWE-122 heap overflow)
Location:   0x4013c9 to 0x4013e1
Severity:   High. The count and the data come from the file.
Cause:      malloc receives count * 4 truncated to 8 bits; the copy uses count * 4
            in 32 bits; the record length is not compared with the count.
Evidence:   A count of 64 gives malloc(0) and a copy of 256 bytes.
Impact:     A heap overflow of up to 768 bytes beyond a small allocation.
Fix:        Compute the size in 64 bits, require length = 1 + 4 * count, and
            allocate that size.
Test:       Counts 0, 1, 63, 64, 65, 255, each with a matching and a mismatching
            length.

Title:      Release: the freed block is used and freed again
Class:      CWE-416 use after free (and CWE-415 double free)
Location:   0x401438 and 0x40147a to 0x401497
Severity:   High. The block is read after it was freed, then freed again.
Cause:      Type 5 frees the block and keeps the pointer and the length.
Evidence:   A file with a data block followed by a record of type 5 is enough;
            in the debugger, the second call to free at 0x401497 receives the
            pointer that the first call at 0x40143b has already released.
Impact:     Reads of freed memory, and corruption of the allocator state.
Fix:        Set the pointer to null and the length to 0 after the free.
Test:       A block then a reset; a reset with no block; a reset twice; a block
            after a reset.
```

## Parte 2: Assemble

La solución de referencia es `solution.asm`. Los puntos que importan:

- **El bucle.** Los bytes que quedan se calculan como `n - pos` antes de cada comparación. También se podría comparar posición más longitud con `n`, pero la resta no puede desbordarse porque `pos` nunca supera `n`. Primero se comprueban los dos bytes de cabecera, luego la carga útil, y solo entonces se examina el tipo, de modo que el registro de tipo 0 sigue la misma regla.
- **Nombres.** Un almacenamiento de 16 bytes borra el campo y después se copian los bytes de la carga útil. El límite es 15.
- **Números.** `test r11b, 3` rechaza las longitudes que no son múltiplo de 4, y una comparación sin signo rechaza las superiores a 32. Los valores se cargan con `movsxd` para que un número negativo se sume correctamente a la suma de 64 bits.
- **Etiquetas.** El byte se compara con 7 antes de indexar la tabla.
- **Bloques de datos.** `lea r10d, [r10*4 + 1]` calcula la longitud esperada en un registro de 32 bits; una cuenta que cabe en un byte no puede desbordarlo. La comparación con la longitud lo decide todo, así que ninguna lectura va más allá de la carga útil. La suma empieza en 0 con cada bloque.
- **Registros.** Solo se usan registros que la función puede modificar, así que no hay nada que guardar, y no hay llamadas.

## Errores que hacen perder tiempo

- **Comparar posición más longitud con `n` en 32 bits.** Con un búfer cerca de lo más alto del espacio de direcciones, o con una longitud grande, la suma da la vuelta. Calcula en su lugar lo que queda.
- **Probar el tipo de registro antes que el truncamiento.** Un registro de tipo 0 con la carga útil cortada debe rechazarse igualmente.
- **Usar una carga sin signo para los números.** Los valores negativos suman una cantidad errónea; la suma difiere.
- **Olvidar borrar el nombre.** Un nombre más corto que sustituye a uno más largo deja bytes antiguos.
- **No poner a cero la suma del bloque.** Un segundo bloque se suma al primero.
- **Leer la cuenta antes de comprobar que la longitud es al menos 1.** Un registro de tipo 4 sin carga útil lee un byte más allá.
- **Comprobar la etiqueta contra 8.** El error de uno del original se copia en la corrección.
