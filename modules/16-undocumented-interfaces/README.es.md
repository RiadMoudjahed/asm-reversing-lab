<!-- source: README.md @ sha256:d99c551debae -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 16. Interfaces sin documentar

Tiempo: unas 2 horas. Requisitos previos: [15. Recuperar datos](../15-recovering-data/README.es.md)

## Por qué importa

Gran parte del código que encuentras llega como una biblioteca sin cabecera y sin documentación: un binario de un proveedor, un componente antiguo cuyo código fuente se perdió, una interfaz de complemento. Para llamarla, sustituirla o comprobar lo que hace, necesitas su interfaz: los nombres, los argumentos, el valor de retorno y las reglas de uso. El binario lo contiene todo. Este módulo enseña un método para recuperar una interfaz a partir del código compilado y para confirmar el resultado con un programa de prueba antes de fiarte de él.

## Conceptos

### Lo que se ve desde fuera

Una biblioteca compartida enumera las funciones que ofrece a otros en su tabla de símbolos dinámicos. Esa lista es la lista de **exportaciones**, y es lo primero que hay que leer. La imagen del laboratorio construye la biblioteca de práctica `libblackbox.so` a partir de `examples/16_blackbox.c`; la lección trabaja solo con el binario.

```bash
make
nm -D build/libblackbox.so
```

```text
                 w _ITM_deregisterTMCloneTable
                 w _ITM_registerTMCloneTable
                 w __cxa_finalize@GLIBC_2.2.5
                 w __gmon_start__
0000000000001126 T bb_checksum
0000000000001119 T bb_clamp
000000000000121f T bb_close
00000000000011dc T bb_lerp
00000000000011a5 T bb_minmax
0000000000001213 T bb_next
00000000000011e9 T bb_open
0000000000001145 T bb_split
                 U calloc@GLIBC_2.2.5
                 U free@GLIBC_2.2.5
```

`T` marca una función que la biblioteca define y `U` una que necesita de fuera. Los nombres ya son una pista: `bb_open`, `bb_next` y `bb_close` parecen un conjunto, y `calloc` y `free` aparecen en la lista de símbolos sin definir. Los nombres pueden engañar; trata cada uno como una hipótesis, nunca como un hecho. Las importaciones también son una buena prueba: una biblioteca que importa `calloc` reserva memoria en algún sitio.

### La firma que buscas

En una biblioteca de C, la **firma** de una función es su tipo de retorno, su nombre y los tipos de sus parámetros. El procesador no guarda tipos, así que los reconstruyes a partir de cómo tratan las instrucciones cada registro. En x86-64 System V, los primeros argumentos enteros o punteros llegan en `rdi`, `rsi`, `rdx`, `rcx`, `r8` y `r9`, los de coma flotante en `xmm0` a `xmm7`, y el resultado sale en `rax` o `xmm0`. El método hace cinco preguntas a cada función.

| Pregunta | Qué buscar |
|---|---|
| ¿Cuántos argumentos? | Registros de argumento que se leen antes de escribirse. El más alto que se lee da el número mínimo de argumentos |
| ¿Qué anchura tiene cada uno? | `edi` es de 32 bits, `rdi` de 64 bits, `dil`, `sil` o `r8b` de 8 bits. La anchura del operando sigue al tipo de C |
| ¿Es un puntero? | El registro se usa entre corchetes, `[rdi]`, o se suma a otro registro como dirección |
| ¿A qué apunta? | La anchura del acceso: `BYTE PTR`, `DWORD PTR`, `QWORD PTR`; la escala de un índice; `movzx` o `movsx` tras una carga |
| ¿Con signo o sin signo? | `jl`, `jg`, `cmovg`, `movsx`, `sar` para con signo; `jb`, `ja`, `movzx`, `shr` para sin signo |
| ¿Qué devuelve? | La última escritura en `eax`, `rax` o `xmm0` antes de `ret`, y su anchura. Si ningún camino escribe el registro, la función no devuelve nada |

Trata el resultado como una hipótesis hasta que una prueba lo confirme. Una función void puede dejar un valor sin relación en `rax`, y un argumento que la función ignora no deja ningún rastro.

