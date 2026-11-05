<!-- source: README.md @ sha256:f31f4f86ea8b -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 15. Recuperar datos

Tiempo: aproximadamente 3 horas. Requisitos previos: [14. Aritmética compilada](../14-compiled-arithmetic/README.es.md)

## Por qué importa

El código máquina no tiene tipos. No hay matrices, ni estructuras, ni clases; solo direcciones, desplazamientos y anchos. Sin embargo, el resultado más útil de una sesión de ingeniería inversa suele ser una descripción de los datos: qué contiene un registro, cómo se enlaza una lista, qué tabla gobierna una decisión. Este módulo te enseña a reconstruir esos tipos a partir de cómo el código usa la memoria: la escala de un índice da el tamaño de un elemento, una constante sumada a un puntero da un campo, una carga desde el inicio de un objeto da una tabla de funciones. Termina con los objetos C++, que añaden un campo oculto a todo.

## Conceptos

### Cómo se hicieron los listados de este módulo

El código fuente C es `examples/15_data.c` y el código fuente C++ es `examples/15_cpp.cpp`. El `Makefile` los compila con GCC y G++ en `-O1` y escribe ensamblador en sintaxis Intel en `build/15_data.s` y `build/15_cpp.s`. Los listados son esos archivos sin las directivas del ensamblador. Todo el código es x86-64 System V; el primer argumento está en `rdi`, el segundo en `rsi` y el resultado en `eax` o `rax`.

```bash
make
```

### Globales, locales y el montón

Dónde vive una variable determina cómo llega a ella el código.

**Una global** tiene una dirección fija. En este ejemplo, construido sin código independiente de la posición, el compilador escribe la dirección en la instrucción. Con código independiente de la posición, el mismo acceso es relativo a `rip`:

```text
bump:
    mov    eax, DWORD PTR counter[rip]
    add    eax, 1
    mov    DWORD PTR counter[rip], eax
    ret
```

`counter[rip]` es una referencia relativa a `rip` a la variable `counter`, que vive en la sección `.data` porque tiene un valor inicial. Las globales sin inicializar viven en `.bss`, las constantes en `.rodata`. Una secuencia de lectura, modificación y escritura en la misma dirección (cargar, sumar, guardar) es `counter += 1`.

**Una local** vive en la pila o en un registro. Una matriz local siempre tiene dirección, así que vive en memoria:

```text
local_sum:
    mov    DWORD PTR [rsp-24], edi
    lea    edx, [rdi+1]
    mov    DWORD PTR [rsp-20], edx
    lea    esi, [rdi+2]
    mov    DWORD PTR [rsp-16], esi
    lea    ecx, [rdi+3]
    mov    DWORD PTR [rsp-12], ecx
    lea    eax, [rdi+rdx]
    add    eax, esi
    add    eax, ecx
    and    edi, 3
    add    eax, DWORD PTR [rsp-24+rdi*4]
    ret
```

La función no tiene puntero de marco ni `sub rsp`: guarda los cuatro elementos de `buf` en `[rsp-24]` hasta `[rsp-12]`, por debajo del puntero de pila, en la **zona roja** (red zone) del módulo 08. El bucle sobre `i` se desenrolló por completo, así que no existe ningún contador; los elementos se guardan con desplazamientos consecutivos separados 4 bytes. El acceso final `[rsp-24+rdi*4]` es la lectura indexada `buf[n & 3]`.

**El montón** es memoria del asignador. Una llamada a `malloc` con una constante te dice el tamaño del objeto; el código que sigue guarda en él a través del puntero devuelto:

```text
push:
    push    rbp
    push    rbx
    sub    rsp, 8
    mov    rbx, rdi
    mov    ebp, esi
    mov    edi, 16
    call    malloc
    test    rax, rax
    je    .L30
    mov    DWORD PTR [rax], ebp
    mov    QWORD PTR [rax+8], rbx
.L28:
    add    rsp, 8
    pop    rbx
    pop    rbp
    ret
.L30:
    mov    rax, rbx
    jmp    .L28
```

