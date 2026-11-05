<!-- source: README.md @ sha256:2c38d8bd75e3 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 21. Máquinas virtuales y bytecode

Tiempo: unas 2,5 horas. Requisitos previos: [20. Leer código ofuscado](../20-reading-obfuscated-code/README.es.md)

## Por qué importa

Algunos programas no contienen su lógica como código máquina. Contienen un pequeño procesador escrito en software, un intérprete, y un bloque de datos que este ejecuta, el bytecode. Los lenguajes usan este diseño para funcionar en cualquier hardware, las aplicaciones lo usan para scripts y macros, y los programas protegidos u ofuscados lo usan para ocultar lo que hacen: la lógica real es un flujo de bytes para un conjunto de instrucciones que no existe en ningún otro sitio, y un desensamblador solo muestra el intérprete. El método para leer un programa así es siempre el mismo. Tratas el intérprete como la documentación de un procesador desconocido, recuperas su conjunto de instrucciones a partir del código de sus manejadores y escribes un desensamblador para él. Este módulo enseña ese método con dos máquinas pequeñas e inofensivas escritas para la lección. El módulo trata de entender programas; las máquinas que contiene no hacen nada salvo calcular e imprimir números.

## Conceptos

### Qué es una máquina virtual

Una máquina virtual es aquí un programa que se comporta como un procesador: tiene un estado, lee instrucciones de la memoria y las ejecuta una por una. El estado es pequeño:

| Parte | Significado |
|---|---|
| Contador de programa | El desplazamiento de la siguiente instrucción de bytecode. Es una variable ordinaria, no el registro `rip`. |
| Almacenamiento de datos | Una pila, o un conjunto de registros virtuales, o ambos. |
| Memoria | Variables, o un arreglo de bytes que el bytecode puede leer y escribir. |
| Indicadores y estado | Un indicador de detenido, un indicador de error, a veces resultados de comparaciones. |
| El programa | El bytecode: un arreglo de bytes que lee el intérprete. Son datos, no código. |

El intérprete es un bucle que hace las mismas tres cosas con cada instrucción: obtiene el código de operación (el número que selecciona la operación) en el contador de programa, lo decodifica junto con sus operandos y lo ejecuta corriendo el código de un manejador. El bucle es el mismo para todos los programas. Solo cambia el bytecode.

La consecuencia para la ingeniería inversa es que la información útil se desplaza. En un programa ordinario el desensamblado es la lógica. En un programa con una máquina virtual el desensamblado es el mismo para todos los programas que usan la máquina, y la lógica está en el bytecode. Leerás el intérprete una vez, y después leerás el bytecode con una herramienta que construyes tú.

### Dos diseños

Las dos máquinas de ejemplo representan los dos diseños habituales.

| | Máquina de pila | Máquina de registros |
|---|---|---|
| Operandos | Implícitos: las operaciones toman sus entradas de la cima de una pila y apilan el resultado | Explícitos: las instrucciones nombran los registros |
| Tamaño de las instrucciones | Corto; algunas instrucciones llevan un byte de operando | Normalmente un tamaño fijo con varios campos |
| Código típico | `push 3`, `push 4`, `add` | `add r1, r2, r3` |
| En el intérprete | Un arreglo de pila y un índice de pila | Un arreglo de registros; el campo de la instrucción es un índice en él |
| Fácil de reconocer por | Un índice que sube y baja en casi todos los manejadores | Manejadores que leen dos o tres campos pequeños e indexan un arreglo |

La primera máquina, `examples/21_stackvm.c`, es una máquina de pila con 11 instrucciones, de un byte de código de operación cada una y algunas con un byte de operando. Lee su programa de un archivo. La segunda, `examples/21_regvm.c`, es una máquina de registros con cuatro registros e instrucciones fijas de 4 bytes. Ambas se describen en los comentarios al principio de sus fuentes; en la lección recuperarás los conjuntos de instrucciones a partir de los programas compilados, como harías con una muestra desconocida.

### Formas de despachar

