<!-- source: WRITEUP.md @ sha256:1cccb2fc3c39 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Resolución: ejercicio 11. Rompecabezas portable

La bandera es `ARL{portable_11_150000000_2410_8d48_4_13}`: el archivo tiene 11 secciones, la base de la imagen es 0x150000000, la RVA del punto de entrada es 0x2410, la RVA 0x12348 está en el desplazamiento 0x8d48 del archivo, se importan cuatro bibliotecas y 13 funciones proceden de `KERNEL32.dll`.

## Parte 1: Reverse

### Las cabeceras

```bash
x86_64-w64-mingw32-objdump -p bin/puzzle.exe | grep -E "^(ImageBase|AddressOfEntryPoint|SectionAlignment|FileAlignment)|DLL Name"
```

```text
AddressOfEntryPoint    0000000000002410
ImageBase              0000000150000000
SectionAlignment       00002000
FileAlignment          00000200
    DLL Name: ADVAPI32.dll
    DLL Name: KERNEL32.dll
    DLL Name: msvcrt.dll
    DLL Name: USER32.dll
```

Las columnas de `objdump -p` están separadas por tabuladores; aquí se muestran con espacios. `AddressOfEntryPoint` es una RVA, así que `R` es `2410` y no la suma con la base de la imagen. `ImageBase` da `I` como `150000000`. Hay cuatro líneas `DLL Name`, así que `D` es 4.

### Las secciones

```bash
python3 ../../modules/12-executable-formats/examples/12_pe_dump.py bin/puzzle.exe
```

```text
name      virtual size  RVA       raw size  raw offset  flags
.text     0x00006cc8    0x002000  0x006e00  0x000400    0x60000060
.data     0x000000d0    0x00a000  0x000200  0x007200    0xc0000040
.rdata    0x00000dc0    0x00c000  0x000e00  0x007400    0x40000040
.ledger   0x00000020    0x00e000  0x000200  0x008200    0x40000040
.pdata    0x00000480    0x010000  0x000600  0x008400    0x40000040
.xdata    0x00000528    0x012000  0x000600  0x008a00    0x40000040
.bss      0x00000da0    0x014000  0x000000  0x000000    0xc0000080
.idata    0x00000780    0x016000  0x000800  0x009000    0xc0000040
.CRT      0x00000060    0x018000  0x000200  0x009800    0xc0000040
.tls      0x00000010    0x01a000  0x000200  0x009a00    0xc0000040
.reloc    0x00000084    0x01c000  0x000200  0x009c00    0x42000040
```

Hay 11 filas, así que `S` es 11. La sección `.ledger` no tiene un nombre estándar; no forma parte de las preguntas.

Para `O`, busca la sección que contiene la RVA 0x12348. `.xdata` empieza en la RVA 0x12000 y su tamaño en bruto es 0x600, así que cubre de 0x12000 a 0x125ff. Aplica la fórmula del módulo 12:

```text
file offset = RVA - section RVA + section raw offset
            = 0x12348 - 0x12000 + 0x8a00
            = 0x348 + 0x8a00
            = 0x8d48
```

### Las importaciones

El directorio de importaciones está en `.idata`. Contando los nombres de cada biblioteca en la tabla de nombres de importación se obtiene para `KERNEL32.dll`:

```text
DeleteCriticalSection, EnterCriticalSection, GetLastError, InitializeCriticalSection,
IsDBCSLeadByteEx, LeaveCriticalSection, MultiByteToWideChar, SetUnhandledExceptionFilter,
Sleep, TlsGetValue, VirtualProtect, VirtualQuery, WideCharToMultiByte
```

Son 13 funciones, así que `K` es 13. El script del módulo 12 imprime solo los seis primeros nombres de cada biblioteca, así que no basta para esta pregunta; sí basta contar las entradas de la tabla o leer `objdump -p`.

La bandera es `ARL{portable_11_150000000_2410_8d48_4_13}`.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. Lo que importa:

- **Leer la tabla de secciones.** Cada entrada ocupa 40 bytes. La dirección virtual está en el desplazamiento 12, el tamaño de datos en bruto en el 16 y el puntero a los datos en bruto en el 20. El script del módulo 12 los desempaqueta en el orden nombre, tamaño virtual, dirección virtual, tamaño en bruto, desplazamiento en bruto.
- **La prueba de pertenencia.** `rva - VirtualAddress` se calcula en 32 bits y se compara con el tamaño de datos en bruto mediante un salto sin signo (`jae`). Si la RVA está por debajo de la sección, la resta se desborda hasta un número grande, así que una sola comparación sustituye a las dos comprobaciones "no está por debajo" y "está por debajo del final".
- **No se usa el tamaño virtual.** Una sección puede tener un tamaño virtual mayor que su tamaño en bruto (los bytes son ceros en memoria). Esas RVA no tienen desplazamiento en el archivo y la función devuelve `0xFFFFFFFF` para ellas.
- **`pe_entry_offset`.** Lee `e_lfanew` en el desplazamiento 0x3c, lo suma al puntero del archivo y comprueba `PE\0\0`. La RVA del punto de entrada está en cabeceras NT + 24 + 16; el número de secciones, en cabeceras NT + 6; el tamaño de la cabecera opcional, en cabeceras NT + 20, y la tabla de secciones empieza en cabeceras NT + 24 + ese tamaño.
- **El marco.** La función hace una llamada, así que guarda `rbx` con `push`, lo que deja `rsp` alineado. Luego reserva 48 bytes: 32 para el espacio de sombra y 16 para una local que guarda la RVA del punto de entrada. La local está por encima del espacio de sombra porque `log_rva` escribe en los 32 bytes situados justo encima de su dirección de retorno.
- **Las llamadas.** `log_rva` destruye los registros volátiles, así que la RVA del punto de entrada se recarga desde la pila para la llamada a `pe_rva_to_offset`, y el puntero a la tabla de secciones se recalcula desde `rbx`.

## Errores que cuestan tiempo

- **Un marco de 40 bytes.** La pila es 8 módulo 16 en la llamada; el verificador informa `align` en cada prueba que llega a `log_rva`.
- **Usar `rbx` sin guardarlo.** Los resultados son correctos y el verificador informa `regs`.
- **Usar `esi` como registro auxiliar.** En la convención System V `rsi` es volátil, pero en la convención de Microsoft no lo es. El verificador informa `regs` en todos los casos.
- **No llamar a `log_rva`, o llamarla con otro valor.** El verificador informa `calls`.
- **Una comparación con signo.** `jge` en lugar de `jae` hace que las RVA por debajo de una sección parezcan estar dentro de ella; fallan los casos "below the first section", "rva zero" y "largest rva".
- **Comparar con el tamaño virtual.** El caso en que la RVA está en la cola rellena de ceros de una sección devuelve un desplazamiento en lugar de `0xFFFFFFFF`.
- **Mantener una local en el espacio de sombra.** `log_rva` la sobrescribe y la RVA del punto de entrada es incorrecta después.
