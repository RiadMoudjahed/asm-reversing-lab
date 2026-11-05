<!-- source: README.md @ sha256:cdab37d79f42 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 11. La caja de herramientas del ingeniero inverso

Tiempo: aproximadamente 3 horas. Requisitos previos: [10. Fundamentos de sistemas operativos para ingenieros inversos](../10-os-for-reversers/README.es.md)

## Por qué importa

El flujo de trabajo del módulo 09 solo funciona si puedes llevar a cabo cada paso con rapidez. Este módulo presenta las herramientas en el orden en que las usas: identificar el archivo, leer su estructura, desensamblarlo, explorarlo de forma interactiva, ejecutarlo bajo un depurador y observarlo desde fuera. Usa un programa pequeño, construido a partir de `examples/11_target.c`, para que puedas comparar lo que cada herramienta muestra sobre el mismo código.

## Conceptos

### El programa de práctica

`11_target` recibe una palabra como argumento, imprime un número calculado a partir de ella e imprime `access granted` para una palabra concreta y `denied` para cualquier otra. Constrúyelo y ejecútalo:

```bash
make
build/11_target hello
build/11_target open-sesame
```

```text
score 99162322
denied
score -1484336485
access granted
```

Los comandos de este módulo se ejecutan desde `modules/11-toolkit/examples`. El programa es deliberadamente sencillo: contiene un bucle (`score`), una comparación (`check`) y una llamada a biblioteca (`strcmp`).

### Paso 1: identificar el archivo

`file` lee las cabeceras y dice qué es el archivo:

```bash
file build/11_target
```

```text
build/11_target: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=3690f8d5a33ef0140c7ee95bff861bc16fb6e86b, for GNU/Linux 3.2.0, not stripped
```

`strings` imprime secuencias de caracteres imprimibles. `-t x` añade el desplazamiento en el archivo en hexadecimal y `-n` fija la longitud mínima:

```bash
strings -t x -n 6 build/11_target | grep sesame
```

```text
   2038 open-sesame
```

`xxd` muestra los bytes en bruto. Este es el comienzo de la cabecera ELF:

```bash
xxd -l 32 build/11_target
```

```text
00000000: 7f45 4c46 0201 0100 0000 0000 0000 0000  .ELF............
00000010: 0200 3e00 0100 0000 7010 4000 0000 0000  ..>.....p.@.....
```

Los cuatro primeros bytes `7f 45 4c 46` son el número mágico (`.ELF`). El byte 4 es `02` para 64 bits y el byte 5 es `01` para little-endian. En el desplazamiento 0x10 el tipo es `02 00` (ejecutable) y la máquina es `3e 00` (x86-64). En el desplazamiento 0x18 el punto de entrada de 8 bytes es `70 10 40 00 00 00 00 00`, que es la dirección 0x401070. El módulo 12 explica cada campo.

### Paso 2: leer la estructura

`readelf` imprime la estructura del archivo sin interpretar el código:

```bash
readelf -h build/11_target | grep -E "Class|Type|Machine|Entry"
readelf -d build/11_target | grep NEEDED
readelf -S -W build/11_target | grep -E " \.(text|rodata|data|bss|got)"
readelf -s build/11_target | grep -E "FUNC.*(score|check|main)"
```

```text
  Class:                             ELF64
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Entry point address:               0x401070
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
  [14] .text             PROGBITS        0000000000401070 001070 0001b5 00  AX  0   0 16
  [16] .rodata           PROGBITS        0000000000402000 002000 000044 00   A  0   0  8
  [22] .got              PROGBITS        0000000000403fd8 002fd8 000010 08  WA  0   0  8
  [23] .got.plt          PROGBITS        0000000000403fe8 002fe8 000038 08  WA  0   0  8
  [24] .data             PROGBITS        0000000000404020 003020 000010 00  WA  0   0  8
  [25] .bss              NOBITS          0000000000404040 003030 000010 00  WA  0   0 32
    28: 0000000000401156    46 FUNC    GLOBAL DEFAULT   14 score
    34: 00000000004011a1   132 FUNC    GLOBAL DEFAULT   14 main
    36: 0000000000401184    29 FUNC    GLOBAL DEFAULT   14 check
```

