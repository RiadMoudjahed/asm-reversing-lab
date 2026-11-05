<!-- source: WRITEUP.md @ sha256:c04d588def8e -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Resolución: ejercicio 16. Biblioteca silenciosa

La bandera es `ARL{silent_16_8_5_13_7}`: el bloque de estado ocupa 16 bytes, el contador de bytes de 64 bits está en el desplazamiento 8, la función de introducción rota 5 bits, la función que lee el resumen rota 13 bits y la función de empaquetado escribe 7 bytes para los cuatro enteros.

## Parte 1: Reverse

### Las exportaciones

```text
$ nm -D --defined-only bin/libsilent.so
0000000000001141 T sl_feed
0000000000001173 T sl_final
000000000000117f T sl_free
000000000000119a T sl_live
0000000000001119 T sl_new
00000000000011a1 T sl_pack
```

Los nombres sugieren un conjunto (`sl_new`, `sl_feed`, `sl_final`, `sl_free`) y dos funciones fuera de él. El conjunto se comprueba con su código, no se acepta por sus nombres.

### El estado: `sl_new`

```text
1119:  push   rbx
111a:  mov    ebx,edi                       ; seed, 32 bits
111c:  mov    esi,0x10
111f:  mov    edi,0x1                       ; calloc(1, 16)
1126:  call   calloc@plt
112b:  test   rax,rax
112e:  je     113f                          ; null stays null
1130:  xor    ebx,0x811c9dc5
1136:  mov    DWORD PTR [rax],ebx           ; first field = seed xor constant
1138:  add    DWORD PTR [rip+0x2edd],0x1    ; a global counter goes up
113f:  pop    rbx
1140:  ret
```

La función toma un valor de 32 bits, reserva un bloque a cero de 16 bytes, guarda la semilla xor 0x811c9dc5 en los cuatro primeros bytes e incrementa una variable global. Así `S` es 16. La global la lee `sl_live` (`mov eax, DWORD PTR [rip+...]`, `ret`) y la decrementa `sl_free`, que además pasa su argumento a `free` tras comprobar que no es nulo. Las dos juntas cuentan los estados abiertos.

### La función de introducción: `sl_feed`

```text
1141:  test   rdx,rdx
1144:  je     116e                          ; no bytes: skip the loop
1146:  mov    r8,rsi
1149:  add    rsi,rdx                       ; end of the buffer
114c:  movzx  eax,BYTE PTR [r8]             ; next byte
1150:  xor    eax,DWORD PTR [rdi]           ; ... xor first field
1152:  imul   eax,eax,0x1000193
1158:  mov    DWORD PTR [rdi],eax           ; first field = (first xor byte) * 16777619
115a:  mov    ecx,DWORD PTR [rdi+0x4]
115d:  rol    ecx,0x5                       ; second field rotated left by 5
1160:  add    eax,ecx
1162:  mov    DWORD PTR [rdi+0x4],eax       ; second field = rol(second, 5) + first
1165:  add    r8,0x1
1169:  cmp    r8,rsi
116c:  jne    114c
116e:  add    QWORD PTR [rdi+0x8],rdx       ; the 64-bit count grows by n
1172:  ret
```

La firma tiene tres argumentos: el estado en `rdi`, el búfer en `rsi` (leído con `movzx ... BYTE PTR`) y la longitud en `rdx`. El estado tiene dos campos de 32 bits en los desplazamientos 0 y 4 que cambian con cada byte, y un campo de 64 bits en el desplazamiento 8 que crece con la longitud total una sola vez, tras el bucle. Así `C` es 8 y `F` es 5. El estado ocupa 4 + 4 + 8 = 16 bytes, lo que concuerda con la reserva.

### El resumen: `sl_final`

```text
1173:  mov    eax,DWORD PTR [rdi+0x4]
1176:  rol    eax,0xd                       ; second field rotated left by 13
1179:  xor    eax,DWORD PTR [rdi+0x8]       ; xor the low 32 bits of the count
117c:  xor    eax,DWORD PTR [rdi]           ; xor the first field
117e:  ret
```

La función lee el estado sin cambiarlo y devuelve un valor de 32 bits: `unsigned sl_final(const struct ctx *)`. Así `G` es 13. Las dos cantidades de rotación están en funciones distintas, que es la trampa de la pregunta.

