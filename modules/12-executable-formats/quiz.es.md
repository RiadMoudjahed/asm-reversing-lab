<!-- source: quiz.md @ sha256:f18a70512af2 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 12. Formatos ejecutables: ELF y PE

Responde primero y luego abre la respuesta.

**1. ¿Cuál es la diferencia entre un desplazamiento en el archivo y una dirección virtual?**

<details>
<summary>Respuesta</summary>

Un desplazamiento en el archivo cuenta bytes desde el inicio del archivo. Una dirección virtual es donde aparecen esos bytes en la memoria del proceso tras la carga. Difieren por la alineación a páginas y porque algunas secciones, como `.bss`, ocupan memoria pero no espacio en el archivo.

</details>

**2. ¿Qué parte de un archivo ELF usa el cargador para mapearlo y qué parte usan herramientas como el enlazador y el depurador?**

<details>
<summary>Respuesta</summary>

El cargador usa las cabeceras de programa (segmentos). El enlazador, el depurador y el ingeniero inverso usan las cabeceras de sección. Un archivo puede ejecutarse sin su tabla de secciones.

</details>

**3. Un segmento `LOAD` tiene tamaño en archivo 0x22c y tamaño en memoria 0x230. ¿Qué significa la diferencia?**

<details>
<summary>Respuesta</summary>

Los 4 bytes adicionales son memoria inicializada a cero sin espacio en el archivo: el `.bss`.

</details>

**4. Explica qué ocurre en la primera llamada a `puts@plt` con enlace perezoso.**

<details>
<summary>Respuesta</summary>

El trozo salta a través de su entrada de la GOT, que al principio apunta a la siguiente instrucción del propio trozo. Esa instrucción apila el índice de reubicación y salta al trozo compartido, que entra en el resolvedor del cargador dinámico. El resolvedor encuentra `puts`, escribe su dirección en la entrada de la GOT y salta a ella. Las llamadas posteriores van directamente a `puts`.

</details>

**5. En un archivo PE, la RVA del punto de entrada es 0x2150 y la sección que la contiene tiene RVA 0x2000 y desplazamiento en bruto 0xa00. ¿Cuál es el desplazamiento en el archivo?**

<details>
<summary>Respuesta</summary>

0x2150 - 0x2000 + 0xa00 = 0xb50.

</details>

**6. ¿Qué estructuras de PE desempeñan el papel de la PLT y la GOT de ELF?**

<details>
<summary>Respuesta</summary>

El directorio de importaciones con la tabla de nombres de importación y la tabla de direcciones de importación. El cargador rellena la IAT antes de que arranque el programa, y el programa llama a través de ella.

</details>

**7. Nombra dos campos que el cargador debe encontrar en un ejecutable ELF mínimo de 64 bits.**

<details>
<summary>Respuesta</summary>

Cualesquiera dos de: el número mágico y la clase, el tipo de máquina, el punto de entrada, el desplazamiento, el tamaño y el número de cabeceras de programa, y en la cabecera de programa el tipo, los indicadores, el desplazamiento, la dirección virtual y los tamaños del segmento `LOAD`.

</details>
