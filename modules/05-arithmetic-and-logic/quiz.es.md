<!-- source: quiz.md @ sha256:016737cf52bb -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 05. Aritmética y lógica

**1. Tras `add rax, 1` con `rax = 0x7FFFFFFFFFFFFFFF`, ¿cuál de CF y OF se activa?**

<details>
<summary>Respuesta</summary>

Solo OF. Como suma sin signo no sale ningún acarreo del registro, pero como suma con signo el resultado pasa del mayor valor positivo al más negativo.

</details>

**2. ¿Por qué `cqo` debe ir antes de `idiv`?**

<details>
<summary>Respuesta</summary>

`idiv` divide el valor de 128 bits `rdx:rax`. `cqo` rellena `rdx` con la extensión de signo de `rax`; sin ella, `rdx` contiene un valor sin relación y el resultado es incorrecto.

</details>

**3. ¿Qué escribe `mul rsi` además de `rax`?**

<details>
<summary>Respuesta</summary>

`rdx`. El producto de 128 bits se guarda como `rdx:rax`, así que cualquier valor que guardaras en `rdx` se sobrescribe.

</details>

**4. `-17 / 5` se calcula con `idiv`. ¿Cuáles son el cociente y el resto?**

<details>
<summary>Respuesta</summary>

Cociente -3 y resto -2. `idiv` redondea hacia cero y el resto toma el signo del dividendo.

</details>

**5. ¿Cuál es la diferencia entre `sar` y `shr`?**

<details>
<summary>Respuesta</summary>

`shr` introduce ceros (desplazamiento lógico, para valores sin signo). `sar` copia el bit de signo (desplazamiento aritmético, para valores con signo), de modo que un número negativo sigue siendo negativo.

</details>

**6. Una función compilada contiene `shr rax, 3`. ¿Qué hizo con más probabilidad el código fuente en C?**

<details>
<summary>Respuesta</summary>

Dividió un valor sin signo entre 8.

</details>

**7. ¿Por qué `mov [rdx], rdi` puede ir entre `add rdi, rsi` y `seto al`?**

<details>
<summary>Respuesta</summary>

`mov` no cambia ningún indicador, así que OF sigue describiendo el `add` cuando `seto` lo lee.

</details>

**8. ¿Por qué `daa` no se puede ensamblar en código de 64 bits?**

<details>
<summary>Respuesta</summary>

Las instrucciones de ajuste decimal no están disponibles en modo de 64 bits. NASM informa `error: instruction not supported in 64-bit mode`.

</details>
