<!-- source: WRITEUP.md @ sha256:daf1bb341543 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Análisis resuelto: ejercicio 19. Mesa de triaje

El flag es `ARL{triage_6_34_1543003_1574002_1071004}`: la configuración tiene 6 entradas codificadas, la entrada decodificada más larga tiene 34 bytes, y los tres comportamientos corresponden a T1543.003 (servicio de Windows), T1574.002 (carga lateral de DLL) y T1071.004 (DNS).

## Parte 1: Reverse

### Identificación y cadenas

Ambos archivos son pequeños y no tienen símbolos. `file` indica un ELF de x86-64 para `dispatch` y un ejecutable de consola PE32+ para `dispatch.exe`. Ejecutar la versión de Linux es seguro, porque el programa no llama a nada que tenga efecto:

```text
[sim] simulator: no action is performed, the steps below are only printed
[sim] would query the list of running processes
[sim] would create the service <cfg 0> that runs <cfg 1>
[sim] would place the library <cfg 2> next to the program <cfg 3>
[sim] would resolve names under <cfg 4> and carry data in the labels
[sim] would clear the event log <cfg 5>
```

La salida responde a la pregunta del comportamiento al nivel de los verbos, y las cadenas del archivo confirman que estas plantillas son el único texto legible: `strings -n 6` muestra las mismas seis líneas con `%d` donde la salida tiene los números. Faltan los datos interesantes: los nombres del servicio y de los archivos, el dominio y el registro. No hay otras cadenas legibles ni ninguna importación relacionada con servicios, archivos o redes: la versión de Linux solo importa `puts` y `__printf_chk`, y la de Windows solo importa las funciones de arranque y de memoria que necesita el entorno de ejecución del compilador de `KERNEL32.dll` y `msvcrt.dll`, además de `puts`. Por tanto, los datos deben estar dentro del archivo en una forma que `strings` no puede leer.

### La tabla y el decodificador

El marcador `<cfg %d>` se rellena a partir de un arreglo de seis ranuras de 64 bytes cada una en memoria escribible. El código que rellena el arreglo es lo primero que hace la función principal después de la primera línea:

```text
401178:  mov    ebx,0x404040            ; destination: six slots of 0x40 bytes
40117d:  mov    r12d,0x4041c0           ; end of the destination
401183:  mov    ebp,0x4021a0            ; source: the table in .rodata
401188:  mov    rsi,rbx
40118b:  mov    rdi,rbp
40118e:  call   401136                  ; decode one entry, returns its length
401193:  lea    rbp,[rbp+rax*1+0x2]     ; next entry = this one + length + 2
401198:  add    rbx,0x40
40119c:  cmp    rbx,r12
40119f:  jne    401188
```

El bucle es la clave de la estructura. El destino va de 0x404040 a 0x4041c0, es decir, 0x180 bytes o seis ranuras de 0x40. El puntero de origen avanza el valor que devuelve la función más 2, de modo que cada entrada de la tabla es una longitud seguida de un byte más y luego esa cantidad de bytes. El número `N` es 6.

La función en 0x401136 es el decodificador:

```text
401136:  movzx  r8d,BYTE PTR [rdi]        ; length
40113b:  movzx  edx,BYTE PTR [rdi+0x1]    ; seed: the initial state
40113f:  test   r8,r8
401141:  je     401161                    ; length 0: nothing to decode
401143:  mov    eax,0x0                   ; position i = 0
401148:  rol    dl,0x3                    ; state = rotate left by 3
40114b:  lea    edx,[rdx+rax*1+0x5d]      ; state = state + i + 0x5d
40114f:  mov    ecx,edx
401151:  xor    cl,BYTE PTR [rdi+rax*1+0x2]  ; plain = encoded xor state
401155:  mov    BYTE PTR [rsi+rax*1],cl
401158:  add    rax,0x1
40115c:  cmp    r8,rax
40115f:  jne    401148
401161:  mov    BYTE PTR [rsi+r8*1],0x0   ; terminating zero byte
401166:  mov    rax,r8
401169:  ret
```

En C la misma función se lee así. El `lea` con tres términos es la suma de la posición y la constante, y solo importa el byte bajo del resultado, porque el estado se usa a través de `dl` y `cl`:

```c-fragment
size_t decode_record(const unsigned char *rec, char *out)
{
    size_t len = rec[0];
    unsigned char k = rec[1];
    for (size_t i = 0; i < len; i++) {
        k = (unsigned char)(rol8(k, 3) + i + 0x5d);
        out[i] = rec[2 + i] ^ k;
    }
    out[len] = 0;
    return len;
}
```

Un esquema con clave fija, como el del módulo 20, no decodificaría esta tabla: la clave cambia con la posición, y cada entrada parte de su propia semilla. Un primer intento que aplique xor a cada byte con una constante produce ruido en todas las entradas, y probar las 256 constantes no encuentra nada que se lea como texto. Reconocer la rotación es lo que lo resuelve.

### Decodificación

Con la regla en un script, o en el emulador del módulo 20, las seis entradas se decodifican como:

| Ranura | Longitud | Semilla | Texto |
|---|---|---|---|
| 0 | 13 | 0x13 | WinUpdSvcHost |
| 1 | 34 | 0xA7 | C:\ProgramData\Intel\svchost32.exe |
| 2 | 11 | 0x5C | version.dll |
| 3 | 19 | 0x2E | OneDriveUpdater.exe |
| 4 | 22 | 0xD1 | ns1.cdn-telemetry.test |
| 5 | 8 | 0x66 | Security |

El texto más largo es la ruta de la ranura 1, con 34 bytes: `L` es 34. El texto decodificado solo existe en memoria después de que se ejecute el bucle, y por eso ni `strings` ni una búsqueda en el archivo lo encuentran. Un depurador que se detenga después del bucle muestra los mismos valores en 0x404040.

### Comportamientos y técnicas

Cada plantilla, rellenada con los valores decodificados, describe un comportamiento:

| Plantilla | Con los valores decodificados | ATT&CK |
|---|---|---|
| query the list of running processes | enumeración del sistema | T1057 Process Discovery |
| create the service `<cfg 0>` that runs `<cfg 1>` | un servicio llamado WinUpdSvcHost ejecuta un programa en una carpeta del fabricante | T1543.003 Create or Modify System Process: Windows Service |
| place the library `<cfg 2>` next to the program `<cfg 3>` | version.dll junto a OneDriveUpdater.exe, que el programa carga primero desde su propia carpeta | T1574.002 Hijack Execution Flow: DLL Side-Loading |
| resolve names under `<cfg 4>` and carry data in the labels | nombres bajo un dominio del operador, con datos en las etiquetas del subdominio | T1071.004 Application Layer Protocol: DNS |
| clear the event log `<cfg 5>` | el registro Security | T1070.001 Indicator Removal: Clear Windows Event Logs |

El flag pide las tres técnicas del medio. Para el informe hay dos cautelas. El programa es un simulador: imprime lo que haría, y la asociación describe el comportamiento que modela, no un comportamiento observado en un sistema. Y un identificador de técnica se deduce del comportamiento y no de un nombre en una cadena: el nombre de servicio WinUpdSvcHost está elegido para parecer un componente del sistema, lo cual es un punto para el informe, pero T1543.003 se sustenta en la creación misma del servicio.

De ahí resulta `ARL{triage_6_34_1543003_1574002_1071004}`: `N` es 6, `L` es 34, y los identificadores sin la `T` y sin el punto son 1543003, 1574002 y 1071004.

### La regla de detección

El archivo no contiene texto decodificado, así que la regla no puede usarlo. Lo que sí contiene el archivo son las seis plantillas de registro en texto claro y los primeros bytes de la primera entrada codificada. La regla de referencia está en `solutions/19/rule.yar`:

```text
rule triage_desk_dispatch
{
    strings:
        $service  = "would create the service <cfg" ascii
        $sideload = "would place the library <cfg" ascii
        $dns      = "would resolve names under <cfg" ascii
        $clear    = "would clear the event log <cfg" ascii
        $discover = "would query the list of running processes" ascii
        $record0  = { 0D 13 A2 64 A9 CB 25 68 90 F4 1A 79 9F 9C 9C }
    condition:
        (uint32(0) == 0x464c457f or uint16(0) == 0x5a4d) and
        3 of ($service, $sideload, $dns, $clear, $discover) and $record0
}
```

