<!-- source: WRITEUP.md @ sha256:9ade8d8d380b -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Resolución: ejercicio 17. Archivo de incógnitas

La bandera es `ARL{ark_16_4_7_122_48}`: una entrada de la tabla ocupa 16 bytes, los bloques de datos se alinean a 4 bytes, la suma de verificación se rota 7 bits por byte, la clave para 3 ficheros es 122 y la herramienta acepta como máximo 48 ficheros.

## Parte 1: Reverse

### Primero, muestras

Un archivo de tres ficheros pequeños da este volcado (`a.txt` contiene `AAA`, `hello.txt` contiene `hello world` y un salto de línea, `empty.bin` está vacío):

```text
00000000: 4152 4b32 0300 7a00 2000 0000 5000 0000  ARK2..z. ...P...
00000010: 3b3a 3d00 121e 1011 115f f7ee f0ef e08f  ;:=......_......
00000020: 0000 0000 1000 0000 0300 0000 b7cb f27b  ...............{
00000030: 0600 0000 1400 0000 0c00 0000 b504 3829  ..............8)
00000040: 1000 0000 2000 0000 0000 0000 df9b 5713  .... .........W.
00000050: 612e 7478 7400 6865 6c6c 6f2e 7478 7400  a.txt.hello.txt.
00000060: 656d 7074 792e 6269 6e00                 empty.bin.
```

Los cuatro primeros bytes son el valor mágico `ARK2`. Después viene `03 00`, que es el número de ficheros. El byte siguiente es `7a`, que es 122, y el que sigue es cero. Luego vienen dos números de 32 bits, 0x20 y 0x50, que son desplazamientos: en 0x20 hay una tabla de tres grupos de 16 bytes y en 0x50 están los nombres `a.txt`, `hello.txt` y `empty.bin`, cada uno terminado por un byte cero. Los bloques de datos están entre la cabecera y la tabla: el fichero `AAA` se guardó como `3b 3a 3d` y `hello world` como 12 bytes que no parecen texto. Los tres bytes `41` se convirtieron en tres valores distintos, así que los datos se transforman con algo que cambia a lo largo del bloque. Los valores 0x3b, 0x3a y 0x3d son 0x41 xor 0x7a, 0x41 xor 0x7b y 0x41 xor 0x7c: la clave 0x7a más la posición del byte, conservando solo el byte bajo. Así, el byte i de un bloque se guarda con xor del byte bajo de key + i.

Cada entrada de la tabla son cuatro números de 32 bits. Para la primera son 0, 0x10, 3 y 0x7bf2cbb7: el desplazamiento del nombre en el área de nombres, el desplazamiento de los datos (0x10, justo tras la cabecera), el tamaño y un valor que depende del contenido. La segunda entrada tiene el desplazamiento de nombre 6 (tras `a.txt` y su byte cero) y el desplazamiento de datos 0x14, que es el final del primer bloque (0x10 + 3) redondeado a un múltiplo de 4. La tercera entrada tiene tamaño 0 y el valor 0x13579bdf: el valor inicial de la suma de verificación, que un bloque vacío no cambia.

### El programa

El código confirma las muestras y añade los límites. La cabecera se construye con estas instrucciones:

```text
40134d:  mov    DWORD PTR [out],0x324b5241   ; "ARK2"
401357:  mov    eax,DWORD PTR [rsp+0x14]     ; number of files
40135b:  mov    BYTE PTR [out+4],al          ; count, low byte
401361:  mov    BYTE PTR [out+5],0x0         ; count, high byte
401368:  imul   eax,eax,0x25                 ; count * 37
40136b:  add    eax,0xb                      ; + 11
40136e:  mov    BYTE PTR [out+6],al          ; the key
401374:  mov    BYTE PTR [out+7],0x0
```

La clave es `count * 37 + 11` truncado a un byte. Para 3 ficheros es 3 * 37 + 11 = 122, así que `K` es 122. Los límites están antes del bucle:

```text
4011fb:  cmp    DWORD PTR [rsp+0x14],0x30    ; more than 48 files: "too many files"
```

El valor 0x30 es 48, así que `N` es 48. Una comparación `cmp rax,0x1f` protege la longitud de un nombre (31 caracteres como máximo) y `cmp rbx,0x10000` el tamaño de un fichero (65536 bytes como máximo). Los desplazamientos de datos se redondean con `add rbp,0x3` y `and rbp,0xfffffffffffffffc`: se suma 3 y se ponen a cero los dos bits bajos, así que la alineación `A` es 4. Las entradas se direccionan con `shl rax,0x4`, un factor de 16, así que `E` es 16. El bucle de la suma de verificación empieza en 0x13579bdf:

```text
40147c:  mov    edx,0x13579bdf
401481:  rol    edx,0x7                      ; rotate left by 7
...
401497:  jne    401481                       ; for every byte
```

así que `R` es 7. Tras el bucle, el valor se combina con el tamaño mediante un o exclusivo. La entrada vacía no puede mostrarlo porque el tamaño es 0, pero la entrada de `AAA` sí: la suma acumulada simple de los tres bytes es 0x7bf2cbb4 y el valor guardado 0x7bf2cbb7 es esa suma xor 3.

El formato completo, con todos los números en little endian:

| Parte | Desplazamiento | Tamaño | Contenido |
|---|---|---|---|
| Cabecera | 0 | 4 | Valor mágico `ARK2` |
| | 4 | 2 | Número de ficheros |
| | 6 | 1 | Clave: `count * 37 + 11`, byte bajo |
| | 7 | 1 | Cero |
| | 8 | 4 | Desplazamiento de la tabla de entradas |
| | 12 | 4 | Desplazamiento del área de nombres |
| Bloques de datos | múltiplos de 4 desde 16 | tamaño | El byte i se guarda con xor del byte bajo de `key + i`; relleno con bytes cero |
| Tabla de entradas | desplazamiento de la tabla | 16 por fichero | Desplazamiento del nombre (relativo al área de nombres), desplazamiento de datos, tamaño, suma de verificación |
| Área de nombres | desplazamiento del área de nombres | | Nombres terminados en cero, uno tras otro |

La suma de verificación de una entrada se calcula sobre los bytes originales: se empieza con 0x13579bdf, por cada byte se rota 7 bits a la izquierda y se suma el byte, y al final se aplica xor con el tamaño.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. Los puntos que importan:

- **`ark_count` hace la validación.** Comprueba la longitud, el valor mágico (una sola comparación de 32 bits con 0x324b5241), la tabla (su desplazamiento de al menos 16 y su final, calculado en 64 bits como desplazamiento más 16 por la cuenta, no más allá de `n`) y el desplazamiento del área de nombres (no más allá de `n`). Las otras dos funciones la llaman, así que la cabecera se comprueba en un solo lugar. Como solo modifica `rax`, `rcx` y `rdx`, quienes la llaman guardan sus argumentos en los demás registros.
- **El índice.** Una comparación sin signo con la cuenta rechaza a la vez los índices negativos y los que necesitan más de 32 bits. Una comparación de 32 bits dejaría pasar el índice 4294967296 como 0.
- **Nombres.** El desplazamiento del nombre se suma al del área de nombres en 64 bits, debe estar dentro del archivo, y el byte cero se busca con un límite en `n`. Solo entonces se compara la capacidad con la longitud más uno y se copia el nombre con su byte cero.
- **Extracción.** El desplazamiento de datos y el tamaño son valores de 32 bits extendidos con ceros. La suma se calcula en 64 bits, así que un desplazamiento 0xfffffff0 con tamaño 0x20 se rechaza y no da la vuelta hasta un número pequeño. A continuación se comprueba la capacidad; después cada byte se descodifica con el byte bajo de `key + i`, se escribe y se suma a la suma de verificación tras una rotación de 7. La suma se termina con el tamaño y se compara con el valor guardado.
- **Registros.** No se usa nada fuera de los registros que puede modificar la función llamada, así que no hay que guardar ni restaurar. Antes del `call` interno hay un `sub rsp, 8` para mantener la pila alineada.

## Errores que hacen perder tiempo

- **Sumar el desplazamiento y el tamaño en 32 bits.** Un archivo dañado con un desplazamiento enorme supera la comprobación de límites y el código lee muy fuera del archivo.
- **Una comparación con signo del índice.** Se acepta un índice negativo y la entrada de la tabla se calcula a partir de un número negativo.
- **Dejar la posición fuera de la clave.** Solo el primer byte de un bloque se descodifica bien.
- **Olvidar el tamaño en la suma de verificación.** Todo archivo con un bloque no vacío se declara dañado, pero un bloque vacío pasa, lo que oculta el error a una prueba apresurada.
- **Rotar 8.** El primer byte se descodifica bien y la suma de verificación nunca coincide.
- **Buscar el byte cero sin límite.** Un archivo cuyo último nombre no tiene terminador hace que el código lea más allá del final.
- **Escribir la salida antes de comprobar la capacidad.** La referencia permite cualquier contenido en `out` tras un error, pero nunca más allá de `cap`.
