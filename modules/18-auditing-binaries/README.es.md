<!-- source: README.md @ sha256:2cc754f9080c -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 18. Auditar binarios en busca de errores

Tiempo: unas 2,5 horas. Requisitos previos: [17. Descifrar formatos de archivo](../17-deciphering-file-formats/README.es.md)

## Por qué importa

La mayoría de los fallos graves de seguridad en código nativo son errores de seguridad de memoria: un programa copia, indexa o libera memoria de una forma que su autor no pretendía. Quien sabe leer código compilado puede encontrar uno de esos errores en un componente sin código fuente, redactar un informe con el que el responsable pueda actuar y proponer una corrección que se pueda probar. Este módulo enseña ese trabajo desde el lado del defensor. Termina en el diagnóstico y la reparación. No enseña a convertir un error en un ataque, y el curso nunca te lo pide. Audita solo código que sea tuyo o que estés autorizado a examinar, y lee [la declaración de ética](../../ETHICS.md) antes de empezar.

## Conceptos

### El método: de las fuentes a los sumideros

Un error de seguridad de memoria necesita dos cosas: un valor que el programa no controla (leído de un archivo, un paquete de red, un argumento o un campo de un registro) y un lugar donde ese valor decide cuánta memoria se toca. Lo primero es una **fuente**, lo segundo un **sumidero**. La auditoría sigue cada fuente hasta cada sumidero y examina las comprobaciones intermedias.

| Sumidero | Pregunta |
|---|---|
| Una copia (`memcpy`, `strcpy`, un bucle que guarda bytes) | ¿Qué limita el número de bytes? ¿Se compara el límite con el tamaño del destino, y con el tamaño correcto? |
| Un índice en un array | ¿Se compara antes con la longitud, y la comparación es con signo o sin signo? |
| Una reserva de memoria (`malloc`, `calloc`, `realloc`) | ¿Cómo se calcula el tamaño? ¿Puede desbordarse el cálculo? ¿Usa el código posterior los mismos números? |
| Una llamada a `free` | ¿Se usa el puntero después por algún camino? ¿Se libera dos veces? |
| Una llamada a una función de biblioteca con un formato o una cadena | ¿Recibe un tamaño? ¿Es correcto el tamaño? |

En un programa compilado, busca primero los sumideros: rastrea en el desensamblado las llamadas a las funciones de copia, reserva y liberación, y los almacenamientos con índice escalado. Después retrocede desde el operando de tamaño (`rdx` en una llamada a `memcpy`, `rdi` en una llamada a `malloc`) hasta el lugar de donde vino el valor y lee todas las comparaciones del camino.

### El código de práctica

El archivo `examples/18_bugs.c` tiene seis funciones pequeñas, cada una con un error de una clase distinta. `examples/18_fixed.c` tiene las mismas funciones reparadas. Los listados de esta lección salen de una compilación de `18_bugs.c` con `-O1`, `-fno-builtin`, `-mstringop-strategy=libcall` y `-D_FORTIFY_SOURCE=0`, que mantiene visibles las llamadas a la biblioteca como en muchos binarios reales. Para confirmar, `make` también compila los programas de demostración con AddressSanitizer, una función del compilador que detiene un programa en el primer acceso incorrecto a memoria e imprime un informe. Puedes usarlo cuando tienes el código fuente; con un binario que no puedes recompilar, usa el depurador.

```bash
make
make run
```

### Error 1: una longitud que nadie comprueba

```text
0000000000000000 <set_name>:
   0:  push   rbp
   1:  push   rbx
   2:  sub    rsp,0x8
   6:  mov    rbx,rdi                      ; rbx = dst
   9:  movzx  ebp,BYTE PTR [rsi]           ; length = first byte of the packet, 0 to 255
   c:  add    rsi,0x1                      ; source = packet + 1
  10:  mov    rdx,rbp                      ; size for memcpy
  13:  call   memcpy
  18:  mov    BYTE PTR [rbx+rbp*1],0x0    ; terminator written at dst[length]
  1c:  add    rsp,0x8
  20:  pop    rbx
  21:  pop    rbp
  22:  ret
```

