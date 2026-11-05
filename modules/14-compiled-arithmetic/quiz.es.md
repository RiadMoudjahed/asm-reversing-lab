<!-- source: quiz.md @ sha256:128a2b5483f9 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 14. Aritmética compilada

Responde primero y luego abre la respuesta.

**1. ¿Qué calcula `lea eax, [rdi+rdi*4]` y lee memoria?**

<details>
<summary>Respuesta</summary>

Calcula `5 * x` (`x + 4*x`) para `x` en `rdi`. No lee memoria y no cambia los indicadores; `lea` solo evalúa la expresión de dirección.

</details>

**2. ¿Qué expresión C es `lea eax, [0+rdi*8]` seguido de `sub eax, edi`?**

<details>
<summary>Respuesta</summary>

`x * 7`, calculado como `8 * x - x`.

</details>

**3. Un `x / 2` con signo se compila como `mov eax, edi / shr eax, 31 / add eax, edi / sar eax`. ¿Por qué no solo `sar`?**

<details>
<summary>Respuesta</summary>

C redondea la división hacia cero, mientras que `sar` redondea hacia menos infinito. Para valores impares negativos ambos difieren en uno. `shr eax, 31` y `add` suman 1 a las entradas negativas antes del desplazamiento para corregir el redondeo.

</details>

**4. Encuentras `imul rax, rdx` tras `mov edx, 3435973837`, y luego `shr rax, 35`. ¿Cuál es el divisor y la operación es con signo?**

<details>
<summary>Respuesta</summary>

El divisor es `2^35 / 3435973837`, es decir, unos 10; es una división entre 10. El desplazamiento es `shr` y no hay corrección de signo, así que es sin signo.

</details>

**5. ¿Qué produce el par `cmp edi, esi` y `sbb eax, eax`?**

<details>
<summary>Respuesta</summary>

Una máscara: todo unos cuando `edi` es menor que `esi` como valores sin signo (el indicador de acarreo está activado) y cero en otro caso. Es `-(a < b)` calculado sin salto.

</details>

**6. Nombra tres diferencias de instrucciones que te indiquen que un valor tiene signo y no es sin signo.**

<details>
<summary>Respuesta</summary>

Cualesquiera tres de: `jl`, `jg` y `setl` en lugar de `jb`, `ja` y `setb`; `sar` en lugar de `shr`; `movsx` en lugar de un simple `mov` a un registro de 32 bits; `cdq` con `idiv` en lugar de `xor edx, edx` con `div`; `cmovl` o `cmovge` en lugar de `cmovb` o `cmovnb`.

</details>

**7. Se compila un resto con signo `x % 8`. ¿Por qué no es un único `and eax, 7`?**

<details>
<summary>Respuesta</summary>

Para `x` negativo, el resultado en C es negativo o cero, mientras que `and` con 7 da un valor de 0 a 7. El compilador suma un sesgo antes del `and` y lo resta después, lo que da la secuencia `sar 31`, `shr 29`, `add`, `and 7`, `sub`.

</details>