Las secciones indican dónde están el código (`.text`, indicadores `AX`: asignado y ejecutable), las constantes (`.rodata`, `A`) y los datos (`.data`, `.bss`, indicadores `WA`: escribible). La tabla de símbolos da la dirección y el tamaño de cada función. Un binario sin símbolos (stripped) no tiene nombres de función; los redescubres tú mismo.

### Paso 3: desensamblar

`objdump -d` desensambla las secciones ejecutables. Imprime sintaxis AT&T por defecto y sintaxis Intel con `-M intel`. Este curso usa sintaxis Intel, pero la salida de los compiladores y muchas herramientas usan AT&T por defecto, así que debes saber leer ambas.

```bash
objdump -d -M intel --no-show-raw-insn build/11_target | awk '/<check>:/{p=1} p{print} /^$/{if(p)exit}'
objdump -d --no-show-raw-insn build/11_target | awk '/<check>:/{p=1} p{print} /^$/{if(p)exit}'
```

```text
0000000000401184 <check>:
  401184:    sub    rsp,0x8
  401188:    lea    rsi,[rip+0xea9]        # 402038 <secret>
  40118f:    call   401040 <strcmp@plt>
  401194:    test   eax,eax
  401196:    sete   al
  401199:    movzx  eax,al
  40119c:    add    rsp,0x8
  4011a0:    ret
```

```text
0000000000401184 <check>:
  401184:    sub    $0x8,%rsp
  401188:    lea    0xea9(%rip),%rsi        # 402038 <secret>
  40118f:    call   401040 <strcmp@plt>
  401194:    test   %eax,%eax
  401196:    sete   %al
  401199:    movzbl %al,%eax
  40119c:    add    $0x8,%rsp
  4011a0:    ret
```

Lectura de la función: carga la dirección de `secret` en `rsi` (el segundo argumento), llama a `strcmp` (el primer argumento ya está en `rdi`) y convierte "el resultado es cero" en un 0 o un 1 en `eax`. La función devuelve si las dos cadenas son iguales.

#### Leer la sintaxis AT&T

| | Intel | AT&T |
|---|---|---|
| Orden de los operandos | destino, origen | origen, destino |
| Registros | `eax` | `%eax` |
| Constantes | `0x8` | `$0x8` |
| Memoria | `[rsi + rcx*4 + 8]` | `8(%rsi,%rcx,4)` |
| Tamaño del operando | En el operando: `dword ptr [rax]` | En el sufijo del mnemónico: `movl`, `movq`, `movzbl` |
| Ejemplo | `movzx eax, al` | `movzbl %al, %eax` |

Para convertir, invierte los operandos y quita los adornos. Las letras del sufijo son `b` (byte), `w` (palabra), `l` (doble palabra), `q` (cuádruple palabra). En `movzbl` las dos letras significan "de byte a long".

### Paso 4: explorar de forma interactiva con Rizin

Rizin es un desensamblador y marco de análisis de línea de comandos. `-A` analiza el archivo al cargarlo, `-q` sale tras los comandos y `-c` los ejecuta.

```bash
rizin -q -e scr.color=0 -A -c "pdf @ sym.check" build/11_target
```

```text
            ; CALL XREF from main @ 0x4011cf
┌ sym.check(const char *s1);
│           ; arg const char *s1 @ rdi
│           0x00401184      sub   rsp, 0x08
│           0x00401188      lea   rsi, qword obj.secret                ; 0x402038 ; "open-sesame" ; const char *s2
│           0x0040118f      call  sym.imp.strcmp                       ; int strcmp(const char *s1, const char *s2)
│           0x00401194      test  eax, eax
│           0x00401196      setz  al
│           0x00401199      movzx eax, al
│           0x0040119c      add   rsp, 0x08
└           0x004011a0      ret
```

Rizin hizo más que `objdump`: nombró el argumento, resolvió la referencia de datos a la cadena y mostró el prototipo de `strcmp`. Los comandos que más usarás:

| Comando | Significado |
|---|---|
| `aaa` | Analiza todo (la opción `-A` lo hace al cargar) |
| `afl` | Lista las funciones encontradas |
| `pdf @ sym.check` | Imprime el desensamblado de una función |
| `axt @ sym.check` | Lista las referencias cruzadas a `sym.check` (quién la llama) |
| `iz` | Lista las cadenas de las secciones de datos |
| `s 0x401184` | Se posiciona en una dirección |
| `?` | Ayuda de cualquier comando; añade `?` a un prefijo como `a?` |

