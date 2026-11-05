<!-- source: WRITEUP.md @ sha256:abe06d06e270 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Resolución: ejercicio 13. Números mágicos

La bandera es `ARL{magic_12_9_60_11_5_3}`: la puntuación es `(u / 12) * 5 + (u % 9) * 3 - (x / 60) + (x % 11) * 4`, donde `u` es `x` leído sin signo; las dos primeras operaciones son sin signo y las dos últimas con signo.

## Parte 1: Reverse

### El código

```bash
objdump -d -M intel --no-show-raw-insn bin/magic
```

```text
0000000000401190 <mix>:
  401190:    mov    edx,edi
  401192:    mov    ecx,0xaaaaaaab
  401197:    mov    rax,rdx
  40119a:    imul   rcx,rdx
  40119e:    imul   rdx,rdx,0x38e38e39
  4011a5:    mov    edi,eax
  4011a7:    sar    edi,0x1f
  4011aa:    shr    rcx,0x23
  4011ae:    shr    rdx,0x21
  4011b2:    lea    esi,[rcx+rcx*4]
  4011b5:    lea    ecx,[rdx+rdx*8]
  4011b8:    mov    edx,eax
  4011ba:    sub    edx,ecx
  4011bc:    lea    edx,[rdx+rdx*2]
  4011bf:    add    esi,edx
  4011c1:    movsxd rdx,eax
  4011c4:    imul   rcx,rdx,0xffffffff88888889
  4011cb:    imul   rdx,rdx,0x2e8ba2e9
  4011d2:    shr    rcx,0x20
  4011d6:    add    ecx,eax
  4011d8:    sar    rdx,0x21
  4011dc:    sar    ecx,0x5
  4011df:    sub    edx,edi
  4011e1:    sub    ecx,edi
  4011e3:    sub    esi,ecx
  4011e5:    lea    ecx,[rdx+rdx*4]
  4011e8:    lea    edx,[rdx+rcx*2]
  4011eb:    sub    eax,edx
  4011ed:    lea    eax,[rsi+rax*4]
  4011f0:    ret
```

El compilador entrelazó los cuatro cálculos, así que léelos siguiendo cada constante.

### Las cuatro divisiones

Cada división es un `imul` con una constante grande seguido de uno o más desplazamientos a la derecha. En cada una, el divisor es `2^shift / constant`, redondeado. El script del módulo 14 hace la cuenta y la comprueba:

```bash
python3 14_magic.py 0xaaaaaaab 35
python3 14_magic.py 0x38e38e39 33
python3 14_magic.py 0x88888889 37 signed
python3 14_magic.py 0x2e8ba2e9 33 signed
```

```text
2**35 / 2863311531 = 11.9999999986  ->  divisor 12
checked 200007 inputs, 0 differences
2**33 / 954437177 = 8.9999999990  ->  divisor 9
checked 200007 inputs, 0 differences
2**37 / 2290649225 = 59.9999999878  ->  divisor 60
checked 200007 inputs, 0 differences
2**33 / 780903145 = 10.9999999962  ->  divisor 11
checked 200007 inputs, 0 differences
```

| División | Indicio | Divisor |
|---|---|---|
| Cociente sin signo | `0xaaaaaaab` (un valor de 32 bits en `ecx`, así que la multiplicación es sin signo) y `shr rcx, 0x23`, que es 35 | 12 |
| Resto sin signo | `0x38e38e39` y `shr rdx, 0x21`, que es 33; el cociente se multiplica luego por 9 con `lea ecx,[rdx+rdx*8]` y se resta de la entrada | 9 |
| Cociente con signo | `0xffffffff88888889` (la constante de 32 bits 0x88888889 con el bit alto activado, por eso se muestra extendida con signo), `shr rcx, 0x20`, el `add ecx, eax` que vuelve a sumar `x`, y `sar ecx, 5`; el desplazamiento total es 32 + 5 = 37 | 60 |
| Resto con signo | `movsxd`, `0x2e8ba2e9` y `sar rdx, 0x21`, que es 33; el cociente se multiplica por 11 con dos instrucciones `lea` y se resta de `x` | 11 |

