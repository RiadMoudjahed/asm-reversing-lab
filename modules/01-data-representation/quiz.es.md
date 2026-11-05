<!-- source: quiz.md @ sha256:28af4ab2262e -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 01. Representación de datos

**1. ¿Cuál es el rango de un valor de 8 bits con signo?**

<details>
<summary>Respuesta</summary>

De -128 a 127.

</details>

**2. ¿Cuál es la representación en complemento a dos de -1 en un byte?**

<details>
<summary>Respuesta</summary>

`0xFF`. Invierte `0000 0001` para obtener `1111 1110` y suma uno para obtener `1111 1111`.

</details>

**3. El byte `0x80` se extiende con signo a 32 bits. ¿Cuál es el resultado?**

<details>
<summary>Respuesta</summary>

`0xFFFFFF80`. El bit superior de `0x80` es 1, así que los nuevos bits se rellenan con 1.

</details>

**4. Un volcado hexadecimal muestra `78 56 34 12` en una dirección de x86. ¿Qué valor de 32 bits hay almacenado allí?**

<details>
<summary>Respuesta</summary>

`0x12345678`. El primer byte es el menos significativo, así que invierte los bytes.

</details>

**5. ¿Cuántos bytes usa la codificación UTF-8 de la letra `A`?**

<details>
<summary>Respuesta</summary>

Un byte, `0x41`. Los caracteres por debajo de 128 son idénticos en ASCII y en UTF-8.

</details>

**6. ¿Cuál es el número 47 en BCD empaquetado, como byte hexadecimal?**

<details>
<summary>Respuesta</summary>

`0x47`. Cada dígito decimal ocupa medio byte.

</details>

**7. Una palabra contiene `0xFFFE`. ¿Cuáles son sus valores sin signo y con signo?**

<details>
<summary>Respuesta</summary>

Sin signo 65534; con signo -2.

</details>
