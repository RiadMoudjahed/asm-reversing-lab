<!-- source: quiz.md @ sha256:314c7156006a -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 02. La CPU y sus registros

**1. `rax` contiene `0x1122334455667788`. Tras `mov al, 0xFF`, ¿qué contiene?**

<details>
<summary>Respuesta</summary>

`0x11223344556677FF`. Una escritura de 8 bits cambia solo los bits 0 a 7.

</details>

**2. `rax` contiene `0x1122334455667788`. Tras `mov eax, 1`, ¿qué contiene?**

<details>
<summary>Respuesta</summary>

`0x0000000000000001`. Una escritura de 32 bits pone a cero los 32 bits superiores.

</details>

**3. ¿Qué indicador informa de un desbordamiento de suma sin signo y cuál de uno con signo?**

<details>
<summary>Respuesta</summary>

CF (acarreo) para sin signo; OF (desbordamiento) para con signo.

</details>

**4. Tras `mov al, 127` y `add al, 1`, ¿está activo CF? ¿Y OF?**

<details>
<summary>Respuesta</summary>

CF está desactivado (127 + 1 = 128 cabe en un byte sin signo). OF está activo (128 no cabe en un byte con signo; el resultado se lee como -128).

</details>

**5. ¿Qué efecto tiene `cmp al, 5` sobre `al`?**

<details>
<summary>Respuesta</summary>

Ninguno. Calcula `al - 5`, descarta el resultado y conserva solo los indicadores.

</details>

**6. ¿Qué registro contiene la dirección de la siguiente instrucción?**

<details>
<summary>Respuesta</summary>

`rip` en código de 64 bits, `eip` en código de 32 bits.

</details>

**7. ¿Qué hace la instrucción `mov eax, eax` en código de 64 bits?**

<details>
<summary>Respuesta</summary>

Extiende `eax` con ceros hasta `rax`, porque la escritura de 32 bits pone a cero los 32 bits superiores. El valor visible en `eax` no cambia.

</details>
