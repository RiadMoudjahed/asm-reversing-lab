<!-- source: glossary.md @ sha256:567da3b1afab -->
[English](glossary.md) | [العربية](glossary.ar.md) | [Deutsch](glossary.de.md) | [Español](glossary.es.md) | [Français](glossary.fr.md)

# Glosario

Términos básicos usados en todo el curso. Se añaden entradas a medida que los módulos introducen vocabulario nuevo.

**análisis de vivacidad (liveness analysis)**: El análisis de flujo de datos que averigua qué valores se leen más adelante; las asignaciones a valores que nunca se leen están muertas y se pueden eliminar.

**análisis dinámico (dynamic analysis)**: Estudiar un programa ejecutándolo y observando su comportamiento, normalmente bajo un depurador.

**análisis estático (static analysis)**: Estudiar un programa sin ejecutarlo.

**aplanamiento del flujo de control (control-flow flattening)**: Una ofuscación que sustituye el grafo de una función por un bucle con un distribuidor y una variable de estado que contiene el número del bloque siguiente.

**arista de retroceso (back edge)**: Una arista del grafo de flujo de control desde un bloque hasta un bloque que lo domina; señala un bucle.

**binario sin símbolos (stripped binary)**: Un ejecutable del que se han eliminado los nombres de símbolos y la información de depuración.

**bloque (chunk)**: Una parte con etiqueta de un archivo: una etiqueta que dice qué es, una longitud y una carga útil.

**bloque básico (basic block)**: Una serie de instrucciones con una entrada y una salida: el control entra en la primera instrucción y sale después de la última.

**bucle de despacho (dispatch loop)**: El bucle central de un intérprete: obtiene un código de operación, elige su manejador y lo ejecuta, y vuelve a empezar.

**bytecode (bytecode)**: Un formato compacto de instrucciones que un intérprete lee y ejecuta; para la máquina que lo ejecuta son datos, no código máquina.

**cadena de pila (stack string)**: Una cadena que el código construye byte a byte a partir de constantes, de modo que nunca existe como una sola pieza en el archivo.

**compilación justo a tiempo (just-in-time compilation)**: La traducción del lenguaje intermedio a código máquina mientras el programa se ejecuta, método a método, la primera vez que se usa el método.

