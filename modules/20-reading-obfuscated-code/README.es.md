<!-- source: README.md @ sha256:002e7d34793e -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 20. Leer código ofuscado

Tiempo: unas 2,5 horas. Requisitos previos: [19. Analizar programas sospechosos](../19-analyzing-suspicious-programs/README.es.md)

## Por qué importa

Algunos programas están escritos para ser difíciles de leer. Sus cadenas están codificadas para que una búsqueda no encuentre nada, su lógica está cortada en trozos y barajada, su código está relleno de instrucciones que no hacen nada y algunos notan cuándo los están observando. Los defensores se topan con esto en el malware, y todo el mundo se topa con ello en el software protegido. La ofuscación nunca cambia lo que hace un programa; solo cambia lo difícil que es verlo. El procesador debe seguir ejecutando el comportamiento real, lo que significa que el comportamiento siempre puede recuperarse. Este módulo enseña las técnicas habituales, cómo reconocer cada una en un desensamblado y cómo elegir la manera más barata de ver a través de ellas. Todos los ejemplos son programas inofensivos escritos para la lección. El módulo trata de leer código para entenderlo; no cubre quitar las protecciones del software comercial.

## Conceptos

### Qué hace la ofuscación y qué hacer al respecto

Los programas se ofuscan para proteger una idea, para esquivar escáneres que buscan cadenas y patrones conocidos y para hacer perder el tiempo a quien los analiza. Una técnica que dificulta la lectura no es una barrera para la comprensión, porque todo programa ofuscado sigue teniendo que hacer su trabajo. Tu tarea es encontrar el camino más barato hasta el comportamiento. Los tres caminos que usa este módulo son:

| Camino | Idea | Bueno para |
|---|---|---|
| Leer el código | Reconocer la técnica y deshacerla a mano o con un script pequeño | Cadenas codificadas, código basura, una función aplanada pequeña |
| Emulación | Ejecutar solo una rutina pequeña en un emulador, sin acceso al sistema | Rutinas de descodificación, derivación de hashes y claves, cualquier cálculo autocontenido |
| Flujo de datos | Partir del resultado que te interesa y seguir solo lo que contribuye a él | Código relleno de basura, funciones grandes, todo aquello donde la mayoría de las instrucciones son ruido |

Ejecutar el programa entero es la última opción, no la primera, y solo se hace en el laboratorio aislado del módulo 19.

### Cadenas codificadas

Las cadenas son la prueba más útil en el triaje, así que son lo primero que se esconde. El programa guarda el texto en forma codificada y lo descodifica cuando lo necesita. Las señales:

- `strings` no encuentra nada donde el programa claramente usa una ruta, un anfitrión o un mensaje. Compara lo que sabes que hace el programa con lo que muestra la lista de cadenas.
- Una tabla de bytes en la sección de datos cuyos valores parecen aleatorios, junto a una función pequeña que la recorre.
- Una función a la que se llama con un destino, un origen, una longitud y una constante pequeña antes de cada uso de una cadena.
- Un bucle con `xor` de un byte de memoria con un registro o una constante.

El ejemplo `examples/20_strings.c` esconde tres cadenas de tres maneras. El programa las imprime:

```bash
make
build/20_strings
strings -a build/20_strings_stripped | grep -iE "http|temp|status|example"
```

```text
https://config.example.test/a
C:\Temp\out.dat
status=ok
```

El segundo comando, ejecutado sobre el programa sin símbolos, no imprime nada: ninguna de las tres cadenas existe en el archivo.

**Esquema 1: una clave fija.** El descodificador es el bucle más corto posible:

```text
401126:  test   rdx,rdx
401129:  je     401144
40112b:  mov    eax,0x0
401130:  mov    r8d,ecx                      ; the key, from the fourth argument
401133:  xor    r8b,BYTE PTR [rsi+rax*1]     ; key xor the encoded byte
401137:  mov    BYTE PTR [rdi+rax*1],r8b     ; store the decoded byte
40113b:  add    rax,0x1
40113f:  cmp    rdx,rax                      ; length in rdx
401142:  jne    401130
401144:  mov    BYTE PTR [rdi+rdx*1],0x0     ; terminator
401148:  ret
```

