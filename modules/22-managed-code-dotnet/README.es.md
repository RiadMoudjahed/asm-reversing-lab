<!-- source: README.md @ sha256:af16eef57f28 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 22. Código administrado: .NET

Tiempo: unas 2 horas. Requisitos previos: [21. Máquinas virtuales y bytecode](../21-virtual-machines-bytecode/README.es.md)

## Por qué importa

Gran parte del software que encontrarás no está compilada a código máquina. Los programas escritos para .NET se compilan a un lenguaje intermedio que un entorno de ejecución traduce mientras el programa se ejecuta. Un archivo así conserva la mayor parte de lo que escribió el autor: los nombres de tipos, métodos y campos, las firmas y la estructura del código. Eso hace que el código administrado sea mucho más fácil de leer que el nativo, y es la razón por la que un descompilador suele devolver un código fuente cercano al original. También significa que los hábitos del análisis nativo no se trasladan sin cambios: no hay registros que seguir ni marcos de pila que reconstruir, y la evidencia útil está en tablas de metadatos y en un conjunto de instrucciones de una máquina de pila. Este módulo enseña cómo se organiza un programa administrado, cómo leer sus metadatos y su código intermedio, cómo usar un descompilador y qué hace bien y mal ese descompilador. Todos los ejemplos son programas inofensivos escritos para la lección.

## Conceptos

### Código administrado y código nativo

La tabla muestra qué cambia cuando el objetivo es administrado.

| | Código nativo | Código administrado (.NET) |
|---|---|---|
| Qué contiene el archivo | Instrucciones máquina para un procesador | Lenguaje intermedio (IL, también llamado CIL) y metadatos, para cualquier procesador |
| Quién lo ejecuta | El procesador, después de que el cargador mapea el archivo | Un entorno de ejecución que carga el archivo, compila la IL a código máquina la primera vez que se ejecuta un método (compilación justo a tiempo) y ejecuta el resultado |
| Nombres | Eliminados al quitar los símbolos | Conservados en los metadatos, porque el entorno de ejecución los necesita para tipos, métodos y campos |
| Tipos | Recuperados mediante análisis | Registrados: cada método tiene una firma con tipos, cada variable local tiene un tipo |
| Memoria | La administra el programa | La administra el entorno de ejecución (un recolector de basura); no hay liberación explícita |
| Lectura | Desensamblador y después razonar sobre registros y pila | Explorador de metadatos, listado de IL y un descompilador que funciona bien |

Algunos programas administrados se compilan por adelantado a código máquina, y algunos contienen partes nativas a las que llaman. Esos casos requieren los métodos nativos de los módulos anteriores. Este módulo trata el caso común de un ensamblado de IL puro.

### Ensamblados y el contenedor del archivo

Un programa o biblioteca de .NET es un ensamblado. En Windows es un archivo PE como cualquier otro, con una pequeña adición: un puntero a una cabecera CLR, que contiene la versión del entorno de ejecución y la ubicación de los metadatos. El archivo es fácil de reconocer en el triaje:

```bash
make
file build/Parcels.dll
python3 22_meta.py build/Parcels.dll
```

```text
build/Parcels.dll: PE32 executable (console) Intel 80386 Mono/.Net assembly, for MS Windows, 3 sections
CLR header: rva 0x2008, size 72
  runtime version 2.5, flags 0x1, entry point token 0x06000006
metadata: signature BSJB, version string v4.0.30319
  stream #~       offset 0x0006c  size 816
  stream #Strings offset 0x0039c  size 892
  stream #US      offset 0x00718  size 184
  stream #GUID    offset 0x007d0  size 16
  stream #Blob    offset 0x007e0  size 332
table rows: Module 1, TypeRef 19, TypeDef 8, Field 8, MethodDef 8, Param 7, MemberRef 19, CustomAttribute 7, Assembly 1, AssemblyRef 2
user strings (#US heap):
  +0x002  ' parcel, '
  +0x016  ' g, cost '
  +0x02a  'NE'
  +0x030  'NW'
  +0x036  'SE'
  +0x03c  'north'
  +0x048  'south'
  +0x054  'east'
  +0x05e  'west'
  +0x068  'northeast'
  +0x07c  'northwest'
  +0x090  'southeast'
  +0x0a4  'unknown'
  +0x0b4  ','
```

