<!-- source: quiz.md @ sha256:ce54d557ec3c -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 19. Analizar programas sospechosos

Responde primero y luego abre la respuesta.

**1. Nombra cuatro precauciones para analizar un programa desconocido.**

<details>
<summary>Respuesta</summary>

Trabajar en un entorno aislado (máquina virtual o contenedor) sin red, hacer primero una instantánea y restaurarla después, no compartir nada con el anfitrión (carpetas, portapapeles, credenciales) y mantener las muestras inertes en reposo (extensión cambiada o en un archivo comprimido) anotando sus hashes. Anotar las órdenes y los hallazgos también forma parte del trabajo.

</details>

**2. Un simulador afirma que solo imprime. ¿Cómo puedes comprobarlo sin leer su código?**

<details>
<summary>Respuesta</summary>

Ejecútalo en el laboratorio bajo un rastreador de llamadas al sistema como `strace -f` y mira las llamadas: un programa que solo imprime carga sus bibliotecas, escribe en su salida y termina. Cualquier creación de procesos, conexión de red, borrado de archivos o acceso inesperado a archivos aparecería en el rastreo.

</details>

**3. ¿Qué sugiere una entropía alta en un ejecutable y por qué no es una prueba?**

<details>
<summary>Respuesta</summary>

Un contenido cercano a 8 bits por byte significa que los datos están comprimidos o cifrados, lo que a menudo indica un programa empaquetado. No es una prueba porque los recursos comprimidos, los instaladores y el software legítimo protegido también tienen entropía alta, y una muestra puede empaquetarse de formas que mantengan la entropía más baja.

</details>

**4. ¿Por qué no basta una ruta del registro como `CurrentVersion\Run` hallada en un archivo para llamarlo malicioso, y qué añades?**

<details>
<summary>Respuesta</summary>

Las herramientas de documentación, los instaladores y las utilidades legítimas mencionan o usan la misma ruta. Añade otras pruebas que encajen: una orden de tarea programada, una ruta de volcado en una carpeta pública, un anfitrión de red, el orden del comportamiento. Un grupo de indicadores vale más que uno.

</details>

**5. ¿Qué ID de técnica cubre un programa que añade un valor bajo una clave Run, y cuál cubre uno que crea una tarea programada?**

<details>
<summary>Respuesta</summary>

T1547.001 (Registry Run Keys / Startup Folder) para el valor de la clave Run y T1053.005 (Scheduled Task) para la tarea programada. Ambas son técnicas de persistencia.

</details>

**6. ¿Qué secuencia de llamadas a la API es característica de la inyección clásica de DLL y a qué ID de técnica corresponde?**

<details>
<summary>Respuesta</summary>

`OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory` y `CreateRemoteThread`, donde el hilo suele empezar en la dirección de `LoadLibrary` y la memoria escrita contiene la ruta de una biblioteca. Corresponde a T1055.001 (DLL injection) dentro de T1055 (Process Injection). Las llamadas por sí solas no son una prueba, ya que depuradores y otras herramientas legítimas también las usan.

</details>

**7. Una regla YARA consiste en una sola cadena común y coincide con tu muestra. ¿Qué debes hacer antes de usarla?**

<details>
<summary>Respuesta</summary>

Probarla con archivos limpios, porque una cadena común coincidirá con programas legítimos (un falso positivo). Después hacerla específica: exigir varias cadenas distintivas a la vez, añadir una comprobación del tipo de archivo como el número mágico MZ o ELF, y volver a probar con la muestra y con la colección limpia.

</details>

**8. ¿Qué contiene una buena nota de detección?**

<details>
<summary>Respuesta</summary>

El comportamiento, el ID de técnica, dónde puede observarse (la fuente de eventos), por qué es sospechoso, qué actividad legítima se le parece (falsos positivos) y qué hacer como respuesta. Sin una fuente de eventos nombrada, nadie puede actuar sobre ella.

</details>