`mov edi, 16` es `sizeof(struct node)`. El puntero devuelto está en `rax`. Los dos almacenamientos siguientes, `[rax]` (4 bytes) y `[rax+8]` (8 bytes), son los campos `value` y `next`. El `test rax, rax / je` es la comprobación de fallo de asignación, que devuelve el antiguo `head`. Un `call malloc` (o `operator new` en C++) seguido de almacenamientos a través del resultado significa que se está inicializando un objeto, y el argumento es su tamaño.

### Matrices y aritmética de punteros

Un elemento de una matriz está en `base + index * element_size`. El modo de direccionamiento x86 `[base + index*scale]` lo expresa directamente, y el **factor de escala** (scale) es el tamaño del elemento cuando es 1, 2, 4 u 8.

```text
get:
    movsx    rdi, edi
    mov    eax, DWORD PTR table[0+rdi*4]
    ret
```

```text
get_local:
    movsx    rsi, esi
    mov    eax, DWORD PTR [rdi+rsi*4]
    ret
```

```text
get_long:
    movsx    rsi, esi
    mov    rax, QWORD PTR [rdi+rsi*8]
    ret
```

```text
get_char:
    movsx    rsi, esi
    movzx    eax, BYTE PTR [rdi+rsi]
    ret
```

Las cuatro funciones leen `p[i]`, y solo el tipo del elemento cambia la instrucción. `get` accede a la matriz global `table` en una dirección fija con escala 4 (es una matriz de `int`). `get_local` toma la base en `rdi` y escala por 4. `get_long` escala por 8 y carga 8 bytes en `rax`. `get_char` no tiene escala (escala 1) y carga un byte con `movzx`. El `movsx rsi, esi` del principio extiende con signo el índice de 32 bits a 64 bits. El tamaño de la carga (`BYTE`, `WORD`, `DWORD`, `QWORD`) y la escala juntos dan el tipo del elemento.

| Indicio | Significado |
|---|---|
| `[base+index*4]` con una carga de 4 bytes | Una matriz de `int` (o `float`, `unsigned`) |
| `[base+index*8]` con una carga de 8 bytes | Una matriz de `long`, punteros o `double` |
| `[base+index]` con una carga de un byte | Una matriz de `char` o bytes |
| Una base que es una dirección fija | Una matriz global; su tamaño aparece en la tabla de símbolos o en la sección |

Para una **matriz bidimensional** o una matriz de estructuras cuyo tamaño no es 1, 2, 4 ni 8, el índice debe multiplicarse primero:

```text
second_row:
    movsx    rsi, esi
    sal    rsi, 4
    add    rdi, rsi
    movsx    rdx, edx
    mov    eax, DWORD PTR [rdi+rdx*4]
    ret
```

`sal rsi, 4` multiplica el índice de fila por 16: cada fila de `int m[][4]` son 4 elementos de 4 bytes. La fila se suma a la base y la columna se escala luego por 4. El tamaño de fila 16 es el ancho de la dimensión interior; el código nunca menciona el número 4 como dimensión. Una matriz de estructuras funciona igual, con el tamaño de la estructura como paso:

```text
point_array:
    movsx    rsi, esi
    mov    eax, DWORD PTR [rdi+4+rsi*8]
    ret
```

`[rdi+4+rsi*8]` lee el campo `y` (desplazamiento 4) del elemento `i` de una matriz de valores `struct point` de 8 bytes. La escala 8 es el tamaño de la estructura entera y el desplazamiento 4 es la posición del campo dentro de ella. Las dos constantes llevan información distinta.

La resta de punteros divide por el tamaño del elemento:

```text
pointer_diff:
    sub    rsi, rdi
    sar    rsi, 2
    mov    eax, esi
    ret
```

