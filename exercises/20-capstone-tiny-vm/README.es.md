<!-- source: README.md @ sha256:0c07042e0978 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 20. Proyecto final: Tiny VM

Nivel 5. Tiempo: unas 8 horas. Arquitectura: x86-64 Linux. Módulos: [13. Reconocer estructuras de código](../../modules/13-code-structures/README.es.md), [15. Recuperar datos](../../modules/15-recovering-data/README.es.md), [21. Máquinas virtuales y bytecode](../../modules/21-virtual-machines-bytecode/README.es.md)

## Escenario

Una vieja herramienta de mantenimiento ejecuta pequeños programas en una máquina virtual que nadie documentó. El fabricante ya no existe y el código fuente se perdió. Lo que queda es el intérprete sin símbolos `tinyvm` y un programa de ejemplo, `sample.tvm`. Tu trabajo es documentar la máquina lo bastante bien como para trabajar con ella: anotar su conjunto de instrucciones, construir en ensamblador las piezas que faltan de un intérprete de reemplazo y escribir para ella un programa nuevo con un ensamblador dirigido por tus notas. Si tus notas son erróneas en algún punto, algo de esta cadena se rompe.

## Objetivos

- Aplicar ingeniería inversa a un intérprete de bytecode: encontrar el estado, el despacho, la tabla de códigos de operación y el comportamiento de cada instrucción, incluidos los indicadores, las máscaras y los casos de error.
- Escribir una especificación completa del conjunto de instrucciones en una forma que pueda usar una herramienta.
- Implementar en NASM ocho manejadores de instrucciones que se comporten exactamente como los originales, sobre una estructura de estado que lees del binario.
- Escribir un programa de bytecode para la máquina que produzca una salida dada dentro de un límite de pasos.

## Parte 1: Reverse

Los archivos son `bin/tinyvm` y `bin/sample.tvm`. Ejecuta el ejemplo para ver qué hace la máquina:

```bash
bin/tinyvm bin/sample.tvm
```

La máquina recibe un argumento, un archivo de programa. El archivo tiene una cabecera de 8 bytes y después el código y los datos: los cuatro bytes `TVM1`, la longitud del código como número de 16 bits, la longitud de los datos como número de 16 bits (ambos con el byte bajo primero), el código y los datos. Los datos se copian al principio de los 256 bytes de memoria de la máquina antes de ejecutar el programa. El número de pasos que la máquina imprime por la salida de error estándar es el número de instrucciones ejecutadas.

La máquina tiene veinte instrucciones. Sus nombres y formas de operandos son fijos, porque tus herramientas los necesitan:

| Nombre | Operandos |
|---|---|
| `movi` | un registro y un número de 0 a 65535 |
| `mov`, `add`, `sub`, `mul`, `xor`, `and`, `cmp` | dos registros |
| `shl`, `shr` | un registro y un contador de 0 a 15 |
| `jmp`, `jz`, `jnz`, `call` | un desplazamiento del código |
| `ret`, `halt` | ninguno |
| `ldb` | un registro y un registro usado como dirección: `ldb r1, [r2]` |
| `stb` | un registro usado como dirección y un registro: `stb [r1], r2` |
| `out`, `outc` | un registro |

Todo lo demás tiene que salir del binario: qué byte de código de operación es cada instrucción, cuántos bytes ocupa cada una, qué nibble contiene el primer registro, el orden de bytes de los números de 16 bits y qué hace cada instrucción con los registros, los indicadores, la memoria y la pila de llamadas.

Recupera lo siguiente:

1. Rellena `starter/isa.json`, un archivo que describe el conjunto de instrucciones. Para cada instrucción da el byte de código de operación (como cadena, por ejemplo `"0x2b"`) y la longitud total en bytes. Da `register_a`, el nibble del byte posterior al código de operación que contiene el primer operando de registro (`"high"` o `"low"`), e `immediate`, el orden de bytes de los números de 16 bits (`"little"` o `"big"`).
2. Tres números para el flag: `N`, el número de códigos de operación válidos; `C`, el byte de código de operación de `call` en decimal; e `I`, el número de instrucciones de `bin/sample.tvm`.

```text
ARL{tinyvm_<N>_<C>_<I>}
```

Los valores 12, 100 y 30 darían, por ejemplo, `ARL{tinyvm_12_100_30}`.