Los argumentos son el destino (`rdi`), los bytes codificados (`rsi`), la longitud (`rdx`) y la clave (`ecx`). Quien llama pasa la clave como constante (`mov ecx,0x5a`), así que la clave es visible en el código. Una clave de un byte también puede encontrarse sin el código: los primeros bytes de una URL o de una ruta se conocen (`http`, `C:\`), y aplicar xor al primer byte codificado con la letra conocida da la clave. El script `examples/20_decode.py` va más lejos y prueba las 256 claves, conservando los resultados que parecen una URL o una ruta:

```bash
python3 20_decode.py
```

```text
blob A, every key that gives plausible text:
  key 0x5a: https://config.example.test/a
blob B with the rolling rule, seed 0x21:
  C:\Temp\out.dat
```

**Esquema 2: una clave rodante.** La clave cambia con cada byte, así que un solo xor de todo el blob no funciona:

```text
401149:  test   rdx,rdx
40114c:  je     40116b
40114e:  mov    eax,0x0
401153:  lea    ecx,[rcx+rcx*4+0x3]          ; k = k * 5 + 3
401157:  mov    r8d,ecx
40115a:  xor    r8b,BYTE PTR [rsi+rax*1]     ; xor with the low byte of k
40115e:  mov    BYTE PTR [rdi+rax*1],r8b
401162:  add    rax,0x1
401166:  cmp    rdx,rax
401169:  jne    401153
40116b:  mov    BYTE PTR [rdi+rdx*1],0x0
40116f:  ret
```

La señal es una actualización del registro de la clave dentro del bucle, aquí `lea ecx,[rcx+rcx*4+3]`, que calcula `k * 5 + 3`. Para descodificar necesitas la regla y la semilla (el valor inicial que pasa quien llama, 0x21). Cuando la regla es simple, unas pocas líneas de script la reproducen, como en `20_decode.py`. Cuando no lo es, emula la función.

**Esquema 3: una cadena de pila.** El texto nunca existe como una sola pieza en el archivo. El código lo construye byte a byte a partir de constantes de las instrucciones:

```text
401170:  mov    BYTE PTR [rdi],0x73          ; s
401173:  mov    BYTE PTR [rdi+0x1],0x74      ; t
401177:  mov    BYTE PTR [rdi+0x2],0x61      ; a
40117b:  mov    BYTE PTR [rdi+0x3],0x74      ; t
40117f:  mov    BYTE PTR [rdi+0x4],0x75      ; u
401183:  mov    BYTE PTR [rdi+0x5],0x73      ; s
401187:  mov    BYTE PTR [rdi+0x6],0x3d      ; =
40118b:  mov    BYTE PTR [rdi+0x7],0x6f      ; o
40118f:  mov    BYTE PTR [rdi+0x8],0x6b      ; k
401193:  mov    BYTE PTR [rdi+0x9],0x0
401197:  ret
```

Una serie de almacenamientos de valores inmediatos en desplazamientos consecutivos es una cadena de pila, y los inmediatos son ASCII imprimible. Un listado de desensamblado muestra las piezas, pero un descompilador o un script corto que lea los inmediatos en orden da la cadena completa. Los compiladores pueden fundir los almacenamientos en otros más anchos (`mov DWORD PTR`, `movabs`), así que busca el mismo patrón con constantes de 4 y 8 bytes cuyos bytes sean ASCII.

### Emular una rutina pequeña

Descodificar a mano sirve para esquemas simples. Para todo lo demás, ejecuta la rutina. Un emulador ejecuta las instrucciones de una función en un modelo del procesador, y te da el resultado sin ejecutar el programa, sin llamadas al sistema y sin ningún efecto fuera de su propia memoria. El script `examples/20_emulate.py` usa el emulador Unicorn: mapea los segmentos cargables del archivo, prepara una pila, una dirección de retorno y los argumentos, ejecuta desde la dirección de la función hasta que retorna y lee el búfer de salida.

```bash
python3 20_emulate.py build/20_strings_stripped 401126 402020 29 5a
python3 20_emulate.py build/20_strings_stripped 401149 402010 15 21
```

```text
179 instructions emulated
https://config.example.test/a
110 instructions emulated
C:\Temp\out.dat
```

Los argumentos son el archivo, la dirección del descodificador, la dirección de los datos codificados, su longitud y la clave. Los cuatro números vienen de quien llama: el `main` sin símbolos los carga en `ecx`, `edx` y `esi` justo antes de cada `call` (`mov ecx,0x5a`, `mov edx,0x1d`, `mov esi,0x402020`, `call 401126`). Ese es el método: encontrar el descodificador por su forma, encontrar una llamada a él, leer sus argumentos en las instrucciones previas a la llamada y emular. El mismo enfoque lee tablas enteras de cadenas codificadas: recorre los puntos de llamada y emula cada uno.

La emulación tiene límites. Una rutina que hace llamadas al sistema o llama a funciones de biblioteca necesita sustitutos para ellas, o hay que cortarla en ese punto. Una rutina que lee memoria que la emulación no tiene (una global que el programa preparó antes) necesita que esa memoria se rellene primero.

### Código basura y predicados opacos

El **código basura** es código sin efecto sobre el resultado: cálculos cuyos valores nunca se usan, llamadas a funciones que no hacen nada, saltos que van a la instrucción siguiente. Alarga las funciones y esconde la estructura real. Un **predicado opaco** (opaque predicate) es una condición que siempre tiene el mismo valor, pero cuyo valor es difícil de ver a simple vista. La rama se toma entonces siempre o nunca, así que un lado es código muerto; el objetivo es agrandar el grafo de flujo de control y añadir trabajo.

La función `score_flat` de `examples/20_flat.c` tiene ambas cosas. Su basura es la actualización de `g_junk` (nadie la lee salvo la rama muerta), y su predicado opaco es `(n * (n + 1)) % 2 == 1`. El producto de un número y su sucesor es siempre par, así que el predicado es falso para todo `n`:

```text
4011d2:  lea    eax,[rdx+0x1]                ; n + 1
4011d5:  imul   eax,edx                      ; n * (n + 1)
4011d8:  test   al,0x1                       ; low bit: the product is odd?
4011da:  je     4011e2                       ; the product is always even, so this jump is always taken
4011dc:  mov    ecx,DWORD PTR [rip+0x2e3a]   ; dead: r = g_junk
4011e2:  ...
```

Para encontrar predicados opacos, pregunta de cada salto condicional si la condición puede depender de algo que cambie. Las identidades aritméticas (un producto de números consecutivos es par; `x * x` nunca es 2; `(x | 1)` nunca es 0) son la fuente habitual. Comprueba tu sospecha emulando con varias entradas o escribiendo la identidad. Cuando un salto va siempre en un sentido, trata el lado muerto como eliminado.

### Aplanamiento del flujo de control

Una función normal es un grafo de bloques que siguen su lógica. El **aplanamiento del flujo de control** (control-flow flattening) sustituye el grafo por un bucle. Una variable contiene el número del bloque que se ejecutará a continuación. Un distribuidor (dispatcher) al principio del bucle lee la variable y salta al bloque correspondiente. Cada bloque termina guardando el número de su sucesor y saltando de vuelta al distribuidor. La estructura del programa sale de la forma del código y pasa a los valores de la variable.

Compara la versión simple de la función con la aplanada (el compilador produjo ambas a partir del código fuente de `20_flat.c`):

```text
0000000000401126 <score_plain>:
  401126:  mov    eax,edi
  401128:  mov    edx,0xaaaaaaab
  40112d:  imul   rax,rdx
  401131:  shr    rax,0x21                     ; n / 3 (module 14)
  401135:  lea    eax,[rax+rax*2]
  401138:  cmp    edi,eax                      ; n == (n / 3) * 3 ?
  40113a:  je     40115c
  ...
  40115b:  ret
```

```text
0000000000401160 <score_flat>:
  401162:  mov    DWORD PTR [rip+0x2eb4],0x0   ; g_state = 0
  401171:  mov    eax,DWORD PTR [rip+0x2ea9]   ; the dispatcher: load the state
  401177:  cmp    eax,0x4
  40117a:  ja     401230                       ; above 4: leave the loop
  401182:  jmp    QWORD PTR [rax*8+0x402008]   ; jump table indexed by the state
  ...                                          ; the blocks
  4011c6:  mov    DWORD PTR [rip+0x2e50],0x5   ; ... each one stores the next state
  4011d0:  jmp    401171                       ; ... and goes back to the dispatcher
```

Las señales del aplanamiento: un bloque central con una tabla de saltos o una cadena de comparaciones sobre una única variable; muchos bloques que terminan todos con un salto de vuelta al centro; asignaciones de constantes a esa variable; y una función cuyo grafo parece una estrella. Ayudan dos ideas.

1. Las asignaciones de cada bloque te dicen sus sucesores. Un bloque que guarda una constante tiene un sucesor. Un bloque que guarda uno de dos valores según una comparación es una bifurcación, y la comparación es la condición original.
2. Puedes recuperar el grafo rastreando. Ejecuta la función con un puñado de entradas y anota cada valor guardado en la variable de estado; las secuencias son caminos por el grafo original. El script `examples/20_trace_states.py` lo hace en el emulador, enganchando las escrituras en la dirección de la variable de estado (en una muestra real es una variable local o un registro, y la idea es la misma):

```bash
python3 20_trace_states.py build/20_flat 401160 404020
```

```text
n= 0: states 0 -> 1 -> 5  result 0
n= 1: states 0 -> 2 -> 4 -> 5  result 84
n= 2: states 0 -> 2 -> 4 -> 5  result 87
n= 3: states 0 -> 1 -> 5  result 6
n= 4: states 0 -> 2 -> 4 -> 5  result 81
n= 5: states 0 -> 2 -> 3 -> 5  result 12
...
n=14: states 0 -> 2 -> 4 -> 5  result 91
edges of the recovered graph:
  0 -> 1, 2
  1 -> 5
  2 -> 3, 4
  3 -> 5
  4 -> 5
```

El grafo recuperado es un árbol con la forma del `if / else if / else` original: el estado 0 comprueba la divisibilidad por 3 (las entradas 0, 3, 6, 9, 12 van al estado 1, cuyo bloque duplica la entrada), el estado 2 comprueba la divisibilidad por 5 (5 y 10 van al estado 3, que suma 7) y el resto va al estado 4 (un xor con 0x55). Eso es exactamente `score_plain`. Observa que el predicado opaco del estado 2 nunca cambió el camino.

### Reconocer las comprobaciones de depurador y de análisis

Algunos programas buscan señales de que los están analizando y cambian su comportamiento cuando las encuentran, por ejemplo terminando, tomando un camino inofensivo o descodificando algo distinto. Necesitas reconocer las comprobaciones durante el triaje, porque un programa que se comporta de otra manera bajo observación puede hacer engañoso un análisis dinámico. Las comprobaciones habituales:

| Comprobación | Qué aspecto tiene |
|---|---|
| Una llamada al sistema que falla si hay un rastreador conectado (Linux: `ptrace` con la petición de ser rastreado; Windows: `IsDebuggerPresent`, `CheckRemoteDebuggerPresent`, una lectura del indicador de depuración en el bloque del proceso) | Una importación o una llamada a estas funciones, seguida de una prueba del resultado y una bifurcación |
| Leer el estado del proceso en busca de un rastreador (Linux: el campo `TracerPid` de `/proc/self/status`) | La cadena `/proc/self/status` y una comparación del campo con 0 |
| Temporización | Dos lecturas de un reloj (`rdtsc`, un contador de ticks) alrededor de un bloque y una comparación con un límite |
| Trucos con puntos de interrupción y excepciones | Código que provoca una excepción o contiene una instrucción de interrupción a propósito y espera que se ejecute un manejador |
| Búsqueda de herramientas de análisis o de máquinas virtuales | Búsquedas en la lista de procesos, nombres de ventanas, claves del registro, nombres de dispositivos e identificadores de hardware |

El ejemplo `examples/20_dbg.c` hace dos comprobaciones y solo informa de lo que encontró. Una ejecución normal, una ejecución bajo un rastreador de llamadas al sistema y una ejecución bajo el depurador dan respuestas distintas:

```bash
build/20_dbg
strace -f -o /dev/null build/20_dbg
gdb -nx -q -batch -ex run build/20_dbg
```

```text
status check: no debugger
ptrace check: no debugger
status check: debugger detected
ptrace check: debugger detected
status check: debugger detected
ptrace check: debugger detected
```

(El primer par es la ejecución normal, el segundo la ejecución bajo `strace`, el tercero la ejecución bajo `gdb`.) El triaje estático encuentra las dos comprobaciones sin ejecutar nada: el programa importa `ptrace` y `fopen`, y contiene las cadenas `/proc/self/status` y `TracerPid:`. Cuando encuentres una comprobación, anota dónde está, qué comprueba y qué hace distinto el programa cuando salta. El análisis estático y la emulación no se ven afectados por estas comprobaciones, porque el programa no se ejecuta en el sentido normal, una razón más para preferirlos. Cuando haya que hacer una observación con el programa en marcha, indica en el informe que hay una comprobación y que la observación puede no representar una ejecución normal.

### Automatizar el análisis con scripts

Cuando se repite el mismo trabajo (muchas cadenas codificadas, muchos puntos de llamada), escribe un script. En Ghidra, un script puede leer bytes en una dirección, descodificarlos y poner el resultado en un comentario para que aparezca junto al código. El esbozo siguiente muestra el patrón para el esquema de clave fija. Usa la interfaz de scripts de Ghidra, así que se ejecuta dentro de Ghidra en el anfitrión, no en la imagen del laboratorio, y las comprobaciones del laboratorio no lo ejecutan; verifica los nombres con la ayuda del gestor de scripts de tu versión de Ghidra.

```text
# Decode an xor-encoded string and write it as a comment (Ghidra Script Manager, Python)
from ghidra.program.model.listing import CodeUnit
addr = toAddr(0x402020)                       # the address of the encoded bytes
data = getBytes(addr, 29)                     # 29 bytes
text = "".join(chr((b & 0xff) ^ 0x5a) for b in data)
currentProgram.getListing().setComment(addr, CodeUnit.EOL_COMMENT, text)
```

Para muchas cadenas, reúne los puntos de llamada del descodificador (las referencias cruzadas a su dirección), lee los argumentos en las instrucciones previas a cada llamada y aplica la misma descodificación a cada uno.

### Un orden de trabajo

1. **Triaje.** Haz las comprobaciones del módulo 19: cadenas, entropía, importaciones. Marca lo que falta (ninguna cadena donde las esperas) y lo que es raro (tablas de aspecto aleatorio, una lista de importaciones diminuta).
2. **Encontrar los descodificadores.** Busca bucles pequeños con un `xor`, funciones a las que se llama muchas veces con una constante y una longitud, y series de almacenamientos de bytes.
3. **Descodificar.** A mano o con un script en los esquemas simples, por emulación en los demás. Pon los resultados en comentarios o en una tabla.
4. **Encontrar la estructura.** Busca un bloque central con muchos saltos de vuelta. Rastrea la variable de estado y reconstruye el grafo.
5. **Cortar por flujo de datos.** Parte de lo que devuelve la función o de los argumentos de una llamada interesante y sigue solo las instrucciones que contribuyen. Todo lo demás es ruido.
6. **Anotar las comprobaciones.** Enumera cada comprobación de análisis que encuentres, con su ubicación y su efecto.

## Pruébalo

Ejecuta los comandos del laboratorio desde `modules/20-reading-obfuscated-code/examples`.

1. Compila con `make`. Busca con `strings` las tres cadenas en el programa sin símbolos y luego imprímelas ejecutando el programa. ¿Cuál de los tres esquemas encontrarías buscando `xor` en el desensamblado?
2. Desensambla `build/20_strings_stripped`. Encuentra los dos descodificadores y el constructor de la cadena de pila. En `main`, lee los argumentos de cada llamada.
3. Usa `20_emulate.py` para descodificar la segunda cadena. Cambia la semilla en uno y mira qué le pasa al resultado.
4. Escribe un descodificador para un tercer esquema propio (por ejemplo, suma la posición del byte a la clave), codifica una cadena con él y comprueba que `20_emulate.py` la descodifica cuando compilas el descodificador.
5. Desensambla `score_flat` y dibuja sus bloques y las aristas entre ellos a partir de los almacenamientos en la variable de estado. Compara el dibujo con la salida de `20_trace_states.py`.
6. Ejecuta `20_dbg` normalmente, bajo `strace` y bajo `gdb`. Encuentra las dos comprobaciones con `strings` y `objdump` sin ejecutar el programa.

## Errores frecuentes

- **Buscar la cadena en lugar del descodificador.** Una cadena codificada no se encuentra por su texto. Encuentra el código que la produce.
- **Suponer que la clave es una constante.** Una clave que cambia en el bucle (rodante) o depende de la posición necesita la regla, no un único valor.
- **Emular sin comprobar el resultado.** Verifica que la salida se parezca a lo que debe: texto imprimible, una ruta verosímil. Un argumento equivocado da ruido, que parece un descodificador fallido.
- **Seguir al distribuidor en lugar de los datos.** En una función aplanada, el orden de los bloques en memoria no significa nada. Sigue la variable de estado.
- **Tomar una rama muerta por real.** Una rama cuya condición es una identidad no es comportamiento. Compruébala antes de documentarla.
- **Olvidar que el programa puede observarte.** Un resultado obtenido bajo un depurador puede diferir de una ejecución normal. Dilo en el informe.

## Resumen

- La ofuscación cambia lo difícil que es leer el código, no lo que hace, así que el comportamiento siempre puede recuperarse.
- Las cadenas codificadas se manifiestan como una cadena ausente, una tabla de bytes de aspecto aleatorio y un pequeño bucle de descodificación; los esquemas son una clave fija, una clave rodante y una cadena de pila.
- Emular una rutina pequeña da su resultado sin ejecutar el programa; encuentra la rutina, lee sus argumentos en los puntos de llamada y ejecútala.
- El código basura y los predicados opacos añaden ruido; una condición que es una identidad aritmética es siempre igual y su lado muerto puede eliminarse.
- El aplanamiento del flujo de control sustituye el grafo por un distribuidor y una variable de estado; las asignaciones o un rastreo de la variable devuelven el grafo original.
- Las comprobaciones de depurador y de análisis se reconocen en el triaje por sus importaciones, cadenas y patrones de comparación y salto, y se anotan en el informe.
- Un orden de trabajo, del triaje a la descodificación, la estructura y el flujo de datos, mantiene barato el trabajo.

## Próximos pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 21, máquinas virtuales y bytecode.
- Ejercicios que usan este módulo: [Ejercicio 19](../../exercises/19-triage-desk/README.es.md).