El tamaño que se pasa a `memcpy` es un byte leído de la entrada y extendido con ceros, y no hay ninguna comparación entre la carga y la llamada. La función no recibe el tamaño del destino, así que dentro de ella no es posible una comprobación correcta: la función misma es el defecto, porque su interfaz carece de la información que necesita. El almacenamiento del terminador en `dst[length]` es una segunda escritura con el mismo problema.

El llamador de la demostración tiene un búfer de 16 bytes y una longitud de 40. En el depurador puedes ver la situación antes de que ocurra la copia:

```text
Breakpoint 1, set_name (dst=0x7ffdef7e5010 "", pkt=0x14a982a0 "(", 'A' <repeats 40 times>) at 18_bugs.c:13
13        unsigned len = pkt[0];
0x14a982a0:    40
#1  0x0000000000401244 in main (argc=2, argv=0x7ffdef7e5168) at 18_demo_bug.c:20
20        set_name(dst, pkt);
$1 = 16
$2 = 40 '('
```

Los comandos fueron `break set_name`, `run name`, `x/ub $rsi`, `up`, `print sizeof dst` y `print pkt[0]`, y después `kill`. El programa se detuvo antes de la copia defectuosa, y los números bastan para el informe: el destino tiene 16 bytes y la longitud es 40. Con la compilación con sanitizer, el informe señala la misma línea:

```text
ERROR: AddressSanitizer: stack-buffer-overflow on address ADDR at pc ADDR bp ADDR sp ADDR
WRITE of size 40 at ADDR thread T0
    #1 ADDR in set_name 18_bugs.c:14
    #2 ADDR in main 18_demo_bug.c:20
```

Corrección: la función debe recibir la capacidad del destino y la longitud del paquete, y rechazar la entrada que no quepa. `18_fixed.c` tiene `set_name(dst, cap, pkt, pkt_len)`, que rechaza una longitud mayor que el resto del paquete o no menor que la capacidad (el último byte es para el terminador).

### Error 2: error de uno

```text
0000000000000023 <copy_label>:
  ...
  30:  mov    rdi,rdx                      ; rdi = src
  33:  call   strlen                       ; rax = n
  38:  cmp    r12,rax                      ; cap against n
  3b:  jb     56 <copy_label+0x33>         ; cap < n: return -1
  3d:  lea    rdx,[rax+0x1]                ; size = n + 1
  41:  mov    rsi,rbx
  44:  mov    rdi,rbp
  47:  call   memcpy                       ; copies n + 1 bytes
```

El código rechaza la cadena solo cuando es más larga que la capacidad. Una cadena de exactamente `cap` caracteres pasa, y la copia escribe entonces `cap + 1` bytes: los caracteres y el terminador. El indicio es la diferencia entre la cantidad comparada (`n`) y la copiada (`n + 1`, el `lea rdx,[rax+0x1]`). Siempre que un tamaño de copia lleva una constante sumada, la comprobación previa debe incluir la misma constante. El sanitizer informa de una escritura de tamaño 9 en un búfer de 8 bytes. En la versión reparada el salto tras `cmp rax,r12` es `jae`: rechazar cuando `n >= cap`.

### Error 3: desbordamiento de enteros antes de una reserva

```text
000000000000005d <make_table>:
  65:  mov    r13d,edi                     ; count
  68:  mov    r12d,esi                     ; elem
  6b:  imul   edi,esi                      ; total = count * elem, in 32 bits
  6e:  call   malloc                       ; malloc(total), rdi zero-extended
  ...
  8b:  mov    rdx,r12                      ; the loop uses elem ...
  93:  call   memset
  98:  add    ebx,0x1
  9e:  cmp    r13d,ebx                     ; ... for count iterations
  a1:  jne    8b
```

