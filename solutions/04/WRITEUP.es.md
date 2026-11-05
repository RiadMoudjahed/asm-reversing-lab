<!-- source: WRITEUP.md @ sha256:8af0bc97c35b -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Explicación de la solución: ejercicio 04. Orden de bytes

La bandera es `ARL{v3_7_p8443_t45000}`. El registro tiene un campo big-endian de 32 bits en el desplazamiento 0, uno little-endian de 16 bits en el 4, uno big-endian de 16 bits en el 6 y uno little-endian de 32 bits en el 8. La solución de referencia lo decodifica con cargas de bytes, desplazamientos y `or`.

## Parte 1: Reverse

### Leer el código

```text
$ objdump -d -M intel --no-show-raw-insn bin/config_probe
0000000000401000 <_start>:
  mov    eax,DWORD PTR [rip+0xffa]        # 402000 <record>
  bswap  eax
  movzx  ecx,WORD PTR [rip+0xff5]         # 402004 <record+0x4>
  movzx  edx,WORD PTR [rip+0xff0]         # 402006 <record+0x6>
  rol    dx,0x8
  mov    ebx,DWORD PTR [rip+0xfe8]        # 402008 <record+0x8>
  add    eax,ecx
  add    eax,edx
  add    eax,ebx
  cmp    eax,0x450cb
```

Cuatro cargas del registro, dos de ellas seguidas de un intercambio:

| Campo | Instrucción | Desplazamiento | Anchura | Orden de bytes |
|---|---|---|---|---|
| A | `mov eax, [record]` y luego `bswap eax` | 0 | 4 bytes | big-endian |
| B | `movzx ecx, word [record+4]` | 4 | 2 bytes | little-endian |
| C | `movzx edx, word [record+6]` y luego `rol dx, 8` | 6 | 2 bytes | big-endian |
| D | `mov ebx, [record+8]` | 8 | 4 bytes | little-endian |

`bswap` y `rol dx, 8` invierten los bytes de aquello sobre lo que actúan, que es como un programa convierte datos big-endian en el valor nativo little-endian. Una carga sin intercambio conserva el orden de la memoria, que en x86 es little-endian.

### Decodificar los bytes

```text
$ objdump -s -j .rodata bin/config_probe
 402000 00030007 fb208001 c8af0000 ...
```

El registro es `00 03 00 07 | fb 20 | 80 01 | c8 af 00 00`.

| Campo | Bytes | Invertidos si hace falta | Valor |
|---|---|---|---|
| A | `00 03 00 07` | ya en orden de lectura (big-endian) | `0x00030007`: mitad alta 3, mitad baja 7 |
| B | `fb 20` | `20 fb` | `0x20fb` = 8443 |
| C | `80 01` | ya en orden de lectura (big-endian) | `0x8001` |
| D | `c8 af 00 00` | `00 00 af c8` | `0xafc8` = 45000 |

### Comprobar con la suma de verificación

El programa suma los cuatro valores decodificados y compara la suma con `0x450cb`:

```text
0x00030007 + 0x20fb + 0x8001 + 0xafc8 = 0x450cb
196615 + 8443 + 32769 + 45000 = 282827
```

Si hubieras leído B como big-endian (`0xfb20` = 64288), la suma no coincidiría. La constante del programa es una clave de respuestas incorporada para tu decodificación.

La bandera es `ARL{v3_7_p8443_t45000}`.

## Parte 2: Assemble

La solución de referencia es `solution.asm`.

`load_le16` es una sola instrucción, porque la carga nativa del procesador es little-endian:

```nasm-fragment
load_le16:
    movzx   eax, word [rdi]
    ret
```

`load_be32` construye el valor byte a byte, empezando por el byte más significativo. Cada paso desplaza 8 bits lo que ya hay y añade el byte siguiente:

```nasm-fragment
    movzx   eax, byte [rdi]             ; first byte
    shl     eax, 8
    movzx   edx, byte [rdi + 1]
    or      eax, edx                    ; now two bytes
    ; ... repeat for bytes 2 and 3
```

`decode_record` hace lo mismo para `version` y `flags`, y copia `port` y `timeout_ms` con cargas y escrituras simples de la misma anchura.

## Errores que hacen perder tiempo

- **Cargar ocho bytes en `load_be32`.** `mov rax, [rdi]` es un comienzo válido, pero lee cuatro bytes más allá de la entrada. Las pruebas colocan la entrada justo antes de una página inaccesible, así que el programa falla con la señal 11.
- **Escribir demasiado ancho en `out`.** `mov [rsi + 4], eax` guarda cuatro bytes en un campo de dos y sobrescribe `flags`. Las pruebas rellenan antes `out` con un patrón conocido, así que el daño se ve.
- **No limpiar el registro.** Cargar un byte con `mov al, [rdi]` deja intactos los bits superiores antiguos de `eax`. Usa `movzx` para que los bits superiores sean cero.
- **Usar `bswap` o `movbe`.** Producen los valores correctos y el verificador los rechaza por regla.
- **Leer B o D como big-endian.** Una carga sin intercambio en el programa original es little-endian. La suma de verificación lo delata de inmediato.
