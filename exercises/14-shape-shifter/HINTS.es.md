<!-- source: HINTS.md @ sha256:050dd4914a1f -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 14

Abre un nivel cada vez. Inténtalo de nuevo entre niveles.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Busca la llamada a `operator new` en la función que gestiona cada variante de kind. Justo después de cada llamada, el código guarda una dirección constante en la primera palabra del nuevo objeto: esa dirección es un puntero a vtable, y cada clase tiene el suyo.

</details>

<details>
<summary>Nivel 2: dirección</summary>

El argumento de `operator new` es el tamaño del objeto. Los demás almacenamientos posteriores a la llamada son los campos, cada uno en un desplazamiento respecto al nuevo objeto. El puntero a la vtable apunta 16 bytes dentro de una tabla de `.rodata`; lee tres direcciones de 8 bytes a partir del puntero. En el bucle principal, las tres llamadas virtuales usan los desplazamientos 0, 8 y 16 a partir del puntero a la vtable. El compilador evalúa los argumentos de la llamada de impresión de derecha a izquierda, así que las llamadas aparecen en el orden 16, 8, 0 mientras el programa imprime los resultados de las ranuras 0, 1 y 2.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Escribe las cuatro tablas una al lado de otra, tres direcciones cada una, y marca las iguales. Dos clases comparten una entrada cuando es la misma dirección en la misma fila. El número de direcciones distintas es el número de entradas diferentes en toda la tabla. La clase con los objetos mayores es la del mayor argumento de `operator new`, y su último campo es el almacenamiento con el mayor desplazamiento en ese código de constructor. Recuerda que todo objeto empieza con el puntero a la vtable; el primer campo está, pues, en el desplazamiento 8.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

`gadget_init` necesita el puntero a la vtable, así que llama a `gadget_vptr(kind)`; guarda sus demás argumentos en registros que debe preservar la función llamada durante esa llamada. Después guarda los campos en los desplazamientos que encontraste en la Parte 1.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Los constructores que leíste en la Parte 1 muestran qué clases guardan un valor en el desplazamiento 12 y cuáles guardan uno en el 16: tres clases tienen un segundo campo y una clase tiene además un tercero. Para `gadget_call`, carga el puntero a la vtable de la primera palabra del objeto, suma 8 veces el número de ranura y llama a la dirección guardada allí. El objeto sigue en `rdi`; mueve primero `arg` a `esi`.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Mantén la pila alineada: a la entrada de una función `rsp` es 8 módulo 16, así que `gadget_call` debe restar 8 (o apilar un registro) antes de su `call`, y `gadget_init` debe apilar un número impar de registros. Extiende con signo el número de ranura antes de usarlo como índice. Si el verificador imprime `vptr`, la primera palabra del objeto es incorrecta; `call` significa que tu llamada no coincide con la llamada virtual de la biblioteca (ranura equivocada, escala equivocada o argumento equivocado); `canary` significa que escribiste demasiado lejos; `regs` significa que se cambió un registro que debe preservar la función llamada.

</details>