El razonamiento sigue a los señuelos, que son la parte del ejercicio que pone a prueba la regla:

- `notes` imprime un texto de ayuda que menciona una clave Run, un servicio y `version.dll`. Una regla sobre esas palabras se activa con él. La muestra no las contiene en texto claro.
- `dryrun` imprime líneas de la forma `[dry-run] would ...`. Una regla sobre la palabra `would`, o sobre un fragmento corto como `would create`, se activa con él. Las plantillas de la muestra llevan el marcador `<cfg`, que la ejecución simulada no tiene.
- `updater` contiene la misma rutina de decodificación y una tabla codificada con textos inofensivos. Una regla sobre los bytes del decodificador se activa con él. La primera entrada codificada de la muestra no aparece en el actualizador.
- `/bin/ls` representa los programas con los que nunca debería activarse nada.

La comprobación del tipo de archivo hace que la regla solo se ejecute sobre ejecutables. No hace falta para separar los señuelos, y el verificador confirma que una regla sin ella también se acepta, pero es el hábito que mantiene rápida una regla en colecciones grandes. Los primeros bytes de la primera entrada son la única parte de la regla que depende de la compilación exacta; una variante de la muestra con otra tabla necesita una regla nueva, que es el estado normal de la detección y no un defecto de esta. Una regla que solo necesite las plantillas, por ejemplo `all of them` sobre las cuatro más largas, también es correcta.

## Parte 2: Assemble

El decodificador de la muestra, con las dos comprobaciones de la especificación añadidas, es la función completa. La solución de referencia está en `solutions/19/solution.asm`:

```nasm-fragment
decode_record:
    cmp     rsi, 2
    jb      .truncated                  ; no room for the length and the seed
    movzx   r8d, byte [rdi]             ; length
    lea     r9, [r8 + 2]
    cmp     r9, rsi
    ja      .truncated                  ; the record is cut off
    lea     r9, [r8 + 1]
    cmp     r9, rcx
    ja      .small                      ; text and zero byte do not fit
    movzx   r10d, byte [rdi + 1]        ; k = seed
    xor     r11d, r11d                  ; i = 0
.next:
    cmp     r11, r8
    jae     .done
    rol     r10b, 3
    add     r10b, r11b                  ; plus i (low byte)
    add     r10b, 0x5d
    mov     al, [rdi + r11 + 2]
    xor     al, r10b
    mov     [rdx + r11], al
    inc     r11
    jmp     .next
```

El orden de las comprobaciones importa. Una entrada que está cortada y además es demasiado larga para `cap` devuelve -1, porque la primera comprobación se aplica primero, y las pruebas incluyen entradas así. La prueba de `avail` menor que 2 va antes de la lectura del byte de longitud, ya que la longitud forma parte de la propia entrada. Hacer la comparación con `avail` como `length + 2 > avail` en un registro de 64 bits evita cualquier desbordamiento, porque la longitud es como máximo 255.

## Errores que cuestan tiempo

- Decodificar con una clave fija. La rotación es fácil de pasar por alto como un "paso de mezcla"; el estado que cambia en cada posición es lo que hace fallar a una clave constante.
- Olvidar que la posición se suma al estado en cada paso, incluido el primero. Empezar la posición en 1, o sumarla después del xor, da un texto erróneo desde el primer byte.
- Usar el registro de 64 bits para el estado. El estado es un byte: la rotación y las sumas deben dar la vuelta a los 8 bits. Las pruebas usan muchas semillas y longitudes distintas, que ponen en evidencia un estado que no da la vuelta.
- Una regla construida sobre una palabra común o solo sobre el decodificador. Los señuelos existen por eso. El verificador nombra los programas con los que la regla da la respuesta equivocada.
- Comprobar `cap` antes que `avail`. La especificación fija el orden, y las entradas cortadas con un `cap` pequeño lo ponen de manifiesto.
- Fiarse del nombre del servicio. El nombre WinUpdSvcHost pretende parecer parte del sistema; la técnica se deduce de lo que hace el programa.
