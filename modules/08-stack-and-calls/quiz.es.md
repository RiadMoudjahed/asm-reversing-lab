<!-- source: quiz.md @ sha256:d4df31272239 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 08. La pila, los procedimientos y las convenciones de llamada

**1. ¿Qué hace `call target` y qué hace `ret`?**

<details>
<summary>Respuesta</summary>

`call` mete la dirección de la instrucción siguiente y salta a `target`. `ret` saca una dirección de la pila hacia `rip`.

</details>

**2. En una función con marco estándar, ¿dónde están la primera variable local y la dirección de retorno respecto a `rbp`?**

<details>
<summary>Respuesta</summary>

Las locales están en desplazamientos negativos, por ejemplo `[rbp - 8]`. El `rbp` guardado está en `[rbp]` y la dirección de retorno en `[rbp + 8]`.

</details>

**3. ¿Qué registros llevan los cuatro primeros argumentos enteros en System V AMD64 y en Microsoft x64?**

<details>
<summary>Respuesta</summary>

System V: `rdi`, `rsi`, `rdx`, `rcx`. Microsoft x64: `rcx`, `rdx`, `r8`, `r9`.

</details>

**4. ¿Cuál es la diferencia entre cdecl y stdcall y cómo se distinguen en la función llamada?**

<details>
<summary>Respuesta</summary>

En cdecl quien llama retira los argumentos de la pila; en stdcall lo hace la función llamada. Una función stdcall termina con `ret N`, mientras que una cdecl termina con un `ret` simple.

</details>

**5. En la primera instrucción de una función System V, ¿dónde está el séptimo argumento entero?**

<details>
<summary>Respuesta</summary>

En `[rsp + 8]`. La dirección de retorno está en `[rsp]` y los argumentos de pila la siguen.

</details>

**6. ¿Por qué `factorial` guarda y restaura `rbx`?**

<details>
<summary>Respuesta</summary>

Usa `rbx` para conservar `n` a través de la llamada recursiva, y `rbx` lo guarda la función llamada, así que debe preservarse el valor original para quien llamó. El `push` también mantiene `rsp` alineado para la llamada.

</details>

**7. ¿Qué valor debe tener `rsp` en el momento de un `call` en código de 64 bits y cómo se arregla en una función que no ha metido nada en la pila?**

<details>
<summary>Respuesta</summary>

Un múltiplo de 16. Al entrar en la función, `rsp + 8` es múltiplo de 16, así que resta 8 (`sub rsp, 8`) antes de la llamada o mete un registro.

</details>

**8. ¿Qué es el espacio de sombra (shadow space)?**

<details>
<summary>Respuesta</summary>

En Microsoft x64, 32 bytes que quien llama reserva sobre la dirección de retorno en cada llamada, donde la función llamada puede guardar los cuatro argumentos de registro.

</details>

**9. ¿Por qué una pila desalineada puede pasar inadvertida en una llamada y provocar un fallo en otra?**

<details>
<summary>Respuesta</summary>

Solo algunos caminos de código usan instrucciones que exigen alineación de 16 bytes, como los almacenamientos SSE alineados. Una llamada a `printf` sin argumentos vectoriales los evita; una con un argumento de punto flotante los ejecuta y falla.

</details>