El despacho (dispatch) es el paso del código de operación a su manejador. Es la parte del intérprete más fácil de reconocer.

| Estilo | En el código fuente | En el desensamblado |
|---|---|---|
| Un `switch` | `switch (op) { case 0: ... }` | Una comprobación de límite (`cmp`, luego `ja` al caso por defecto) y después un `jmp` indirecto a través de una tabla de direcciones de código |
| Una tabla de manejadores | `table[op](state, instruction)` | La carga de un puntero de un arreglo indexado por el código de operación, una prueba de puntero nulo y después un `call` indirecto |
| Despacho enhebrado | Cada manejador termina saltando al siguiente | Muchos saltos indirectos, uno al final de cada manejador, y ningún bucle central |
| Una cadena de comparaciones | `if (op == 1) ... else if (op == 2) ...` | Una serie de `cmp` y saltos condicionales; lenta, propia de intérpretes simples o deliberadamente austeros |

Los dos primeros son el tema de los ejemplos. En ambos casos el objeto clave es la tabla: un arreglo de punteros en la sección de datos de solo lectura. Leer la tabla te da, para cada código de operación, la dirección de su manejador. Un código de operación sin manejador tiene una entrada cero (la tabla de manejadores) o va al caso de error (el `switch`). Varios códigos de operación pueden compartir un manejador.

### Reconocer una máquina virtual

Busca estas señales juntas; ninguna basta por sí sola.

- Una función grande con un nudo central: un bucle que contiene un salto o una llamada indirectos, y muchos bloques pequeños que vuelven al principio del bucle.
- La carga de un byte (o una palabra) de un búfer en una posición variable, seguida de una comprobación de límite contra un número pequeño.
- Una tabla de direcciones de código en la sección de datos de solo lectura (una lista de valores que apuntan todos a la sección de texto).
- Un búfer de bytes en la sección de datos, o un archivo leído al arrancar, sin cadenas legibles y con una estructura que se repite.
- Manejadores cortos que cambian todos las mismas pocas variables: el índice de pila, el arreglo de registros, el contador de programa.
- Un contador de programa que solo se modifica en pocos manejadores (los saltos) y que en los demás se incrementa en una cantidad fija.

### Recuperar el conjunto de instrucciones

El método es una secuencia que puedes aplicar a cualquier intérprete.

1. **Encontrar el estado.** Identifica las variables que todos los manejadores leen o escriben. Una avanza con cada instrucción (el contador de programa). Una es un índice que sube y baja (un puntero de pila) o un arreglo indexado por un campo pequeño (los registros). Las demás son la memoria y el estado.
2. **Encontrar el despacho.** Encuentra la carga del código de operación y la transferencia indirecta que le sigue. Anota el registro que contiene el código de operación y la comprobación de límite, que te dice cuántos códigos de operación hay.
3. **Leer la tabla.** Vuelca la tabla de la sección de datos y enumera cada código de operación con la dirección de su manejador. Anota los códigos que comparten manejador o que faltan.
4. **Leer cada manejador.** Para cada manejador, anota qué operandos consume (cuánto avanza el contador de programa), qué lee, qué escribe y cuál es la operación. Los manejadores que comparten código pueden diferir en una prueba sobre el código de operación.
5. **Escribir la especificación.** Una fila por código de operación: número, nombre que elijas, operandos, efecto. Los nombres son tuyos; el comportamiento es el del intérprete.
6. **Escribir un desensamblador** a partir de la tabla (más abajo), comprobarlo con bytecode cuyo comportamiento conozcas y usarlo después con el bytecode que te interesa.

#### La máquina de pila en el desensamblado

La compilación sin símbolos no tiene nombres, y el intérprete está integrado en `main`. Este es el despacho:

```bash
make
objdump -d -M intel --no-show-raw-insn build/21_stackvm_stripped | sed -n '/401286:/,/4012aa:/p'
```