### Ejemplo 1: una función con valores simples

```bash
objdump -d -M intel --no-show-raw-insn build/libblackbox.so | sed -n '/ 1119:/,/ 1125:/p'
```

```text
    1119:    mov    eax,esi
    111b:    cmp    edi,edx
    111d:    cmovle edx,edi
    1120:    cmp    edi,esi
    1122:    cmovge eax,edx
    1125:    ret
```

La función lee `edi`, `esi` y `edx` y escribe `eax`: tres argumentos de 32 bits y un resultado de 32 bits. Todas las comparaciones son con signo (`cmovle`, `cmovge`), así que los valores son `int`. Sigue la lógica. El resultado empieza como `esi`. El primer `cmp` compara `edi` con `edx`, y cuando `edi` es menor o igual que `edx` sustituye `edx` por `edi`; así `edx` pasa a ser el menor de `edi` y `edx`. Si `edi` es mayor o igual que `esi`, el resultado pasa a ser ese valor limitado; si no, sigue siendo `esi`. La conclusión: `esi` es una cota inferior, `edx` es una cota superior y `edi` es el valor que se limita.

```c
int bb_clamp(int v, int lo, int hi);
```

Los nombres `v`, `lo` y `hi` son tuyos. Elegir nombres que describan el papel forma parte del trabajo.

### Ejemplo 2: un búfer y una longitud

```text
0000000000001126 <bb_checksum>:
    1126:    mov    eax,edx
    1128:    test   rsi,rsi
    112b:    je     1144
    112d:    mov    rdx,rdi
    1130:    add    rdi,rsi
    1133:    rol    eax,0x5
    1136:    movzx  ecx,BYTE PTR [rdx]
    1139:    xor    eax,ecx
    113b:    add    rdx,0x1
    113f:    cmp    rdx,rdi
    1142:    jne    1133
    1144:    ret
```

- `rdi` se usa como dirección y se le suma `rsi` para obtener una dirección final: `rdi` es un puntero y `rsi` es una cuenta. La cuenta se prueba como valor de 64 bits (`test rsi,rsi`), así que es `size_t`, no `int`.
- Cada vuelta carga un byte con `movzx ecx,BYTE PTR [rdx]` y lo extiende con ceros. La extensión con ceros indica que el tipo del elemento es `unsigned char`. El código solo lee el búfer, así que el puntero es `const`.
- `eax` empieza como `edx`: el tercer argumento es un valor inicial de 32 bits. Cada vuelta rota `eax` 5 bits a la izquierda y le aplica un xor con el byte. El resultado está en `eax`.

```c-fragment
unsigned bb_checksum(const unsigned char *p, size_t n, unsigned seed);
```

El comportamiento es una hipótesis que puedes enunciar con exactitud: empezar con `seed` y, para cada byte, rotar 5 bits a la izquierda y aplicar xor con el byte. Más adelante la probarás con un programa independiente.

### Ejemplo 3: un valor de retorno que es un código de error

```text
0000000000001145 <bb_split>:
    1145:    mov    r9d,esi
    1148:    test   rcx,rcx
    114b:    je     1195
    114d:    movzx  r8d,BYTE PTR [rdi]
    1151:    cmp    r8b,sil
    1154:    je     118b
    1156:    test   r8b,r8b
    1159:    je     118b
    115b:    cmp    rcx,0x1
    115f:    jbe    119d
    1161:    mov    eax,0x1
    1166:    mov    BYTE PTR [rdx+rax*1-0x1],r8b
    116b:    movzx  r8d,BYTE PTR [rdi+rax*1]
    1170:    cmp    r8b,r9b
    1173:    je     1190
    1175:    test   r8b,r8b
    1178:    je     1190
    117a:    add    rax,0x1
    117e:    cmp    rcx,rax
    1181:    jne    1166
    1183:    mov    rax,0xffffffffffffffff
    118a:    ret
    118b:    mov    eax,0x0
    1190:    mov    BYTE PTR [rdx+rax*1],0x0
    1194:    ret
    1195:    mov    rax,0xffffffffffffffff
    119c:    ret
    119d:    mov    rax,0xffffffffffffffff
    11a4:    ret
```

