<!-- source: README.md @ sha256:dcf22bed5b5c -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 01. Representación de datos

Tiempo: unos 90 minutos. Requisitos previos: [00. Preparación del laboratorio](../00-setup/README.es.md)

## Por qué es importante

Un procesador almacena todo como bits y no sabe si un byte es un número, una letra o parte de una dirección. Tú decides qué significan los bits, y lo mismo ocurre con cada programa que analices. La mayoría de los errores de principiante en ensamblador y en ingeniería inversa (un signo equivocado, un valor invertido, un carácter que falta) vienen de leer mal los mismos bits.

## Conceptos

### Bits, bytes y tamaños

Un **bit** vale 0 o 1. Un **byte** son 8 bits. Las unidades mayores se cuentan en bytes:

| Nombre | Bytes | Bits | Rango sin signo |
|---|---|---|---|
| byte | 1 | 8 | 0 a 255 |
| palabra (word) | 2 | 16 | 0 a 65535 |
| palabra doble (dword) | 4 | 32 | 0 a 4294967295 |
| palabra cuádruple (qword) | 8 | 64 | 0 a 2^64 - 1 |

Estos nombres vienen de los primeros procesadores de 16 bits y se mantienen en el código de 32 y de 64 bits, así que una "palabra" en x86 son siempre 2 bytes.

### Binario y hexadecimal

El binario es exacto pero largo. El **hexadecimal** (base 16) escribe cuatro bits como un dígito, de modo que un byte son exactamente dos dígitos hexadecimales. NASM escribe el hexadecimal como `0x1F` o `1Fh`; este curso usa `0x`.

Para convertir, agrupa los dígitos binarios de cuatro en cuatro desde la derecha:

```text
1111 0110  ->  F 6  ->  0xF6
```

### Enteros sin signo y con signo

El patrón de bits `0xF6` es 246 si se lee sin signo. Los ordenadores guardan los números negativos en **complemento a dos**: para negar un número se invierten todos sus bits y se suma uno.

```text
 10 = 0000 1010
~10 = 1111 0101
+1  = 1111 0110  = 0xF6  = -10
```

Por tanto, el mismo byte `0xF6` es 246 sin signo y -10 con signo. El procesador no registra cuál querías; las instrucciones que elijas después (como los saltos con signo frente a los saltos sin signo del módulo 06) llevan ese significado. Los rangos con signo son asimétricos: los valores de 8 bits con signo van de -128 a 127, y los de 32 bits de -2147483648 a 2147483647. El bit superior indica el signo.

### Extensión de signo y extensión con ceros

Ampliar un valor a más bits puede hacerse de dos maneras, y la elección importa:

- La **extensión con ceros** rellena los nuevos bits superiores con 0. Es correcta para valores sin signo: `0xF6` (246) pasa a ser `0x000000F6` (246).
- La **extensión de signo** copia el bit superior en los nuevos bits. Es correcta para valores con signo: `0xF6` (-10) pasa a ser `0xFFFFFFF6` (-10).

Usar la equivocada cambia el número. El módulo 04 presenta las instrucciones `movzx` y `movsx`, que hacen cada una de ellas.

### Orden de bytes

Un valor de varios bytes debe escribirse en memoria byte a byte. x86 es **little-endian**: el byte menos significativo va a la dirección más baja. El valor `0x11223344` se almacena así:

```text
address:   +0  +1  +2  +3
bytes:     44  33  22  11
```

Cuando leas un volcado hexadecimal, invierte los bytes para obtener el valor: `44 33 22 11` es `0x11223344`. Los protocolos de red suelen usar **big-endian** (el byte más significativo primero), así que te encontrarás con ambos.

### Texto

El texto son bytes que siguen un acuerdo. En **ASCII**, un byte es un carácter: `0x48` es `H`, `0x69` es `i`, `0x21` es `!`, `0x0A` es el salto de línea y `0x00` suele usarse como terminador. **UTF-8** amplía ASCII: los caracteres por debajo de 128 son el mismo byte único y los demás usan de dos a cuatro bytes. Quien solo lee ASCII ve el texto UTF-8 correcto para el inglés y como bytes inexplicables por encima de `0x7F` para todo lo demás.

### Dígitos decimales en bytes

Dos formatos más antiguos almacenan dígitos decimales y aún aparecen en algunos formatos de datos:

- **BCD desempaquetado**: un dígito decimal por byte. El número 59 es `05 09`.
- **BCD empaquetado**: dos dígitos por byte, uno por cada medio byte. El número 59 es `0x59`.

Leer `0x59` como binario da 89, no 59, así que debes saber qué formato tienes.

### La perspectiva de la ingeniería inversa

En un volcado hexadecimal ves bytes, no significado. Las preguntas que hay que hacer a cualquier valor que encuentres son: cuántos bytes, con signo o sin signo, qué orden de bytes y si es texto. Un valor de 4 bytes `44 33 22 11` en un programa little-endian es el número `0x11223344`; los mismos bytes leídos como texto son `D3"` seguido de un carácter de control, lo que sería una coincidencia. El contexto, como la forma en que el programa usa los bytes, lo decide.

## Pruébalo

Compila y ejecuta el programa de ejemplo. Está en [01_representation.c](examples/01_representation.c).

```bash
cd modules/01-data-representation/examples
make run
```

Salida esperada:

```text
0xF6 as unsigned: 246
0xF6 as signed:   -10
0xF6 as binary:   11110110
-10 by hand:      -10 (0xF6)
sign-extended:    0xFFFFFFF6
zero-extended:    0x000000F6
0x11223344 in memory: 44 33 22 11
"Hi!" bytes:   48 69 21
```

Relaciona cada línea con una sección anterior. Después comprueba tú mismo la regla del complemento a dos con Python:

```bash
python3 -c "print(hex(-10 & 0xFF))"
```

Esto imprime `0xf6`. Ahora modifica el ejemplo: haz que `word` valga `0xDEADBEEF`, vuelve a compilar y predice los cuatro bytes antes de ejecutarlo. La respuesta está en la regla del orden de bytes de arriba.

## Errores frecuentes

- **Leer un volcado hexadecimal de izquierda a derecha como un valor.** En x86 el primer byte es el menos significativo. Inviértelo.
- **Tratar con signo y sin signo como datos distintos.** Son los mismos bits; solo cambia tu interpretación.
- **Extender con la regla equivocada.** Extender con ceros un número negativo lo convierte en uno positivo grande.
- **Confundir `0x59` con 59.** En BCD empaquetado son iguales; en binario no.
- **Suponer que un carácter es un byte.** Eso solo vale para ASCII.

## Resumen

- Un byte son 8 bits, una palabra son 2 bytes, una palabra doble son 4 y una palabra cuádruple son 8.
- El complemento a dos representa los números negativos: invierte los bits y suma uno.
- La extensión con ceros conviene a valores sin signo; la extensión de signo, a valores con signo.
- x86 es little-endian: el byte menos significativo está en la dirección más baja.
- Los mismos bits pueden ser un número, texto o una dirección; el contexto decide.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con [02. La CPU y sus registros](../02-cpu-and-registers/README.es.md).
- Ejercicios que usan este módulo: [Ejercicio 04](../../exercises/04-byte-order/README.es.md), [Ejercicio 05](../../exercises/05-ticket-counter/README.es.md).
