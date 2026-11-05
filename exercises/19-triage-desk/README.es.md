<!-- source: README.md @ sha256:091712ebb014 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Ejercicio 19. Mesa de triaje

Nivel 5. Tiempo: unas 5 horas. Arquitectura: x86-64 Linux y Windows. Módulos: [19. Analizar programas sospechosos](../../modules/19-analyzing-suspicious-programs/README.es.md), [20. Leer código ofuscado](../../modules/20-reading-obfuscated-code/README.es.md)

## Escenario

El equipo de seguridad de una pequeña empresa ha recibido, a raíz de una alerta en un puesto de trabajo, un programa llamado `dispatch`, junto con una versión para Windows del mismo programa, `dispatch.exe`. Nadie sabe qué hace. Hay que evaluarlo antes de que acabe el día: ¿qué hace, cómo puede el equipo encontrar otras copias y cómo puede leer su configuración sin ejecutar nada arriesgado? El programa de este ejercicio es un simulador escrito para el curso. Solo imprime lo que haría y no realiza ninguna acción. Tú escribes las notas de triaje y la regla de detección, y después la rutina que decodifica su configuración.

## Objetivos

- Evaluar un programa desconocido en un orden fijo: identificación, cadenas, importaciones, comportamiento y decodificación de datos ocultos.
- Encontrar una configuración codificada, recuperar la regla de decodificación a partir del código y decodificar cada entrada.
- Asociar el comportamiento observado a identificadores de técnicas de ATT&CK y indicar qué respaldan las pruebas.
- Escribir una regla YARA que encuentre la muestra y no se active con programas que solo se le parecen.
- Reimplementar el decodificador en ensamblador para poder aplicarlo a configuraciones de otras muestras.

## Parte 1: Reverse

Los archivos son `bin/dispatch` (Linux) y `bin/dispatch.exe` (Windows). Imprimen unas pocas líneas de registro y terminan. Ejecutarlos es seguro, y es por donde debes empezar, pero las líneas de registro solo contienen marcadores en lugar de los datos interesantes: los datos mismos están almacenados codificados y solo se decodifican en memoria.

```bash
bin/dispatch
file bin/dispatch bin/dispatch.exe
strings -n 6 bin/dispatch
```

Sigue el triaje del módulo 19 y los métodos de decodificación del módulo 20. Encuentra la tabla que contiene las entradas codificadas, recupera la regla de decodificación a partir del código y decodifícalas todas. Después prepara estos valores y archivos:

1. `N`: el número de entradas de la configuración codificada, en decimal.
2. `L`: la longitud en bytes de la entrada decodificada más larga, en decimal.
3. Los identificadores de técnicas de ATT&CK de tres comportamientos, cada uno escrito como identificador completo de la técnica o subtécnica (una `T`, cuatro dígitos, un punto y tres dígitos), pero sin la `T` y sin el punto en el flag: la creación de un servicio, la colocación de una biblioteca junto a un programa de confianza para que el programa la cargue, y el uso de nombres DNS como portador de datos.
4. Una regla YARA en `starter/rule.yar`. Debe coincidir con `bin/dispatch` y con `bin/dispatch.exe`, y no debe coincidir con los demás programas que prueba el verificador: tres programas que mencionan los mismos temas de forma inofensiva (un texto de ayuda, una ejecución simulada, un actualizador que usa la misma rutina de decodificación para textos inofensivos) en ambos formatos, y un programa del sistema. Una regla que se active con cualquiera de ellos inundaría de falsas alarmas a los analistas.

El flag combina los tres primeros valores en este orden: `N`, `L` y luego los tres identificadores (servicio, carga lateral, DNS) sin la `T` y sin el punto.

```text
ARL{triage_<N>_<L>_<service>_<sideload>_<dns>}
```

Los valores 3 y 20 y los identificadores T1234.567, T1234.568 y T1234.569 darían, por ejemplo, `ARL{triage_3_20_1234567_1234568_1234569}`.

Las líneas de registro describen además dos comportamientos más que debes asociar en tus notas, aunque el flag no los pida: cómo averigua el programa cosas sobre el sistema que lo rodea y qué hace con los rastros que deja. Escribir las notas forma parte del ejercicio. El verificador no las lee; el análisis resuelto en `solutions/19/` contiene un modelo con el que compararte después.

## Parte 2: Assemble

Edita `starter/solution.asm` para implementar una función con la convención de llamada System V:

```c-fragment
long decode_record(const unsigned char *rec, size_t avail, char *out, size_t cap);
```

`rec` apunta a una entrada codificada con la estructura que encontraste en la parte 1, y `avail` es el número de bytes que puedes leer a partir de `rec`. La función escribe el texto decodificado en `out`, seguido de un byte cero, y devuelve la longitud del texto. Las reglas, en el orden en que se aplican:

- Si `avail` es menor que 2, o menor que 2 más la longitud almacenada en el primer byte de la entrada, la entrada no está completa dentro de los bytes que puedes leer: devuelve -1 y no escribas nada.
- Si `cap` es menor que la longitud del texto más uno, la salida no cabe: devuelve -2 y no escribas nada.
- En caso contrario, decodifica la entrada exactamente como lo hace el simulador, escribe el texto y el byte cero final, y devuelve la longitud del texto. Una entrada de longitud 0 da un texto vacío, el byte cero y el valor de retorno 0.

Las pruebas incluyen las entradas de la muestra, las mismas entradas cortadas en cada posición, entradas con longitudes y semillas en los extremos, y entradas aleatorias. Comparan el valor de retorno y la salida con los de una referencia.

## Reglas

- No leas nunca un byte en `rec + avail` ni más allá. Las pruebas colocan la entrada justo antes de una página no mapeada, y una lectura más allá del final termina la ejecución de las pruebas con una señal.
- No escribas nunca más allá de los `cap` bytes de `out`, y no escribas nada cuando devuelvas un error.
- Conserva los registros que debe preservar el llamado: `rbx`, `rbp` y de `r12` a `r15`.
- Mantén la pila alineada en cada `call`.

## Comprueba tu trabajo

```bash
make check EX=19 ARGS='--flag ARL{your_answer_here}'
```

El verificador informa de la parte 1 y de la parte 2 por separado. La parte 1 solo se supera cuando el flag es correcto y la regla YARA de `starter/rule.yar` es correcta; la salida del verificador nombra cada programa con el que la regla da la respuesta equivocada. Sin `--flag`, solo comprueba la parte 2.

## Pistas

Abre [HINTS.md](HINTS.es.md) un nivel cada vez.