`sub rsi, rdi` calcula la distancia en bytes y `sar rsi, 2` la divide entre 4: el resultado es un número de elementos `int`. Un desplazamiento tras la resta de dos punteros revela el tamaño del elemento.

### Estructuras

Una estructura es un bloque de memoria en el que cada campo tiene un desplazamiento fijo. El código accede a un campo como `[pointer + offset]`; ninguna instrucción menciona el nombre del campo. Recuperas la disposición reuniendo todos los `[reg + constant]` usados con el mismo puntero base, junto con el ancho de cada acceso.

```text
point_sum:
    mov    eax, DWORD PTR [rdi]
    add    eax, DWORD PTR [rdi+4]
    ret
```

`point_sum` lee los desplazamientos 0 y 4, ambos de 4 bytes: una estructura con dos campos `int`. Los compiladores colocan los campos en direcciones adecuadas a su tamaño, lo que deja **relleno** (padding) entre algunos campos. Un ejemplo mayor:

```text
record_id:
    mov    eax, DWORD PTR [rdi+4]
    ret
```

```text
record_flags:
    movzx    eax, WORD PTR [rdi+8]
    ret
```

```text
record_total:
    mov    rax, QWORD PTR [rdi+16]
    ret
```

```text
record_name:
    movsx    rsi, esi
    movzx    eax, BYTE PTR [rsi+24+rdi]
    ret
```

Reuniendo lo que muestran estas funciones de acceso para `struct record`:

| Desplazamiento | Ancho | Visto en | Campo |
|---|---|---|---|
| 0 | 1 | (no se lee aquí) | `char tag` |
| 4 | 4 | `[rdi+4]`, `DWORD` | `int id` |
| 8 | 2 | `[rdi+8]`, `WORD` | `short flags` |
| 16 | 8 | `[rdi+16]`, `QWORD` | `long total` |
| 24 | 1 por elemento | `[rsi+24+rdi]`, `BYTE` | `char name[6]` |

Los huecos son relleno. `id` está en 4 y no en 1 porque un campo de 4 bytes se alinea a 4. `total` está en 16 y no en 10 porque un campo de 8 bytes se alinea a 8. La estructura termina en 30 y se redondea hacia arriba a un múltiplo de 8: su tamaño es 32. El informe del propio compilador coincide (`offsetof` y `sizeof` dan para esta estructura `tag 0 id 4 flags 8 total 16 name 24 size 32`). El tamaño total es visible en otros lugares del programa: como argumento de `malloc`, como paso de una matriz de tales estructuras o como la cantidad restada de `rsp` para una local.

Reglas para leer una estructura:

- Toda constante sumada al mismo puntero base es un desplazamiento de campo.
- El ancho de la carga o el almacenamiento, y la elección de `movzx` o `movsx`, dan el tipo del campo.
- Un hueco entre desplazamientos es relleno o un campo que este código no usa.
- Un acceso con un registro índice en `[base + const + idx*scale]` es una matriz dentro de la estructura (`name`) cuando la escala es el tamaño del elemento.
- Dos funciones que usan distintos desplazamientos desde el mismo puntero miran el mismo objeto; une sus campos.

### Listas enlazadas

Un nodo de lista contiene un valor y un puntero al nodo siguiente. El recorrido es un bucle que sigue el puntero hasta que es nulo.

```text
list_sum:
    test    rdi, rdi
    je    .L16
    mov    eax, 0
.L15:
    add    eax, DWORD PTR [rdi]
    mov    rdi, QWORD PTR [rdi+8]
    test    rdi, rdi
    jne    .L15
    ret
.L16:
    mov    eax, 0
    ret
```

El bucle suma `[rdi]` (un `value` de 4 bytes) a la suma y sustituye `rdi` por `[rdi+8]`, el puntero `next` de 8 bytes. Se repite mientras `rdi` no sea cero. La firma de una lista enlazada es un bucle en el que el registro usado como base se **recarga desde la memoria a la que apunta**: `mov rdi, [rdi+8]`. La estructura es `{ int value; struct node *next; }`, con 4 bytes de relleno tras `value`. El `test rdi, rdi / je` anterior al bucle es la comprobación de lista vacía.

