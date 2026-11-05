<!-- source: HINTS.md @ sha256:cbf7ef633e3e -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 13

Abre un nivel cada vez. Inténtalo de nuevo entre niveles.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Busca instrucciones `imul` que multipliquen por una constante grande, cada una seguida de un desplazamiento a la derecha. Hay cuatro de esos pares en `mix`, uno por cada división. Anota cada constante y su desplazamiento en una tabla.

</details>

<details>
<summary>Nivel 2: dirección</summary>

El divisor es 2 elevado al desplazamiento total, dividido por la constante, redondeado al entero más cercano. El desplazamiento total cuenta todos los desplazamientos a la derecha que siguen a la multiplicación, incluido un `shr` de 32 seguido de otro menor. Ejecuta `14_magic.py` del módulo 14 para comprobar tu lectura. Para `K` y `M`, mira las instrucciones `lea` que siguen a cada cociente: `lea r, [x+x*4]` es una multiplicación por 5, `lea r, [x+x*2]` por 3, `lea r, [x+x*8]` por 9.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Una de las secuencias con signo carga una constante que el desensamblador muestra con muchos dígitos `f`, como `0xffffffff88888889`. Es una constante de 32 bits con el bit alto activado que se extendió con signo; el verdadero número mágico son los 32 bits bajos leídos como positivo, y el `add` adicional de `x` tras el `shr 32` compensa el signo. El resto sin signo se calcula como el valor menos el cociente por el divisor, y el cociente por el divisor es un `lea`; el factor `M` multiplica el resultado de esa resta. Para comprobar una fórmula candidata, ejecuta `bin/magic` con algunas entradas y compara.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Escribe la fórmula en función de los cuatro divisores que encontraste. Después escribe un bloque para cada término. Cada bloque convierte una división por una constante en `x * M` seguido de un desplazamiento a la derecha, con `M` y el desplazamiento tomados del binario.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Usa registros de 64 bits para los productos: `mov eax, edi` extiende `x` con ceros para los términos sin signo, y `movsxd rax, edi` lo extiende con signo para los términos con signo. Una multiplicación por una constante de 32 bits con el bit alto activado necesita la constante cargada como valor positivo de 64 bits, por ejemplo con `mov edx, constant`. El resto es `x - q * d`. En los términos con signo, desplaza aritméticamente (`sar`) y suma 1 al cociente cuando `x` es negativo; el valor `x >> 31` es -1 para `x` negativo, así que réstalo.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Para el cociente sin signo basta `(x * M) >> k` con la constante y el desplazamiento del binario. El cociente y el resto con signo necesitan la corrección para entradas negativas: si el cociente sale una unidad menor para `x` negativo, olvidaste restar `x >> 31`. Si solo fallan las entradas desde 2^31, los términos sin signo se trataron con extensión de signo. Si el verificador dice que se encontró la instrucción `div`, sustitúyela por la multiplicación; puedes combinar todos los términos con `lea` al final.

</details>
