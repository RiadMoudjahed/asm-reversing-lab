<!-- source: WRITEUP.md @ sha256:019af3edf455 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Explicación de la solución: ejercicio 07. Barrendero de cadenas

La bandera es `ARL{sweeper_2_5_1_3_6_4}`: `fn2` es `strlen`, `fn5` es `memchr`, `fn1` es `memcpy`, `fn3` es `memmove`, `fn6` es `memset` y `fn4` es `memcmp`. Cada rutina se identifica por su instrucción de cadena, su prefijo y los registros que prepara antes.

## Parte 1: Reverse

### Las seis rutinas

```text
fn1:  mov rax,rdi            fn2:  xor eax,eax
      mov rcx,rdx                  mov rcx,-1
      shr rcx,3                    repnz scas al,[rdi]
      rep movs QWORD               not rcx
      mov ecx,edx                  lea rax,[rcx-1]
      and ecx,7                    ret
      rep movs BYTE
      ret

fn3:  mov rax,rdi            fn4:  mov rcx,rdx
      mov rcx,rdx                  xor eax,eax
      cmp rdi,rsi                  repz cmps [rsi],[rdi]
      jbe .forward                 je .same
      lea r8,[rsi+rdx]             movzx eax,[rdi-1]
      cmp rdi,r8                   movzx ecx,[rsi-1]
      jae .forward                 sub eax,ecx
      std                          ret
      lea rsi,[rsi+rdx-1]
      lea rdi,[rdi+rdx-1]    fn5:  mov eax,esi
      rep movs BYTE                mov rcx,rdx
      cld                          test rcx,rcx
      ret                          je .none
.forward: rep movs BYTE            repnz scas al,[rdi]
      ret                          jne .none
                                   lea rax,[rdi-1]
fn6:  mov r8,rdi                   ret
      movzx eax,sil
      mov rcx,rdx
      rep stos BYTE
      mov rax,r8
      ret
```

### Identificar cada una

| Rutina | Indicio | Función |
|---|---|---|
| `fn1` | `rep movs` copia `rcx` palabras cuádruples y después los bytes restantes; sin tratamiento de la dirección; devuelve `rdi` | `memcpy` |
| `fn2` | `al` es cero y `rcx` es -1; `repnz scas` recorre hasta el byte cero; `not rcx` y restar uno convierten la cuenta en una longitud | `strlen` |
| `fn3` | Compara las direcciones y, cuando el destino está por encima del origen y se solapa, ejecuta `std`, copia desde el último byte y restaura `cld` | `memmove` |
| `fn4` | `repz cmps` se detiene en la primera diferencia; los dos bytes distintos se restan para dar el resultado | `memcmp` |
| `fn5` | `al` viene del segundo argumento y `rcx` del tercero; `repnz scas` se detiene en una coincidencia; devuelve `rdi - 1` o cero | `memchr` |
| `fn6` | `al` toma el byte del segundo argumento y `rep stos` lo guarda `rcx` veces | `memset` |

Las dos rutinas que usan `repnz scas` se diferencian en con qué comparan y en qué devuelven: `fn2` busca un cero y devuelve una cuenta, `fn5` busca un byte dado por quien llama y devuelve un puntero. Las dos copias se diferencian en el tratamiento de la dirección: solo `fn3` tiene `std` y `cld`. En el orden strlen, memchr, memcpy, memmove, memset, memcmp, los números son 2, 5, 1, 3, 6 y 4.

La bandera es `ARL{sweeper_2_5_1_3_6_4}`.

## Parte 2: Assemble

La solución de referencia es `solution.asm`, con las mismas formas que `fn2`, `fn5`, `fn1` (simplificada a una copia byte a byte) y `fn3`. Los puntos que importan:

- **`my_strlen`.** `al` vale 0, `rcx` vale -1, `repnz scasb`, y después `not rcx` y restar uno.
- **`my_memchr`.** Comprueba si `n` es 0 antes del recorrido, porque con `rcx` a 0 el recorrido no hace nada y el indicador de cero conserva el valor que tuviera. Tras el recorrido, el indicador de cero dice si se encontró una coincidencia.
- **`my_memcpy`.** Guarda `rdi` para el valor de retorno, pon `n` en `rcx` y ejecuta `rep movsb`.
- **`my_memmove`.** Una copia hacia delante es segura cuando el destino no está por encima del origen, o cuando empieza en el final del origen o después. En caso contrario copia hacia atrás con `std` y pon a cero el indicador antes de volver.

## Errores que hacen perder tiempo

- **Copiar siempre hacia delante en `my_memmove`.** Los casos con solapamiento salen mal; fallan 8 de los 74 casos de prueba.
- **Dejar activo el indicador de dirección.** El verificador informa `BAD direction flag left set`. La siguiente instrucción de cadena del llamador se ejecutaría hacia atrás.
- **Copiar demasiados bytes.** Escribir más allá del final del bloque choca con la página inaccesible y la prueba falla con la señal 11.
- **Omitir la comprobación de `n` igual a 0 en `my_memchr`.** La prueba llama a la rutina con el indicador de cero activo y `n` igual a 0; una rutina que confía en el indicador antiguo devuelve un puntero erróneo y falla ese caso.
- **Devolver el puntero un byte más allá.** Tras una coincidencia, `rdi` ya está un byte después del byte coincidente; fallan 11 de los 74 casos.
- **Contar el terminador en `my_strlen`.** El resultado es uno mayor; fallan 16 casos, empezando por la cadena vacía.
- **Llamar a otra rutina.** `call` está prohibido; mantén cada rutina autocontenida.