```text
  401286:  lea    r12,[r8+0x1]
  40128a:  movzx  ecx,BYTE PTR [rsp+r8*1+0xa0]
  401293:  add    rbp,0x1
  401297:  cmp    cl,0xa
  40129a:  ja     4013e9 <__fprintf_chk@plt+0x369>
  4012a0:  movzx  eax,cl
  4012a3:  jmp    QWORD PTR [rax*8+0x402078]
```

`r8` es el contador de programa: el código de operación se carga del búfer en `[rsp+r8+0xa0]`, y el contador de programa más uno se guarda en `r12`. `rbp` se incrementa con cada instrucción, así que es un contador de pasos. El código de operación se compara con 0xa, y todo lo que lo supera va al caso de error, de modo que hay 11 códigos de operación, del 0 al 10. La tabla está en 0x402078. Al volcarla se obtiene el manejador de cada código de operación:

```bash
objdump -s -j .rodata build/21_stackvm_stripped
```

```text
 402070 00000000 00000000 37124000 00000000  ........7.@.....
 402080 55124000 00000000 aa124000 00000000  U.@.......@.....
 402090 aa124000 00000000 aa124000 00000000  ..@.......@.....
 4020a0 e8124000 00000000 09134000 00000000  ..@.......@.....
 4020b0 44134000 00000000 7e134000 00000000  D.@.....~.@.....
 4020c0 95134000 00000000 c0134000 00000000  ..@.......@.....
```

Lee los valores de ocho bytes a partir de 0x402078, en little-endian. El código 0 va a 0x401237, el 1 a 0x401255, los códigos 2, 3 y 4 van todos a 0x4012aa, el 5 a 0x4012e8, y así sucesivamente. Tres códigos comparten un manejador: ese manejador debe probar el código de operación para elegir entre tres operaciones, y el código en 0x4012aa empieza con `cmp rbx,0x1` y `jbe`, una comprobación de que la pila tiene dos valores, seguida de dos extracciones. Esa es la forma de las operaciones binarias. Al leer los demás manejadores del mismo modo se obtiene la especificación de los comentarios del código fuente; el manejador del código 1 lee un byte más e incrementa el contador de programa en 2, los manejadores de 8 y 9 asignan el contador de programa desde el operando, y el manejador de 10 llama a `printf` con la cadena de formato `%d`.

Seguir el contador de programa es la manera de decidir el tamaño de una instrucción. Un manejador que lee el byte posterior al código de operación consume un operando. Si te equivocas en el tamaño de una instrucción, todas las posteriores del desensamblado son erróneas, que es la señal que debes buscar: un desensamblado que empieza a producir disparates en mitad de un programa tiene un tamaño equivocado antes de ese punto.

#### La máquina de registros en el desensamblado

La máquina de registros usa una tabla de manejadores. La llamada tiene este aspecto:

```text
  4012b5:  lea    rsi,[rsp+r8*4+0x20]
  4012ba:  movzx  ecx,BYTE PTR [rsi]
  4012bd:  movsxd rax,ecx
  4012c0:  mov    rax,QWORD PTR [rax*8+0x4020a0]
  4012c8:  test   rax,rax
  4012cb:  je     401359 <__fprintf_chk@plt+0x2d9>
  4012d1:  mov    rdi,rsp
  4012d4:  call   rax
```

La dirección de la instrucción se calcula como la base más cuatro veces el contador de programa, lo que muestra que las instrucciones tienen 4 bytes de ancho. El código de operación es el primer byte de la instrucción, la tabla está en 0x4020a0, una entrada nula va al caso de error, y el manejador se llama con `rdi` apuntando al estado de la máquina y `rsi` apuntando a la instrucción. La tabla tiene 256 entradas, porque el código de operación es un byte entero, y solo siete de ellas no son cero:

```text
0x10 -> 0x401176
0x11 -> 0x40118e
0x12 -> 0x4011ad
0x13 -> 0x4011cc
0x14 -> 0x4011ec
0x15 -> 0x40120a
0xff -> 0x401202
```

Los manejadores son cortos y regulares. Este es el manejador del código de operación 0x11:

```text
  40118e:  movzx  eax,BYTE PTR [rsi+0x1]
  401192:  and    eax,0x3
  401195:  movzx  ecx,BYTE PTR [rsi+0x2]
  401199:  and    ecx,0x3
  40119c:  movzx  edx,BYTE PTR [rsi+0x3]
  4011a0:  and    edx,0x3
  4011a3:  mov    edx,DWORD PTR [rdi+rdx*4]
  4011a6:  add    edx,DWORD PTR [rdi+rcx*4]
  4011a9:  mov    DWORD PTR [rdi+rax*4],edx
  4011ac:  ret
```

Los tres bytes posteriores al código de operación se enmascaran con 3 y se usan como índices en un arreglo de valores de 32 bits al principio del estado: un banco de registros con cuatro registros. El manejador suma los registros que nombran el segundo y el tercer campo y guarda el resultado en el registro que nombra el primero. Eso es `add a, b, c`. La máscara significa que un número de registro mayor que 3 da la vuelta en lugar de fallar, un detalle que pertenece a la especificación. El manejador de 0x40118e se diferencia de los de 0x4011ad y 0x4011cc solo en la operación (`sub`, `imul`). El manejador de 0x4011ec compara un registro con cero y, si no es cero, escribe un byte de la instrucción en un campo del estado en el desplazamiento 0x10: ese campo es el contador de programa, así que es un salto condicional. El manejador de 0x401202 fija un campo en el desplazamiento 0x18, el indicador de detenido.

### Leer el bytecode

Con la especificación puedes leer un programa. El ensamblador `examples/21_asm.py` construye los programas de ejemplo a partir de texto, y el desensamblador `examples/21_disasm.py` los vuelve a imprimir. El programa de la máquina de pila de `programs/fact.asm` calcula un producto en un bucle y lo imprime:

```bash
python3 21_asm.py stack programs/fact.asm build/fact.bin
xxd build/fact.bin
build/21_stackvm build/fact.bin
```

```text
build/fact.bin: 32 bytes, 18 instructions
00000000: 0105 0700 0101 0701 0600 091c 0601 0600  ................
00000010: 0407 0106 0001 0103 0700 0808 0601 0a00  ................
120
halt after 64 steps
```

Los bytes no muestran nada por sí solos. El desensamblador los convierte en un listado:

```bash
python3 21_disasm.py stack build/fact.bin
```

```text
0000  01 05           push 5
0002  07 00           store 0
0004  01 01           push 1
0006  07 01           store 1
0008  06 00           load 0
000a  09 1c           jz @28
000c  06 01           load 1
000e  06 00           load 0
0010  04              mul
0011  07 01           store 1
0013  06 00           load 0
0015  01 01           push 1
0017  03              sub
0018  07 00           store 0
001a  08 08           jmp @8
001c  06 01           load 1
001e  0a              print
001f  00              halt
```

El listado tiene la estructura de un bucle: el salto de 0x1a vuelve al desplazamiento 8, el salto condicional de 0x0a sale del bucle hacia el desplazamiento 0x1c cuando la variable 0 es cero, y el cuerpo multiplica la variable 1 por la variable 0 y decrementa la variable 0. El programa imprime el factorial de 5. Observa que los operandos de salto son desplazamientos en bytes, no números de instrucción, y que un desensamblador que los imprime como `@28` es más útil si además marca la línea de destino con una etiqueta.

### Escribir un desensamblador

Un desensamblador para un conjunto de instrucciones propio tiene tres partes: una tabla que describe el conjunto de instrucciones, un bucle que recorre los bytes y un formateador. El código de `21_disasm.py` tiene esta forma:

```c-fragment
for each position in the bytecode:
    look up the opcode in the table
    if it is not in the table:
        print the byte as data and move on by one
    read as many operand bytes as the table says
    print the address, the bytes, the name and the formatted operands
    move on by 1 + the number of operand bytes
```

Mantén el conjunto de instrucciones como datos, una tabla por máquina, y mantén el bucle genérico. Cuando recuperes otra máquina, rellenas una tabla nueva y no tocas el bucle. Cuatro puntos marcan la diferencia entre un desensamblador que funciona y uno que solo lo parece:

- **Bytes desconocidos.** No te detengas nunca en un byte que no es un código de operación. Imprímelo como dato y continúa, para ver todo el programa y notar dónde está incompleta tu tabla.
- **Tamaño de la instrucción.** El tamaño forma parte de la tabla. Un tamaño erróneo desplaza todo lo que viene después, así que comprueba la salida con un programa cuyo comportamiento conozcas, por ejemplo uno que imprima un resultado que puedas calcular a mano.
- **Significado de los operandos.** Un número de registro, un valor inmediato, un desplazamiento de destino y un número de variable son clases distintas de operando. Imprime cada clase a su manera (`r2`, `5`, `@28`).
- **Destinos.** En un salto, la salida útil es la línea a la que lleva. Reúne los destinos en una primera pasada e imprime una etiqueta en cada uno en una segunda.

El mismo desensamblador maneja la máquina de registros con una segunda tabla. Su salida para `programs/sum.asm`:

```bash
python3 21_asm.py reg programs/sum.asm build/sum.bin
python3 21_disasm.py reg build/sum.bin
build/21_regvm build/sum.bin
```

```text
build/sum.bin: 32 bytes, 8 instructions
0000  10 00 0a 00     movi r0, 10
0004  10 01 00 00     movi r1, 0
0008  10 02 01 00     movi r2, 1
000c  11 01 01 00     add r1, r1, r0
0010  12 00 00 02     sub r0, r0, r2
0014  14 00 03 00     jnz r0, @3
0018  15 01 00 00     out r1
001c  ff 00 00 00     halt
55
halt after 35 steps
```

Aquí el destino del salto es un número de instrucción, `@3`, y no un desplazamiento en bytes: el desensamblador de una máquina con instrucciones de tamaño fijo puede usar cualquiera de los dos, y la especificación debe decir cuál usa la máquina.

### Trazar la máquina

La lectura estática te da el conjunto de instrucciones. Para comprobarlo, y para seguir un programa que todavía no puedes desensamblar, traza la máquina mientras se ejecuta: detente en el despacho e imprime el código de operación cada vez. El script `examples/21_trace.gdb` lo hace para la máquina de pila. La dirección del salto de despacho sale del desensamblado de esta compilación exacta, así que es el único lugar donde adaptas el script a otro binario:

```bash
gdb -nx -q -batch -x 21_trace.gdb build/21_stackvm_stripped 2>&1 | grep "^offset" | head -9
```

```text
offset   0  opcode 01  stack depth 0
offset   2  opcode 07  stack depth 1
offset   4  opcode 01  stack depth 0
offset   6  opcode 07  stack depth 1
offset   8  opcode 06  stack depth 0
offset  10  opcode 09  stack depth 1
offset  12  opcode 06  stack depth 0
offset  14  opcode 06  stack depth 1
offset  16  opcode 04  stack depth 2
```

La traza es una lista de las instrucciones ejecutadas, en orden, y coincide con el listado: push 5, store 0, push 1, store 1, load 0, un salto condicional, y así sucesivamente. La ejecución completa son 64 líneas, el mismo número que los pasos que informa el programa. Una traza es también el modo de sortear el problema de un bytecode que todavía no sabes analizar: muestra solo las instrucciones que se ejecutan de verdad, con los tamaños correctos, aunque no conozcas del todo el conjunto de instrucciones.

### Un orden de trabajo

1. **Triaje.** Busca las señales de una máquina virtual: la función grande con un nudo central, la tabla de direcciones de código, el búfer sin cadenas.
2. **Encontrar el estado y el despacho.** Nombra el contador de programa, el índice de pila o el arreglo de registros, y la carga del código de operación.
3. **Volcar la tabla.** Enumera cada código de operación y su manejador. Anota las entradas compartidas y las que faltan.
4. **Leer los manejadores.** Escribe una fila de la especificación para cada uno, con los operandos que consume.
5. **Desensamblar.** Rellena la tabla de tu desensamblador y compárala con una traza.
6. **Leer el bytecode.** Encuentra bucles, llamadas y las instrucciones que producen salida, y describe el comportamiento en términos de lo que hace el programa.