Las señales de un ensamblado administrado son: `file` dice `Mono/.Net assembly`, la tabla de importaciones tiene una única entrada de `mscoree.dll` (`_CorExeMain` para un programa), el directorio de datos número 14 no está vacío, y los metadatos empiezan con la firma `BSJB` seguida de una cadena de versión. El script `examples/22_meta.py` lee esas estructuras con una biblioteca de PE y nada más.

### Metadatos

Los metadatos son un conjunto de tablas y montículos (los flujos de la salida anterior). Las tablas describen lo que contiene el ensamblado:

| Tabla | Contiene | Filas en el ejemplo |
|---|---|---|
| TypeDef | Los tipos definidos aquí: clases, estructuras, enumeraciones | 8 |
| MethodDef | Los métodos definidos aquí, con su firma y la ubicación de su IL | 8 |
| Field | Los campos definidos aquí, con sus tipos | 8 |
| TypeRef, MemberRef | Tipos y miembros que el ensamblado usa de otros ensamblados | 19 y 19 |
| AssemblyRef | Los otros ensamblados a los que hace referencia | 2 |
| CustomAttribute | Atributos asociados a tipos y métodos, muchos añadidos por el compilador | 7 |

Los montículos contienen los datos a los que apuntan las tablas. `#Strings` contiene nombres (de tipos, métodos y campos) en UTF-8, de modo que una ejecución sencilla de `strings` los encuentra, como encontró `WriteLine`, `AppendFormatted` y `<>c__DisplayClass1_0` en el ejemplo. `#US` contiene los literales de cadena del programa, las cadenas de usuario, en UTF-16, y por eso los literales de un programa administrado no aparecen en la salida de `strings` con su configuración por defecto. Usa `strings -el` para ver texto UTF-16, o lee el montículo con el script como arriba. `#Blob` contiene firmas y datos constantes, y `#GUID` contiene identificadores.

Tres consecuencias importan para el análisis. Primero, las referencias a otros ensamblados muestran lo que el programa puede hacer: una referencia a las bibliotecas de red, de archivos o de procesos es el equivalente administrado de una tabla de importaciones, y la lista de filas `MemberRef` nombra los métodos exactos a los que se llama. Segundo, como los nombres son datos de `#Strings`, un programa que quiere ocultar su propósito renombra sus tipos y métodos con nombres cortos sin significado; esto cambia cómo se lee, pero no lo que hace. Tercero, el compilador añade muchas cosas que el autor no escribió, y sus nombres lo delatan: un nombre que empieza por `<` o contiene `>` es generado, por ejemplo `<>c__DisplayClass1_0` y `<Surcharges>b__0` en el ejemplo, que son la clase auxiliar y el método de una lambda.

### El lenguaje intermedio

IL es el conjunto de instrucciones de una máquina de pila, lo que lo acerca a la máquina del módulo 21. Los operandos se apilan en una pila de evaluación, una instrucción toma sus entradas de la cima de la pila y apila su resultado, y las instrucciones no tienen operandos de registro. Los argumentos y las variables locales viven en ranuras numeradas y se mueven a la pila y de vuelta. El código del método `Cost` del programa de ejemplo empieza así (el listado completo se obtiene en la sección siguiente):

```text
IL_0000: ldarg.0
IL_0001: ldfld int32 Parcels.Parcel::weightGrams
IL_0006: ldc.i4 499
IL_000b: add
IL_000c: ldc.i4 500
IL_0011: div
IL_0012: stloc.0
```

| Instrucción | Efecto | Pila después |
|---|---|---|
| `ldarg.0` | Apila el argumento 0, que es `this` en un método de instancia | this |
| `ldfld ...::weightGrams` | Extrae el objeto y apila el valor de su campo | weightGrams |
| `ldc.i4 499` | Apila la constante de 32 bits 499 | weightGrams, 499 |
| `add` | Extrae dos valores y apila su suma | weightGrams + 499 |
| `ldc.i4 500` | Apila 500 | weightGrams + 499, 500 |
| `div` | Extrae dos valores y apila el cociente | el cociente |
| `stloc.0` | Extrae hacia la variable local 0 | (vacía) |

Esta es la línea `int units = (weightGrams + 499) / 500;` del código fuente. Los grupos de instrucciones que verás con más frecuencia:

| Grupo | Instrucciones | Significado |
|---|---|---|
| Carga y almacenamiento | `ldarg`, `ldloc`, `stloc`, `ldfld`, `stfld`, `ldc.i4`, `ldstr` | Mover argumentos, variables locales, campos, constantes y literales de cadena hacia la pila y desde ella |
| Aritmética | `add`, `sub`, `mul`, `div`, `rem`, `neg`, `and`, `or`, `xor`, `shl`, `shr` | Operan sobre el valor o los dos valores de la cima |
| Saltos | `br`, `brtrue`, `brfalse`, `beq`, `bgt`, `blt`, `switch` | Saltar, de forma incondicional o según una condición; los destinos son desplazamientos de IL |
| Llamadas | `call`, `callvirt`, `newobj`, `ret` | Llamar a un método (el token de metadatos dice a cuál), crear un objeto, retornar |
| Conversiones y arreglos | `conv.i8`, `ldelem`, `stelem`, `ldlen` | Cambiar el ancho de un número, acceder a arreglos |

El sufijo `.s` (por ejemplo `ldloc.s`, `br.s`) indica una forma corta con un operando de un byte, y `ldarg.0`, `ldc.i4.1` y similares son formas cortas con el operando dentro del código de operación. No cambian el significado. IL lleva tipos: las variables locales tienen tipos declarados, y `ldfld` y `call` llevan el nombre completo del campo o del método. En una compilación sin información de depuración las variables locales no tienen nombres, solo números de ranura.

#### Leer un método

El método `Cost` completo está debajo, producido por el modo IL del descompilador. Léelo con la estructura del código fuente en mente:

```bash
ilspycmd -il -t Parcels.Parcel build/Parcels.dll 2>/dev/null | sed -n '/instance int32 Cost/,/end of method Parcel::Cost/p'
```

```text
        instance int32 Cost () cil managed 
    {
        // Method begins at RVA 0x2068
        // Header size: 12
        // Code size: 85 (0x55)
        .maxstack 2
        .locals init (
            [0] int32,
            [1] int32,
            [2] int32,
            [3] int32,
            [4] valuetype Parcels.Zone,
            [5] int32
        )

        IL_0000: ldarg.0
        IL_0001: ldfld int32 Parcels.Parcel::weightGrams
        IL_0006: ldc.i4 499
        IL_000b: add
        IL_000c: ldc.i4 500
        IL_0011: div
        IL_0012: stloc.0
        IL_0013: ldarg.0
        IL_0014: ldfld valuetype Parcels.Zone Parcels.Parcel::zone
        IL_0019: stloc.s 4
        IL_001b: ldloc.s 4
        IL_001d: ldc.i4.1
        IL_001e: beq.s IL_0027

        IL_0020: ldloc.s 4
        IL_0022: ldc.i4.2
        IL_0023: beq.s IL_002b

        IL_0025: br.s IL_002f

        IL_0027: ldc.i4.3
        IL_0028: stloc.3
        IL_0029: br.s IL_0032

        IL_002b: ldc.i4.5
        IL_002c: stloc.3
        IL_002d: br.s IL_0032

        IL_002f: ldc.i4.s 9
        IL_0031: stloc.3

        IL_0032: ldloc.3
        IL_0033: stloc.1
        IL_0034: ldc.i4.0
        IL_0035: stloc.2
        IL_0036: ldc.i4.0
        IL_0037: stloc.s 5
        IL_0039: br.s IL_004e
        // loop start (head: IL_004e)
            IL_003b: ldloc.2
            IL_003c: ldloc.1
            IL_003d: add
            IL_003e: stloc.2
            IL_003f: ldloc.s 5
            IL_0041: ldc.i4.4
            IL_0042: blt.s IL_0048

            IL_0044: ldloc.2
            IL_0045: ldc.i4.1
            IL_0046: add
            IL_0047: stloc.2

            IL_0048: ldloc.s 5
            IL_004a: ldc.i4.1
            IL_004b: add
            IL_004c: stloc.s 5

            IL_004e: ldloc.s 5
            IL_0050: ldloc.0
            IL_0051: blt.s IL_003b
        // end loop

        IL_0053: ldloc.2
        IL_0054: ret
    } // end of method Parcel::Cost
```

