<!-- source: WRITEUP.md @ sha256:33109aef56b5 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Explicación de la solución: ejercicio 02. Dos mundos

La bandera es `ARL{worlds_36_162_1_60}`. Los dos binarios hacen las mismas tres llamadas (`write`, `sync`, `exit`), pero con instrucciones, números y registros distintos. El port de la Parte 2 cambia las tres cosas.

## Parte 1: Reverse

### Identificar los archivos

```text
$ file bin/two_worlds32 bin/two_worlds64
bin/two_worlds32: ELF 32-bit LSB executable, Intel 80386, version 1 (SYSV), statically linked, not stripped
bin/two_worlds64: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, not stripped
```

### El comportamiento es idéntico

```text
$ strace ./bin/two_worlds32
[ Process PID=... runs in 32 bit mode. ]
write(1, "two worlds\n", 11)            = 11
sync()                                  = 0
exit(0)                                 = ?

$ strace ./bin/two_worlds64
write(1, "two worlds\n", 11)            = 11
sync()                                  = 0
exit(0)                                 = ?
```

`strace` indica que el primer proceso se ejecuta en modo de 32 bits y muestra las mismas tres llamadas para ambos. Los números no aparecen, así que léelos en el desensamblado.

### El código es distinto

```text
$ objdump -d -M intel bin/two_worlds32
08049000 <_start>:
 8049000:  mov  eax,0x4
 8049005:  mov  ebx,0x1
 804900a:  mov  ecx,0x804a000
 804900f:  mov  edx,0xb
 8049014:  int  0x80
 8049016:  mov  eax,0x24
 804901b:  int  0x80
 804901d:  mov  eax,0x1
 8049022:  xor  ebx,ebx
 8049024:  int  0x80

$ objdump -d -M intel bin/two_worlds64
0000000000401000 <_start>:
  401000:  mov  eax,0x1
  401005:  mov  edi,0x1
  40100a:  lea  rsi,[rip+0xfef]        # 402000 <text>
  401011:  mov  edx,0xb
  401016:  syscall
  401018:  mov  eax,0xa2
  40101d:  syscall
  40101f:  mov  eax,0x3c
  401024:  xor  edi,edi
  401026:  syscall
```

| | 32 bits | 64 bits |
|---|---|---|
| Instrucción | `int 0x80` | `syscall` |
| Número de `write` | 0x4 = 4 | 0x1 = 1 |
| Número de `sync` | 0x24 = 36 | 0xa2 = 162 |
| Número de `exit` | 0x1 = 1 | 0x3c = 60 |
| Registro del descriptor | `ebx` | `rdi` |
| Registro de la dirección | `ecx` | `rsi` |
| Registro de la longitud | `edx` | `rdx` |
| Dirección del texto | absoluta: `mov ecx, 0x804a000` | relativa: `lea rsi, [rip+0xfef]` |

La última fila es una diferencia que se pasa por alto con facilidad. El código de 32 bits en una dirección fija carga la dirección como una constante; el de 64 bits la calcula relativa a `rip`.

La bandera es `ARL{worlds_36_162_1_60}`.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. En comparación con el programa inicial, cambia:

- `bits 32` por `bits 64`, y añade `default rel` para la dirección del texto.
- `int 0x80` por `syscall` en los tres sitios.
- Los números: de 4 a 1 para `write`, de 36 a 162 para `sync`, de 1 a 60 para `exit`.
- Los registros de argumentos: `ebx`, `ecx`, `edx` por `rdi`, `rsi`, `rdx`.
- `mov ecx, message` por `lea rsi, [message]`.

## Errores que hacen perder tiempo

- **Cambiar solo la cabecera.** Un programa de 32 bits ensamblado como de 64 sigue funcionando, porque Linux acepta `int 0x80` en procesos de 64 bits y lee la tabla de llamadas de 32 bits. Por eso el verificador busca la propia instrucción y no solo la salida. En código de 64 bits no se pueden pasar así direcciones por encima de 4 GiB, de modo que un programa que hoy funciona por casualidad puede fallar más adelante.
- **Mantener los números antiguos.** Con `syscall`, el número de llamada 4 es `stat`, no `write`. La salida desaparece o la llamada falla.
- **Leer los números como decimales.** `0xa2` es 162, no 2.
- **Omitir `sync`.** No tiene efecto visible, por eso es fácil olvidarla. El verificador compara la secuencia de llamadas.