- Cuatro argumentos: se leen `rdi`, `sil` (el byte bajo de `esi`), `rdx` y `rcx`. `sil` se compara con bytes (`cmp r8b,sil`), así que el segundo argumento es un `char`.
- `rdi` se lee byte a byte hasta que un byte es igual a `sil` o es cero: una cadena de origen y un separador.
- `rdx` se escribe byte a byte (`mov BYTE PTR [rdx+rax*1-0x1],r8b`) y termina con un byte cero (`mov BYTE PTR [rdx+rax*1],0x0`): un búfer de destino que recibe una cadena.
- `rcx` se compara con la posición y acota el bucle: la capacidad del destino, como `size_t`.
- El resultado se fija con `mov rax,0xffffffffffffffff` en tres caminos. Es un `-1` de 64 bits, así que el tipo de retorno es `long` (o `ssize_t`), y los tres caminos son los casos de error: capacidad cero, capacidad uno con un campo que no está vacío, y un campo de origen que no cabe. Si todo va bien, el resultado es el número de caracteres copiados.

```c-fragment
long bb_split(const char *s, char sep, char *out, size_t cap);
```

Dos detalles solo salen de una lectura cuidadosa: la capacidad incluye el cero final, y en caso de fallo la función puede haber escrito ya parte de la salida. Una prueba comprobará ambos.

### Ejemplo 4: parámetros de salida

```text
00000000000011a5 <bb_minmax>:
    11a5:    mov    r9d,DWORD PTR [rdi]
    11a8:    cmp    rsi,0x1
    11ac:    jbe    11d7
    11ae:    lea    r8,[rdi+0x4]
    11b2:    lea    rdi,[rdi+rsi*4]
    11b6:    mov    esi,r9d
    11b9:    mov    eax,DWORD PTR [r8]
    11bc:    cmp    r9d,eax
    11bf:    cmovg  r9d,eax
    11c3:    cmp    esi,eax
    11c5:    cmovl  esi,eax
    11c8:    add    r8,0x4
    11cc:    cmp    r8,rdi
    11cf:    jne    11b9
    11d1:    mov    DWORD PTR [rdx],r9d
    11d4:    mov    DWORD PTR [rcx],esi
    11d6:    ret
    11d7:    mov    esi,r9d
    11da:    jmp    11d1
```

- `rdi` apunta a elementos de 4 bytes, y `rsi` es el número de elementos (la dirección final es `rdi+rsi*4`): `const int *a, size_t n`.
- `rdx` y `rcx` no se leen; las últimas instrucciones guardan en `[rdx]` y `[rcx]`. Un puntero a través del cual la función solo escribe es un **parámetro de salida**: quien llama pasa la dirección de una variable y recibe el valor allí.
- Ningún camino escribe `eax` como resultado, así que la función no devuelve nada: `void`. (`eax` se usa como registro auxiliar dentro del bucle.)
- La primera instrucción lee `[rdi]` antes de comprobar la cuenta. Por tanto la función necesita que `n` sea al menos 1, una regla de uso que ningún tipo muestra.

```c-fragment
void bb_minmax(const int *a, size_t n, int *lo, int *hi);
```

### Ejemplo 5: valores de coma flotante

```text
00000000000011dc <bb_lerp>:
    11dc:    subsd  xmm1,xmm0
    11e0:    mulsd  xmm1,xmm2
    11e4:    addsd  xmm0,xmm1
    11e8:    ret
```

El sufijo `sd` significa escalar de doble precisión. La función lee `xmm0`, `xmm1` y `xmm2` y devuelve en `xmm0`. Con `a` en `xmm0`, `b` en `xmm1` y `t` en `xmm2`, el código calcula `a + (b - a) * t`. Los registros enteros no intervienen, así que la firma no tiene argumentos enteros. Un sufijo `ss` significaría `float`.

```c
double bb_lerp(double a, double b, double t);
```

