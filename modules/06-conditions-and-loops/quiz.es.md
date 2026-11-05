<!-- source: quiz.md @ sha256:12817111b472 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 06. Condiciones, bifurcaciones y bucles

**1. ¿Cuál es la diferencia entre `cmp rdi, rsi` y `sub rdi, rsi`?**

<details>
<summary>Respuesta</summary>

Ambas calculan `rdi - rsi` y fijan los mismos indicadores. `cmp` descarta el resultado; `sub` lo guarda en `rdi`.

</details>

**2. Tras `cmp rax, rbx`, ¿qué salto se toma cuando `rax` es mayor que `rbx` como números con signo y cuál como números sin signo?**

<details>
<summary>Respuesta</summary>

`jg` con signo y `ja` sin signo.

</details>

**3. ¿Por qué `max_signed(-1, 1)` devuelve 1 mientras que `max_unsigned(-1, 1)` devuelve 18446744073709551615?**

<details>
<summary>Respuesta</summary>

Los bits de -1 son todos unos. Con signo es menor que 1; sin signo es el mayor valor posible. `cmovl` lee los indicadores con signo y `cmovb` los de sin signo.

</details>

**4. ¿Cuál tiene garantizado ejecutar el cuerpo al menos una vez, un bucle while o un bucle do-while?**

<details>
<summary>Respuesta</summary>

Un bucle do-while, porque su comprobación va después del cuerpo.

</details>

**5. ¿Por qué `xor eax, eax` debe ir antes de `cmp` y no entre `cmp` y `setg al`?**

<details>
<summary>Respuesta</summary>

`xor` cambia los indicadores. Colocada después de `cmp`, sustituye el resultado de la comparación antes de que `setg` lo lea.

</details>

**6. ¿Qué hace `test rdi, rcx` cuando `rcx` es `rdi - 1`?**

<details>
<summary>Respuesta</summary>

Calcula `rdi AND (rdi - 1)` solo para sus indicadores. El resultado es cero cuando `rdi` tiene como máximo un bit activo, así que `setz` después indica una potencia de dos (una vez excluido el cero por separado).

</details>

**7. Una función compilada termina un bucle con `cmp rax, rsi` seguida de `jne` hacia el principio del cuerpo. ¿Qué indica esto sobre cómo se optimizó?**

<details>
<summary>Respuesta</summary>

El compilador movió la comprobación del bucle al final (inversión del bucle) y reescribió el contador, con una guarda antes del bucle para el caso en que no debe ejecutarse en absoluto.

</details>

**8. ¿Qué ocurre si `loop` se ejecuta con `rcx` igual a 0?**

<details>
<summary>Respuesta</summary>

`loop` resta 1 primero, así que `rcx` da la vuelta a un valor enorme distinto de cero y el bucle se ejecuta 2^64 veces.

</details>
