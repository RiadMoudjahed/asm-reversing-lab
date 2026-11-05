<!-- source: WRITEUP.md @ sha256:5cd10ad8d812 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Resolución: ejercicio 10. Cirujano de cabeceras

La bandera es `ARL{header_10000000_520_11_152_17_m}`: la base de la imagen es 0x10000000, el punto de entrada está en el desplazamiento 0x520 del archivo, hay 11 cabeceras de programa, el segmento escribible tiene 152 bytes que existen solo en memoria, la sección propia `.vault` ocupa 17 bytes y la biblioteca adicional es `libm`.

## Parte 1: Reverse

### La cabecera ELF y las cabeceras de programa

```bash
readelf -h -l -W bin/surgeon
```

```text
  Type:                              EXEC (Executable file)
  Entry point address:               0x10000520
  Number of program headers:         11

Program Headers:
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  LOAD           0x000000 0x0000000010000000 0x0000000010000000 0x000760 0x000760 R E 0x1000
  LOAD           0x000de8 0x0000000010001de8 0x0000000010001de8 0x000240 0x0002d8 RW  0x1000
```

El listado está abreviado a las líneas que responden a las preguntas.

| Valor | De dónde sale |
|---|---|
| `B` | La primera entrada `LOAD` tiene `VirtAddr` 0x10000000, la más baja de las dos. Escribe `10000000` |
| `E` | El punto de entrada es 0x10000520. La primera entrada `LOAD` mapea el desplazamiento 0 del archivo en 0x10000000, así que el desplazamiento en el archivo es 0x10000520 - 0x10000000 = 0x520. Escribe `520` |
| `P` | `Number of program headers: 11` |
| `M` | La entrada `LOAD` con `RW` tiene `MemSiz` 0x2d8 y `FileSiz` 0x240; la diferencia es 0x98, que son 152 en decimal |

El valor 0x10000000 es inusual para un programa no independiente de la posición, cuyo valor por defecto es 0x400000. Se indicó al enlazador que colocara el primer segmento más arriba, y todas las direcciones del archivo se derivan de esa base.

`M` es mayor que los 0x80 bytes de la sección `.bss` porque el segmento también se extiende sobre el espacio de alineación entre el final de los datos del archivo y el inicio de `.bss`.

### Las secciones

```bash
readelf -S -W bin/surgeon | grep -E "\.(rodata|vault|data|bss) "
```

```text
  [15] .rodata           PROGBITS        00000000100006a4 0006a4 00001c 00   A  0   0  4
  [16] .vault            PROGBITS        00000000100006c0 0006c0 000011 00   A  0   0 16
  [24] .data             PROGBITS        0000000010002010 001010 000018 00  WA  0   0  8
  [25] .bss              NOBITS          0000000010002040 001028 000080 00  WA  0   0 32
```

`.vault` es la única sección cuyo nombre no es estándar. Su columna `Size` es 0x11, que son 17 en decimal. La lista de secciones se muestra abreviada; las demás secciones son `.interp`, `.dynsym`, `.text`, `.dynamic`, `.got` y otros nombres estándar.

### Las bibliotecas

```bash
readelf -d bin/surgeon | grep NEEDED
```

```text
 0x0000000000000001 (NEEDED)             Shared library: [libm.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
```

La biblioteca distinta de la biblioteca C es `libm.so.6`; sin el prefijo `lib` y el sufijo `.so.6` es `m`.

La bandera es `ARL{header_10000000_520_11_152_17_m}`.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. La disposición:

```text
file offset   0x000  ELF header                  64 bytes
              0x040  PT_LOAD, R + X              56 bytes
              0x078  PT_LOAD, R + W              56 bytes
              0x0b0  PT_GNU_STACK                56 bytes
              0x0e8  code                        the end is at 0x107
              0x200  message (18 bytes)
```

Lo que importa:

- **La base de la imagen.** `org 0x10000000` hace que cada etiqueta sea una dirección virtual absoluta, y `$$` es el inicio del archivo, de modo que `label - $$` es un desplazamiento en el archivo. La base es la encontrada en la Parte 1.
- **El primer segmento.** `p_offset` es 0 y `p_vaddr` es la base, así que las cabeceras están dentro del segmento, y el punto de entrada 0x10000000 + 0xe8 es una dirección dentro de él. `p_filesz` es el final del código.
- **El segmento de datos.** Está en el desplazamiento 0x200 del archivo y en la dirección virtual base + 0x1200. Ambos valores dejan el mismo resto, 0x200, módulo la alineación 0x1000. Esa es la condición con la que el cargador puede mapear directamente una página del archivo. Si no coinciden, el cargador mapea la página en el lugar equivocado y el programa se cuelga en el primer acceso.
- **La cabecera de pila.** `PT_GNU_STACK` con indicadores 6 indica que el programa necesita una pila legible y escribible y no necesita ejecutar código en ella. Los compiladores siempre la emiten; este ejercicio la exige.
- **El código.** `write` recibe la dirección de los datos en `esi`, que es 0x10001200; el tamaño del mensaje está en `edx`. Después viene `exit` con estado 0.

El archivo mide 530 bytes: 0x200 bytes de cabeceras, código y relleno, y 18 bytes de mensaje.

## Errores que cuestan tiempo

- **Usar la base de imagen por defecto 0x400000.** Todo lo demás es correcto y el verificador informa de que la dirección `PT_LOAD` más baja debe coincidir con `bin/surgeon`.
- **Un segmento con indicadores 7.** Un segmento de lectura, escritura y ejecución se rechaza, y además no se encuentra como segmento de lectura y escritura.
- **Un desplazamiento y una dirección que no coinciden módulo 0x1000.** El verificador informa de la regla de congruencia y el programa muere con la señal 11 porque el cargador mapea la página en el lugar equivocado.
- **Sin `PT_GNU_STACK`.** El programa se ejecuta, pero el verificador exige la cabecera. Si `e_phnum` es menor que el número de cabeceras escritas, la última se ignora.
- **Un punto de entrada fuera del segmento de código.** El programa muere con la señal 11 nada más empezar.
- **Un mensaje con otro texto.** El verificador informa de que el texto falta en el archivo y de que la salida difiere.
- **Un byte `class` de 1 en `e_ident`.** El archivo dice ser un ELF de 32 bits. El programa aún puede ejecutarse, pero la cabecera es incorrecta y el verificador lo informa.
