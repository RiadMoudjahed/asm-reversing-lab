<!-- source: README.md @ sha256:312dbb9b2163 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 17. Descifrar formatos de archivo

Tiempo: unas 2,5 horas. Requisitos previos: [16. Interfaces sin documentar](../16-undocumented-interfaces/README.es.md)

## Por qué importa

Los programas guardan sus datos en archivos cuya estructura nadie escribió: la exportación de un proveedor, un formato de guardado antiguo, un registro escrito por un dispositivo. Para leer un archivo así, convertirlo, repararlo o comprobar que es seguro abrirlo, necesitas su estructura, y está escondida en dos lugares: los bytes de archivos de muestra y el código del programa que los lee o los escribe. Este módulo enseña a combinar ambos, a reconocer las partes habituales de un formato, a identificar una suma de verificación y a convertir los hallazgos en una especificación que luego pruebas con un analizador propio.

## Conceptos

### El formato de práctica

La lección trabaja con un formato pequeño para registros de sensores. El objetivo es el programa lector `17_reader`, construido sin símbolos a partir de `examples/17_reader.c`; la lección usa solo el programa compilado y archivos de muestra creados con `examples/17_make.py`.

```bash
make run
```

```text
python3 17_make.py build/sample.lg
build/17_reader build/sample.lg
channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
```

El lector imprime un resumen y nada más. Lo que el archivo contiene de verdad lo tienes que descubrir tú.

### Paso 1: mirar los bytes

Empieza con un volcado hexadecimal. `xxd` muestra el desplazamiento, los bytes y su forma imprimible. `strings -a -t x` lista tramos de caracteres imprimibles con sus desplazamientos en el archivo.

```bash
xxd build/sample.lg
strings -a -t x build/sample.lg
```

```text
00000000: 4c47 5231 0200 0200 401f 0000 4d45 5441  LGR1....@...META
00000010: 1300 0000 756e 6974 3d6d 560a 7369 7465  ....unit=mV.site
00000020: 3d6e 6f72 7468 0a44 4154 410c 0000 0064  =north.DATA....d
00000030: 00c8 002c 0190 01f4 0158 024e 4f54 4507  ...,.....X.NOTE.
00000040: 0000 0063 6865 636b 6564 454e 4420 0400  ...checkedEND ..
00000050: 0000 0898 7769                           ....wi
      0 LGR1
      c META
     14 unit=mV
     1c site=north
     27 DATA
     3b NOTE
     43 checkedEND 
```

Léelo como un conjunto de hipótesis.

| Observación | Hipótesis |
|---|---|
| El archivo empieza por `LGR1` | Un **valor mágico**: un identificador fijo en el desplazamiento 0 que dice a un lector de qué formato se trata |
| Aparecen palabras de cuatro letras `META`, `DATA`, `NOTE`, `END`, cada una seguida de un número pequeño y luego contenido | Bloques con etiqueta: una etiqueta, una longitud, una carga útil. Tras `META` viene `13 00 00 00`, y el texto que sigue mide 19 bytes (0x13) |
| `0c 00 00 00` tras `DATA`, y luego 12 bytes | El mismo patrón: longitud 12 y 12 bytes de carga útil, que serían 6 valores de 2 bytes |
| Números como `13 00 00 00` tienen primero el byte bajo | **Little endian**, el orden de bytes de x86 (módulo 04) |
| Los cuatro últimos bytes parecen aleatorios | Una **suma de verificación**: un valor calculado a partir de los demás bytes que detecta daños |
| Los bytes 4 a 11 son `02 00 02 00 40 1f 00 00` | Una cabecera de campos con tamaños por determinar: un valor de 1 byte igual a 2, un valor de 1 byte igual a 0, luego 2 y 8000 (0x1f40) en 2 y 4 bytes |

Un **campo** es una parte con nombre del archivo, con posición fija o dada por campos anteriores. Su **desplazamiento** cuenta bytes desde el inicio del archivo. Su **longitud** puede ser fija o estar guardada en otro campo.

### Paso 2: cambiar una sola cosa cada vez

La herramienta más potente para localizar campos es la comparación. Crea dos muestras que difieran en exactamente un parámetro y compara los volcados. Aquí está disponible el programa que crea muestras; con un archivo que solo recibiste, compara dos archivos que tengas o cambia tú un byte y mira qué dice el lector.

