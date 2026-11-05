<!-- source: calling-conventions.md @ sha256:5e066f9fba0a -->
[English](calling-conventions.es.md) | [العربية](calling-conventions.ar.md) | [Deutsch](calling-conventions.de.md) | [Español](calling-conventions.es.md) | [Français](calling-conventions.fr.md)

# Convenciones de llamada

## 32 bits

| | cdecl | stdcall | fastcall |
|---|---|---|---|
| Argumentos | Pila, de derecha a izquierda | Pila, de derecha a izquierda | Los dos primeros enteros en `ecx` y `edx`, el resto en la pila |
| Quién retira los argumentos de la pila | Llamador | Función llamada, con `ret N` | Función llamada, con `ret N` |
| Resultado | `eax` | `eax` | `eax` |
| Lo guarda la función llamada | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` |

## 64 bits

| | System V AMD64 (Linux, macOS) | Microsoft x64 (Windows) |
|---|---|---|
| Argumentos enteros y punteros | `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9` | `rcx`, `rdx`, `r8`, `r9` |
| Argumentos de punto flotante | `xmm0` a `xmm7` | `xmm0` a `xmm3` |
| Más argumentos | Pila | Pila, por encima del espacio de sombra |
| Resultado | `rax` (`xmm0` para punto flotante) | `rax` (`xmm0` para punto flotante) |
| Espacio de sombra | Ninguna | 32 bytes reservados por el llamador |
| Zona roja | 128 bytes bajo `rsp` para funciones hoja | Ninguna |
| Llamadas variádicas | `al` contiene el número de registros vectoriales usados | Los argumentos de punto flotante se copian también a registros enteros |
| Lo guarda la función llamada | `rbx`, `rbp`, `r12` a `r15` | `rbx`, `rbp`, `rdi`, `rsi`, `r12` a `r15`, `xmm6` a `xmm15` |

## Alineación de la pila

En código de 64 bits, `rsp` debe ser múltiplo de 16 en el momento de cada `call`. En la primera instrucción de una función es 8 módulo 16, porque la dirección de retorno acaba de apilarse.

## Reconocer una convención en un binario

- Un `ret` seguido de un número: stdcall o fastcall.
- `ecx` o `edx` leídos antes de escribirse, en código de 32 bits: fastcall.
- Un llamador que ejecuta `add esp, N` tras una llamada: cdecl.
- `rdi` y `rsi` leídos primero: System V. `rcx` y `rdx` leídos primero, con escrituras en `[rsp + 8]` y por encima: Microsoft x64.
- Un llamador que ejecuta `sub rsp, 0x28` (o un múltiplo de 8 mayor que conserve la alineación) antes de una llamada: espacio de sombra de Microsoft x64.
