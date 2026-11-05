<!-- source: README.md @ sha256:b4dcee2d886b -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 10. Cirujano de cabeceras

Level 3. Tiempo: unas 3 horas. Arquitectura: x86-64 Linux. Módulos: [12. Formatos ejecutables: ELF y PE](../../modules/12-executable-formats/README.es.md)

## Escenario

Un proveedor entregó un programa ELF, `surgeon`, cuyos ajustes de compilación nadie anotó. El equipo de despliegue necesita su base de imagen, su punto de entrada y algunos datos de tamaño antes de poder colocarlo en un lanzador restringido. También necesitan un ejecutable de prueba mínimo que siga la misma base de imagen pero sin bibliotecas ni compilador detrás, para poder comprobar el lanzador sin dependencias. Ese ejecutable de prueba hay que escribirlo a mano, cabecera por cabecera.

## Objetivos

- Leer con herramientas de línea de comandos las cabeceras, las cabeceras de programa, las secciones, las entradas dinámicas y las reubicaciones de un archivo ELF.
- Convertir entre direcciones virtuales y desplazamientos en el archivo.
- Escribir un ejecutable válido con `nasm -f bin`, incluida la regla de congruencia entre desplazamientos en el archivo y direcciones virtuales.

## Parte 1: Reverse

El binario es `bin/surgeon`. Se ejecuta e imprime una línea.

```bash
bin/surgeon
readelf -h -l -S -d bin/surgeon
```

Encuentra estos seis valores:

1. `B`: la base de la imagen, la dirección virtual más baja de cualquier segmento `PT_LOAD`. Escríbela en hexadecimal en minúsculas sin `0x`.
2. `E`: el desplazamiento en el archivo del punto de entrada, en hexadecimal en minúsculas sin `0x`.
3. `P`: el número de cabeceras de programa, en decimal.
4. `M`: cuántos bytes del segmento `PT_LOAD` de lectura y escritura existen solo en memoria, es decir, `MemSiz` menos `FileSiz`, en decimal.
5. `V`: el tamaño en bytes, en decimal, de la única sección cuyo nombre no es un nombre estándar de compilador o enlazador.
6. `L`: el nombre de la biblioteca compartida distinta de la biblioteca C, sin el prefijo `lib` y sin el sufijo `.so` y la versión (para `libz.so.1` escribe `z`).

```text
ARL{header_<B>_<E>_<P>_<M>_<V>_<L>}
```

Por ejemplo, los valores 400000, 1050, 13, 8, 21 y z darían `ARL{header_400000_1050_13_8_21_z}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para que `nasm -f bin` escriba un archivo ejecutable completo. No se añade nada: escribes tú cada byte de las cabeceras. El programa imprime `header surgery ok` seguido de un salto de línea en la salida estándar y termina con estado 0.

El archivo debe cumplir todo lo siguiente:

- Es un ejecutable ELF de 64 bits, little-endian, para x86-64.
- La dirección virtual más baja de un `PT_LOAD` es la base de imagen `B` de `bin/surgeon`.
- Tiene un segmento `PT_LOAD` con indicadores de lectura + ejecución que contiene las cabeceras y el código, y otro segmento `PT_LOAD` con indicadores de lectura + escritura que contiene el mensaje. Ningún segmento puede ser a la vez escribible y ejecutable.
- En cada segmento `PT_LOAD`, `p_offset` y `p_vaddr` tienen el mismo resto módulo `p_align`.
- El punto de entrada está dentro del segmento de lectura + ejecución.
- Una cabecera de programa `PT_GNU_STACK` con indicadores de lectura + escritura marca la pila como no ejecutable.
- El texto del mensaje se almacena en el rango de archivo del segmento de lectura y escritura, y el código lo lee de allí.
- El archivo mide como máximo 1024 bytes.

El verificador lee `B` del propio `bin/surgeon`, luego inspecciona las cabeceras de tu archivo y lo ejecuta.

## Reglas

- Usa solo `nasm -f bin`: el verificador ensambla tu archivo de ese modo y ejecuta el resultado directamente.
- No uses un enlazador, una biblioteca C ni carga dinámica.
- No se exigen cabeceras de sección.

## Comprueba tu trabajo

```bash
make check EX=10 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y la Parte 2 por separado. Sin `--flag`, solo comprueba la Parte 2. Para la Parte 2 enumera cada requisito que incumple tu archivo.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
