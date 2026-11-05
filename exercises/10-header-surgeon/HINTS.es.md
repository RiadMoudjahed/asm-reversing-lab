<!-- source: HINTS.md @ sha256:25348698dd82 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 10

Abre un nivel cada vez. Inténtalo de nuevo entre niveles.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

Ejecuta `readelf -h`, `readelf -l`, `readelf -S` y `readelf -d` sobre el archivo. Cada valor que necesitas está en uno de esos cuatro listados.

</details>

<details>
<summary>Nivel 2: dirección</summary>

La base de la imagen es la `VirtAddr` de la primera entrada `LOAD`. El punto de entrada es una dirección virtual; la primera entrada `LOAD` tiene desplazamiento 0 en el archivo, así que el desplazamiento en el archivo es el punto de entrada menos la base. El número de cabeceras de programa está en la cabecera ELF. `MemSiz` y `FileSiz` son columnas de las entradas `LOAD`. Los nombres de sección que no reconoces son raros; busca uno en la lista de secciones. Las entradas `NEEDED` están en la sección dinámica.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Usa la entrada `LOAD` con los indicadores `RW` para `M`: resta los dos tamaños. Para `V`, la sección que no está en la lista habitual de `.text`, `.data`, `.rodata`, `.bss` y demás tiene un tamaño en la columna `Size`; está en hexadecimal, así que conviértelo. Hay dos bibliotecas `NEEDED`; una es la biblioteca C `libc.so.6` y la otra da `L`.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Escribe el archivo en el orden en que aparece en el disco: cabecera ELF, cabeceras de programa, código, relleno, datos. Usa `org` con la base de la imagen para que las etiquetas tengan las direcciones virtuales correctas, y usa `$$` para calcular desplazamientos en el archivo.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Necesitas tres cabeceras de programa: un `PT_LOAD` para las cabeceras y el código (indicadores 5), un `PT_LOAD` para el mensaje (indicadores 6) y un `PT_GNU_STACK` (indicadores 6). La cabecera ELF mide 64 bytes y cada cabecera de programa 56. Coloca el mensaje en un desplazamiento del archivo como 0x200, rellena el archivo hasta él con `times` y da al segmento una dirección virtual con el mismo resto módulo 0x1000: por ejemplo la base de la imagen más 0x1000 más 0x200.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

En la cabecera ELF fija `e_phoff` en 64, `e_phentsize` en 56 y `e_phnum` en 3, y deja `e_shoff`, `e_shentsize`, `e_shnum` y `e_shstrndx` en cero. Para el primer segmento usa `p_offset` 0, `p_vaddr` igual a la base y `p_filesz` igual al final del código. El código hace `write` con la dirección de los datos en `esi` y luego `exit`. Si el verificador dice que el desplazamiento y la dirección deben coincidir módulo `p_align`, réstalos y comprueba que la diferencia sea múltiplo de 0x1000. Si el programa muere con la señal 11, el punto de entrada o la dirección del mensaje están fuera de los rangos mapeados.

</details>