```bash
rizin -q -e scr.color=0 -A -c "axt @ sym.check; iz~sesame" build/11_target
```

```text
main 0x4011cf [CALL] call sym.check
0x00002038 0x00402038  11   12 .rodata ascii open-sesame
```

La primera línea dice que `main` llama a `check` en 0x4011cf. La segunda dice que la cadena está en el desplazamiento de archivo 0x2038 y en la dirección virtual 0x402038, lo que coincide con `strings -t x`.

### Paso 5: Ghidra

Ghidra es un conjunto gráfico con desensamblador y descompilador. Lo instalaste en tu equipo en el módulo 00. Este es el flujo de trabajo; es el mismo para todos los binarios del curso.

1. Crea un proyecto, importa el binario desde `exercises/NN-name/bin/` y acepta el formato detectado.
2. Abre el archivo en el navegador de código y deja que se ejecute el análisis por defecto.
3. Mira el árbol de símbolos para las funciones y la ventana de cadenas definidas (Defined Strings) para las cadenas. Haz doble clic en una cadena y mira sus referencias cruzadas para encontrar el código que la usa.
4. Lee el listado (desensamblado) y el descompilador lado a lado. El descompilador es aproximado, así que compáralo con las instrucciones cuando algo parezca incorrecto.
5. Mejora el programa a medida que aprendes. Renombra funciones y variables (tecla por defecto `L`), cambia tipos (tecla por defecto `Ctrl+L` sobre una variable) y añade comentarios (tecla por defecto `;`). Muestra las referencias cruzadas de un elemento con la tecla por defecto `X`.

El objetivo de renombrar y retipar es que el descompilador vuelve a dibujar su salida con tus nombres y tipos, de modo que cada paso facilita la lectura de la siguiente función. Los atajos de teclado pueden diferir entre versiones y configuraciones de Ghidra; los menús Edit y Navigation muestran los actuales.

### Paso 6: ejecutar bajo un depurador

GDB ejecuta el programa bajo tu control. La imagen del laboratorio carga GEF, un complemento que imprime registros, pila y el código alrededor de `rip` en cada parada. La forma por lotes siguiente desactiva los complementos (`-nx`) para que la salida sea texto simple; en el laboratorio normalmente usarás la forma interactiva.

```bash
gdb -nx -q -batch -ex "break check" -ex "run hello" -ex "info registers rdi" \
    -ex "x/s \$rdi" -ex "x/s 0x402038" -ex "finish" -ex "print \$rax" build/11_target
```

```text
Breakpoint 1, 0x0000000000401184 in check ()
rdi            0x7fffd18f3e21      140736709213729
0x7fffd18f3e21:    "hello"
0x402038 <secret>:    "open-sesame"
0x00000000004011d4 in main ()
$1 = 0
```

En el punto de interrupción, `rdi` apunta a la cadena del argumento, que es "hello". El segundo `x/s` imprime la cadena con la que `check` la compara. Tras `finish`, `check` ha vuelto a `main` y `rax` es 0, el caso "denied". La dirección de `rdi` está en la pila y varía de una ejecución a otra.

| Comando | Significado |
|---|---|
| `break check` or `b *0x401184` | Se detiene en una función o en una dirección exacta |
| `run ARGS` | Inicia el programa con argumentos |
| `continue` | Reanuda |
| `stepi`, `nexti` | Ejecuta una instrucción, entrando en las llamadas o saltándolas |
| `finish` | Ejecuta hasta que la función actual retorna |
| `info registers` | Muestra registros; añade nombres para mostrar solo esos |
| `x/NFU ADDR` | Examina memoria: `N` unidades de formato `F` y tamaño `U`, como `x/4gx $rsp`, `x/s $rdi`, `x/5i $pc` |
| `watch EXPR` | Se detiene cuando cambia un valor |
| `set disassembly-flavor intel` | Muestra la sintaxis Intel (la imagen del laboratorio lo establece) |

Una segunda sesión, recorriendo el bucle de `score`:

```bash
gdb -nx -q -batch -ex "break score" -ex "run ab" -ex "x/i \$pc" -ex "stepi 3" \
    -ex "x/i \$pc" -ex "info registers eax ecx" build/11_target
```