### Ejemplo 6: un manejador con vida limitada

Tres funciones trabajan juntas.

```text
00000000000011e9 <bb_open>:
    11e9:    push   rbp
    11ea:    push   rbx
    11eb:    sub    rsp,0x8
    11ef:    mov    ebp,edi
    11f1:    mov    ebx,esi
    11f3:    mov    esi,0xc
    11f8:    mov    edi,0x1
    11fd:    call   1040 <calloc@plt>
    1202:    test   rax,rax
    1205:    je     120c
    1207:    mov    DWORD PTR [rax],ebp
    1209:    mov    DWORD PTR [rax+0x4],ebx
    120c:    add    rsp,0x8
    1210:    pop    rbx
    1211:    pop    rbp
    1212:    ret

0000000000001213 <bb_next>:
    1213:    add    DWORD PTR [rdi+0x8],0x1
    1217:    mov    eax,DWORD PTR [rdi+0x4]
    121a:    add    eax,DWORD PTR [rdi]
    121c:    mov    DWORD PTR [rdi],eax
    121e:    ret

000000000000121f <bb_close>:
    121f:    sub    rsp,0x8
    1223:    call   1030 <free@plt>
    1228:    add    rsp,0x8
    122c:    ret
```

- `bb_open` toma dos valores de 32 bits, llama a `calloc(1, 12)` y guarda los dos valores en el bloque nuevo en los desplazamientos 0 y 4. Devuelve el bloque, o nulo cuando falla la reserva (la rama `test rax,rax` se salta los almacenamientos). El bloque ocupa 12 bytes.
- `bb_next` toma el bloque, incrementa el campo de 32 bits en el desplazamiento 8, suma el campo del desplazamiento 4 al campo del desplazamiento 0 y devuelve el nuevo valor del campo del desplazamiento 0.
- `bb_close` pasa su argumento a `free`.

El bloque es un **manejador opaco**: quien llama recibe un puntero, lo pasa a otras funciones y nunca necesita conocer la disposición. Desde fuera, su tipo es `void *`. Desde dentro, has recuperado la disposición: un valor actual en 0, un paso en 4 y un contador de llamadas en 8. Las reglas de uso salen del conjunto: cada manejador de `bb_open` debe ir a `bb_close` exactamente una vez, un manejador no debe usarse después de cerrarlo, y hay que comprobar un resultado nulo de `bb_open`.

```c
void *bb_open(int start, int step);
int bb_next(void *c);
void bb_close(void *c);
```

### Los puntos de llamada son pruebas

Cuando un programa usa la biblioteca, el código que rodea cada llamada muestra cómo se usa el resultado. El archivo `examples/16_caller.c` llama a `bb_split` e informa de un fallo. Compilado con `-O1`:

```text
copy_key:
    sub    rsp, 8
    mov    rdx, rsi
    mov    ecx, 32
    mov    esi, 61
    call    bb_split@PLT
    test    rax, rax
    mov    rdx, -1
    cmovs    rax, rdx
    add    rsp, 8
    ret
```

Quien llama pasa su segundo parámetro (`rsi`) como tercer argumento, la constante 32 como capacidad y el carácter 61, que es `=`, como separador. Tras la llamada, `test rax,rax` y `cmovs` miran el signo de los 64 bits de `rax`: el resultado es un valor de 64 bits con signo, y uno negativo significa fallo. Si tienes un programa que usa una biblioteca, busca primero en él las llamadas. Las constantes, las pruebas tras cada llamada y el orden de las llamadas dicen qué esperaba el autor.

### Confirmar con un programa de prueba

Una hipótesis se convierte en conocimiento cuando un programa escrito a partir de ella se comporta como se predijo. Escribe los prototipos recuperados, llama a las funciones con valores normales y luego con los límites, y compara con lo que predijiste antes de ejecutar. El archivo `examples/16_probe.c` hace esto con la biblioteca de práctica:

```bash
make run
```