### Árboles

Un nodo de árbol binario tiene dos punteros a hijos. Una búsqueda sigue uno de ellos según una comparación.

```text
tree_has:
    test    rdi, rdi
    jne    .L22
    mov    eax, 0
    ret
.L20:
    mov    rdi, QWORD PTR [rdi+16]
.L21:
    test    rdi, rdi
    je    .L26
.L22:
    mov    eax, DWORD PTR [rdi]
    cmp    eax, esi
    je    .L24
    jle    .L20
    mov    rdi, QWORD PTR [rdi+8]
    jmp    .L21
.L26:
    mov    eax, 0
    ret
.L24:
    mov    eax, 1
    ret
```

El puntero al nodo está en `rdi`. El nodo tiene `key` en el desplazamiento 0 y dos punteros en los desplazamientos 8 y 16. El código compara `[rdi]` con la clave buscada `esi`: igual significa encontrada; `jle` (la clave del nodo es menor o igual que la buscada, o sea que la buscada es mayor) carga `[rdi+16]`, el hijo derecho; si no, carga `[rdi+8]`, el hijo izquierdo. Dos campos de puntero cargados alternativamente, elegidos por una comparación con la clave, son un árbol binario de búsqueda. El hijo izquierdo es el que se toma cuando la clave buscada es menor que la clave del nodo. Leer qué desplazamiento se toma a cada lado de la comparación te dice qué puntero es `left` y cuál es `right`.

### Objetos C++

Un objeto C++ con funciones virtuales empieza con un campo oculto: el **puntero a la tabla virtual** (vptr), que apunta a una tabla de direcciones de funciones, la **vtable**. Los campos de datos de la clase van detrás, primero los de la clase base.

El ejemplo `examples/15_cpp.cpp` tiene tres clases: `Shape`, con las funciones virtuales `area` y `sides`, `Rect`, derivada de `Shape`, y `Square`, derivada de `Rect`. Los constructores están expandidos en línea en la función que crea los objetos:

```text
_Z16make_and_measurev:
    push    rbx
    sub    rsp, 64
    mov    DWORD PTR [rsp+40], 1
    mov    QWORD PTR [rsp+32], OFFSET FLAT:_ZTV4Rect+16
    mov    DWORD PTR [rsp+44], 3
    mov    DWORD PTR [rsp+48], 4
    mov    DWORD PTR [rsp+8], 2
    mov    DWORD PTR [rsp+12], 5
    mov    DWORD PTR [rsp+16], 5
    mov    QWORD PTR [rsp], OFFSET FLAT:_ZTV6Square+16
    lea    rdi, [rsp+32]
    call    _Z10total_areaPK5Shape
    mov    ebx, eax
    mov    rdi, rsp
    call    _Z10total_areaPK5Shape
    add    eax, ebx
    add    rsp, 64
    pop    rbx
    ret
```

Mira el objeto en `[rsp+32]`. Empieza con el almacenamiento de una dirección, `OFFSET _ZTV4Rect+16`, en el desplazamiento 0: es el vptr, que apunta 16 bytes dentro de la vtable de `Rect`. Los siguientes almacenamientos son `1` en el desplazamiento 8, `3` en el 12 y `4` en el 16. Por tanto, la disposición de `Rect` es:

| Desplazamiento | Campo |
|---|---|
| 0 | vptr, oculto |
| 8 | `int id`, de `Shape` |
| 12 | `int w` |
| 16 | `int h` |
| 20 | relleno hasta un múltiplo de 8; el tamaño es 24 |

El tamaño 24 lo confirma el destructor de borrado, que se lo pasa a `operator delete`:

```text
_ZN4RectD0Ev:
    sub    rsp, 8
    mov    esi, 24
    call    _ZdlPvm
    add    rsp, 8
    ret
```