La multiplicación es un `imul` de 32 bits de dos valores que vienen de la entrada, y el resultado va directo a `malloc`. El bucle que rellena la tabla usa `count` y `elem` por separado y nunca mira el tamaño reservado. Si el producto da la vuelta (una cuenta de 0x10000001 con un tamaño de elemento de 16 da 16), la reserva es minúscula y el bucle escribe mucho más allá. La firma que hay que buscar es una instrucción aritmética sobre valores derivados de la entrada cuyo resultado es un tamaño de reserva, con las mismas entradas usadas de nuevo más tarde para un bucle o un índice. El informe del sanitizer es una escritura de tamaño 16 pasado el final de una región de montón de 16 bytes. La reparación calcula el tamaño en 64 bits, que no puede desbordarse con entradas de 32 bits, y aplica un límite de política antes de reservar. `__builtin_mul_overflow` hace lo mismo cuando los operandos ya son de 64 bits.

### Error 4: con signo y sin signo

```text
00000000000000af <copy_block>:
  af:  cmp    edx,0x40                     ; len against 64
  b2:  jg     c3 <copy_block+0x14>         ; signed: greater than 64 is refused
  b4:  push   rbx
  b5:  mov    ebx,edx
  b7:  movsxd rdx,edx                      ; sign-extend len to 64 bits
  ba:  call   memcpy                       ; size = rdx
```

Dos instrucciones juntas forman la firma: una comparación con signo (`jg`) que protege solo el extremo superior, y una extensión de signo (`movsxd`) cuando el valor se ensancha al tamaño de 64 bits. Un `len` negativo pasa la comparación, se extiende con signo a un número enorme sin signo y llega a `memcpy` como tamaño. El sanitizer lo llama `negative-size-param`. En la versión reparada el compilador emite una sola comparación sin signo (`cmp edx,0x40` y luego `ja`), porque el código fuente rechaza también `len < 0` y un valor negativo se lee como un valor grande sin signo. Recuerda la regla: un tamaño es sin signo, y una comprobación del límite superior de un valor con signo está incompleta sin una comprobación del inferior.

### Error 5: llamadas inseguras a la biblioteca

```text
00000000000000c9 <greet>:
  c9:  push   rbx
  ca:  sub    rsp,0x20                     ; a 32-byte frame; the buffer is 24 bytes of it
  ce:  mov    rdx,rdi                      ; the user string
  d1:  mov    rbx,rsi
  d4:  mov    esi,0x0                      ; the format "hello, %s" (a relocation)
  d9:  mov    rdi,rsp                      ; destination: a buffer on the stack
  dc:  mov    eax,0x0
  e1:  call   sprintf                      ; no size anywhere
  e6:  mov    rsi,rsp
  e9:  mov    rdi,rbx
  ec:  call   strcpy                       ; and the copy to the caller's buffer is unbounded too
```

Algunas funciones de biblioteca no pueden usarse con seguridad porque no reciben el tamaño del destino: `strcpy`, `strcat`, `sprintf`, `vsprintf`, `gets` y `scanf` con una conversión `%s` sin anchura. Otras reciben un tamaño y aun así se usan mal: `strncpy` no añade terminador cuando el origen es tan largo como el límite, y `memcpy` recibe un tamaño que debe comprobar quien llama. Una llamada es insegura cuando el destino es un búfer de tamaño fijo (una zona del marco de pila o una global, como `rdi = rsp` aquí), el origen depende de la entrada y no hay entre ambos ninguna comparación con el tamaño del destino. Muchas distribuciones compilan con fortificación (fortification), que sustituye esas llamadas por `__sprintf_chk` y funciones similares que reciben el tamaño del destino como argumento adicional (la compilación `-O1` con opciones por omisión muestra `__sprintf_chk` con el tamaño 0x18). Eso convierte la sobrescritura silenciosa en una interrupción, así que el efecto es más suave, pero el error sigue ahí y sigue mereciendo un informe. Ver una función `_chk` en un binario indica que el código se compiló con la protección; ver la función simple indica que no. La reparación usa `snprintf` con la capacidad y comprueba el resultado.