```text
Breakpoint 1, 0x0000000000401156 in score ()
=> 0x401156 <score>:    movzbl (%rdi),%eax
0x000000000040115d in score ()
=> 0x40115d <score+7>:    mov    $0x0,%ecx
eax            0x61                97
ecx            0x403e00            4210176
```

Se carga el primer carácter de "ab": `eax` es 0x61, que es "a". El valor de `ecx` es lo que había en el registro antes de que el código lo inicializara, lo cual ocurre en 0x40115d. GDB imprime aquí sintaxis AT&T porque los complementos estaban desactivados; la configuración de la imagen del laboratorio la cambia a Intel.

### Paso 7: observar desde fuera

`strace` y `ltrace`, presentados en el módulo 10, son las herramientas de observación más rápidas:

```bash
strace -e trace=write build/11_target hello
ltrace build/11_target hello
```

```text
write(1, "score 99162322\ndenied\n", 22score 99162322
denied
) = 22
+++ exited with 1 +++
```

```text
__printf_chk(2, 0x402014, 0x5e918d2, 0x5e918d2)  = 15
strcmp("hello", "open-sesame")                   = -7
puts("denied")                                   = 7
score 99162322
denied
+++ exited (status 1) +++
```

`ltrace` revela directamente el argumento de `strcmp`, que suele ser la forma más rápida de saber con qué compara un programa su entrada. La primera llamada es `printf` compilada como `__printf_chk`; el segundo argumento es un puntero a la cadena de formato y los otros dos son el número impreso (0x5e918d2 es 99162322).

### Elegir una herramienta

| Pregunta | Empieza con |
|---|---|
| ¿Qué tipo de archivo es? | `file`, `xxd` |
| ¿Qué texto contiene? | `strings -t x` |
| ¿Qué secciones, símbolos y bibliotecas? | `readelf` |
| ¿Qué hacen las instrucciones? | `objdump`, Rizin, Ghidra |
| ¿Quién llama a esta función o usa esta cadena? | Rizin `axt`, referencias cruzadas de Ghidra |
| ¿Cuáles son los valores reales en tiempo de ejecución? | GDB |
| ¿Qué hace visto desde fuera? | `strace`, `ltrace` |

## Pruébalo

1. Sin ejecutar el programa con la palabra correcta, encuentra la palabra que espera usando solo herramientas estáticas. Usa `strings` y confirma después con `rizin` averiguando con qué compara `check`.
2. Con GDB, detente en `score` con el argumento `abc` y recorre el bucle hasta que `ecx` contenga la puntuación final. Compárala con el número que imprime el programa.
3. Convierte en papel el listado AT&T de `check` de nuevo a sintaxis Intel y compáralo después con el listado Intel.
4. Con `ltrace`, averigua a qué función llama `main` para imprimir la puntuación.

## Errores comunes

- **Leer AT&T como Intel.** El orden de los operandos está invertido. Comprueba la sintaxis antes de leer.
- **Fiarse de `strings` como imagen completa.** Las cadenas construidas o decodificadas en tiempo de ejecución no están en el archivo.
- **Creer al descompilador.** Es una reconstrucción. Compara con las instrucciones.
- **Olvidar que el análisis es una conjetura.** El análisis automático puede omitir funciones o etiquetar datos como código. Comprueba los límites de las funciones cuando un resultado parezca raro.
- **Detenerse en el lugar equivocado.** Un punto de interrupción en el nombre de una función se detiene en su primera instrucción; los argumentos ya están en registros, pero las variables locales aún no están inicializadas.

## Resumen

- Identifica un archivo con `file`, `strings` y `xxd`; lee su estructura con `readelf`.
- Desensambla con `objdump`, Rizin o Ghidra; conoce las sintaxis Intel y AT&T.
- Rizin ofrece análisis interactivo desde la línea de comandos; Ghidra añade un descompilador y permite renombrar, cambiar tipos y comentar.
- GDB muestra valores reales: pon puntos de interrupción, avanza paso a paso y examina registros y memoria.
- `strace` y `ltrace` muestran las llamadas al sistema y a bibliotecas de un programa sin leer código.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 12, los formatos ejecutables.
- Hojas de referencia: [GDB](../../cheatsheets/gdb.es.md), [Rizin](../../cheatsheets/rizin.es.md), [Ghidra](../../cheatsheets/ghidra.es.md).