El segundo objeto, en `[rsp]`, es un `Square`: la misma disposición, con el vptr apuntando a la vtable de `Square` y con ancho y alto iguales a 5.

La vtable en sí son datos, en `.rodata`. Con los símbolos y `c++filt` para restaurar los nombres (demangle), las dos tablas son:

```text
vtable for Rect (_ZTV4Rect)             vtable for Square (_ZTV6Square)
    0                                       0
    0                                       0
    Rect::~Rect()      (complete)           Square::~Square()  (complete)
    Rect::~Rect()      (deleting)           Square::~Square()  (deleting)
    Rect::area() const                      Rect::area() const
    Rect::sides() const                     Square::sides() const
```

Toda vtable empieza con dos palabras, el desplazamiento hasta el inicio del objeto y un puntero a información de tipo (cero aquí, porque el ejemplo se construyó sin información de tipos en tiempo de ejecución, que un binario real suele tener: el puntero conduce entonces a una estructura que nombra la clase). El vptr apunta justo después de ellas, de modo que la primera entrada a la que apunta el vptr es la primera función virtual: la ranura 0 es el destructor completo, la ranura 1 el destructor de borrado, la ranura 2 `area` y la ranura 3 `sides`. **`Square` hereda `area` de `Rect`**, así que la ranura 2 contiene la misma dirección en ambas tablas; **`Square` sobrescribe `sides`**, así que la ranura 3 difiere.

Una llamada virtual carga el vptr del objeto, carga una dirección de función de la tabla y la llama con el puntero al objeto como primer argumento:

```text
_Z10total_areaPK5Shape:
    push    rbp
    push    rbx
    sub    rsp, 8
    mov    rbx, rdi
    mov    rax, QWORD PTR [rdi]
    call    [QWORD PTR [rax+16]]
    mov    ebp, eax
    mov    rax, QWORD PTR [rbx]
    mov    rdi, rbx
    call    [QWORD PTR [rax+24]]
    add    eax, ebp
    add    rsp, 8
    pop    rbx
    pop    rbp
    ret
```

`mov rax, [rdi]` carga el vptr y `call [rax+16]` llama a la función de la ranura 2 (el desplazamiento 16 son 2 ranuras de 8 bytes): `area`. La segunda llamada, `call [rax+24]`, es la ranura 3: `sides`. El puntero al objeto `this` permanece en `rdi` (se guarda en `rbx` durante la primera llamada y se repone). Qué función concreta se ejecuta depende de la vtable que tenga el objeto. Una llamada a través de un registro cargado desde `[object]` más una constante pequeña es una llamada virtual; la constante dividida entre 8 es el número de ranura.

Y una función miembro no virtual es una llamada ordinaria con `this` como primer argumento:

```text
_ZNK4Rect4areaEv:
    mov    eax, DWORD PTR [rdi+12]
    imul    eax, DWORD PTR [rdi+16]
    ret
```

`Rect::area` lee `w` en el desplazamiento 12 y `h` en el 16 a partir de `this` en `rdi` y los multiplica. Nada en el código dice que sea una función miembro salvo el nombre decorado (mangled) y los desplazamientos desde el puntero.

| Indicio | Significado |
|---|---|
| Un almacenamiento de una dirección de la sección de código en `[obj]` durante la construcción | El vptr; la dirección es una vtable más 16 |
| `mov rax, [rdi]` y luego `call [rax+N]` | Una llamada virtual a la ranura `N / 8` |
| Una tabla de direcciones de funciones en `.rodata`, precedida por dos palabras | Una vtable |
| Dos tablas que comparten la mayoría de las entradas | Una clase base y una derivada; las diferencias son las sobrescrituras |
| El primer argumento `rdi` usado como base en todas las funciones de un grupo | Un conjunto de funciones miembro de una clase |

### Construir tipos en Ghidra

