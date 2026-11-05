<!-- source: quiz.md @ sha256:0a87bd2a93a8 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 11. La caja de herramientas del ingeniero inverso

Responde primero y luego abre la respuesta.

**1. ¿Qué herramienta te dice qué tipo de archivo tienes y cuál imprime el texto legible que contiene con sus desplazamientos?**

<details>
<summary>Respuesta</summary>

`file` identifica el tipo y `strings -t x` imprime el texto con desplazamientos en hexadecimal.

</details>

**2. En la lista de secciones, ¿qué significan los indicadores `AX` y `WA`?**

<details>
<summary>Respuesta</summary>

`AX` es asignado y ejecutable, es decir, código como `.text`. `WA` es escribible y asignado, es decir, datos como `.data` y `.bss`.

</details>

**3. Convierte `movzbl %al,%eax` y `lea 0xea9(%rip),%rsi` a sintaxis Intel.**

<details>
<summary>Respuesta</summary>

`movzx eax, al` y `lea rsi, [rip + 0xea9]`. Invierte los operandos, quita `%` y `$`, mueve el desplazamiento entre corchetes y sustituye el sufijo de tamaño por un mnemónico explícito o un prefijo de tamaño.

</details>

**4. ¿Qué lista `axt @ sym.check` de Rizin y por qué es útil?**

<details>
<summary>Respuesta</summary>

Lista las referencias cruzadas a `sym.check`, que son los lugares que la llaman o la referencian. Responde "¿quién usa esto?", la forma habitual de ir desde una cadena o función hasta el código que importa.

</details>

**5. ¿Por qué comparar la salida del descompilador con las instrucciones?**

<details>
<summary>Respuesta</summary>

El descompilador reconstruye C a partir del código máquina y puede equivocarse con los tipos, las formas de bucle y los límites de las variables. Las instrucciones son la verdad de referencia.

</details>

**6. En un punto de interrupción en la primera instrucción de una función, ¿dónde están los dos primeros argumentos enteros en código System V de 64 bits y cómo se imprime un argumento de cadena en GDB?**

<details>
<summary>Respuesta</summary>

En `rdi` y `rsi`. Un argumento de cadena se imprime con `x/s $rdi`.

</details>

**7. Un programa compara tu entrada con una cadena fija. Nombra dos formas rápidas de conocer esa cadena sin leer el código.**

<details>
<summary>Respuesta</summary>

`ltrace` muestra los argumentos de `strcmp` y `strings` lista el texto almacenado en el archivo. Ninguna funciona cuando la cadena se construye o decodifica en tiempo de ejecución.

</details>
