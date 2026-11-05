<!-- source: quiz.md @ sha256:26a6df12e021 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Cuestionario: 00. Preparación del laboratorio

Responde primero y después abre la respuesta.

**1. ¿Dónde aparece la carpeta de tu repositorio dentro del laboratorio y por qué importa?**

<details>
<summary>Respuesta</summary>

En `/lab`. La carpeta se monta, no se copia, de modo que un archivo que editas en tu equipo es visible al instante en el laboratorio y al revés.

</details>

**2. ¿Qué hace el comando `ld` que no hace `nasm`?**

<details>
<summary>Respuesta</summary>

`nasm` ensambla el código fuente en un archivo objeto. `ld` es el enlazador: convierte uno o más archivos objeto en un ejecutable.

</details>

**3. Tras `./build/00_exit42`, ¿cómo lees el estado de salida del programa en la shell?**

<details>
<summary>Respuesta</summary>

Con `echo $?`, que imprime el estado de salida del último comando. La prueba de humo imprime 42.

</details>

**4. ¿Por qué una bandera nunca se guarda en texto plano en las carpetas de los ejercicios?**

<details>
<summary>Respuesta</summary>

Para que leer el repositorio no revele la respuesta. El verificador guarda un hash con sal de la bandera y compara tu respuesta con él.

</details>

**5. Has descargado Ghidra en tu equipo anfitrión. ¿Qué ruta usas para importar `exercises/01-signal-flare/bin/signal_flare`: la del anfitrión o la de `/lab`?**

<details>
<summary>Respuesta</summary>

La del anfitrión. Ghidra se ejecuta en el anfitrión y no puede ver el montaje `/lab`, que solo existe dentro del contenedor.

</details>

**6. Una comprobación imprime `Part 1 (Reverse): skipped, no --flag given`. ¿Qué olvidaste?**

<details>
<summary>Respuesta</summary>

Pasar tu bandera: `make check EX=01 ARGS='--flag ARL{...}'`. La parte 2 se ejecuta igualmente sin ella.

</details>