```bash
python3 17_make.py /tmp/r.lg --rate 16000
diff <(xxd build/sample.lg) <(xxd /tmp/r.lg)
```

```text
1c1
< 00000000: 4c47 5231 0200 0200 401f 0000 4d45 5441  LGR1....@...META
---
> 00000000: 4c47 5231 0200 0200 803e 0000 4d45 5441  LGR1.....>..META
6c6
< 00000050: 0000 0898 7769                           ....wi
---
> 00000050: 0000 5ef2 5aa3                           ..^.Z.
```

Cambiar la tasa de 8000 a 16000 cambia los bytes 8 y 9 (0x1f40 pasó a 0x3e80), así que esos cuatro bytes en el desplazamiento 8 contienen la tasa. Los cuatro últimos bytes también cambiaron: dependen del contenido. El mismo experimento con `--channels 3` cambia el byte 6 y los cuatro últimos bytes, lo que sitúa el número de canales en el desplazamiento 6. Con `--samples 7`, la longitud tras `DATA` pasa de 0x0c a 0x0e, aparece un valor nuevo en la carga útil, los desplazamientos de todo lo posterior se mueven 2 y la suma de verificación vuelve a cambiar. Eso muestra que la longitud cuenta bytes (7 muestras de 2 bytes son 14) y que los bloques se suceden sin ningún índice.

### Paso 3: encontrar el analizador en el programa

Los bytes dan hipótesis; el programa que lee el archivo las decide. Busca el código que hace la lectura, usando lo que sea más fácil de encontrar.

- **Mensajes de error.** `strings -a -t x build/17_reader` muestra `bad magic`, `unsupported version`, `truncated chunk header`, `checksum mismatch`, `chunk too large`. Los mensajes de un lector nombran sus comprobaciones. Encuentra el código que hace referencia a cada cadena; la condición que salta a ella es la comprobación.
- **Importaciones.** El programa importa `fopen`, `fread` y `memcmp`. Las llamadas a `fread` muestran cuántos bytes se leen cada vez; sus argumentos son el tamaño del siguiente campo o estructura.
- **Constantes.** Una comparación con una etiqueta o un valor mágico, o un bucle sobre una tabla.

En Rizin, `axt` lista el código que usa una dirección, y en Ghidra se hace lo mismo con las referencias cruzadas (módulo 11). La primera parte del listado siguiente es `main` después de abrir el archivo y construir la tabla. Cada llamada a `fread` está anotada.

```text
401248:  mov    rcx,r15
40124b:  mov    edx,0xc                     ; size = 12
401250:  mov    esi,0x1                     ; one-byte items
401255:  lea    rdi,[rsp+0x34]              ; buffer
40125a:  call   fread@plt
40125f:  cmp    rax,0xc                     ; fewer than 12 bytes read: "truncated header"
401263:  jne    4012f5
401269:  mov    edx,0x4
40126e:  mov    esi,0x402047                ; a 4-byte constant in .rodata
401273:  lea    rdi,[rsp+0x34]
401278:  call   memcmp@plt                  ; compare the first 4 bytes with it
40127d:  test   eax,eax
40127f:  jne    401307                      ; different: "bad magic"
401285:  cmp    BYTE PTR [rsp+0x38],0x2     ; byte at offset 4 must be 2: "unsupported version"
40128a:  jne    401319
401290:  movzx  r14d,BYTE PTR [rsp+0x39]    ; byte at offset 5: kept for the output (flags)
```

El programa lee una cabecera de 12 bytes, como sugería el volcado. La constante de la dirección 0x402047 está en `.rodata`. En este archivo el desplazamiento es igual a la dirección menos la dirección de carga 0x400000, así que `xxd -s 0x2047 -l 4 build/17_reader` muestra `LGR1`. El byte del desplazamiento 4 se compara con 2 y el byte del desplazamiento 5 se conserva. Después vienen los bytes de los desplazamientos 6 y 7 y de los desplazamientos 8 a 11; el código los ensambla con desplazamientos de bits en un número de 16 bits y otro de 32 bits, que es el aspecto de little endian en el código:

```text
4013b4:  movzx  ecx,BYTE PTR [rsp+0x1d]     ; byte 1 of the rate
4013b9:  shl    ecx,0x8
4013bc:  movzx  eax,BYTE PTR [rsp+0x1e]     ; byte 2
4013c1:  shl    eax,0x10
4013c4:  or     ecx,eax
4013c6:  movzx  eax,BYTE PTR [rsp+0x1c]     ; byte 0
4013cb:  or     ecx,eax
...
```

