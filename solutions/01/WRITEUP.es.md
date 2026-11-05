<!-- source: WRITEUP.md @ sha256:312a383646e6 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Explicación de la solución: ejercicio 01. Bengala de señales

La bandera es `ARL{flare_42_23_7}`. El programa escribe 42 bytes en la salida estándar, 23 bytes en la salida de error estándar y termina con el estado 7. Esta explicación muestra cómo llegar a ese resultado y cómo reconstruirlo.

## Parte 1: Reverse

### Identificar el archivo

```text
$ file bin/signal_flare
bin/signal_flare: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, not stripped
```

"Statically linked" significa que no se carga ninguna biblioteca. "Not stripped" significa que los nombres de los símbolos se conservan, por eso `_start` es visible en el desensamblado.

### Observar las llamadas al sistema

```text
$ strace ./bin/signal_flare
execve("./bin/signal_flare", ["./bin/signal_flare"], ...) = 0
write(1, "FLARE LAUNCHED\nCOLOR: RED\nALTITU"..., 42) = 42
write(2, "[flare] burn time 30 s\n", 23[flare] burn time 30 s
) = 23
exit(7)                                 = ?
+++ exited with 7 +++
```

`strace` imprime los primeros 32 bytes de cada búfer, así que el primer mensaje aparece cortado, pero el número de bytes (el último argumento) y el resultado están completos. El segundo mensaje aparece dentro de la línea de strace porque la salida de error estándar y la propia salida de strace comparten terminal. Es normal y no es un fallo del programa.

Las tres respuestas ya están a la vista: 42 bytes al descriptor 1, 23 bytes al descriptor 2, estado de salida 7.

### Confirmar en el desensamblado

`strace` muestra lo que ocurrió en una ejecución. El desensamblado muestra lo que hace el código, lo cual importa cuando un programa se comporta de otro modo con otras entradas.

```text
$ objdump -d -M intel bin/signal_flare
0000000000401000 <_start>:
  401000:  b8 01 00 00 00        mov    eax,0x1
  401005:  bf 01 00 00 00        mov    edi,0x1
  40100a:  48 8d 35 ef 0f 00 00  lea    rsi,[rip+0xfef]        # 402000 <report>
  401011:  ba 2a 00 00 00        mov    edx,0x2a
  401016:  0f 05                 syscall
  401018:  b8 01 00 00 00        mov    eax,0x1
  40101d:  bf 02 00 00 00        mov    edi,0x2
  401022:  48 8d 35 01 10 00 00  lea    rsi,[rip+0x1001]       # 40202a <notice>
  401029:  ba 17 00 00 00        mov    edx,0x17
  40102e:  0f 05                 syscall
  401030:  b8 3c 00 00 00        mov    eax,0x3c
  401035:  bf 07 00 00 00        mov    edi,0x7
  40103a:  0f 05                 syscall
```

Leído como tres llamadas:

| Llamada | `eax` | `edi` | `rsi` | `edx` | Significado |
|---|---|---|---|---|---|
| 1 | 0x1 | 0x1 | dirección de `report` | 0x2a = 42 | `write` de 42 bytes a la salida estándar |
| 2 | 0x1 | 0x2 | dirección de `notice` | 0x17 = 23 | `write` de 23 bytes a la salida de error estándar |
| 3 | 0x3c = 60 | 0x7 | no se usa | no se usa | `exit` con estado 7 |

La bandera es `ARL{flare_42_23_7}`.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. Su estructura sigue la tabla anterior: un bloque por llamada.

```nasm-fragment
_start:
    mov     eax, SYS_WRITE      ; write(STDOUT, banner, banner_len)
    mov     edi, STDOUT
    lea     rsi, [banner]
    mov     edx, banner_len
    syscall
    ...
    mov     eax, SYS_EXIT       ; exit(7)
    mov     edi, 7
    syscall
```

Dos detalles hacen que las longitudes sean correctas sin contar a mano:

- `banner_len equ $ - banner` se coloca inmediatamente después de la última línea `db` del texto. `$` es la dirección actual, así que la diferencia es el número de bytes entre la etiqueta y ese punto.
- El `10` al final de cada línea es el byte de salto de línea y cuenta para la longitud. El primer texto tiene 14 + 1, 10 + 1 y 15 + 1 bytes, que suman 42.

## Errores que hacen perder tiempo

- **Contar el texto a ojo.** Los bytes de salto de línea se olvidan con facilidad. Usa `$ - label`.
- **Olvidar `exit`.** El procesador sigue ejecutando lo que haya tras la última instrucción y el programa muere con un fallo de segmentación. El verificador informa "terminated by signal 11".
- **Intercambiar los descriptores.** El descriptor 1 es la salida estándar y el 2 la salida de error estándar. Ejecuta `./solution 2>/dev/null` para ver solo la salida estándar y confirmarlo.
- **Leer `edx` como decimal.** `0x2a` es 42, no 2 ni 20.