**complemento a dos (two's complement)**: La forma estándar de representar enteros con signo: para negar un número se invierten sus bits y se suma uno.

**convención de llamada (calling convention)**: El acuerdo sobre dónde coloca el llamador los argumentos, dónde devuelve su resultado la función llamada y qué registros debe preservar cada parte.

**código administrado (managed code)**: Código que se ejecuta bajo un entorno de ejecución como .NET, que lo carga, lo compila y administra su memoria.

**código de operación (opcode)**: El número de una instrucción que selecciona la operación que se debe realizar.

**depurador (debugger)**: Una herramienta que ejecuta un programa de forma controlada para poder pausarlo, avanzar paso a paso e inspeccionar registros y memoria.

**desbordamiento de enteros (integer overflow)**: Un resultado aritmético que no cabe en su tipo y da la vuelta, de modo que un tamaño o índice acaba siendo mucho menor de lo previsto.

**descompilador (decompiler)**: Una herramienta que convierte el código máquina en código fuente de alto nivel aproximado, como C.

**desensamblador (disassembler)**: Una herramienta que convierte los bytes del código máquina en instrucciones de ensamblador.

**dirección de memoria (memory address)**: Un número que identifica un byte en la memoria.

**elevación (lifting)**: La etapa de un descompilador que convierte las instrucciones máquina en operaciones sobre variables, y los indicadores en condiciones.

**enlazador (linker)**: Un programa que combina archivos objeto y bibliotecas en un ejecutable y resuelve las referencias entre símbolos.

**ensamblado (assembly, .NET)**: Un programa o biblioteca de .NET: un archivo PE que contiene lenguaje intermedio y metadatos.

**ensamblador (assembler)**: Un programa que traduce el código fuente en lenguaje ensamblador a código máquina almacenado en un archivo objeto.

**epílogo (epilogue)**: Las instrucciones al final de una función que restauran los registros guardados y la pila y luego retornan.

**error de uno (off-by-one error)**: Un límite equivocado en una unidad, como una comprobación que acepta una longitud igual a la capacidad cuando la copia necesita además sitio para un terminador.

**exportación (export)**: Una función o variable que una biblioteca pone a disposición de otros programas y que figura en su tabla de símbolos dinámicos.

**firma (signature)**: El tipo de retorno, el nombre y los tipos de los parámetros de una función; en una función sin documentar se recupera a partir de cómo su código usa los registros.

**grafo de flujo de control (control-flow graph)**: Los bloques de una función como nodos y los saltos entre ellos como aristas.

**indicador (flag)**: Un solo bit del registro de indicadores que registra una propiedad del último resultado, como cero o acarreo.

**indicador (indicator)**: Un hecho observable, como una cadena, un hash, una función importada o un comportamiento, que apunta a un programa, una familia o una actividad (no es el indicador del procesador).

**ingeniería inversa**: Recuperar cómo funciona un programa o producto a partir del propio producto terminado, sin sus documentos de diseño.

**instrucción (instruction)**: Una orden individual que el procesador puede ejecutar, formada por un código de operación y cero o más operandos.

**intérprete (interpreter)**: Un programa que lee instrucciones en otro formato, como el bytecode, y las ejecuta una por una.

**lenguaje intermedio (intermediate language)**: El conjunto de instrucciones que producen los compiladores administrados, llamado también IL o CIL; un entorno de ejecución lo compila a código máquina cuando hace falta.

**llamada al sistema (system call)**: Una petición de un programa al núcleo del sistema operativo para realizar una operación privilegiada.

**manejador opaco (opaque handle)**: Un puntero que una biblioteca devuelve y espera recibir de vuelta, cuyo contenido quien llama nunca lee ni escribe; identifica un recurso que debe liberarse.

**marco de pila (stack frame)**: La parte de la pila que pertenece a una llamada de función.

**metadatos (metadata)**: Tablas y montículos de un ensamblado administrado que describen sus tipos, métodos, campos, referencias y literales.

**modo de direccionamiento (addressing mode)**: La regla que usa una instrucción para indicar dónde está un operando: dentro de la propia instrucción, en un registro o en una dirección de memoria calculada.

**máquina virtual (virtual machine)**: Un programa que se comporta como un procesador, con su propio contador de programa, almacenamiento y conjunto de instrucciones, y que ejecuta bytecode.

**nota de detección (detection note)**: Una descripción breve para los defensores de un comportamiento que buscar: qué se observa y dónde, por qué es sospechoso, qué se le parece pero es inofensivo y a qué técnica corresponde.

**ofuscación (obfuscation)**: Cambiar un programa para que sea más difícil de leer sin cambiar lo que hace.

**operando (operand)**: Un valor sobre el que opera una instrucción: una constante, un registro o una posición de memoria.

**orden de bytes (endianness)**: El orden en que se almacenan en memoria los bytes de un valor de varios bytes.

**parámetro de salida (output parameter)**: Un argumento puntero a través del cual una función solo escribe, para que quien llama reciba un resultado en una variable propia.

**persistencia (persistence)**: Una forma de que un programa vuelva a iniciarse tras un reinicio o un cierre de sesión, como un valor Run del registro o una tarea programada.

**pila (stack)**: Una región de memoria usada como estructura de último en entrar, primero en salir para direcciones de retorno, argumentos y variables locales.

**predicado opaco (opaque predicate)**: Una condición que siempre tiene el mismo valor pero es difícil de ver a simple vista, de modo que un lado de la rama es código muerto.

**prólogo (prologue)**: Las instrucciones al comienzo de una función que guardan registros y preparan su marco de pila.

**punto de entrada (entry point)**: La dirección de la primera instrucción que se ejecuta cuando arranca un programa.

**punto de interrupción (breakpoint)**: Una marca que hace que el depurador detenga el programa cuando la ejecución llega a una dirección elegida.

**registro (register)**: Una pequeña ubicación de almacenamiento muy rápida dentro del procesador.

**regla YARA (YARA rule)**: Una regla con cadenas y una condición que compara archivos con patrones y sirve para encontrar programas de un mismo tipo.

**sanitizer (sanitizer)**: Una función del compilador que añade comprobaciones a un programa y lo detiene con un informe en el primer acceso incorrecto a memoria.

**suma de verificación (checksum)**: Un valor calculado a partir de los bytes de un archivo o mensaje que permite a un lector detectar daños o cambios.

**sumidero (sink)**: Un lugar de un programa donde un valor decide cuánta memoria se toca, como una copia, un índice, una reserva o una liberación.

**triaje**: Un primer examen rápido de un archivo, sin ejecutarlo, para identificar qué es y decidir dónde seguir buscando.

**uso tras liberación (use after free)**: Usar memoria a través de un puntero después de que la memoria se haya liberado.

**valor mágico (magic value)**: Un identificador fijo al principio de un formato de archivo que dice a un lector de qué formato se trata.