### La función de empaquetado: `sl_pack`

```text
11f0:  lea    edx,[rcx+rcx*1]               ; v + v
11f3:  sar    ecx,0x1f                      ; 0 or -1 from the sign of v
11f6:  xor    edx,ecx                       ; zigzag value
11f8:  cmp    edx,0x7f
11fb:  jbe    11d0                          ; fits in one byte
...
1218:  add    rax,0x1
121c:  mov    ecx,edx
121e:  or     ecx,0xffffff80                ; low 7 bits with bit 7 set
1221:  mov    BYTE PTR [rdi+rax*1-0x1],cl
1225:  shr    edx,0x7
1228:  cmp    edx,0x7f
122b:  ja     1218
```

Se leen cuatro argumentos: los enteros en `rdi` (leídos con escala de 4 bytes), la cuenta en `rsi`, el búfer de salida en `rdx` (movido a `rdi`) y la capacidad en `rcx` (movida a `r11`). Los caminos de fallo fijan `mov eax,0xffffffff`: un menos uno de 32 bits, así que el tipo de retorno es `int`. Cada valor se transforma primero con `(v + v) xor (v >> 31)` usando un desplazamiento aritmético, de modo que las magnitudes pequeñas de cualquier signo den números pequeños. El número transformado se escribe luego de siete en siete bits, primero los bajos, con el bit 7 activo en cada byte salvo el último. Antes de escribir un valor, el código suma su longitud a los bytes escritos hasta entonces y compara el total con la capacidad (`cmp r11,rsi` / `jb` hacia el camino de fallo); la comprobación es por valor, así que un valor cabe entero o no se escribe, y los anteriores permanecen en el búfer.

Para `B`: 0 se transforma en 0 (un byte), -1 en 1 (un byte), 300 en 600 (dos bytes, porque 600 supera 127 y es menor que 16384) y -70000 en 139999 (tres bytes, porque supera 16383 y es menor que 2097152). El total es 1 + 1 + 2 + 3 = 7. Un programa corto en C que llame a la biblioteca lo confirma: el área de salida contiene `00 01 d8 04 df c5 08`.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. Los puntos que importan:

- **`digest_buffer`.** Cuatro llamadas: `sl_new(seed)`, `sl_feed(handle, p, n)`, `sl_final(handle)`, `sl_free(handle)`. El manejador y los argumentos del búfer viven en registros que debe preservar la función llamada porque tienen que sobrevivir a las llamadas, y el resumen se pasa a uno de esos registros antes de `sl_free` para que el resultado sobreviva también a esa llamada. Tres push y la dirección de retorno mantienen `rsp` alineado.
- **`pack_values`.** No necesita llamadas ni registros que preservar. Para cada valor calcula el número en zigzag, cuenta los bytes que necesita, compara el total con la capacidad y devuelve -1 o escribe los bytes. La comprobación de capacidad usa una comparación sin signo; la longitud se calcula antes de escribir nada, así que un valor que no cabe deja la salida intacta.
- **El paso de zigzag.** El desplazamiento aritmético `sar r10d, 31` convierte el signo en 0 o -1, y el o exclusivo invierte todos los bits de un doble negativo.

## Errores que hacen perder tiempo

- **Olvidar `sl_free`.** Los resultados son correctos y el verificador informa de `handle`.
- **Liberar el manejador dos veces.** El contador de la biblioteca se vuelve negativo o el programa aborta en `free`.
- **Un desplazamiento lógico en el paso de zigzag.** Los valores positivos funcionan. Los negativos se codifican mal.
- **Comprobar la capacidad byte a byte.** El valor que falla se escribe en parte y el área de salida difiere de la de la biblioteca.
- **Un error de uno en la comprobación de capacidad.** Se rechaza un valor que cabe justo, o se escribe un byte de más.
- **Declarar la función de empaquetado como `long`.** El camino de fallo fija `eax` a un menos uno de 32 bits, así que el resultado es un `int`; un prototipo erróneo en tu programa de prueba lo oculta.
- **Perder el resumen al llamar a `sl_free`.** `rax` lo sobrescribe la llamada; el resumen debe guardarse antes.