El cociente con signo muestra una variante que la lección no detalla. El número mágico para 60 no cabe en un valor positivo de 32 bits, así que el compilador multiplica por el valor de 32 bits leído como negativo y suma `x` de nuevo tras tomar la mitad alta: `(x * (M - 2^32)) >> 32` más `x` es igual a `(x * M) >> 32`. El script usa `M` leído como positivo, que es el valor que necesita la fórmula del divisor.

El `sar edi, 0x1f` cerca del principio calcula una vez el signo de `x` (0 o -1). Ambos cocientes con signo lo restan (`sub edx, edi` y `sub ecx, edi`), que es la corrección para entradas negativas. Los términos sin signo parten de `mov edx, edi` y `mov rax, rdx`, que extienden `x` con ceros; los términos con signo usan `movsxd`.

### Los factores

Tras calcular el cociente sin signo en `rcx`, `lea esi, [rcx+rcx*4]` lo multiplica por 5, así que `K` es 5. Tras el resto `u - q * 9`, `lea edx, [rdx+rdx*2]` lo multiplica por 3, así que `M` es 3. Los dos resultados se suman (`add esi, edx`). Después se resta el cociente con signo (`sub esi, ecx`), y el resto con signo se multiplica por 4 y se suma en el `lea eax, [rsi+rax*4]` final.

La fórmula es `(u / 12) * 5 + (u % 9) * 3 - (x / 60) + (x % 11) * 4`, y la bandera es `ARL{magic_12_9_60_11_5_3}`.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. Lo que importa:

- **Cociente sin signo.** `mov eax, edi` extiende `x` con ceros en `rax`; un `imul` de 64 bits con `0xAAAAAAAB` da el producto completo, y `shr rax, 35` es el cociente. `lea r8d, [rax+rax*4]` lo multiplica por 5.
- **Resto sin signo.** El cociente por 9 sale de `0x38E38E39` y un desplazamiento de 33. El resto es `u` menos cociente por 9 (`lea eax, [rax+rax*8]`), y `lea edx, [rdx+rdx*2]` lo multiplica por 3.
- **Cociente con signo.** `movsxd rax, edi` extiende con signo. La constante `0x88888889` se carga con `mov edx, 0x88888889`, que extiende con ceros y por tanto la mantiene positiva, y el producto de 64 bits se desplaza 37 a la derecha. Como `sar` redondea hacia menos infinito, se suma 1 para entradas negativas, lo que hace `sub edx, ecx` con `ecx = x >> 31`.
- **Resto con signo.** La misma corrección de signo se aplica al cociente `(x * 0x2E8BA2E9) >> 33`, y `q * 11` es `q + q*5*2`, calculado con dos instrucciones `lea`.
- **Combinación.** El último `lea` suma `d * 4` a la suma acumulada, que ya contiene los tres primeros términos.

## Errores que cuestan tiempo

- **Sin corrección de signo.** Sin la resta de `x >> 31`, un cociente con signo es una unidad menor en magnitud para entradas negativas: la puntuación se desvía en 1 cuando falta la corrección en `x / 60`, y en 44 (cuatro veces el divisor 11) cuando falta en `x % 11`.
- **Un desplazamiento erróneo.** `shr rax, 34` en lugar de 35 cambia el cociente para toda entrada desde 6.
- **Extender con ceros los términos con signo.** `mov eax, edi` en lugar de `movsxd` da resultados erróneos para toda entrada negativa.
- **Leer la constante como negativa.** El caso 60 necesita la constante como el positivo 2290649225 cuando no vuelves a sumar `x`.
- **Usar `div`.** El verificador la rechaza aunque los resultados sean correctos.
