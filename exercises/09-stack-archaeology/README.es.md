<!-- source: README.md @ sha256:5695d04dc8a5 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 09. Arqueología de la pila

Level 3. Tiempo: unas 3 horas. Arquitectura: x86-64 Linux. Módulos: [08. La pila, los procedimientos y las convenciones de llamada](../../modules/08-stack-and-calls/README.es.md), [13. Reconocer estructuras de código](../../modules/13-code-structures/README.es.md)

## Escenario

Una compilación de una herramienta interna, `stackdig`, se bloquea con un desbordamiento de pila ante entradas profundas, y se perdió el código fuente de su función recursiva `chain`. Solo queda el binario optimizado. Para decidir si la función puede reescribirse de forma más económica, alguien debe medir exactamente qué coloca en la pila un nivel de recursión. Después, un reemplazo escrito a mano debe comportarse igual y sin usar más pila por nivel.

## Objetivos

- Leer el prólogo y el epílogo de una función optimizada que no tiene puntero de marco.
- Reconstruir un marco de pila: registros guardados, datos locales, espacio sin usar y la dirección de retorno.
- Escribir una función recursiva que mantenga la pila alineada a 16 bytes en cada llamada y preserve los registros que debe conservar la función llamada.

## Parte 1: Reverse

El binario es `bin/stackdig`. Ejecuta un cálculo e imprime `stackdig: ok`. La función a estudiar es el símbolo global `chain`. Se compiló con optimización y sin puntero de marco, y llama a otra función, `note`, una vez por nivel, con un puntero a una tabla de tres valores de 8 bytes que guarda en su pila.

```bash
bin/stackdig
objdump -d -M intel bin/stackdig
```

Encuentra estos cinco números, todos en decimal:

1. `P`: cuántos registros que debe conservar la función llamada apila `chain` con push.
2. `S`: cuántos bytes resta el prólogo de `rsp`.
3. `L`: cuántos bytes de pila ocupa un nivel de recursión, contando la dirección de retorno, los registros guardados y el espacio restado.
4. `O`: la distancia en bytes desde la primera entrada de la tabla, tal como la ve `note`, hasta la dirección de retorno guardada de ese mismo nivel.
5. `U`: cuántos bytes del espacio restado no usa la tabla.

```text
ARL{stack_<P>_<S>_<L>_<O>_<U>}
```

Por ejemplo, los valores 1, 8, 24, 16 y 0 darían `ARL{stack_1_8_24_16_0}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para escribir `chain_asm`, una versión de `chain` escrita a mano en la convención de llamada System V:

```c
long chain_asm(long depth, long seed);

/* Provided by the tests. Clobbers every caller-saved register. */
void note(long depth, const long *entries);
```

`chain_asm` debe hacer lo que hace `chain`: construir la misma tabla, llamar a `note(depth, table)` una vez por nivel, llamarse recursivamente y devolver el mismo valor. Lee las fórmulas en el código de `chain`. Las entradas de la tabla y la aritmética forman parte de lo que la Parte 1 te permite ver.

## Reglas

- `rsp` debe ser múltiplo de 16 en cada `call note`. La prueba lo comprueba dentro de `note`.
- `rbx`, `rbp` y de `r12` a `r15` deben tener sus valores originales cuando `chain_asm` retorna.
- `note` destruye `rax`, `rcx`, `rdx`, `rsi`, `rdi` y de `r8` a `r11`. Guarda en otro lugar todo lo que necesites después de la llamada.
- Un nivel de tu función no puede usar más pila que un nivel de `chain`. La prueba compara el puntero de pila en llamadas consecutivas a `note` con el número `L` de la Parte 1.
- La recursión es la única forma de descender; las pruebas llegan hasta 2000 niveles.
- Los resultados se desbordan a 64 bits.

## Comprueba tu trabajo

```bash
make check EX=09 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y la Parte 2 por separado. Sin `--flag`, solo comprueba la Parte 2. En la Parte 2 cada prueba imprime el resultado seguido de `ok` o de una lista de lo que salió mal: `result`, `align`, `table`, `calls`, `regs` o `stack`.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