Escribe un desensamblador para la máquina para encontrar `I` y comprobar tus notas; el método es el del módulo 21, y el ejemplo es el programa con el que probarlo. El análisis resuelto de `solutions/20/` contiene un desensamblador modelo con el que compararte después.

## Parte 2: Assemble

La parte 2 tiene dos piezas. Ambas dependen de lo que encontraste en la parte 1.

### Los manejadores

Un intérprete de reemplazo llama a una función por cada instrucción. Los otros doce manejadores están escritos; tú escribes estos ocho, en `starter/solution.asm`:

```c-fragment
void h_mul(struct vm *m, const unsigned char *ins);
void h_shl(struct vm *m, const unsigned char *ins);
void h_shr(struct vm *m, const unsigned char *ins);
void h_cmp(struct vm *m, const unsigned char *ins);
void h_call(struct vm *m, const unsigned char *ins);
void h_ret(struct vm *m, const unsigned char *ins);
void h_ldb(struct vm *m, const unsigned char *ins);
void h_stb(struct vm *m, const unsigned char *ins);
```

`m` apunta al estado de la máquina, descrito en `starter/vm.h`, que tiene la disposición de la estructura del binario: ocho registros de 32 bits, el contador de programa, el indicador de cero, el indicador de acarreo, un código de fallo, un indicador de detenido, la profundidad de la pila de llamadas, la pila de llamadas de dieciséis direcciones de retorno de 16 bits y 256 bytes de memoria. `ins` apunta a los bytes de la instrucción, empezando por el byte de código de operación, y tiene exactamente la longitud de esa instrucción. Cuando se llama a un manejador, el contador de programa ya se ha movido más allá de la instrucción.

Cada manejador debe dejar el estado exactamente como lo deja la instrucción en `tinyvm`, incluidos los indicadores, qué instrucciones dejan un indicador intacto, el efecto de los números de registro superiores a 7 y lo que ocurre en los extremos. Se dan dos casos: `call` con la pila de llamadas llena fija el código de fallo en 3 y el indicador de detenido en 1 y no cambia nada más, y `ret` con la pila de llamadas vacía fija el código de fallo en 2 y el indicador de detenido en 1 y no cambia nada más. El resto del comportamiento te toca encontrarlo a ti.

Las pruebas construyen muchos estados, de corrientes a extremos, ejecutan un manejador sobre cada uno y comparan todo el estado resultante con el del original.

### El programa

Escribe en `starter/program.tvs` un programa de bytecode que guarde los doce primeros números de Fibonacci (1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144) en la memoria de la máquina como bytes, uno en cada dirección de 0 a 11, y que después los imprima con `out` del último al primero, uno por línea, de modo que la salida sea:

```text
144
89
55
34
21
13
8
5
3
2
1
1
```

El programa debe usar `call` y `ret` (la impresión es una subrutina), `stb` (para guardar los números) y `ldb` (para leerlos de nuevo), y debe terminar con `halt` en no más de 400 pasos. Ensámblalo con el ensamblador del ejercicio, que se guía por tu `isa.json`, y ejecútalo en la máquina original:

```bash
python3 starter/tvasm.py --isa starter/isa.json starter/program.tvs starter/program.tvm
bin/tinyvm starter/program.tvm
```

El formato fuente del ensamblador se describe al principio de `starter/tvasm.py`. Si tus notas son erróneas, el programa ensamblado no se ejecutará correctamente en la máquina original.

## Reglas

- No leas ningún byte fuera del estado o de la instrucción en un manejador y no escribas fuera del estado. Las pruebas colocan ambos justo antes de una página no mapeada, y un acceso más allá del final termina la ejecución de las pruebas con una señal.
- Conserva los registros que debe preservar el llamado: `rbx`, `rbp` y de `r12` a `r15`.
- Mantén la pila alineada en cada `call`, si haces alguno.
- Los archivos que se comprueban son `isa.json`, `solution.asm` y `program.tvs` de `starter/`. No cambies `tvasm.py`.

## Comprueba tu trabajo

```bash
make check EX=20 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la parte 1 y de la parte 2 por separado. La parte 1 se supera cuando el flag es correcto y `starter/isa.json` es correcto; la salida nombra cada instrucción cuya entrada es errónea. La parte 2 se supera cuando pasan todas las pruebas de los manejadores y el programa imprime la salida correcta dentro de los límites. Cuando falla una prueba de un manejador, el verificador enumera los campos del estado que difieren, con sus valores antes de la instrucción, el valor esperado y tu valor. Sin `--flag`, omite la parte 1.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