## Pruébalo

Ejecuta los comandos del laboratorio desde `modules/21-virtual-machines-bytecode/examples`.

1. Compila con `make` y ejecuta ambos programas. Desensambla `build/fact.bin` y `build/sum.bin` y describe en una frase qué calcula cada programa.
2. Desensambla `build/21_stackvm_stripped`. Encuentra la carga del código de operación, la comprobación de límite y el salto indirecto. ¿Cuántos códigos de operación tiene la máquina? ¿Cuáles comparten un manejador?
3. Vuelca la tabla de la máquina de registros de `build/21_regvm_stripped` y anota la dirección del manejador de cada código de operación. Lee el manejador del código 0x14 y describe su efecto en una línea.
4. Ejecuta `21_trace.gdb` y compara las primeras líneas con el desensamblado. Cambia el programa de `programs/fact.asm` (por ejemplo, empieza en 6), vuelve a construirlo y compara la longitud de la traza con el número de pasos que informa el programa.
5. Escribe un programa propio para la máquina de registros que imprima 2 elevado a 10 mediante multiplicación repetida en un bucle, usando solo las instrucciones de la tabla. Ensámblalo con `21_asm.py` y ejecútalo.
6. Amplía la máquina de pila en tu propia copia de `21_stackvm.c` con un código de operación nuevo, por ejemplo `swap`. Añádelo a la tabla de `21_disasm.py`. ¿Qué partes del desensamblador has cambiado?
7. Marca los destinos de los saltos: cambia `21_disasm.py` para que imprima una etiqueta en cada línea a la que lleve un salto, y que en el salto imprima la etiqueta en lugar del desplazamiento.

## Errores frecuentes

- **Leer el intérprete como si fuera el programa.** El intérprete es el mismo para todos los programas; la lógica está en el bytecode. Si dedicas tu tiempo a los manejadores después de tener la especificación, no estás leyendo el programa.
- **Un tamaño de instrucción equivocado.** Un solo tamaño erróneo convierte en disparate todo lo que sigue. Compara con una traza y busca el punto donde la salida se vuelve disparate.
- **Pasar por alto los manejadores compartidos.** Varios códigos de operación pueden compartir un manejador y elegir la operación con una prueba en su interior. Lee la tabla antes que los manejadores y busca direcciones repetidas.
- **Ignorar máscaras y límites.** El `and 3` sobre un número de registro, la comprobación de límite sobre el código de operación y la comprobación de la profundidad de la pila forman parte del comportamiento. Ponlos en la especificación y prueba qué ocurre en los extremos.
- **Confundir los dos contadores de programa.** El registro `rip` ejecuta el intérprete; el contador de programa virtual indexa el bytecode. Los destinos de los saltos en el bytecode son valores del segundo.
- **Fiarse de la dirección de una tabla de otra compilación.** Las direcciones de un script de traza o de una nota pertenecen a un binario. Búscalas de nuevo en cada compilación.

## Resumen

- Una máquina virtual es un bucle intérprete con un estado y un bloque de bytecode; el desensamblado muestra el intérprete, y la lógica está en el bytecode.
- Las máquinas son de pila (operandos implícitos) o de registros (operandos nombrados); el despacho es un `switch`, una tabla de manejadores o saltos enhebrados.
- Se reconoce una por un bucle con nudo central, una tabla de direcciones de código, una carga de byte con comprobación de límite y manejadores cortos que cambian las mismas pocas variables.
- Recuperar el conjunto de instrucciones significa encontrar el estado y el despacho, volcar la tabla y leer cada manejador hasta obtener una fila de una especificación.
- Un desensamblador es una tabla más un bucle genérico; imprime los bytes desconocidos como datos y se comprueba con una traza de la máquina en ejecución.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 22, código administrado.
- Ejercicios que usan este módulo: [Ejercicio 20](../../exercises/20-capstone-tiny-vm/README.es.md).
