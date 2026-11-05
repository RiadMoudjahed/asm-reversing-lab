<!-- source: HINTS.md @ sha256:2eebb8b3da83 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 17

Abre un nivel cada vez. Inténtalo de nuevo entre niveles.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Empieza con muestras: empaqueta ficheros cuyo contenido sea fácil de reconocer, como una sola letra repetida, y mira el volcado hexadecimal. Encuentra el valor mágico, los números que parecen cuentas o desplazamientos y los nombres. Después empaqueta un fichero más y mira qué se mueve. En el programa, busca el código que guarda los primeros bytes de la salida; guarda constantes y valores calculados a partir del número de ficheros.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Compara archivos hechos con distinto número de ficheros y con distintos contenidos. Un fichero de bytes idénticos no aparece como bytes idénticos en el archivo, así que el programa cambia los datos; y un bucle corto con una rotación calcula un valor que se guarda por fichero. El paso entre entradas de la tabla es un desplazamiento de bits o una multiplicación en el código. Para los límites, busca comparaciones con constantes antes de leer ningún fichero: protegen los mensajes de error que ves con `strings`.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

El desplazamiento de cada bloque de datos se redondea hacia arriba con una suma y una máscara; la máscara indica la alineación. La clave se calcula a partir del número de ficheros con una multiplicación y una suma, ambas con constantes en el código; evalúala para 3 ficheros. La instrucción de rotación está en el bucle que recorre los bytes de datos. Comprueba cada número empaquetando un fichero con la herramienta y leyendo el resultado en el volcado. El límite del número de ficheros es la constante de la comparación que precede al mensaje de demasiados ficheros.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Escribe primero `ark_count` y haz que las otras dos la llamen, de modo que la cabecera se valide en un solo lugar. Usa registros de 64 bits para cada desplazamiento, tamaño y suma, y extiende con ceros lo que cargues del archivo.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Calcula dónde está la entrada de la tabla para un índice: el desplazamiento de la tabla más 16 por el índice. Una comparación sin signo del índice con la cuenta rechaza los índices negativos a la vez que los grandes. Para los nombres, busca el byte cero con un límite en el final del archivo y no más allá. Para la extracción, descodifica cada byte con la clave y la posición, actualiza la suma de verificación con el byte descodificado y compara al final; haz las comprobaciones de límites y capacidad antes de tocar los datos.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Como el desplazamiento de la tabla, el de los datos y el tamaño son valores de 32 bits del archivo, súmalos en registros de 64 bits: `mov ecx, [..]` ya borra la mitad alta. Un rango de datos termina en desplazamiento más tamaño, y esa suma no debe superar `n`. El byte de clave de la posición `i` es el byte bajo de la suma de la clave e `i`, que da un `lea` a un registro de 32 bits seguido de un `xor` de byte. La suma de verificación empieza en la constante que encontraste, se rota y se le suma cada byte descodificado, y al final se combina con el tamaño. Si el verificador termina con una señal, has leído fuera del archivo; revisa primero la búsqueda de nombres y las sumas.

</details>