La estructura se ve incluso sin el código fuente. La variable local 0 recibe el número de unidades. Las dos comparaciones del valor de zona con 1 y 2 seleccionan las constantes 3 y 5, y el caso restante guarda 9: es una elección de tres vías con los valores de zona como claves, guardada en la variable 3 y copiada a la variable 1, la tarifa. El salto en IL_0039 va a la prueba del bucle en IL_004e, que compara la variable 5 con la variable 0 y salta de vuelta a IL_003b mientras el contador sea menor. El cuerpo suma la tarifa (variable 1) al total (variable 2), y el `blt.s` en IL_0042 se salta el incremento extra mientras el contador sea menor que 4. Se devuelve la variable 2. Las ranuras no dicen cómo se llama cada variable; los papeles se deducen de cómo se usan, igual que nombrabas los registros en código nativo.

La línea `.maxstack 2` indica la mayor profundidad que alcanza la pila de evaluación, lo que sirve para comprobar tu lectura: ninguna secuencia del método deja más de dos valores en la pila.

### Descompilar con ILSpy

Un descompilador convierte la IL otra vez en un lenguaje de alto nivel, normalmente C#. La imagen del laboratorio incluye `ilspycmd`, la forma de línea de comandos de ILSpy; el ILSpy gráfico da los mismos resultados con un árbol de tipos y acceso directo a las definiciones. Comandos útiles:

| Comando | Salida |
|---|---|
| `ilspycmd Assembly.dll` | Todo el ensamblado como C# en la salida estándar |
| `ilspycmd -l c Assembly.dll` | Una lista de las clases |
| `ilspycmd -t Namespace.Type Assembly.dll` | Un tipo |
| `ilspycmd -il -t Namespace.Type Assembly.dll` | La IL de un tipo |
| `ilspycmd -p -o dir Assembly.dll` | Un proyecto completo, para leerlo en un editor |
| `ilspycmd -usepdb Assembly.dll` | Usar los nombres de las variables locales del archivo de depuración situado junto al ensamblado |

Las clases del programa de ejemplo:

```bash
ilspycmd -l c build/Parcels.dll 2>/dev/null
```

```text
Class <Module>
Class Parcels.Parcel
Class Parcels.Routes
Class Parcels.Program
Class <PrivateImplementationDetails>
Class Parcels.Routes.<>c__DisplayClass1_0
```

Tres de las seis las genera el compilador: `<Module>` contiene datos a nivel de módulo, `<PrivateImplementationDetails>` contiene datos constantes como el inicializador del arreglo, y la clase de visualización contiene la variable capturada de la lambda. El método `Cost` descompilado:

```bash
ilspycmd -t Parcels.Parcel build/Parcels.dll 2>/dev/null | sed -n '/public int Cost/,/^\t}/p'
```

```text
    public int Cost()
    {
        int num = (weightGrams + 499) / 500;
        int num2 = zone switch
        {
            Zone.Local => 3, 
            Zone.Regional => 5, 
            _ => 9, 
        };
        int num3 = 0;
        for (int i = 0; i < num; i++)
        {
            num3 += num2;
            if (i >= 4)
            {
                num3++;
            }
        }
        return num3;
    }
```

Este es el método del código fuente con tres diferencias: las variables locales se llaman `num`, `num2`, `num3`, el comentario ha desaparecido, y `total += 1` se ha convertido en `num3++`. La estructura, incluida la elección por zona y el bucle, es la misma. El descompilador encontró el bucle y la elección a partir de los saltos que se ven en la IL de arriba.

### Qué recupera un descompilador y qué pierde

La comparación del código fuente con el programa descompilado es la mejor forma de aprender en qué se puede confiar. Del ejemplo:

| Elemento | En el código fuente | En la salida descompilada | Por qué |
|---|---|---|---|
| Nombres de tipos, métodos y campos | `Parcel`, `Cost`, `weightGrams` | Idénticos | Son metadatos |
| Firmas y tipos | `int Cost()` | Idénticos | Son metadatos |
| Flujo de control | `for`, `if`, una expresión `switch` | Recuperado como las mismas construcciones | Los saltos se corresponden con ellas |
| Nombres de variables locales | `units`, `rate`, `total` | `num`, `num2`, `num3` | Los nombres de las variables locales están en el archivo de depuración, no en el ensamblado |
| Comentarios | Presentes | Ausentes | Nunca llegan al archivo compilado |
| `foreach` sobre un arreglo | `foreach (Parcel p in parcels)` | `for (int i = 0; i < array.Length; i++)` con `array[i]` | El compilador ya lo convirtió en un bucle indexado |
| `total += 1` | Como se escribió | `num3++` | El compilador emite la misma IL; el descompilador elige una forma |
| Switch de cadenas | `switch (code) { case "N": ... }` | Una expresión `switch` con los mismos casos | El compilador probó la longitud y luego los caracteres; el descompilador reconoce ese patrón y restaura las cadenas |
| Lambda | Una lambda que captura `limit` | Una lambda `(int w) => ...` | El descompilador oculta la clase de visualización; la IL la muestra |

Cuando el mismo programa se compila con un archivo de depuración, el descompilador puede tomar de él los nombres de las variables locales:

```bash
ilspycmd -usepdb -t Parcels.Parcel build/debug/Parcels.dll 2>/dev/null | sed -n '/public int Cost/,/^\t}/p'
```

```text
    public int Cost()
    {
        int units = (weightGrams + 499) / 500;
        int rate = zone switch
        {
            Zone.Local => 3, 
            Zone.Regional => 5, 
            _ => 9, 
        };
        int total = 0;
        for (int i = 0; i < units; i++)
        {
            total += rate;
            if (i >= 4)
            {
                total++;
            }
        }
        return total;
    }
```

Los nombres `units`, `rate` y `total` vuelven. Los analistas casi nunca tienen el archivo de depuración de una muestra, así que espera los nombres genéricos y nombra tú mismo las variables a medida que averiguas lo que hacen.

Otro detalle sale de las cadenas de usuario del primer listado. El switch tiene siete casos, y las cadenas `N`, `S`, `E` y `W` no aparecen en el montículo `#US`, aunque `NE`, `NW` y `SE` sí. El compilador compara las cadenas de un solo carácter por su longitud y su carácter en lugar de guardar un literal, y el descompilador convierte la comparación otra vez en el literal en su salida. El código fuente descompilado contiene, pues, literales que el montículo de cadenas no tiene. Buscar cadenas en el archivo es un buen primer paso y no una lista completa.

### Leer código administrado ofuscado

Los métodos del módulo 20 se aplican con cambios. Los ofuscadores administrados suelen renombrar tipos, métodos y campos con nombres cortos sin significado, cifrar los literales de cadena para que un método los decodifique en tiempo de ejecución, aplanar el flujo de control y añadir relleno. Compáralos con los casos nativos:

- **Símbolos renombrados.** La salida del descompilador tiene nombres como `a`, `b`, `c` o caracteres no imprimibles. La estructura y los tipos siguen ahí. Renombra a medida que aprendes: ILSpy permite buscar por firma, y las referencias de `MemberRef` siguen diciéndote lo que el programa llama en el framework.
- **Cadenas cifradas.** El montículo `#US` tiene pocos literales y el código tiene muchas llamadas a un método pequeño que toma un entero o un arreglo de bytes y devuelve una cadena. Es el decodificador del módulo 20, escrito en IL. Puedes leer el método y reimplementarlo, o ejecutarlo: el ensamblado es un programa, y un script pequeño o un arnés de pruebas en el laboratorio aislado puede llamar al decodificador con los argumentos que leas en los puntos de llamada.
- **Flujo de control aplanado.** La misma estructura de despachador y estado que en el módulo 20, legible en IL como un `switch` sobre una variable local dentro de un bucle.
- **Llamadas mediante reflexión.** Un programa puede llamar a un método por un nombre que construye en tiempo de ejecución. La lista `MemberRef` no lo mostrará; busca las llamadas del framework que encuentran un tipo o un método por su nombre.

Como el entorno de ejecución puede recuperar la IL de cualquier método, nada de la lógica está oculto para un lector decidido; el trabajo consiste en quitar el ruido. Ejecuta un ensamblado desconocido solo en el laboratorio aislado del módulo 19.

### Un orden de trabajo

