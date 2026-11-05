<!-- source: README.md @ sha256:c29540243c82 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 05. Contador de entradas

Level 2. Tiempo: aproximadamente 2 horas. Arquitectura: x86 (32 bits) Linux. Módulos: [01. Representación de datos](../../modules/01-data-representation/README.es.md), [03. Tu primer programa](../../modules/03-first-program/README.es.md), [05. Aritmética y lógica](../../modules/05-arithmetic-and-logic/README.es.md), [06. Condiciones, bifurcaciones y bucles](../../modules/06-conditions-and-loops/README.es.md)

## Escenario

Una taquilla suma las ventas de dos ventanillas con una herramienta antigua de 32 bits llamada `ticket_counter`. Los números se escriben como texto y pueden ser más largos de lo que cabe en un registro de 32 bits: un número de 12 dígitos es mayor que 2^32. La taquilla se pasa a un sistema nuevo y necesita que el comportamiento de la herramienta quede documentado con exactitud, incluido lo que hace con las entradas incorrectas, antes de poder sustituirla.

## Objetivos

- Deducir las reglas de un programa a partir de su desensamblado y de experimentos.
- Leer la comprobación de un dígito ASCII y el estado de salida en código de 32 bits.
- Sumar números dados como texto, dígito a dígito, con acarreo, sin convertirlos a enteros.

## Parte 1: Reverse

El binario es `bin/ticket_counter`. Lee dos líneas de la entrada estándar e imprime un resultado.

```bash
printf '12\n34\n' | bin/ticket_counter ; echo "exit status: $?"
objdump -d -M intel bin/ticket_counter
strace bin/ticket_counter
```

Experimenta con las entradas y lee el código para responder estas preguntas:

1. ¿Cuántos dígitos tiene la entrada más larga que acepta el programa?
2. ¿Qué estado de salida devuelve cuando una entrada no es válida?
3. ¿Qué estado de salida devuelve cuando la suma es demasiado grande para imprimirse?
4. ¿Qué imprime para las entradas `000` y `0000`? (Comprueba tu respuesta ejecutándolo.)

Construye la bandera con tus respuestas:

```text
ARL{ticket_<longest input>_<status for invalid input>_<status for a sum that is too large>}
```

Por ejemplo, si la entrada más larga fuera de 8 dígitos, el primer estado 3 y el segundo 4, la bandera sería `ARL{ticket_8_3_4}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para escribir un programa completo de 32 bits que se comporte exactamente como `bin/ticket_counter`: el mismo texto en la salida estándar y el mismo estado de salida para cualquier entrada. Compílalo y pruébalo desde la carpeta `starter`:

```bash
make run
```

El verificador ejecuta tu programa con 80 entradas: sumas normales, acarreos que recorren todos los dígitos, ceros a la izquierda, entradas justo en cada límite y justo después, y entradas mal formadas de varios tipos.

## Reglas

- Escribe código de 32 bits que use la interfaz `int 0x80` y ninguna biblioteca de C.
- No conviertas los números a un único registro. Un número de 12 dígitos no cabe en 32 bits, así que suma dígito a dígito.
- Tu programa no debe ejecutar `ticket_counter`.

## Comprueba tu trabajo

```bash
make check EX=05 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y de la Parte 2 por separado. Sin `--flag`, comprueba solo la Parte 2.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