Sigue el bucle sobre los bloques:

```text
40143b:  mov    rcx,r15
40143e:  mov    edx,0x8                     ; read 8 bytes: a chunk header
401443:  mov    esi,0x1
401448:  lea    rdi,[rsp+0x2c]
40144d:  call   fread@plt
401452:  cmp    rax,0x8
401456:  jne    40132b                      ; "truncated chunk header"
...                                         ; bytes 4 to 7 combined into a 32-bit length in ebp
40147f:  mov    edx,0x4
401484:  mov    esi,0x402081                ; the tag "END "
401489:  lea    rdi,[rsp+0x2c]
40148e:  call   memcmp@plt                  ; first 4 bytes against "END "
401493:  mov    r13d,eax
401496:  test   eax,eax
401498:  je     40133d                      ; equal: handle the end chunk
40149e:  cmp    ebp,0x10000
4014a4:  ja     401413                      ; length above 65536: "chunk too large"
4014aa:  mov    r14d,ebp
4014ad:  mov    rcx,r15
4014b0:  mov    rdx,r14                     ; read length bytes
4014b3:  mov    esi,0x1
4014b8:  mov    edi,0x404060                ; into a static buffer
4014bd:  call   fread@plt
4014c5:  cmp    r14,rax
4014c8:  jne    401425                      ; fewer than length bytes: "truncated chunk"
```

Cada cabecera de bloque mide 8 bytes: una etiqueta de 4 bytes seguida de una longitud little endian de 4 bytes. La carga útil se lee según la longitud, con un límite superior de 0x10000 bytes. Nada en el bucle alinea ni rellena, lo que concuerda con el volcado. Tras la carga útil, el código llama dos veces a una función (en 0x4014db y 0x4014ea): una con los 8 bytes de la cabecera y otra con la carga útil, y los resultados se encadenan a través de `r12d`. Es un valor acumulado sobre todo lo leído hasta ese momento. Esa es la suma de verificación.

Tras la actualización de la suma de verificación, tres llamadas más a `memcmp` comparan la etiqueta con `DATA`, `META` y `NOTE`. Solo se cuentan esas tres etiquetas; cualquier otra se lee y se incluye en la suma de verificación, pero por lo demás se ignora. Es un hecho sobre el formato que ninguna muestra podría mostrar.

### Paso 4: identificar la suma de verificación

Una suma de verificación es una función sobre bytes. Las habituales se reconocen por sus constantes y su estructura.

| Tipo | Cómo reconocerla |
|---|---|
| Suma simple o xor de bytes | Un bucle con `add` o `xor` de cada byte en un acumulador |
| Adler-32 | Dos sumas acumuladas, módulo 65521 (0xfff1) |
| CRC-32 | La constante 0xedb88320 (o 0x04c11db7), o una tabla de 256 valores que empieza por 0x00000000, 0x77073096, 0xee0e612c, 0x990951ba |

En `17_reader`, la función de 0x401166 es un bucle que procesa un byte por vuelta:

```text
401173:  mov    edx,eax                     ; eax holds the running value
401175:  xor    dl,BYTE PTR [rcx]           ; low byte xor the next input byte
401177:  movzx  edx,dl                      ; ... as an index from 0 to 255
40117a:  shr    eax,0x8                     ; running value shifted right by one byte
40117d:  xor    eax,DWORD PTR [rdx*4+0x414060]   ; xor the table entry for that index
401184:  add    rcx,0x1
401188:  cmp    rcx,rsi
40118b:  jne    401173
```

Un bucle anterior rellena la tabla, y contiene la instrucción `and eax,0xedb88320`:

```text
401231:  mov    eax,edx
401233:  and    eax,0x1
401236:  neg    eax                         ; 0 or 0xffffffff
401238:  and    eax,0xedb88320              ; the polynomial, reflected
40123d:  shr    edx,1
40123f:  xor    edx,eax
401241:  sub    ebx,0x1
401244:  jne    401231                      ; eight rounds for each table entry
```

