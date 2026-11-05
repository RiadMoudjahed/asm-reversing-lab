<!-- source: WRITEUP.md @ sha256:58eca3846a74 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Explicación de la solución: ejercicio 05. Contador de entradas

La bandera es `ARL{ticket_12_1_2}`. El programa acepta como máximo 12 dígitos por número, devuelve el estado de salida 1 para una entrada no válida y devuelve el estado de salida 2 cuando la suma tiene 13 dígitos. Suma los números como texto, dígito a dígito, de derecha a izquierda. La solución de referencia hace lo mismo.

## Parte 1: Reverse

### Experimentos

```text
$ printf '12\n34\n' | bin/ticket_counter ; echo $?
46
0
$ printf '000\n0000\n' | bin/ticket_counter ; echo $?
0
0
$ printf '12a\n5\n' | bin/ticket_counter ; echo $?
ERR
1
$ printf '999999999999\n1\n' | bin/ticket_counter ; echo $?
OVERFLOW
2
$ printf '1234567890123\n1\n' | bin/ticket_counter ; echo $?
ERR
1
```

Los ceros a la izquierda se eliminan (`000` más `0000` imprime `0`). Una entrada de 13 dígitos se rechaza como no válida, lo que sugiere un límite de longitud para cada número. Una suma de dos números de 12 dígitos que necesita 13 dígitos imprime otro mensaje y otro estado. Los experimentos muestran el comportamiento y el desensamblado muestra los límites exactos.

### Lee la rutina de comprobación

```text
parse_line:
  mov    edi,esi
  xor    ecx,ecx
.scan:
  movzx  eax,BYTE PTR [esi]
  cmp    al,0xa                 ; newline: the line ends
  je     .end
  sub    al,0x30                ; '0' is 0x30
  cmp    al,0x9
  ja     fail                   ; not '0'..'9'
  inc    esi
  inc    ecx                    ; count the digits
  jmp    .scan
.end:
  inc    esi
  test   ecx,ecx
  je     fail                   ; empty line
  cmp    ecx,0xc                ; 12
  ja     fail                   ; too long
  ret
```

- **La comprobación del dígito.** `sub al, 0x30` convierte de `0` a `9` en 0 a 9, y cualquier otra cosa en un número mayor que 9 (porque la resta da la vuelta con los caracteres menores que `0`). `cmp al, 9` con `ja` envía todo eso a `fail`.
- **El límite de longitud.** `cmp ecx, 0xc` con `ja fail`: 12 dígitos es la entrada más larga aceptada.
- **Un salto de línea ausente.** Al final de la entrada el búfer contiene un byte cero. No es un salto de línea ni un dígito, así que el recorrido termina en `fail`.

### Los estados de salida

Ambos caminos finales escriben su texto y después cargan `ebx` antes de la llamada de salida (`eax` = 1, `int 0x80`):

```text
overflow:  ...  mov ebx,0x2   -> exit status 2
fail:      ...  mov ebx,0x1   -> exit status 1
```

El estado de una ejecución correcta es 0, con `ebx` a cero. Las respuestas son 12, 1 y 2, y la bandera es `ARL{ticket_12_1_2}`.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. Funciona en cuatro pasos:

1. **Leer** hasta 63 bytes de la entrada estándar con `read` y poner un byte cero a continuación.
2. **Comprobar** cada una de las dos líneas con una rutina como `parse_line`, anotando el inicio y la longitud de cada número.
3. **Sumar** de derecha a izquierda en 13 posiciones. En cada posición toma el dígito de cada número (0 cuando uno se acaba), súmalos con el acarreo, resta 10 y activa el acarreo si la suma es 10 o más, y guarda el carácter en el búfer.
4. **Imprimir** los dígitos sin ceros a la izquierda, o el texto de desbordamiento si la primera de las 13 posiciones no es `0`.

El núcleo de la suma es la aritmética de los dígitos:

```nasm-fragment
    add     eax, ebx                    ; digit of a + digit of b
    add     eax, edx                    ; plus the carry from the previous position
    xor     edx, edx
    cmp     eax, 10
    jb      .store
    sub     eax, 10                     ; 10 or more: keep the low digit
    mov     edx, 1                      ; and carry 1 into the next position
```

## Errores que hacen perder tiempo

- **Convertir los números a enteros.** Un valor de 12 dígitos supera los 32 bits. Suma el texto dígito a dígito.
- **Un límite uno por encima o uno por debajo.** Aceptar 13 dígitos hace fallar 18 de los 80 casos; las pruebas usan entradas de exactamente 12 y exactamente 13 dígitos.
- **No quitar los ceros a la izquierda.** El programa imprime entonces resultados como `0004294967296`; fallan 53 de los 80 casos.
- **El estado de salida equivocado.** Un error que termina con 3 en lugar de 1 hace fallar 21 casos aunque el texto sea correcto.
- **Aceptar una entrada sin salto de línea final.** El original lo trata como un error; las entradas de prueba lo incluyen.
- **Un programa vacío.** El enlazador genera un archivo sin código y el verificador informa de que no pudo iniciarlo.
