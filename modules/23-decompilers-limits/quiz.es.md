<!-- source: quiz.md @ sha256:093870c388f5 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 23. Los descompiladores y sus límites

Responde primero y luego abre la respuesta.

**1. ¿Por qué un descompilador no deshace el compilador?**

<details>
<summary>Respuesta</summary>

El compilador descartó nombres, tipos, comentarios, la estructura de los bucles y la elección de las construcciones del código fuente. El descompilador solo puede construir un programa plausible con el mismo comportamiento, y tiene que conjeturar donde falta la información.

</details>

**2. Nombra las etapas por las que pasa un descompilador.**

<details>
<summary>Respuesta</summary>

Desensamblado y grafo de flujo de control, elevación (instrucciones a operaciones sobre variables, indicadores a condiciones), análisis de flujo de datos (eliminar asignaciones que nadie lee, fundir valores en expresiones, recuperar variables locales), recuperación de tipos, estructuración (reconstruir `if` y bucles, con `goto` donde falla) e impresión.

</details>

**3. ¿Qué es una arista de retroceso y qué le dice a un descompilador?**

<details>
<summary>Respuesta</summary>

Una arista de un bloque a otro bloque que lo domina, es decir, que está en todos los caminos desde la entrada hasta el primer bloque. Señala un bucle, con el destino como cabecera del bucle.

</details>

**4. ¿Qué problema tiene un descompilador cuando un bloque cambia un registro que lee la propia condición de salto del bloque?**

<details>
<summary>Respuesta</summary>

La condición se construye con los valores que tenían los registros al empezar el bloque, pero en el texto impreso las asignaciones van primero. Si se imprimen en ese orden, la condición usa los valores nuevos. El descompilador debe guardar antes la condición en una variable u ordenar las sentencias para que cada una lea valores antiguos. Un error aquí cambia el significado sin cambiar cómo se lee el texto.

</details>

**5. El código compilado lee un byte con `movsx`. Un listado muestra `p[0]` para un puntero a `unsigned char`, como un simple `int`. ¿Qué está mal y cómo lo viste?**

<details>
<summary>Respuesta</summary>

`movsx` es extensión de signo, así que el byte se lee como un número con signo (una conversión a `signed char`), y para 0x80 el valor es -128. Un simple `p[0]` de un `unsigned char` se extiende con ceros y da 128. El mnemónico del desensamblado lo muestra.

</details>

**6. ¿Por qué `x & (x - 1) == 0` no es lo mismo que `(x & (x - 1)) == 0` en C?**

<details>
<summary>Respuesta</summary>

La comparación `==` se aplica antes que el `&` a nivel de bits, así que la primera significa `x & ((x - 1) == 0)`. Quitar paréntesis al reescribir o simplificar la salida de un descompilador puede cambiar la función sin que se note.

</details>

**7. Una lectura de una función difiere del binario solo en 3 de 215 entradas de prueba. ¿Qué muestra eso sobre las pruebas?**

<details>
<summary>Respuesta</summary>

Una prueba es tan fuerte como sus entradas. La lectura errónea coincide en todas las entradas que no son potencias de dos, y el conjunto de pruebas tiene pocas. Incluye valores extremos y los casos especiales que destaca la lógica, no solo valores aleatorios.

</details>

**8. ¿Cuándo debes leer las instrucciones en lugar del código descompilado?**

<details>
<summary>Respuesta</summary>

En rutinas pequeñas donde importa cada operación, ancho e indicador (sumas de comprobación, cifrados, trucos de bits), en ensamblador escrito a mano e instrucciones sin equivalente en C, en flujo de control inusual (tablas de saltos no encontradas, una máquina virtual), en salidas con muchas sentencias `goto` o tipos cambiantes, y cuando el descompilador falla o un resultado parece erróneo.

</details>
