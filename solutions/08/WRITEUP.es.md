<!-- source: WRITEUP.md @ sha256:8b7f8b149302 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Resolución: ejercicio 08. Tarjeta de presentación

La bandera es `ARL{calling_scfcmv}`: `f1` es stdcall, `f2` es cdecl, `f3` es fastcall, `f4` es cdecl, `g1` es Microsoft x64 y `g2` es System V AMD64. Cada convención deja una firma en dos lugares: cómo la función lee sus argumentos y cómo se equilibra la pila tras la llamada.

## Parte 1: Reverse

### Las funciones de 32 bits

```text
f1:  mov eax,[esp+4]       f2:  mov eax,[esp+0xc]
     lea eax,[eax+eax*2]        imul eax,eax,0x64
     mov edx,[esp+8]            mov edx,[esp+8]
     lea eax,[eax+edx*2]        lea edx,[edx+edx*4]
     add eax,[esp+0xc]          lea eax,[eax+edx*2]
     ret 0xc                    add eax,[esp+4]
                                ret

f3:  lea eax,[ecx+ecx*4]   f4:  mov eax,[esp+4]
     sub eax,edx                imul eax,[esp+8]
     add eax,[esp+4]            ret
     ret 4
```

| Función | Evidencia | Convención |
|---|---|---|
| `f1` | Tres argumentos en la pila en `[esp+4]`, `[esp+8]`, `[esp+0xc]`; termina en `ret 0xc`, así que la función llamada los retira; el llamador no toca `esp` tras la llamada | stdcall |
| `f2` | Tres argumentos en la pila; `ret` simple; el llamador ejecuta `add esp,0xc` tras la llamada | cdecl |
| `f3` | Lee `ecx` y `edx` sin escribirlos antes; un argumento en la pila en `[esp+4]`; termina en `ret 4`; el llamador carga `ecx` y `edx` y apila un valor | fastcall |
| `f4` | Dos argumentos en la pila; `ret` simple; el llamador ejecuta `add esp,8` tras la llamada | cdecl |

Los llamadores de `_start` confirman cada lectura. Tras `call f1` no hay ajuste de pila, tras `call f2` hay `add esp,0xc`, antes de `call f3` solo se ejecuta un `push` junto con dos cargas de registros, y tras `call f4` hay `add esp,8`. Tres de las funciones solo reciben argumentos en la pila, así que la ubicación de los argumentos por sí sola no separa stdcall de cdecl; el `ret` es la instrucción decisiva.

Las fórmulas, leídas del código: `f1(a, b, c) = 3a + 2b + c`, `f2(a, b, c) = a + 10b + 100c` y `f3(a, b, c) = 5a - b + c`, con `a` en `ecx` y `b` en `edx`.

### Las funciones de 64 bits

```text
g1:  mov [rsp+8],rcx        g2:  lea rax,[rsi+rsi]
     mov [rsp+0x10],rdx          add rax,rdi
     mov rax,[rsp+8]             lea rdx,[rdx+rdx*2]
     imul rax,rax,7              add rax,rdx
     add rax,[rsp+0x10]          ret
     add rax,r8
     ret
```

| Función | Evidencia | Convención |
|---|---|---|
| `g1` | Los argumentos llegan en `rcx`, `rdx` y `r8`; la función guarda `rcx` y `rdx` en los 32 bytes por encima de su dirección de retorno (el espacio de sombra que reservó el llamador); el llamador ejecuta `sub rsp,0x28` antes de la llamada | Microsoft x64 |
| `g2` | Los argumentos llegan en `rdi`, `rsi` y `rdx`; sin espacio de sombra ni ajuste de pila en el llamador | System V AMD64 |

En el orden `f1`, `f2`, `f3`, `f4`, `g1`, `g2` las letras son `s`, `c`, `f`, `c`, `m`, `v`.

La bandera es `ARL{calling_scfcmv}`.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. Lo que importa:

- **`cc_f1` (stdcall).** Los tres argumentos están en `[esp+4]`, `[esp+8]` y `[esp+12]`. El resultado es `3a + 2b + c`, calculado con `lea` y un `add`. La función termina en `ret 12`.
- **`cc_f2` (cdecl).** El resultado es `a + 10b + 100c`. La función termina en un `ret` simple, porque el llamador retira los argumentos.
- **`cc_f3` (fastcall).** `a` está en `ecx` y `b` en `edx`; solo `c` está en la pila, en `[esp+4]`. El resultado es `5a - b + c`. La función termina en `ret 4`.

Toda la aritmética se desborda a 32 bits, lo que `lea`, `add` e `imul` hacen sin código adicional. No se toca ningún registro que deba preservar la función llamada, así que no hay nada que guardar.

## Errores que cuestan tiempo

- **Usar `ret` donde hace falta un número.** El resultado es correcto, pero el puntero de pila difiere tras la llamada; el verificador informa `stack` en todos los casos.
- **Usar `ret N` en la función cdecl.** El llamador retira entonces los argumentos por segunda vez; el puntero de pila se desplaza y el programa suele terminar con la señal 11.
- **Olvidar `ret 4` en la función fastcall.** El argumento apilado se queda en la pila; el verificador informa `stack`.
- **Leer los argumentos de fastcall de la pila.** Solo `c` está allí. Si `a` y `b` se toman de las posiciones equivocadas, fallan 18 de los 27 vectores de prueba.
- **Confundir `b` y `c` en la fórmula de cdecl.** Los pesos 10 y 100 no son simétricos, así que solo lo revelan entradas con `b` y `c` distintos.
