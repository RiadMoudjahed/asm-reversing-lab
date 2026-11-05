<!-- source: README.md @ sha256:d6d12a704b76 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 08. Tarjeta de presentación

Level 3. Tiempo: unas 3 horas. Arquitectura: x86 y x86-64 Linux. Módulos: [08. La pila, los procedimientos y las convenciones de llamada](../../modules/08-stack-and-calls/README.es.md)

## Escenario

Un proveedor entregó dos pequeños programas de prueba, `callcard32` y `callcard64`, junto con un archivo de cabecera que se perdió. La cabecera indicaba la convención de llamada de cada función, y el equipo de integración debe ahora llamar a estas funciones desde otro código. Sin la cabecera, la convención hay que leerla en el código máquina. Después hay que escribir un reemplazo para tres de las funciones, con las mismas convenciones, para que los llamadores existentes sigan funcionando.

## Objetivos

- Distinguir cdecl, stdcall y fastcall en código de 32 bits por la forma de leer los argumentos y de limpiar la pila tras la llamada.
- Distinguir System V AMD64 de Microsoft x64 en código de 64 bits por los registros de argumentos, el espacio de sombra (shadow space) y el ajuste de pila del llamador.
- Escribir funciones cuyo efecto sobre la pila coincida con su convención, de modo que un llamador escrito para esa convención quede equilibrado.

## Parte 1: Reverse

Los binarios son `bin/callcard32` (32 bits) y `bin/callcard64` (64 bits). Cada uno ejecuta una autoprueba de sus funciones e imprime `callcard32: ok` o `callcard64: ok`. Las funciones son los símbolos globales `f1` a `f4` en `callcard32` y `g1` y `g2` en `callcard64`.

```bash
bin/callcard32
bin/callcard64
objdump -d -M intel bin/callcard32
objdump -d -M intel bin/callcard64
```

Cada función usa una de cinco convenciones. Asigna una letra a cada función:

| Letra | Convención |
|---|---|
| `c` | cdecl: argumentos en la pila, el llamador los retira |
| `s` | stdcall: argumentos en la pila, la función llamada los retira |
| `f` | fastcall (32 bits): los dos primeros argumentos en `ecx` y `edx`, el resto en la pila, la función llamada retira esos |
| `v` | System V AMD64: primeros argumentos en `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9` |
| `m` | Microsoft x64: primeros argumentos en `rcx`, `rdx`, `r8`, `r9`, con 32 bytes de espacio de sombra |

Forma la bandera con las seis letras, en el orden `f1`, `f2`, `f3`, `f4`, `g1`, `g2`:

```text
ARL{calling_<f1><f2><f3><f4><g1><g2>}
```

Por ejemplo, si las letras fueran `c`, `c`, `s`, `f`, `v` y `m`, la bandera sería `ARL{calling_ccsfvm}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para implementar tres funciones en código de 32 bits. Cada una debe calcular el mismo resultado que la función de la misma posición en la Parte 1 y usar la misma convención de llamada que esa función:

- `cc_f1` corresponde a `f1`.
- `cc_f2` corresponde a `f2`.
- `cc_f3` corresponde a `f3`.

Las tres reciben tres argumentos `int` y devuelven un `int`. Las pruebas las llaman mediante prototipos que declaran cada convención:

```c
int __attribute__((stdcall))  cc_f1(int a, int b, int c);
int                           cc_f2(int a, int b, int c);   /* cdecl */
int __attribute__((fastcall)) cc_f3(int a, int b, int c);
```

Lee la fórmula de cada función en su código de `callcard32` y fíjate en qué argumento está dónde.

## Reglas

- El arnés compara el puntero de pila antes y después de cada llamada. Una función que retira muy pocos o demasiados bytes falla, aunque su resultado sea correcto.
- Conserva `ebx`, `esi`, `edi` y `ebp`.
- No uses `call`.
- Los resultados se desbordan a 32 bits; las pruebas incluyen valores muy grandes y muy pequeños.

## Comprueba tu trabajo

```bash
make check EX=08 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y la Parte 2 por separado. Sin `--flag`, solo comprueba la Parte 2.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
