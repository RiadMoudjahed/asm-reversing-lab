<!-- source: README.md @ sha256:09da60cefe79 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 01. Bengala de señales

Level 1. Tiempo: unos 45 minutos. Arquitectura: x86-64 Linux. Módulos: [00. Preparación del laboratorio](../../modules/00-setup/README.es.md), [03. Tu primer programa](../../modules/03-first-program/README.es.md)

## Escenario

Un pequeño programa baliza llamado `signal_flare` informa de su lanzamiento y termina. Su código fuente se ha perdido y quienes lo mantienen necesitan saber exactamente qué hace para poder reconstruirlo. Tienes el binario y nada más.

## Objetivos

- Leer las llamadas al sistema de un programa con `strace` y `objdump`.
- Determinar el número, los argumentos y el resultado de cada llamada.
- Reconstruir el programa en ensamblador con llamadas al sistema directas y sin biblioteca de C.

## Parte 1: Reverse

El binario es `bin/signal_flare`. Ejecuta estos comandos desde la carpeta del ejercicio dentro del laboratorio:

```bash
file bin/signal_flare
strace ./bin/signal_flare
objdump -d -M intel bin/signal_flare
```

`strace` lista cada llamada al sistema que hace el programa. `objdump -d -M intel` muestra las instrucciones con sintaxis Intel; el código que necesitas empieza en la etiqueta `_start`. La [hoja de referencia de llamadas al sistema de Linux](../../cheatsheets/syscalls-linux.es.md) indica qué significa cada número de llamada y qué registro lleva cada argumento.

Responde estas preguntas:

1. ¿Qué números de llamada al sistema usa el programa, por orden?
2. Para cada `write`, ¿qué descriptor de archivo usa y cuántos bytes envía?
3. ¿Qué estado de salida devuelve el programa?

Construye la bandera con tus respuestas:

```text
ARL{flare_<bytes written to descriptor 1>_<bytes written to descriptor 2>_<exit status>}
```

Por ejemplo, si los números fueran 10, 20 y 3, la bandera sería `ARL{flare_10_20_3}`.

## Parte 2: Assemble

Edita `starter/solution.asm` para que tu programa se comporte exactamente como `signal_flare`:

- Los mismos bytes en la salida estándar.
- Los mismos bytes en la salida de error estándar.
- El mismo estado de salida.

Compílalo y ejecútalo desde la carpeta `starter`:

```bash
make run
```

## Reglas

- Usa solo las llamadas al sistema `write` y `exit`. No enlaces la biblioteca de C.
- Escribe tú mismo el texto y las llamadas. Tu programa no debe iniciar `signal_flare`.
- Define las longitudes de los textos con NASM (`$ - label`), no contándolas a mano.

## Comprueba tu trabajo

```bash
make check EX=01 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y de la Parte 2 por separado. Sin `--flag`, comprueba solo la Parte 2.

## Pistas

Abre [HINTS.es.md](HINTS.es.md) un nivel cada vez.
