<!-- source: quiz.md @ sha256:a31f786f90fe -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 17. Descifrar formatos de archivo

Responde primero y luego abre la respuesta.

**1. Un archivo empieza por los bytes `50 4b 03 04` y los mismos cuatro bytes encabezan todas tus muestras. ¿Qué es esto y qué te permite hacer?**

<details>
<summary>Respuesta</summary>

Un valor mágico, un identificador fijo en el desplazamiento 0. Permite a un lector reconocer el formato y rechazar otros archivos, y a ti te permite buscar en un programa el código que lo comprueba, como una comparación con esta constante o con una cadena de esos bytes.

</details>

**2. Cambias un parámetro al crear un archivo y el volcado difiere en dos sitios: cuatro bytes cerca del principio y los cuatro últimos bytes. ¿Qué concluyes?**

<details>
<summary>Respuesta</summary>

Los cuatro bytes cercanos al principio contienen el parámetro que cambiaste, y los cuatro últimos dependen del resto del archivo, así que probablemente son una suma de verificación o un hash. El experimento también muestra que la suma de verificación cubre al menos la región que contiene el parámetro.

</details>

**3. El código ensambla un valor a partir de cuatro bytes con cargas `movzx`, `shl` de 8, 16 y 24 y `or`, y el byte de la dirección más baja no tiene desplazamiento. ¿Cuál es el orden de bytes?**

<details>
<summary>Respuesta</summary>

Little endian: el byte de la dirección más baja es el menos significativo. Un lector big endian desplazaría el primer byte 24 bits.

</details>

**4. Encuentras un bucle que rellena 256 entradas de una tabla con ocho vueltas de `shr 1` y un xor condicional con 0xedb88320. ¿Qué es y qué tres detalles debes establecer aún?**

<details>
<summary>Respuesta</summary>

Construye la tabla de un CRC-32 (el polinomio reflejado 0xedb88320). Faltan el valor inicial de la suma acumulada, si el resultado se complementa al final y el rango de bytes que cubre la suma de verificación. Cambian el resultado, así que cada uno debe tomarse del código y confirmarse después con una muestra.

</details>

**5. ¿Cómo encuentras el código que lee un archivo cuando el programa no tiene símbolos?**

<details>
<summary>Respuesta</summary>

Parte de lo que sobrevive a la eliminación de símbolos: los mensajes de error y otras cadenas (sigue sus referencias cruzadas hasta las condiciones que saltan a ellas), las funciones importadas como `fopen`, `fread` y `memcmp`, y constantes como etiquetas, valores mágicos y polinomios. Los tamaños que se pasan a `fread` dan los tamaños del siguiente campo o estructura.

</details>

**6. ¿Por qué una especificación no está terminada hasta que escribes un analizador a partir de ella y lo comparas con el original?**

<details>
<summary>Respuesta</summary>

La especificación es un conjunto de afirmaciones, y escribir código a partir de ella obliga a precisar cada una. Comparar el analizador con el original en archivos válidos y dañados muestra las afirmaciones erróneas o ausentes, como el orden de las comprobaciones, los límites, las etiquetas ignoradas y los bytes posteriores al final de los datos.

</details>

**7. Un lector acepta un archivo con bytes adicionales tras el último bloque. ¿Es un error de tu analizador que también lo acepte?**

<details>
<summary>Respuesta</summary>

No. La especificación debe registrar que el original no mira los bytes posteriores al bloque final, y un analizador equivalente se comporta igual. Si más adelante un lector más estricto usa el formato, es otra cuestión, y la especificación debe mencionarlo como observación.

</details>

**8. ¿Por qué editas primero el campo y al final la suma de verificación cuando construyes un archivo de prueba?**

<details>
<summary>Respuesta</summary>

La suma de verificación depende del campo. Si corriges primero la suma de verificación y luego cambias el campo, el lector se detiene en la comprobación de la suma y nunca llega al código que trata el campo que querías probar.

</details>