```text
clamp(5, 0, 3) = 3
clamp(-5, 0, 3) = 0
clamp(2, 0, 3) = 2
checksum("abc", 3, 0) = 0x00018823
checksum("abc", 3, 1) = 0x00010823
checksum("abc", 2, 0) = 0x00000c42
split("key=value", '=', out, 8) = 3, out = "key"
split("abcdefghij", ',', out, 8) = -1
split("abcdefg", ',', out, 8) = 7
minmax = -3 12
lerp(10.0, 20.0, 0.25) = 12.50
next = 105
next = 110
next = 115
counter bytes: 73 00 00 00 05 00 00 00 03 00 00 00 
```

Lee los resultados como pruebas de las hipótesis. `clamp` limita por ambos lados. La llamada a `split` con capacidad 8 falla con un campo de 10 caracteres y funciona con uno de 7, así que un campo de 7 caracteres más el terminador cabe exactamente en 8 bytes; eso confirma que la capacidad incluye el byte cero. La última línea vuelca el manejador: el valor es 0x73 (115), el paso es 5 y el contador es 3, la disposición recuperada de `bb_open` y `bb_next`.

Para la suma de verificación, la hipótesis es lo bastante precisa como para probarla con una implementación independiente en otro lenguaje. El script `examples/16_checksum.py` sigue la regla "rotar 5 bits a la izquierda y luego aplicar xor con el byte":

```bash
python3 16_checksum.py abc 3 0
python3 16_checksum.py abc 3 1
python3 16_checksum.py abc 2 0
```

```text
0x00018823
0x00010823
0x00000c42
```

Los tres coinciden con la biblioteca. La coincidencia con una entrada es una prueba débil; la coincidencia con muchas entradas, incluidos los límites (un búfer vacío, una semilla cero, búferes largos), es fuerte.

Una trampa del programa de prueba: C no define el orden en que se evalúan los argumentos de una llamada. Una línea como `printf("%d %d", bb_next(c), bb_next(c))` puede imprimir los valores en cualquier orden. El programa de prueba llama a la función en un bucle, una llamada por sentencia.

### Escribir la cabecera

El producto del trabajo es una cabecera que otro programador pueda usar. Contiene los prototipos, un comentario breve para cada regla de uso y nada más. También indica qué afirmaciones están confirmadas y cuáles no.

```c
#include <stddef.h>

/* Limits v to the range lo..hi. Requires lo <= hi. */
int bb_clamp(int v, int lo, int hi);

/* Rotate-and-xor checksum of n bytes, starting from seed. */
unsigned bb_checksum(const unsigned char *p, size_t n, unsigned seed);

/* Copies the text of s up to the first sep into out and ends it with a zero byte.
   cap is the size of out including the zero byte. Returns the number of characters
   copied, or -1 when the field does not fit. */
long bb_split(const char *s, char sep, char *out, size_t cap);

/* Smallest and largest of n values; n must be at least 1. */
void bb_minmax(const int *a, size_t n, int *lo, int *hi);

/* a + (b - a) * t */
double bb_lerp(double a, double b, double t);

/* Counter handle. bb_open returns null when memory runs out; every handle must be passed
   to bb_close exactly once and not used afterwards. bb_next adds step to the value and
   returns it. */
void *bb_open(int start, int step);
int bb_next(void *c);
void bb_close(void *c);
```

### Dónde el método necesita más cuidado

- **Argumentos que no se leen en algunos caminos.** Un parámetro usado solo en una rama que rara vez se toma puede parecer sin uso. Lee todos los caminos.
- **Estructuras pasadas o devueltas por valor.** Una estructura de más de 16 bytes se devuelve mediante un puntero oculto que llega en `rdi`, así que cada argumento visible se desplaza una posición. Las estructuras pequeñas viajan en registros repartidos por tipo.
- **Funciones variádicas.** `al` contiene el número de registros vectoriales usados. Una función que lee `al` al principio toma una lista variable de argumentos.
- **Retrollamadas.** Un argumento al que la función llama (`call rdx`, `call rax`) es un puntero a función. Su firma se deduce de cómo la función prepara los registros antes de esa llamada.
- **Salida que depende de un estado oculto.** Una función que lee una variable global da respuestas distintas con los mismos argumentos. Ejecuta el programa de prueba dos veces en estados distintos.

