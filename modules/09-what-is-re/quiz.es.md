<!-- source: quiz.md @ sha256:62ae4799ebaf -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 09. Qué es la ingeniería inversa

Responde primero y luego abre la respuesta.

**1. ¿Cuál es el objetivo de la ingeniería inversa y en qué se diferencia de reescribir un programa?**

<details>
<summary>Respuesta</summary>

El objetivo es comprender: recuperar cómo funciona un programa a partir del propio programa. Reescribir produce código nuevo; puede derivarse de la comprensión, pero no es la meta.

</details>

**2. Nombra tres usos legítimos de la ingeniería inversa.**

<details>
<summary>Respuesta</summary>

Cualesquiera tres de: interoperabilidad, auditoría de seguridad, análisis de malware, recuperación de conocimiento perdido, depuración sin código fuente, aprender cómo un compilador traduce el código.

</details>

**3. ¿Qué tres cosas debes comprobar antes de analizar un programa que no es tuyo?**

<details>
<summary>Respuesta</summary>

Quién es su propietario, qué dice su licencia o contrato sobre el análisis y qué dice la ley de tu país.

</details>

**4. Indica una ventaja y una desventaja del análisis estático.**

<details>
<summary>Respuesta</summary>

Ventaja: es seguro y ve todo el código, incluso las rutas que nunca se ejecutan. Desventaja: no ve los valores que se calculan en tiempo de ejecución, y el código ofuscado es difícil de leer.

</details>

**5. ¿Por qué el análisis dinámico nunca se hace en la máquina que contiene tus archivos personales?**

<details>
<summary>Respuesta</summary>

Porque ejecuta código que todavía no entiendes del todo. Usa un entorno desechable, como el contenedor del laboratorio de este curso o, para muestras reales, una máquina virtual sin red y con una instantánea.

</details>

**6. Ordena estos pasos: leer y nombrar, triaje, informar, observar, probar la idea, formular una pregunta, encontrar el código.**

<details>
<summary>Respuesta</summary>

Triaje, observar, formular una pregunta, encontrar el código, leer y nombrar, probar la idea, informar.

</details>

**7. Tras el triaje de un archivo ves `statically linked, not stripped` y la cadena `NASM`. ¿Qué puedes concluir y qué no?**

<details>
<summary>Respuesta</summary>

Puedes concluir que el archivo no depende de bibliotecas compartidas, que los nombres de símbolos siguen presentes y que probablemente se ensambló con NASM. No puedes concluir qué hace el programa; las cadenas y las cabeceras son pistas que el análisis del código o la observación deben confirmar.

</details>
