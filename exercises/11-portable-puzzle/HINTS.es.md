<!-- source: HINTS.md @ sha256:2278a21c0cab -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Pistas: ejercicio 11

Abre un nivel cada vez. Inténtalo de nuevo entre niveles.

## Parte 1: Reverse

<details>
<summary>Nivel 1: empujón</summary>

`objdump -p` imprime los valores de la cabecera opcional, la tabla de secciones y las tablas de importación. El script del módulo 12 imprime la tabla de secciones con las RVA y con los desplazamientos en el archivo.

</details>

<details>
<summary>Nivel 2: dirección</summary>

La base de la imagen, la RVA del punto de entrada y el número de secciones son campos de las cabeceras. Para `O`, busca la sección cuyo rango de RVA contiene 0x12348 y aplica: desplazamiento en el archivo = RVA - RVA de la sección + desplazamiento en bruto de la sección. El script del módulo 12 muestra solo los primeros nombres de función de cada biblioteca; para `K`, cuenta todos los nombres de la lista de importaciones de `KERNEL32.dll`, por ejemplo con `objdump -p`.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

Busca en la lista de secciones la que empieza en 0x12348 o justo por debajo, y comprueba después que 0x12348 es menor que su inicio más su tamaño en bruto. El resultado está en hexadecimal y se escribe sin `0x`. El número de bibliotecas importadas es el número de líneas `DLL Name`. El valor del punto de entrada es una RVA, no una dirección: no sumes la base de la imagen.

</details>

## Parte 2: Assemble

<details>
<summary>Nivel 1: empujón</summary>

Empieza por `pe_rva_to_offset`: solo necesita registros volátiles y ninguna llamada. Escríbela como un bucle sobre la tabla de secciones, prueba la RVA contra cada sección y devuelve `0xFFFFFFFF` tras el bucle. Después escribe `pe_entry_offset` apoyándote en ella.

</details>

<details>
<summary>Nivel 2: dirección</summary>

Una cabecera de sección mide 40 bytes. Los campos que necesitas son la dirección virtual, el tamaño de datos en bruto y el puntero a los datos en bruto; lee sus posiciones en la tabla de secciones que imprimiste en la Parte 1. Para la prueba sin signo, resta la dirección virtual de la RVA y compara el resultado con el tamaño de datos en bruto mediante un salto sin signo: si la RVA está por debajo de la sección, la resta se desborda hasta un número enorme y la comparación falla por sí sola. En `pe_entry_offset`, la cabecera opcional empieza 24 bytes después de la firma NT, la RVA del punto de entrada está en el desplazamiento 16 dentro de ella, y la tabla de secciones empieza tras la cabecera opcional, cuyo tamaño es un campo de la cabecera de archivo.

</details>

<details>
<summary>Nivel 3: casi la respuesta</summary>

`pe_entry_offset` hace una llamada, así que necesita un marco: guarda la dirección de las cabeceras NT en `rbx` (apílalo primero), reserva al menos 32 bytes de espacio de sombra y asegúrate de que `rsp` sea múltiplo de 16 en la llamada. Tras un `push` la pila está alineada, así que resta un múltiplo de 16. Guarda la RVA del punto de entrada en una posición de la pila por encima del espacio de sombra, porque `log_rva` destruye `rcx` y todos los demás registros volátiles. Si el verificador imprime `align`, el tamaño del marco es incorrecto; `regs` significa que se cambió un registro no volátil y no se restauró (recuerda `rsi` y `rdi`); `calls` significa que `log_rva` no se llamó exactamente una vez con la RVA del punto de entrada.

</details>
