<!-- source: instructions.md @ sha256:34a18bb7f3bc -->
[English](instructions.md) | [العربية](instructions.ar.md) | [Deutsch](instructions.de.md) | [Español](instructions.es.md) | [Français](instructions.fr.md)

# Instrucciones comunes

Sintaxis Intel: el destino va primero. Columna de indicadores: qué indicadores cambia la instrucción.

## Movimiento de datos

| Instrucción | Efecto | Indicadores |
|---|---|---|
| `mov d, s` | Copia `s` en `d` | ninguno |
| `movzx d, s` | Copia con extensión de ceros | ninguno |
| `movsx d, s` | Copia con extensión de signo | ninguno |
| `lea d, [m]` | Carga la dirección `m`, sin leer memoria | ninguno |
| `xchg a, b` | Intercambia `a` y `b` | ninguno |
| `push s` | Disminuye `rsp` y guarda `s` | ninguno |
| `pop d` | Carga `d` desde la pila y aumenta `rsp` | ninguno |

## Aritmética y lógica

| Instrucción | Efecto | Indicadores |
|---|---|---|
| `add d, s` / `sub d, s` | Suma y resta | todos los indicadores aritméticos |
| `adc d, s` / `sbb d, s` | Suma o resta con acarreo | todos los indicadores aritméticos |
| `inc d` / `dec d` | Suma o resta 1 | todos excepto `CF` |
| `neg d` | Negación en complemento a dos | todos los indicadores aritméticos |
| `imul d, s` | Multiplicación con signo, truncada | `CF`, `OF` |
| `mul s` / `div s` | Multiplicación o división sin signo con `rax` y `rdx` | varía |
| `idiv s` | División con signo con `rax` y `rdx` | indefinidos |
| `and`, `or`, `xor`, `not` | Operaciones bit a bit (`not` no modifica los indicadores) | `ZF`, `SF`, `PF`; `CF` y `OF` a cero |
| `shl`, `shr`, `sar` | Desplazamiento a la izquierda, a la derecha lógico y aritmético | `CF`, `ZF`, `SF`, `PF` |
| `rol`, `ror` | Rotación | `CF`, `OF` |
| `cmp a, b` | Calcula `a - b` y conserva solo los indicadores | todos los indicadores aritméticos |
| `test a, b` | Calcula `a & b` y conserva solo los indicadores | `ZF`, `SF`, `PF` |

## Control de flujo

| Instrucción | Efecto |
|---|---|
| `jmp t` | Salta a `t` |
| `je` / `jne` | Salta si es igual / distinto (`ZF`) |
| `ja` / `jb` / `jae` / `jbe` | Salta si es mayor / menor, sin signo (`CF`, `ZF`) |
| `jg` / `jl` / `jge` / `jle` | Salta si es mayor / menor, con signo (`SF`, `OF`, `ZF`) |
| `js` / `jns` | Salta si el signo está activado / desactivado |
| `call t` | Apila la dirección de retorno y salta a `t` |
| `ret` / `ret N` | Desapila la dirección de retorno en `rip`; `ret N` retira además `N` bytes |
| `loop t` | Disminuye `rcx` y salta si no es cero |

## Instrucciones de cadena

| Instrucción | Efecto |
|---|---|
| `movs`, `stos`, `lods` | Copia, guarda `al`/`ax`/`eax`/`rax`, carga en él |
| `cmps`, `scas` | Compara dos operandos de memoria, compara memoria con `al`/`ax`/`eax`/`rax` |
| `rep`, `repe`, `repne` | Repite mientras `rcx` no sea cero (y `ZF` esté activado o desactivado, en `repe` y `repne`) |
| `cld` / `std` | Borra o activa el indicador de dirección |

## Sistema

| Instrucción | Efecto |
|---|---|
| `syscall` | Llamada al sistema de 64 bits |
| `int 0x80` | Llamada al sistema Linux de 32 bits |
| `nop` | No hace nada |
| `leave` | `mov rsp, rbp` y luego `pop rbp` |
