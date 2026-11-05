<!-- source: README.md @ sha256:ff34d51c3293 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 02. Dos mundos

Level 1. Tiempo: aproximadamente 1 hora. Arquitectura: x86 y x86-64 Linux. Módulos: [02. La CPU y sus registros](../../modules/02-cpu-and-registers/README.es.md), [03. Tu primer programa](../../modules/03-first-program/README.es.md)

## Escenario

Un equipo mantiene dos compilaciones de la misma utilidad pequeña: una para sistemas antiguos de 32 bits y otra para sistemas de 64 bits. Nadie recuerda en qué se diferencian las dos compilaciones al nivel más bajo. Tu trabajo es documentar la diferencia a partir de los binarios y luego usar ese conocimiento para llevar un tercer programa antiguo de 32 bits a 64 bits.

## Objetivos

- Comparar cómo entran en el núcleo las interfaces de Linux de 32 y de 64 bits.
- Identificar números de llamada al sistema y registros de argumentos en el desensamblado.
- Portar un programa de la interfaz de 32 bits a la de 64 bits.

## Parte 1: Reverse

Los binarios son `bin/two_worlds32` y `bin/two_worlds64`. Se comportan igual. Se diferencian en cómo están escritos.

```bash
file bin/two_worlds32 bin/two_worlds64
objdump -d -M intel bin/two_worlds32
objdump -d -M intel bin/two_worlds64
strace ./bin/two_worlds32
strace ./bin/two_worlds64
```

Usa la [hoja de referencia de llamadas al sistema de Linux](../../cheatsheets/syscalls-linux.es.md) para buscar los números. Responde estas preguntas:

1. ¿Qué tres números de llamada al sistema carga cada binario en `eax`, por orden? (Los números del listado están en hexadecimal.)
2. ¿Qué instrucción entra en el núcleo en cada binario?
3. En cada binario, ¿qué registro lleva el primer argumento de `write`?

Construye la bandera con la llamada que no es `write` y con `exit`:

```text
ARL{worlds_<sync in 32-bit>_<sync in 64-bit>_<exit in 32-bit>_<exit in 64-bit>}
```

## Parte 2: Assemble

`starter/solution.asm` es un programa antiguo de 32 bits. Imprime una línea, llama a `sync` y termina con el estado 5. Edítalo para que sea un programa de 64 bits con el mismo comportamiento:

- La misma salida en la salida estándar.
- Las mismas tres llamadas al sistema, en el mismo orden.
- El mismo estado de salida.

Compílalo y ejecútalo desde la carpeta `starter`:

```bash
make run
```

## Reglas

- El resultado debe ensamblarse con `nasm -f elf64` y enlazarse con `ld` normal.
- No uses `int 0x80`. Usa la instrucción `syscall` de 64 bits.
- Conserva las tres llamadas, incluida `sync`. El verificador observa las llamadas que hace el programa.

## Comprueba tu trabajo

```bash
make check EX=02 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y de la Parte 2 por separado. Sin `--flag`, comprueba solo la Parte 2.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