### Error 6: uso tras liberación

```text
00000000000000f7 <finish>:
  f7:  push   rbx
  f8:  mov    rbx,rdi                      ; rbx = the item
  fb:  test   esi,esi
  fd:  je     103 <finish+0xc>             ; ok == 0: go to the free
  ff:  mov    eax,DWORD PTR [rbx]          ; read it->id
 101:  pop    rbx
 102:  ret
 103:  call   free                          ; free(rbx)
 108:  jmp    ff <finish+0x8>              ; and then continue at the read through rbx
```

La firma de un uso tras liberación (use after free) en el desensamblado es un camino que pasa por un `call free` sobre un puntero y luego llega a una instrucción que desreferencia de nuevo el mismo puntero, aquí por el salto de vuelta a `ff`. Sigue el registro o la ranura de pila que contenía el argumento de `free`: si se lee después de la llamada, por cualquier camino, el código usa memoria liberada. Un segundo `call free` sobre el mismo valor es una doble liberación. El sanitizer informa de `heap-use-after-free` con una lectura de tamaño 4. En la versión reparada, `18_fixed.c` lee el campo antes de liberar. Otro buen hábito es guardar un puntero nulo tras liberar, de modo que un uso posterior falle de inmediato.

### Confirmar un hallazgo sin causar daño

Un hallazgo es más útil con pruebas. Las formas seguras de prueba son:

- Ejecuta tu propia compilación del código bajo un sanitizer, con una entrada que provoque el camino. Es la mejor prueba cuando tienes el código fuente.
- En un depurador, detente en el sumidero antes de que se ejecute y lee los números: el tamaño y la capacidad. No hace falta ningún fallo ni una escritura excesiva.
- Con el binario solo, muestra el camino en el desensamblado: la fuente, la comparación ausente o equivocada y el sumidero.

No fabriques entradas cuyo propósito sea tomar el control del programa. Para la auditoría no importa lo que un atacante podría hacer después; importa que el programa toca memoria que no debe y que la corrección elimina la causa.

### Redactar el informe

Un informe lo lee la persona que debe corregir el error. Tiene que ser exacto y breve. Usa los mismos campos cada vez.

```text
Title:      Unchecked packet length in set_name
Class:      CWE-787 out-of-bounds write (CWE-120, copy without checking the size)
Location:   set_name, call to memcpy at offset 0x13 (source line 14)
Severity:   High. The length and the bytes both come from the input, and the
            destination is a fixed buffer.
Cause:      set_name copies the number of bytes given by the first byte of the
            packet (0 to 255) into a destination whose size it does not know.
            No comparison separates the load of the length from the call.
Evidence:   Debugger, stopped before the copy: destination 16 bytes, length 40.
            Sanitizer build: stack-buffer-overflow, WRITE of size 40 at
            18_bugs.c:14.
Impact:     Memory outside the destination is overwritten with bytes from the
            packet. This corrupts adjacent data and can crash the program. What
            is next to the buffer depends on the caller; that was not assessed.
Fix:        Pass the capacity and the packet length. Reject a length larger than
            the rest of the packet or not smaller than the capacity. See
            set_name in 18_fixed.c.
Test:       Packets with lengths 0, 15, 16, 17 and 255 and a one-byte packet.
            The first two succeed, the rest are rejected without a write. Run
            the tests under the sanitizer.
```

Elige de la lista pública de debilidades (CWE) la clase que corresponde a la causa, no al síntoma: 787 para una escritura fuera de límites, 125 para una lectura, 193 para un error de uno, 190 para un desbordamiento o envoltura de enteros, 195 para una conversión de con signo a sin signo, 416 para el uso tras liberación, 415 para una doble liberación y 676 para el uso de una función peligrosa. Indica la gravedad razonándola: quién controla la entrada, qué se escribe y dónde. Di lo que no evaluaste. Si encontraste el error en software ajeno, infórmalo en privado a su responsable y dale tiempo para corregirlo antes de decir nada en público.

### Comprobar la corrección

