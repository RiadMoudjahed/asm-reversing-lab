<!-- source: README.md @ sha256:39876dd642a0 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 17. Archivo de incógnitas

Level 4. Tiempo: unas 4 horas. Arquitectura: x86-64 Linux. Módulos: [17. Descifrar formatos de archivo](../../modules/17-deciphering-file-formats/README.es.md)

## Escenario

Una herramienta de empaquetado, `arkpack`, reúne varios archivos en un solo archivo comprimido (archive). Nadie conservó la descripción del formato y se perdió la herramienta de desempaquetado. Ahora un dispositivo embebido tiene que leer estos archivos y solo tiene sitio para una rutina pequeña escrita en ensamblador. Primero recuperas el formato estudiando el programa que lo escribe. Después escribes el lector: listar los nombres, extraer los contenidos y rechazar los archivos dañados.

## Objetivos

- Recuperar la estructura de un formato de archivo (cabecera, tabla, registros, nombres, suma de verificación) leyendo el programa que lo escribe y probándolo con archivos propios.
- Reconocer la codificación aplicada a los datos y la suma de verificación, y reproducir ambas.
- Escribir en ensamblador un lector que nunca se fíe de los desplazamientos y tamaños guardados en el archivo.

## Parte 1: Reverse

El binario es `bin/arkpack`. Ejecútalo como `arkpack OUTPUT FILE...` y examina la salida con un volcado hexadecimal.

```bash
printf 'AAA' > a.txt
printf 'hello world\n' > hello.txt
bin/arkpack t.ark a.txt hello.txt
xxd t.ark
objdump -d -M intel bin/arkpack
```

Encuentra estos cinco valores:

1. `E`: el tamaño en bytes de una entrada de la tabla que describe los archivos guardados, en decimal.
2. `A`: la alineación en bytes del inicio de los datos de cada archivo guardado (cada desplazamiento de datos es múltiplo de `A`), en decimal.
3. `R`: el número de bits que se rota a la izquierda la suma de verificación por cada byte de datos, en decimal.
4. `K`: el valor del byte de clave que la herramienta guarda en la cabecera de un archivo con 3 ficheros, en decimal.
5. `N`: el mayor número de ficheros que acepta la herramienta, en decimal.

```text
ARL{ark_<E>_<A>_<R>_<K>_<N>}
```

Por ejemplo, los valores 32, 8, 3, 100 y 20 darían `ARL{ark_32_8_3_100_20}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para implementar tres funciones en la convención de llamada System V. El archivo son los `n` bytes en `a`.

```c-fragment
long ark_count(const unsigned char *a, size_t n);
long ark_name(const unsigned char *a, size_t n, long index, char *out, size_t cap);
long ark_extract(const unsigned char *a, size_t n, long index, unsigned char *out, size_t cap);
```

- `ark_count` devuelve el número de ficheros del archivo, o -1 si el archivo no es válido. No es válido si es más corto que la cabecera, tiene un valor mágico erróneo, o su cabecera describe una tabla que no está entera dentro del archivo (debe empezar tras la cabecera y terminar en el final del archivo o antes) o un área de nombres que empieza más allá del final del archivo.
- `ark_name` copia en `out` el nombre del fichero número `index` (contado desde 0), con su byte cero final, y devuelve la longitud del nombre sin ese byte. Devuelve -1 si el archivo no es válido o `index` no es un número de fichero válido, -2 si el nombre no empieza dentro del archivo o no tiene byte cero final antes del final del archivo, y -3 si `cap` es menor que la longitud del nombre más uno.
- `ark_extract` escribe en `out` el contenido del fichero número `index` y devuelve su tamaño en bytes. Devuelve -1 si el archivo no es válido o `index` no es un número de fichero válido, -2 si los datos guardados no están dentro del archivo (deben empezar tras la cabecera y terminar en el final del archivo o antes), -3 si `cap` es menor que el tamaño, y -4 si la suma de verificación de los bytes descodificados difiere de la guardada. Las comprobaciones se hacen en este orden. Ante un error no se examina el contenido de `out`, pero no debe escribirse nada más allá de `cap` bytes.

Todos los desplazamientos y tamaños del archivo son números de 32 bits sin signo, y sus sumas pueden superar los 32 bits. `index` es un número de 64 bits con signo.

Las pruebas construyen archivos con `arkpack` y también los dañan de muchas maneras. Para cada orden comparan el valor de retorno y los bytes escritos con los de una referencia.

## Reglas

- No leas nunca un byte fuera de los `n` bytes del archivo, diga lo que diga la cabecera. Las pruebas colocan el archivo justo antes de una página sin mapear, y una lectura más allá de su final termina la ejecución de las pruebas con una señal.
- No escribas nunca más allá de `cap` bytes de `out`.
- Conserva los registros que debe preservar la función llamada: `rbx`, `rbp` y de `r12` a `r15`.
- Mantén la pila alineada en cada `call`.

## Comprueba tu trabajo

```bash
make check EX=17 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y la Parte 2 por separado. Sin `--flag`, solo comprueba la Parte 2. Cada resultado de una prueba de la Parte 2 va seguido de `ok` o de una lista de lo que salió mal: `overrun` o `regs`.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
