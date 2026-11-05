<!-- source: quiz.md @ sha256:d1015bcb390d -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 07. Cadenas e instrucciones de cadena

**1. ¿Qué registros usa `rep movsb` y qué contiene cada uno?**

<details>
<summary>Respuesta</summary>

`rsi` contiene la dirección de origen, `rdi` la de destino y `rcx` el número de bytes que copiar. Ambos punteros avanzan un byte por pasada.

</details>

**2. ¿Qué controla el indicador de dirección y cuál debe ser su valor cuando vuelve una función?**

<details>
<summary>Respuesta</summary>

Decide si las instrucciones de cadena mueven sus punteros hacia arriba (DF = 0, `cld`) o hacia abajo (DF = 1, `std`). La ABI System V exige que esté a cero cuando vuelve una función.

</details>

**3. ¿Cuál es la diferencia entre `repe` y `repne`?**

<details>
<summary>Respuesta</summary>

`repe` repite mientras la última comparación fue igual (ZF = 1) y `rcx` es mayor que cero. `repne` repite mientras fue distinta (ZF = 0) y `rcx` es mayor que cero.

</details>

**4. Si `repne scasb` termina con `rcx` igual a 0 y ZF igual a 0, ¿qué significa para una implementación de `memchr`?**

<details>
<summary>Respuesta</summary>

Se compararon todos los bytes y ninguno coincidió, así que el byte no se encontró y la función devuelve NULL.

</details>

**5. ¿Por qué `my_memmove` copia hacia atrás cuando el destino está por encima del origen y se solapa con él?**

<details>
<summary>Respuesta</summary>

Una copia hacia delante sobrescribiría bytes del origen antes de leerlos. Empezar por el último byte lee cada byte antes de que pueda ser sobrescrito.

</details>

**6. ¿Qué hace `mov ecx, 0x40` seguido de `rep stos DWORD` con `eax` igual a 0?**

<details>
<summary>Respuesta</summary>

Pone a cero 0x40 por 4 = 256 bytes a partir de `[rdi]`, que es `memset(p, 0, 256)`.

</details>

**7. ¿Qué ocurre con los indicadores cuando `repe cmpsb` se ejecuta con `rcx` igual a 0?**

<details>
<summary>Respuesta</summary>

No se ejecuta nada y los indicadores conservan sus valores anteriores. Por eso `my_memcmp` fija ZF con `xor eax, eax` antes de la instrucción, de modo que una longitud cero cuente como igual.

</details>

**8. En `memcmp(a, b, n)`, `a` llega en `rdi` y `b` en `rsi`. ¿Qué bytes restas para producir el resultado tras una diferencia, y en qué orden?**

<details>
<summary>Respuesta</summary>

El byte de `a` menos el byte de `b`, es decir `[rdi - 1]` menos `[rsi - 1]` (ambos punteros ya han pasado de los bytes distintos). El orden contrario invierte el signo del resultado.

</details>
