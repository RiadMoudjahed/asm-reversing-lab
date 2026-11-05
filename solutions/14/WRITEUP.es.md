<!-- source: WRITEUP.md @ sha256:0381fee8b79f -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Resolución: ejercicio 14. Cambiaformas

La bandera es `ARL{shape_4_3_24_16_3_9}`: hay 4 clases, cada vtable tiene 3 funciones, los objetos mayores miden 24 bytes con su último campo en el desplazamiento 16, tres pares de clases comparten una función en la misma ranura, y las cuatro vtables juntas contienen 9 direcciones de función distintas.

## Parte 1: Reverse

### La fábrica y los objetos

La función que gestiona `kind` llama a `operator new` una vez por clase y guarda el puntero a la vtable y los campos. Los cuatro caminos de código, con el argumento de `new` y los almacenamientos que le siguen:

```text
mov    edi,0x10                       ; new(16)
call   _Znwm@plt
mov    QWORD PTR [rax],0x402030       ; vptr
mov    DWORD PTR [rax+0x8],ebx        ; first field

mov    edi,0x10                       ; new(16)
call   _Znwm@plt
mov    DWORD PTR [rax+0x8],ebx
mov    QWORD PTR [rax],0x402058       ; vptr
mov    DWORD PTR [rax+0xc],ebp        ; second field

mov    edi,0x18                       ; new(24)
call   _Znwm@plt
mov    DWORD PTR [rax+0x8],ebx
mov    DWORD PTR [rax+0xc],ebp
mov    QWORD PTR [rax],0x402080       ; vptr
mov    DWORD PTR [rax+0x10],r12d      ; third field

mov    edi,0x10                       ; new(16)
call   _Znwm@plt
mov    DWORD PTR [rax+0x8],ebx
mov    QWORD PTR [rax],0x4020a8       ; vptr
mov    DWORD PTR [rax+0xc],ebp        ; second field
```

Se guardan cuatro punteros a vtable distintos, así que `N` es 4. Tres clases tienen objetos de 16 bytes y una tiene un objeto de 24 bytes, así que `Z` es 24. El objeto de 24 bytes tiene campos en los desplazamientos 8, 12 y 16, y el último está en 16, así que `P` es 16. Todo objeto empieza con el puntero a la vtable en el desplazamiento 0, y el primer campo está en el desplazamiento 8, lo que deja 4 bytes de relleno en los objetos de 16 bytes tras el desplazamiento 12.

Las clases pueden nombrarse por lo que comparten. La clase con los tres campos es la más derivada; la clase cuya vtable comparte con ella la primera función es su padre (tiene dos campos); la raíz es la clase con un solo campo; y la clase restante tiene dos campos y deriva de la raíz.

### Las vtables

Cada puntero guardado apunta 16 bytes dentro de una tabla de `.rodata`. Las dos palabras anteriores son el desplazamiento al inicio del objeto y la información de tipos en tiempo de ejecución que falta, ambas a cero. Las tres palabras a partir del puntero son las funciones virtuales:

```text
vtable pointer   slot 0 (offset 0)   slot 1 (offset 8)   slot 2 (offset 16)
0x402030         0x401272            0x40127a            0x401280
0x402058         0x401286            0x40127a            0x401290
0x402080         0x401286            0x40129a            0x4012ac
0x4020a8         0x4012ba            0x4012c2            0x401280
```

En el bucle principal, tres llamadas virtuales usan `[rax+0x10]`, `[rax+0x8]` y `[rax]` a partir del puntero a la vtable, así que hay tres ranuras y `S` es 3.

Comparando las filas ranura por ranura:

| Ranura | Direcciones iguales | Entre qué tablas |
|---|---|---|
| 0 | 0x401286 | la segunda y la tercera |
| 1 | 0x40127a | la primera y la segunda |
| 2 | 0x401280 | la primera y la cuarta |

Son tres pares de clases con una dirección igual en la misma ranura, así que `E` es 3. Las nueve direcciones distintas son 0x401272, 0x40127a, 0x401280, 0x401286, 0x401290, 0x40129a, 0x4012ac, 0x4012ba y 0x4012c2, así que `F` es 9.

El patrón de lo que se comparte da la jerarquía. La primera tabla es la clase base: su segunda ranura (0x40127a) y su tercera ranura (0x401280) las heredan otras. La segunda tabla comparte la ranura 1 con la base y sobrescribe las ranuras 0 y 2. La tercera tabla comparte la ranura 0 con la segunda y sobrescribe las otras dos, por lo que es una clase derivada de la segunda. La cuarta tabla comparte la ranura 2 con la base y sobrescribe las ranuras 0 y 1. Las clases a las que pertenecen los objetos son, por tanto: la raíz (16 bytes), una derivada con un campo añadido (16 bytes), una derivada de nuevo con otro campo (24 bytes) y una segunda clase derivada de la raíz (16 bytes).

La bandera es `ARL{shape_4_3_24_16_3_9}`.

## Parte 2: Assemble

La solución de referencia es `solution.asm`. Lo que importa:

- **La disposición.** El desplazamiento 0 contiene el puntero a la vtable. El desplazamiento 8 contiene el primer campo de todas las clases. El desplazamiento 12 contiene el segundo campo, que existe en tres clases. El desplazamiento 16 contiene el tercer campo, que solo tiene la clase mayor.
- **El puntero a la vtable.** `gadget_init` llama a `gadget_vptr(kind)` y guarda el resultado en la primera palabra. Los valores de kind son aquellos sobre los que hace switch la función fábrica de `bin/gadgets`, y `gadget_vptr` devuelve el puntero de la clase correspondiente, que es la dirección 16 bytes dentro de su tabla.
- **Campos.** Los argumentos `a`, `b` y `c` van a los desplazamientos 8, 12 y 16, y una clase recibe solo los campos que tiene. Como la llamada a `gadget_vptr` destruye los registros de argumentos, `gadget_init` los conserva en registros que debe preservar la función llamada y apila esos registros primero.
- **La llamada virtual.** `gadget_call` carga el puntero a la vtable con `mov rax, [rdi]`, escala el número de ranura por 8, mueve `arg` a `esi` y llama a `[rax + rcx*8]`. El puntero al objeto sigue en `rdi`, que es `this` para la función llamada. La pila se alinea con un único `sub rsp, 8` porque a la entrada `rsp` es 8 módulo 16.
- **Registros.** Solo se modifican registros que debe preservar la función llamada que se guardaron antes, y se restauran en orden inverso.

## Errores que cuestan tiempo

- **Guardar un campo en un desplazamiento equivocado.** Las llamadas virtuales de la biblioteca leen valores distintos de los que calcula el binario y los resultados de las pruebas difieren.
- **El puntero a la vtable en el lugar equivocado.** El verificador informa `vptr` antes de hacer ninguna llamada, porque una llamada a través de un puntero erróneo se cuelga.
- **Escalar la ranura por 4.** La llamada va a una dirección dentro de una función o a basura, y el programa termina con la señal 11.
- **No mover `arg` a `esi`.** La función llamada recibe lo que hubiera en `esi` y el verificador informa `call`.
- **Destruir un registro que debe preservar la función llamada.** Los resultados son correctos y el verificador informa `regs`.
- **Escribir más allá del objeto.** Un almacenamiento en el desplazamiento 40, fuera de todos los objetos, se informa como `canary`.
