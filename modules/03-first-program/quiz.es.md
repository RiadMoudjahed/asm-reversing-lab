<!-- source: quiz.md @ sha256:8c505eab92e0 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 03. Tu primer programa

**1. ¿Qué parte de `mov eax, 1` es el destino?**

<details>
<summary>Respuesta</summary>

`eax`. En la sintaxis Intel de NASM el destino va primero y el origen después.

</details>

**2. ¿Por qué `.bss` no ocupa espacio en el archivo ejecutable?**

<details>
<summary>Respuesta</summary>

Porque solo contiene un tamaño. El cargador proporciona memoria puesta a cero de ese tamaño cuando arranca el programa, así que no hace falta almacenar bytes.

</details>

**3. ¿Qué calcula `message_len equ $ - message` cuando se coloca justo después de una cadena?**

<details>
<summary>Respuesta</summary>

El número de bytes entre la etiqueta `message` y la posición actual, que es la longitud de la cadena incluidos bytes como el salto de línea.

</details>

**4. Completa los registros de 64 bits para `write(1, text, 17)`: número de llamada, descriptor, dirección, longitud.**

<details>
<summary>Respuesta</summary>

`rax` = 1, `rdi` = 1, `rsi` = dirección del texto, `rdx` = 17.

</details>

**5. ¿Qué diferencias hay entre la forma de 64 bits y la de 32 bits de llamar a `write`?**

<details>
<summary>Respuesta</summary>

64 bits: instrucción `syscall`, número de llamada 1 en `rax`, argumentos en `rdi`, `rsi`, `rdx`. 32 bits: `int 0x80`, número de llamada 4 en `eax`, argumentos en `ebx`, `ecx`, `edx`.

</details>

**6. Tras una instrucción `syscall`, ¿qué dos registros sobrescribe el procesador además de `rax`?**

<details>
<summary>Respuesta</summary>

`rcx` (recibe la dirección de la instrucción siguiente) y `r11` (recibe los indicadores guardados).

</details>

**7. Desensamblas un programa sin código fuente y ves `mov eax, 0x3c` seguido de `syscall`. ¿Qué hace?**

<details>
<summary>Respuesta</summary>

Llama a `exit` (número de llamada 60 en 64 bits = 0x3c). El valor de `edi` antes es el estado de salida.

</details>

**8. El programa se ejecuta y luego falla con un fallo de segmentación tras imprimir. ¿Cuál es la causa más probable?**

<details>
<summary>Respuesta</summary>

Al programa le falta la llamada `exit` al final, así que el procesador sigue ejecutando los bytes posteriores a la última instrucción.

</details>
