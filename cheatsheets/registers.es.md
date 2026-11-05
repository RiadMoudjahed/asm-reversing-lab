<!-- source: registers.md @ sha256:430608b4d7d1 -->
[English](registers.md) | [العربية](registers.ar.md) | [Deutsch](registers.de.md) | [Español](registers.es.md) | [Français](registers.fr.md)

# Registros x86 y x86-64

## Registros de propósito general

Escribir un registro de 32 bits en modo de 64 bits borra los 32 bits superiores del registro completo. Escribir un registro de 8 o 16 bits deja intactos los demás bits.

| 64 bits | 32 bits | 16 bits | 8 bits bajo | 8 bits alto |
|---|---|---|---|---|
| `rax` | `eax` | `ax` | `al` | `ah` |
| `rbx` | `ebx` | `bx` | `bl` | `bh` |
| `rcx` | `ecx` | `cx` | `cl` | `ch` |
| `rdx` | `edx` | `dx` | `dl` | `dh` |
| `rsi` | `esi` | `si` | `sil` | ninguno |
| `rdi` | `edi` | `di` | `dil` | ninguno |
| `rbp` | `ebp` | `bp` | `bpl` | ninguno |
| `rsp` | `esp` | `sp` | `spl` | ninguno |
| `r8` a `r15` | `r8d` a `r15d` | `r8w` a `r15w` | `r8b` a `r15b` | ninguno |

Los registros `r8` a `r15` y los nombres de 8 bits `sil`, `dil`, `bpl` y `spl` existen solo en modo de 64 bits.

## Registros especiales

| Registro | Propósito |
|---|---|
| `rip` / `eip` | Dirección de la siguiente instrucción. La cambian los saltos, `call` y `ret`. En código de 64 bits puede usarse como base de direccionamiento, como en `[rel label]`. |
| `rsp` / `esp` | Puntero de pila. Apunta al elemento apilado más recientemente. |
| `rbp` / `ebp` | Puntero de marco por convención; un registro de propósito general en código optimizado. |
| `rflags` / `eflags` | Indicadores de estado y de control (véase más abajo). |

## Indicadores

| Indicador | Nombre | Se activa cuando |
|---|---|---|
| `CF` | Acarreo | una operación sin signo generó acarreo o préstamo |
| `ZF` | Cero | el resultado es cero |
| `SF` | Signo | el bit más alto del resultado está activado |
| `OF` | Desbordamiento | una operación con signo se desbordó |
| `PF` | Paridad | el byte bajo del resultado tiene un número par de bits 1 |
| `AF` | Ajuste | un acarreo o préstamo cruzó el bit 3 (lo usan las instrucciones BCD) |
| `DF` | Dirección | las instrucciones de cadena avanzan hacia atrás; se borra con `cld` |

## Quién debe conservar qué

| Registro | System V AMD64 | Microsoft x64 | 32 bits cdecl, stdcall, fastcall |
|---|---|---|---|
| `rax` | Lo guarda el llamador, resultado | Lo guarda el llamador, resultado | Lo guarda el llamador, resultado en `eax` |
| `rbx` | Lo guarda la función llamada | Lo guarda la función llamada | Lo guarda la función llamada |
| `rcx`, `rdx` | Lo guarda el llamador | Lo guarda el llamador | Lo guarda el llamador |
| `rsi`, `rdi` | Lo guarda el llamador | Lo guarda la función llamada | Lo guarda la función llamada |
| `rbp` | Lo guarda la función llamada | Lo guarda la función llamada | Lo guarda la función llamada |
| `r8` a `r11` | Lo guarda el llamador | Lo guarda el llamador | no disponible |
| `r12` a `r15` | Lo guarda la función llamada | Lo guarda la función llamada | no disponible |

Los registros de argumentos están en [Convenciones de llamada](calling-conventions.es.md).