Una corrección se verifica como el error. Ejecuta de nuevo la misma entrada y comprueba que el informe ha desaparecido. Ejecuta los valores límite: el mayor valor aceptado, el menor rechazado, cero, negativo y el valor que se desbordaría. El programa `demo_fix` lo hace para las seis funciones reparadas:

```text
name:   -1
name:   0 (alice)
label:  -1
label:  0
table:  refused
block:  -1
greet:  -1
greet:  0 (hello, bob)
finish: 7
```

Cada entrada peligrosa se rechaza sin ningún acceso a memoria, y la entrada normal sigue funcionando. Una corrección que solo traslada el problema (por ejemplo, un búfer más grande) no es una corrección.

## Pruébalo

Ejecuta los comandos del laboratorio desde `modules/18-auditing-binaries/examples`.

1. Compila con `make` y ejecuta `make run`. Para cada uno de los seis casos, anota la clase del error y la línea del informe antes de leer la explicación de la lección.
2. Desensambla `build/18_bugs.o` con `objdump -dr -M intel`. Encuentra la llamada a `memcpy` en `copy_block` y rastrea el origen de `rdx`. ¿Qué instrucción lo produjo a partir de `edx`?
3. Compara `build/18_bugs.o` y `build/18_fixed.o` con el mismo comando. Para `copy_label` y `copy_block`, enumera las instrucciones que cambiaron.
4. Ejecuta `gdb -nx -q build/demo_plain` y usa los comandos de la lección para detener `set_name` antes de su copia y leer los números. No continúes más allá del punto de interrupción con la entrada excesiva.
5. Redacta un informe en el formato anterior para el error de `finish`. Incluye la clase, la ubicación, la causa, la prueba y la corrección.
6. Busca en un programa tuyo (uno pequeño que hayas escrito) llamadas a `strcpy`, `sprintf` y `memcpy` y decide para cada una si se comprueba el tamaño del destino.

## Errores frecuentes

- **Auditar el síntoma.** Un fallo en una línea suele venir de un error anterior, como un tamaño de reserva mal calculado. Sigue el valor hasta su fuente.
- **Comparar la cantidad equivocada.** La comprobación debe usar la cantidad que se copia, incluidos terminadores y bytes de cabecera.
- **Fiarse de un solo tipo.** Un tamaño leído como `int` y pasado luego como `size_t` cambia de significado cuando es negativo. Encuentra dónde cambia el tipo.
- **Dar por segura la llamada fortificada.** Una función `_chk` convierte la corrupción en una interrupción, pero un programa que se interrumpe con una entrada de apariencia válida sigue teniendo un error.
- **Corregir un solo sitio.** El mismo patrón suele aparecer en otros lugares. Búscalo tras corregir el primer caso.
- **Demostrar de más.** El informe necesita la causa y la prueba, no una demostración de lo que podría hacer un atacante.

## Resumen

- Sigue los valores de entrada desde las fuentes hasta los sumideros y lee todas las comparaciones intermedias.
- Seis patrones cubren la mayoría de los hallazgos: una longitud sin comprobar, una comparación con error de uno, un desbordamiento aritmético antes de una reserva, un valor con signo usado como tamaño, una llamada insegura a la biblioteca y un uso tras liberación.
- Cada patrón tiene una firma en el desensamblado: un operando de tamaño sin comparación, una cantidad comparada distinta de la copiada, un `imul` sobre valores de entrada antes de `malloc`, `jg` con `movsxd`, una llamada sin tamaño y una desreferencia tras `free`.
- Confirma los hallazgos con una compilación con sanitizer o un depurador detenido antes del sumidero, nunca con una entrada dañina.
- Un informe indica la clase, la ubicación, la causa, la prueba, el impacto, la corrección y la prueba de verificación; una corrección se verifica con los valores límite.

## Próximos pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 19, analizar programas sospechosos.
- Ejercicios que usan este módulo: [Ejercicio 18](../../exercises/18-bug-hunt-report/README.es.md).