1. **Triaje.** Confirma que el archivo es administrado. Enumera las clases, los ensamblados referenciados y las filas `MemberRef`. Lee las cadenas de los montículos `#Strings` y `#US`.
2. **Descompilar.** Lee primero el C# del punto de entrada (el token del punto de entrada de la cabecera CLR nombra el método) y después sigue las llamadas.
3. **Comprobar los nombres.** Si los nombres no significan nada, renombra sobre la marcha y anota lo que hace cada tipo.
4. **Comprobar la salida con la IL** cuando el código descompilado parezca extraño o cuando un detalle decida la respuesta.
5. **Buscar código generado.** Los nombres con `<` y `>` son ayudas del compilador: lambdas, iteradores, métodos asíncronos. Léelos como el código que implementan.
6. **Tratar la protección.** Encuentra el decodificador de cadenas, reimpleméntalo o llámalo, y anota el resultado en el informe.

## Pruébalo

Ejecuta los comandos del laboratorio desde `modules/22-managed-code-dotnet/examples`.

1. Compila con `make` y ejecuta `dotnet build/Parcels.dll`. Ejecuta `22_meta.py` sobre él y compara el número de filas de `MethodDef` con el número de métodos del código fuente (cuentan los constructores y el método generado por el compilador).
2. Ejecuta `strings -n 5` y `strings -el -n 4` sobre el ensamblado. ¿Qué literales del programa aparecen en cada salida? ¿Por qué `N` no aparece en ninguna?
3. Descompila todo el ensamblado. Anota tres diferencias con el código fuente de `22_parcels/Program.cs`.
4. Imprime la IL de `Routes.Region`. Encuentra la llamada que lee la longitud de la cadena y las llamadas que leen caracteres sueltos. Explica en una frase cómo convirtió el compilador el switch de cadenas en comparaciones.
5. Imprime la IL de `Routes.Surcharges`. Encuentra dónde se crea la clase auxiliar y dónde se guarda la variable capturada. ¿Qué instrucción carga la dirección del método de la clase auxiliar en el delegado?
6. Descompila `build/debug/Parcels.dll` con y sin `-usepdb` y compara los nombres de las variables locales.
7. Cambia `Cost` en el código fuente para que el céntimo extra empiece después de la sexta unidad, vuelve a compilar y encuentra la constante modificada en la IL.

## Errores frecuentes

- **Buscar literales con una búsqueda ASCII.** Los literales de un programa administrado están en UTF-16 en un montículo aparte. Usa `strings -el` o lee el montículo.
- **Fiarse de los nombres locales.** Sin archivo de depuración los nombres se generan. Un nombre como `num2` no dice nada sobre el papel de la variable.
- **Olvidar que el compilador escribe código.** Una clase con `<` en el nombre es generada. Una lectura que la trate como diseño del autor se confundirá.
- **Leer solo el código descompilado.** El descompilador toma decisiones. Cuando un resultado parezca erróneo o importe un detalle, lee la IL de ese método.
- **Suponer que los nombres significan lo que dicen.** Los nombres son texto libre. Un método llamado `Verify` puede hacer cualquier cosa.
- **Ejecutar un ensamblado desconocido fuera del laboratorio.** Leer un ensamblado nunca lo ejecuta. Ejecutarlo sí, y un programa administrado tiene el mismo acceso que uno nativo.

## Resumen

- El código administrado es IL más metadatos, ejecutado por un entorno de ejecución que lo compila cuando hace falta; el archivo conserva los nombres, los tipos y la estructura del programa.
- Un ensamblado administrado es un archivo PE con una cabecera CLR, una única importación de `mscoree.dll` y tablas y montículos de metadatos; los literales están en UTF-16 en `#US` y los nombres en `#Strings`.
- IL es una máquina de pila con variables locales y campos con tipos; se lee como el bytecode del módulo 21, y los papeles de las variables locales se deducen de cómo se usan.
- Un descompilador como ILSpy devuelve C# que conserva tipos, firmas y flujo de control, y pierde los comentarios y, sin archivo de depuración, los nombres de las variables locales.
- Comprueba la salida descompilada con la IL cuando importe un detalle, y trata los nombres con `<` y `>` como código generado por el compilador.
- El código administrado ofuscado se lee con los métodos del módulo 20: renombrar, encontrar el decodificador de cadenas y deshacer el aplanado del flujo de control.

## Siguientes pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 23, los descompiladores y sus límites.