## Pruébalo

Ejecuta los comandos del laboratorio desde `modules/16-undocumented-interfaces/examples`.

1. Compila con `make`. Lista las exportaciones con `nm -D build/libblackbox.so`. Antes de leer código, escribe qué esperas de cada nombre.
2. Desensambla `bb_clamp` y `bb_lerp`. Escribe cada firma con la tabla de esta lección y compárala después con la lección.
3. Ejecuta la biblioteca bajo GDB y observa una llamada a `bb_split` mientras ocurre (el comando y la salida están debajo de la lista).
4. Cambia el programa de prueba para llamar a `bb_minmax` con `n` igual a 0. Predice qué ocurre y ejecútalo.
5. Escribe la cabecera de una biblioteca propia: compila un archivo C pequeño con tres funciones a una biblioteca compartida, olvida el código fuente y recupera los prototipos solo a partir del binario.

Para el paso 3:

```bash
gdb -nx -q -batch -ex "break bb_split" -ex "run" -ex "info registers rdi rsi rdx rcx" \
    -ex "x/s \$rdi" -ex "print/c \$rsi" -ex "finish" -ex "print \$rax" -ex "x/s \$rdx" build/16_probe
```

```text
Breakpoint 1, 0x00007ed57bbc4145 in bb_split () from /lab/modules/16-undocumented-interfaces/examples/build/libblackbox.so
rdi            0x598f749bb0a8      98472671555752
rsi            0x3d                61
rdx            0x7ffe9bb3f5a0      140731510683040
rcx            0x8                 8
0x598f749bb0a8:    "key=value"
$1 = 61 '='
0x0000598f749ba3cd in main ()
$2 = 3
0x7ffe9bb3f5a0:    "key"
```

Los cuatro registros contienen la cadena de origen, el separador `=` (61), el búfer de destino y la capacidad 8. Tras `finish`, `rax` es 3 y el destino contiene `key`. Las direcciones varían de una ejecución a otra.

## Errores frecuentes

- **Fiarse de los nombres.** Una función llamada `bb_next` podría hacer cualquier cosa. Lee el código y pruébalo.
- **Leer el tipo de retorno solo de `eax`.** Si `rax` se fija con una constante de 64 bits como -1, el tipo de retorno tiene 64 bits. Una escritura de 32 bits es un resultado de 32 bits.
- **Pasar por alto un parámetro que solo se reenvía.** Un argumento que se pasa a otra función se lee una vez y parece sin uso. Sigue cada registro de argumento hasta su último uso.
- **Llamar con la anchura equivocada.** Un prototipo con `int` donde la biblioteca lee un registro de 64 bits deja indefinida la mitad alta del registro. La llamada puede funcionar por casualidad.
- **Olvidar las reglas de uso.** Una firma correcta no basta: una cuenta que no puede ser cero, un tamaño de búfer que incluye el terminador y un manejador que debe cerrarse una vez forman parte de la interfaz.
- **Concluir a partir de una prueba.** Una entrada que coincide no confirma nada. Prueba los límites y los valores inusuales.

## Resumen

- La lista de exportaciones es la primera vista de una biblioteca sin documentar, y los nombres son hipótesis.
- Una firma se reconstruye a partir de qué registros de argumento se leen, la anchura de cada uso, cómo se desreferencian los punteros y qué se escribe por última vez en el registro de resultado.
- Un puntero a través del cual solo se escribe es un parámetro de salida; un puntero que devuelve una función y se pasa a otras es un manejador con ciclo de vida.
- Los puntos de llamada, las constantes y las pruebas tras una llamada muestran cómo debe usarse la interfaz.
- Un programa de prueba confirma cada hipótesis, incluidos los límites, y el resultado es una cabecera con los prototipos y las reglas de uso.

## Próximos pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 17, descifrar formatos de archivo.
- Ejercicios que usan este módulo: [Ejercicio 16](../../exercises/16-silent-library/README.es.md).