Cuando conozcas una disposición, regístrala para que el descompilador pueda usarla.

1. Abre el Data Type Manager y crea una **estructura**. Añade los campos con sus desplazamientos, tamaños y nombres. Usa campos de relleno "undefined" para llegar a los desplazamientos correctos.
2. En el descompilador, selecciona una variable que contenga un puntero al objeto y cambia su tipo (tecla por defecto `Ctrl+L`) a un puntero a tu estructura. El descompilador reescribe `*(int *)(p + 4)` como `p->id`.
3. Para una clase con funciones virtuales, crea una estructura para la vtable cuyos campos sean punteros a función y haz que el primer campo del objeto sea un puntero a ella. El descompilador mostrará entonces las llamadas virtuales como `obj->vtable->area(obj)`.
4. Renombra las funciones a medida que sepas lo que hacen y fija su primer parámetro al puntero de la clase.

Los tipos se propagan: cuando una función usa la estructura, todas las funciones que reciben el mismo puntero se vuelven más legibles.

## Pruébalo

1. Ejecuta `make` y abre `build/15_data.s`. En `record_name`, averigua qué significan el desplazamiento 24 y la ausencia de escala.
2. Con `readelf -S -W build/15_data.o` y `nm build/15_data.o`, averigua qué sección contiene `table` y `counter` y qué tamaño tiene `table`.
3. Añade un campo `char pad` tras `flags` en `struct record` y predice qué desplazamientos cambian antes de reconstruir. Luego reconstruye y comprueba.
4. Escribe una función C que devuelva la clave situada más a la derecha de un `struct tree` y lee el bucle en la salida compilada.
5. Ejecuta `nm -C build/15_cpp.o` y `objdump -d -M intel -C build/15_cpp.o`. Encuentra `Rect::area() const` y las dos tablas. Añade una nueva función virtual a `Shape`, reconstruye y mira qué números de ranura cambian.

## Errores comunes

- **Leer el relleno como un campo.** Un hueco entre desplazamientos puede ser alineación o un campo que esta función no usa. Espera a que otra función lo confirme.
- **Confundir la escala con el tamaño de la estructura.** Una escala de 8 puede ser una matriz de estructuras de 8 bytes o una matriz de `long`. El desplazamiento y el ancho del acceso marcan la diferencia.
- **Pasar por alto el vptr oculto.** En una clase C++ con funciones virtuales, el desplazamiento 0 es el vptr y el primer miembro está en el desplazamiento 8.
- **Olvidar que `this` es el primer argumento.** En una función miembro, `rdi` es el puntero al objeto y los argumentos visibles empiezan en `rsi`.
- **Tratar todo seguimiento de punteros como una lista.** Un bucle que recarga su registro base desde el objeto es una lista, pero uno que recarga desde `[rdi+8]` o `[rdi+16]` según una comparación es un árbol.

## Resumen

- La escala de un índice y el ancho de una carga dan el tipo de un elemento de matriz; una constante sumada a un puntero es un campo.
- Una estructura es un conjunto de desplazamientos y anchos reunidos de todos los accesos, con relleno entre ellos; su tamaño es visible en asignaciones, pasos y espacio de pila.
- Una lista enlazada es un bucle que recarga su registro base desde el nodo al que apunta; un árbol es lo mismo con dos punteros elegidos por una comparación.
- Un objeto C++ con funciones virtuales empieza con un vptr; una llamada virtual es `mov rax, [obj]` y luego `call [rax+8*slot]`.
- Anotar la disposición recuperada como un tipo acelera el resto del análisis.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 16.
- Hojas de referencia: [Modismos del compilador](../../cheatsheets/compiler-idioms.es.md).
- Ejercicios que usan este módulo: [Ejercicio 14](../../exercises/14-shape-shifter/README.es.md), [Ejercicio 15](../../exercises/15-linked-inventory/README.es.md), [Ejercicio 20](../../exercises/20-capstone-tiny-vm/README.es.md).
