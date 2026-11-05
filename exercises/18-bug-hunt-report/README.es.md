<!-- source: README.md @ sha256:e0ff7aa3c115 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 18. Informe de caza de errores

Level 4. Tiempo: unas 4 horas. Arquitectura: x86-64 Linux. Módulos: [18. Auditar binarios en busca de errores](../../modules/18-auditing-binaries/README.es.md)

## Escenario

Un programa pequeño, `recparse`, lee un archivo de registros de un dispositivo de monitorización e imprime un resumen. Hay que revisarlo antes de usarlo con archivos que vienen de fuera. No se dispone del código fuente. Auditas el binario, encuentras sus defectos de seguridad de memoria y redactas un informe para sus responsables. Después escribes en ensamblador la rutina de análisis corregida, para que los responsables puedan sustituir la defectuosa.

## Objetivos

- Auditar un binario sin símbolos: encontrar los sumideros, rastrear tamaños e índices hasta la entrada y leer cada comprobación.
- Identificar defectos de varias clases y exponer con precisión la causa de cada uno.
- Redactar un informe con el que el responsable del código pueda actuar, sin explotar nada.
- Escribir en ensamblador un analizador que rechace la entrada malformada sin tocar nunca memoria que no debe.

## Parte 1: Reverse

El binario es `bin/recparse`. Lee el archivo indicado en la línea de órdenes e imprime un resumen de los registros que contiene.

```bash
bin/recparse sample.rec
objdump -d -M intel bin/recparse
```

El programa es defectuoso a propósito. Ejecútalo solo con archivos que hayas creado tú, y usa el depurador para detenerlo antes de una operación sospechosa en lugar de dejarlo correr más allá del final de un búfer. Audita el código como describe el módulo 18. El programa tiene seis defectos de seguridad de memoria: uno en la forma de pasar de un registro al siguiente y uno en cada uno de los cinco tipos de registro. Encuéntralos.

Anota estos cinco valores:

1. `N`: la mayor longitud de nombre que acepta el tipo de registro para nombres, en decimal.
2. `V`: el menor byte de longitud, en decimal, para el que la comprobación del tipo de registro para listas de números deja pasar un tamaño imposible.
3. `T`: el mayor valor de etiqueta que acepta el tipo de registro para etiquetas, en decimal.
4. `B`: el menor número de elementos, en decimal, para el que el tamaño calculado en el tipo de registro para bloques de datos deja de coincidir con el número de bytes que se copian después.
5. `U`: el número de tipo, en decimal, del tipo de registro que libera memoria que se vuelve a usar después.

```text
ARL{bughunt_<N>_<V>_<T>_<B>_<U>}
```

Por ejemplo, los valores 8, 200, 4, 32 y 3 darían `ARL{bughunt_8_200_4_32_3}`.

Después redacta un informe. Crea un archivo `REPORT.md` y da a cada uno de los seis defectos una entrada en el formato del módulo 18: título, clase, ubicación, gravedad, causa, prueba, impacto, corrección y verificación. El verificador no lee el informe. La resolución en `solutions/18/` contiene un informe modelo para que lo compares después.

## Parte 2: Assemble

Edita `starter/solution.asm` para implementar una función en la convención de llamada System V:

```c-fragment
struct out {
    char     name[16];
    int64_t  sum;
    uint32_t tags[8];
    uint32_t blob_sum;
    uint32_t pad;
};

long parse_records(const unsigned char *buf, size_t n, struct out *o);
```

El búfer son los `n` bytes en `buf`. La estructura `o` está a cero antes de la llamada. Un registro es un byte de tipo, un byte de longitud y esa cantidad de bytes de carga útil. La función procesa registros uno tras otro hasta encontrar el registro de tipo 0, que termina la lista. Devuelve el número de registros anteriores a ese, o un código de error negativo. Las reglas, en el orden en que se aplican a cada registro:

- Si quedan menos de 2 bytes, o menos de `length` bytes tras los dos bytes de cabecera, el registro está truncado: devuelve -1. Esto vale también para el registro de tipo 0, y un búfer que termina antes de un registro de tipo 0 está truncado.
- El tipo 0 termina la lista. Los bytes posteriores se ignoran, y su carga útil, si la hay, se salta.
- El tipo 1 fija el nombre. Necesita una longitud de 15 como máximo, si no devuelve -2. El campo de nombre pasa a ser exactamente los bytes de la carga útil seguidos de bytes cero hasta completar 16 bytes.
- El tipo 2 suma números a `sum`. La longitud debe ser múltiplo de 4 y de 32 como máximo, si no devuelve -3. Cada grupo de 4 bytes de carga útil es un número de 32 bits con signo, primero el byte bajo, y se suma a `sum`.
- El tipo 3 cuenta etiquetas. Cada byte de la carga útil debe ser 7 como máximo, si no devuelve -4. Por cada byte se incrementa `tags[byte]`.
- El tipo 4 fija un bloque de datos. Necesita una longitud de al menos 1 que sea igual a 1 más 4 veces el valor del primer byte de la carga útil, si no devuelve -5. `blob_sum` pasa a ser la suma de los bytes de la carga útil posteriores al primero, de modo que un bloque posterior sustituye a uno anterior.
- El tipo 5 descarta el bloque de datos. Necesita una longitud 0, si no devuelve -5. `blob_sum` pasa a ser 0.
- Cualquier otro tipo: devuelve -6.

Cuando el valor devuelto es negativo no se examina el contenido de `o`, pero no debe escribirse nada fuera de `o` y el campo `pad` debe seguir a cero.

Las pruebas construyen muchos búferes, de válidos a truncados, dañados y aleatorios, y comparan el valor devuelto y cada campo con los de una referencia.

## Reglas

- No leas nunca un byte fuera de los `n` bytes del búfer, digan lo que digan los registros. Las pruebas colocan el búfer justo antes de una página sin mapear, y una lectura más allá de su final termina la ejecución de las pruebas con una señal.
- No escribas nunca fuera de los 64 bytes de `o`, y deja `pad` a cero.
- Conserva los registros que debe preservar la función llamada: `rbx`, `rbp` y de `r12` a `r15`.
- Mantén la pila alineada en cada `call`.

## Comprueba tu trabajo

```bash
make check EX=18 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la Parte 1 y la Parte 2 por separado. Sin `--flag`, solo comprueba la Parte 2. Cada resultado de una prueba de la Parte 2 termina con `ok` o con una lista de lo que salió mal: `overrun` o `regs`.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