Esta es la forma habitual de construir la tabla del CRC-32, y los primeros valores son los citados arriba. Hay que fijar tres detalles de una suma de verificación porque cambian el resultado: el valor inicial (aquí `0xffffffff`, el argumento `edi` de la primera llamada), el paso final (aquí `not r12d` antes de la comparación con el valor guardado, en 0x401377) y el rango de bytes que cubre. El rango sale del bucle: la cabecera (12 bytes) y cada bloque anterior al bloque final, cada uno con su cabecera de 8 bytes. El bloque final no se incluye. El valor guardado se lee como cuatro bytes, primero el byte bajo. El `zlib.crc32` de Python implementa el mismo CRC-32 con ese valor inicial y ese paso final, así que la regla se puede probar directamente:

```bash
python3 -c "import zlib;d=open('build/sample.lg','rb').read();print(hex(zlib.crc32(d[:-12])), d[-4:].hex())"
```

```text
0x69779808 08987769
```

El valor calculado 0x69779808 equivale a los bytes guardados `08 98 77 69` leídos en little endian. Cuando no puedas deducir el rango del código, prueba los candidatos (el archivo entero sin los cuatro últimos bytes, sin la cabecera, sin el bloque final) y mira cuál coincide con la muestra.

### Paso 5: escribir la especificación

La especificación es el producto del trabajo. Debe ser lo bastante precisa para que otra persona pueda escribir un lector o un escritor a partir de ella, y debe indicar lo que no se sabe.

| Desplazamiento | Tamaño | Campo | Notas |
|---|---|---|---|
| 0 | 4 | Mágico | Los bytes `LGR1` |
| 4 | 1 | Versión | Debe ser 2; otros valores se rechazan |
| 5 | 1 | Indicadores | Se conservan y se muestran; el lector no los interpreta |
| 6 | 2 | Canales | Little endian; se muestra |
| 8 | 4 | Tasa en Hz | Little endian; se muestra |
| 12 | | Bloques | Uno tras otro, sin relleno, hasta el bloque final |

Estructura de un bloque: una etiqueta de 4 bytes, una longitud little endian de 4 bytes de como máximo 65536 y la carga útil. Etiquetas: `DATA` (se cuenta como longitud / 2 muestras), `META` y `NOTE` (se cuentan), cualquier otra etiqueta (se ignora, pero la cubre la suma de verificación). El bloque `END` (su etiqueta son las tres letras seguidas de un espacio) tiene longitud 4 y una carga útil con el CRC-32 (polinomio 0xedb88320, valor inicial 0xffffffff, complemento final) de todos los bytes desde el desplazamiento 0 hasta el inicio del bloque `END`, guardado en little endian. Los bytes posteriores al bloque final no se leen. Sin determinar: qué significan los indicadores y cómo se organiza entre canales la carga útil de `DATA`, porque el lector nunca los mira.

Indica las comprobaciones en el orden en que las hace el lector, porque el orden decide qué error produce un archivo dañado: tamaño de la cabecera, valor mágico, versión y, por cada bloque, el tamaño de su cabecera, el bloque final, el límite de longitud, el tamaño de la carga útil y, al final, la suma de verificación.

### Paso 6: validar con un analizador propio

Una especificación se prueba implementándola de forma independiente y comparándola con el original en archivos que el original acepta y en archivos que rechaza. El archivo `examples/17_parse.py` está escrito solo a partir de la tabla anterior. Puede volcar lo que decodifica:

```bash
python3 17_parse.py build/sample.lg --dump
```

```text
0x0000  header: version=2 flags=0x00 channels=2 rate=8000
0x000c  META  length=19
0x0027  DATA  length=12
0x003b  NOTE  length=7
0x004a  END   length=4 crc stored=0x69779808 computed=0x69779808
channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
```

El script `examples/17_compare.py` construye variantes dañadas de la muestra (un valor mágico, versión, tasa o byte de indicadores cambiado; un byte de carga útil cambiado; truncamiento en varios puntos; bytes adicionales; una longitud imposible) y compara el veredicto del lector original con el del analizador:

```bash
python3 17_compare.py build/17_reader build/sample.lg
```

