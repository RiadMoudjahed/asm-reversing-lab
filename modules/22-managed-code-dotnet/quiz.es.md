<!-- source: quiz.md @ sha256:9c23cc07031e -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 22. Código administrado: .NET

Responde primero y luego abre la respuesta.

**1. ¿Por qué un descompilador suele recuperar más de un ensamblado administrado que de un ejecutable nativo?**

<details>
<summary>Respuesta</summary>

El ensamblado conserva en sus metadatos y en la IL los nombres de tipos, métodos y campos, sus firmas con tipos y la estructura del código, porque el entorno de ejecución los necesita. Un ejecutable nativo tiene instrucciones máquina sin nombres ni tipos, y el descompilador tiene que deducirlos.

</details>

**2. Nombra tres señales en el triaje de que un archivo es un ensamblado administrado.**

<details>
<summary>Respuesta</summary>

`file` indica `Mono/.Net assembly`; la tabla de importaciones tiene una única entrada de `mscoree.dll` (`_CorExeMain` para un programa); el directorio de datos número 14 (la cabecera CLR) no está vacío; y los metadatos empiezan con la firma `BSJB` y una cadena de versión como `v4.0.30319`.

</details>

**3. ¿Por qué `strings` con su configuración por defecto no muestra los literales de cadena de un programa administrado?**

<details>
<summary>Respuesta</summary>

Los literales se guardan en el montículo `#US` como texto UTF-16. La búsqueda por defecto busca caracteres de un byte. Usa `strings -el` para encontrar texto UTF-16, o lee el montículo con un script. Los nombres de tipos, métodos y campos están en `#Strings` en UTF-8 y sí aparecen.

</details>

**4. ¿Qué hace la secuencia de IL `ldarg.0`, `ldfld`, `ldc.i4 499`, `add`, `stloc.0`?**

<details>
<summary>Respuesta</summary>

Apila `this`, lo sustituye por el valor de un campo, apila la constante 499, sustituye ambos por su suma y extrae el resultado hacia la variable local 0. Es la manera de escribir `x = field + 499` con la pila de evaluación.

</details>

**5. Un método descompilado usa variables llamadas `num`, `num2` y `num3`. ¿Adónde fueron los nombres originales y cómo se recuperan?**

<details>
<summary>Respuesta</summary>

Los nombres de las variables locales están en el archivo de depuración (PDB), no en el ensamblado, así que una compilación sin él solo tiene ranuras numeradas y el descompilador inventa nombres. Si tienes el PDB, descompila con la opción que lo usa (`-usepdb`). Si no, nombra tú las variables según cómo se usan.

</details>

**6. Una clase de la salida descompilada se llama `<>c__DisplayClass1_0`. ¿Qué es?**

<details>
<summary>Respuesta</summary>

Una clase auxiliar generada por el compilador, no escrita por el autor. Esta contiene una variable local que captura una lambda, y el cuerpo de la lambda es un método de la clase. Los nombres que contienen `<` y `>` marcan código generado y se leen como el código que implementan.

</details>

**7. ¿Cuándo debes leer la IL en lugar de fiarte del C# descompilado?**

<details>
<summary>Respuesta</summary>

Cuando el código descompilado parece erróneo o raro, cuando un detalle decide la respuesta (una constante, una comparación, el orden de las operaciones) y cuando el código está tan ofuscado que el descompilador se rinde. El descompilador toma decisiones; la IL es lo que ejecuta el entorno de ejecución.

</details>

**8. ¿Cómo tratas las cadenas que un ofuscador cifró en un ensamblado administrado?**

<details>
<summary>Respuesta</summary>

El montículo `#US` tiene pocos literales y el código llama a un método pequeño que devuelve una cadena. Ese método es el decodificador: lee su IL y reimpleméntalo, o llámalo desde un arnés de pruebas en el laboratorio aislado con los argumentos que leas en los puntos de llamada, como en el módulo 20.

</details>
