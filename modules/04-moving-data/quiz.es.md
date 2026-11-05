<!-- source: quiz.md @ sha256:2b0e947aeaa7 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 04. Mover datos y modos de direccionamiento

**1. ¿Por qué `mov [rdi], 1` no se puede ensamblar y cómo se arregla?**

<details>
<summary>Respuesta</summary>

Nada indica la anchura del almacenamiento: una constante no tiene tamaño y `[rdi]` tampoco. Añade una palabra clave de tamaño, por ejemplo `mov dword [rdi], 1`.

</details>

**2. ¿Qué significa normalmente `[rdi + rsi*4]` en código compilado?**

<details>
<summary>Respuesta</summary>

El elemento `rsi` de una matriz de valores de 4 bytes cuya dirección está en `rdi`, como `a[i]` en una matriz de `int`.

</details>

**3. ¿Cuál es la diferencia entre `lea rax, [rdi + 8]` y `mov rax, [rdi + 8]`?**

<details>
<summary>Respuesta</summary>

`lea` guarda la dirección `rdi + 8` en `rax` y no lee memoria. `mov` lee 8 bytes de esa dirección y guarda el valor.

</details>

**4. ¿Qué instrucción carga un byte con signo y lo extiende a 32 bits?**

<details>
<summary>Respuesta</summary>

`movsx eax, byte [rdi]`. `movzx` trataría el byte como si no tuviera signo.

</details>

**5. Una función compilada contiene `shl rsi, 4` seguido de `mov rax, [rdi + rsi + 8]`. ¿Qué sugiere esto sobre el código fuente en C?**

<details>
<summary>Respuesta</summary>

Que indexa una matriz de estructuras de 16 bytes (el desplazamiento de 4 bits multiplica el índice por 16) y lee un campo en el desplazamiento 8 del elemento seleccionado.

</details>

**6. ¿Por qué no existe `movzx rax, eax`?**

<details>
<summary>Respuesta</summary>

Porque una escritura normal de 32 bits, como `mov eax, eax`, ya pone a cero los 32 bits superiores del registro de 64 bits, así que extiende con ceros por sí sola.

</details>

**7. Los bytes `34 12` se leen como una palabra en x86. ¿Qué valor resulta?**

<details>
<summary>Respuesta</summary>

`0x1234`. x86 es little-endian, así que el primer byte es el byte bajo.

</details>