```text
same  unchanged                    channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
same  empty file                   error: truncated header
same  magic changed                error: bad magic
same  version 1                    error: unsupported version
same  version 3                    error: unsupported version
same  flags changed                error: checksum mismatch
same  rate changed                 error: checksum mismatch
same  one payload byte changed     error: checksum mismatch
same  stored checksum changed      error: checksum mismatch
same  tag changed to unknown       error: checksum mismatch
same  cut inside header            error: truncated header
same  cut inside a chunk header    error: truncated chunk header
same  cut inside a payload         error: truncated chunk
same  cut before end chunk         error: truncated chunk header
same  cut inside end chunk         error: bad end chunk
same  bytes appended               channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
same  huge length                  error: chunk too large
same  end length 5                 error: bad end chunk
18 of 18 verdicts agree
```

Todas las líneas dicen `same`, así que la especificación cubre estos casos. Varias líneas muestran puntos que una especificación debe registrar: un indicador o una tasa cambiados solo los detecta la suma de verificación (el lector no comprueba los valores), y se aceptan bytes añadidos tras el bloque final. Cuando una línea dice `DIFF`, o bien el analizador o bien la especificación está mal: busca el primer byte donde el lector se detiene y compáralo con la regla. Ante cualquier duda, ejecuta el original; es la referencia definitiva.

## Pruébalo

Ejecuta los comandos del laboratorio desde `modules/17-deciphering-file-formats/examples`.

1. Compila con `make`. Vuelca `build/sample.lg` con `xxd` y marca en papel la cabecera, cada cabecera de bloque y cada carga útil antes de leer la tabla de la lección.
2. Crea muestras con `17_make.py --flags 1`, `--samples 0` y `--samples 1`. Predice cómo cambia el volcado y compáralo después con `diff <(xxd a) <(xxd b)`.
3. En Rizin, ejecuta `axt` sobre la dirección de la cadena `checksum mismatch` (`rizin -A build/17_reader` y luego `iz~checksum` para la dirección) y lee el código que salta allí.
4. Abre `build/17_reader` en Ghidra. Encuentra la llamada a `memcmp` que compara cuatro bytes con el valor mágico, renombra la función que actualiza la suma de verificación y el búfer que contiene la tabla, y escribe la tabla de campos a partir del código descompilado.
5. Quita el último bloque de `sample.lg` con un script y corrige la suma de verificación calculando de nuevo el CRC-32. ¿Qué error da el lector si calculas la suma de verificación sobre un rango de bytes equivocado?
6. Amplía `17_parse.py` para que imprima el contenido de `META` como pares clave y valor. Compruébalo con `sample.lg`.

## Errores frecuentes

- **Suponer big endian.** Lee el campo como lo ensambla el código. Un desplazamiento del primer byte en 8 o más significa que ese byte no es el menos significativo.
- **Adivinar a partir de una muestra.** Dos muestras que difieren en un parámetro localizan un campo; una muestra no puede.
- **Olvidar recalcular la suma de verificación.** Tras cada edición de un campo, calcula de nuevo la suma de verificación, o el lector rechaza el archivo antes de llegar al campo que cambiaste.
- **Pasar por alto una dependencia entre campos.** Una longitud que cuenta bytes de carga útil, una cuenta que cuenta elementos y un desplazamiento desde el inicio de una sección se comportan de forma distinta cuando cambias un tamaño.
- **Documentar solo lo que muestran las muestras.** La etiqueta que el lector ignora, el límite de longitud y los bytes posteriores al bloque final aparecen solo en el código.
- **Saltarse los casos de fallo.** Un analizador que acepta todo lo que acepta el lector, pero también lo que el lector rechaza, no es equivalente. Prueba archivos dañados.

## Resumen

- Un formato se descifra a partir de dos fuentes: los bytes de las muestras y el código que los lee o los escribe. Usa ambas.
- Las partes habituales son un valor mágico, una cabecera de campos, longitudes, cuentas, etiquetas, desplazamientos y una suma de verificación; hay que fijar el orden de bytes de cada número.
- Compara muestras que difieren en un parámetro para localizar campos, y lee el analizador para conocer límites, orden de las comprobaciones y casos ignorados.
- Una suma de verificación se identifica por sus constantes; su valor inicial, su paso final y el rango cubierto se determinan después a partir del código y se prueban con una función de biblioteca.
- La especificación enumera campos con desplazamientos, tamaños y significados, las reglas y el orden de las comprobaciones, y dice lo que se desconoce. Un analizador escrito a partir de ella y comparado con el original en archivos dañados la valida.

## Próximos pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 18, auditar binarios en busca de errores.
- Ejercicios que usan este módulo: [Ejercicio 17](../../exercises/17-archive-of-unknowns/README.es.md).
