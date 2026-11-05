<!-- source: README.md @ sha256:cbf3cf5f6a4f -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 19. Analizar programas sospechosos

Tiempo: unas 3 horas. Requisitos previos: [18. Auditar binarios en busca de errores](../18-auditing-binaries/README.es.md)

## Por qué importa

Un defensor recibe con regularidad un archivo del que alguien sospecha: un adjunto, una descarga, un programa hallado en un servidor. El trabajo consiste en responder cuatro preguntas con rapidez y seguridad. ¿Qué es? ¿Qué haría? ¿Cómo podemos reconocerlo en otro sitio? ¿Qué debemos decir a quienes tienen que responder? Este módulo enseña ese trabajo desde el lado del defensor. Trata de reconocer y describir comportamientos para poder detectarlos. No enseña a escribir ese tipo de software. Todas las muestras de este curso son simuladores inofensivos que imprimen lo que harían y nada más; el malware real nunca aparece en este repositorio y no debes llevarlo al laboratorio. Lee [la declaración de ética](../../ETHICS.md) antes de empezar.

## Conceptos

### Práctica segura de análisis

Trata todo programa desconocido como peligroso hasta que hayas demostrado lo contrario, y organiza el trabajo de modo que un error no cueste nada.

| Regla | Por qué |
|---|---|
| No ejecutes nunca un programa desconocido en tu propio ordenador | Una sola ejecución puede cambiar archivos, instalar persistencia o enviar tus datos fuera |
| Trabaja en un entorno aislado: una máquina virtual o un contenedor sin red | Sin red, el programa no puede recibir instrucciones ni sacar datos |
| Haz primero una instantánea y restáurala después | Siempre vuelves a un estado conocido |
| No compartas nada con el anfitrión: ni carpetas compartidas, ni portapapeles, ni credenciales | Una muestra no debe poder alcanzar nada que importe |
| Mantén las muestras inertes en reposo: cambia la extensión, guárdalas en un archivo comprimido, anota el hash | Un doble clic por error no hace nada |
| Anota lo que haces: órdenes, horas, hashes, hallazgos | Un informe vale tanto como las notas en que se apoya |
| Usa solo simuladores en este curso | Los simuladores están escritos para ser inofensivos; las muestras reales necesitan un entorno profesional y autorización |

La imagen del laboratorio se ejecuta en un contenedor que solo ve la carpeta que montas. Para una ejecución más estricta, monta la carpeta en solo lectura y desactiva la red. Esta es una orden para el anfitrión:

```bash
docker run --rm --network none --read-only -v "$PWD":/lab:ro -w /lab asm-reversing-lab \
    modules/19-analyzing-suspicious-programs/examples/build/19_sim
```

Incluso un programa inofensivo se comprueba antes de confiar en él. El simulador `examples/19_sim.c` promete no hacer nada más que imprimir. Puedes verificar la promesa desde fuera, sin mirar el código, con `strace`, que enumera las llamadas al sistema que hace un programa:

```bash
make
strace -f -e trace=execve,connect,socket,clone,openat,unlink,write build/19_sim 2>&1 >/dev/null
```

```text
execve("build/19_sim", ["build/19_sim"], 0x7fff3dd23cc8 /* 15 vars */) = 0
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
write(1, "[sim] simulator: no action is pe"..., 1018) = 1018
+++ exited with 0 +++
```

El programa carga la biblioteca de C, escribe 1018 bytes en su salida estándar y termina. No inicia ningún proceso, no hace ninguna conexión, no borra nada y no abre ningún archivo salvo los del cargador. Una muestra real mostraría lo contrario, y este es el tipo de prueba en que se apoyan el triaje y el análisis de comportamiento.

### Triaje: la primera mirada, sin ejecutar nada

El triaje decide hasta dónde profundizar. Reúne primero los datos baratos. El script `examples/19_triage.py` los reúne para archivos ELF y PE y nunca ejecuta el archivo:

```bash
python3 19_triage.py build/19_sim.exe
```

```text
file:     build/19_sim.exe
size:     251253 bytes
sha256:   656c3556a79829ebae38d56c31bc16b7f3b10174e89323862279217c015affe8
type:     PE (MZ header)
entropy:  5.89 bits per byte (above about 7.2 suggests packed or encrypted data)
imports:  48
    KERNEL32.dll!DeleteCriticalSection
    KERNEL32.dll!EnterCriticalSection
    KERNEL32.dll!GetLastError
    KERNEL32.dll!InitializeCriticalSection
    KERNEL32.dll!IsDBCSLeadByteEx
    KERNEL32.dll!LeaveCriticalSection
    KERNEL32.dll!MultiByteToWideChar
    KERNEL32.dll!SetUnhandledExceptionFilter
    KERNEL32.dll!Sleep
    KERNEL32.dll!TlsGetValue
    KERNEL32.dll!VirtualProtect
    KERNEL32.dll!VirtualQuery
    ... and 36 more (the C runtime and basic KERNEL32 functions)
strings worth a second look:
    [registry path] HKCU\Software\Microsoft\Windows\CurrentVersion\Run
    [command line] schtasks /create /tn "%s" /tr "%s" /sc daily /st 09:00
    [url or host] updates.contoso-cdn.test
    [windows path] C:\Users\Public\upd\updsvc.exe
    [process name] updsvc.exe
    [process name] explorer.exe
```

| Dato | Qué te dice |
|---|---|
| El hash | Un nombre para el archivo. Búscalo en tus registros y en bases de datos públicas; infórmalo. Compilaciones distintas tienen hashes distintos, así que una coincidencia prueba identidad pero una discrepancia no prueba nada |
| El tipo y el tamaño | Qué herramientas se aplican y si el tamaño encaja con el propósito declarado |
| La entropía | Cerca de 8 bits por byte significa contenido comprimido o cifrado, lo que a menudo significa empaquetado; el código y el texto normales quedan muy por debajo. Mira también la entropía de cada sección |
| Las importaciones | Las funciones que el programa pide al sistema. Una lista de importaciones diminuta puede significar que el programa resuelve las funciones por sí mismo; una lista con funciones de procesos, memoria, registro y red dice lo que puede hacer |
| Las cadenas | Rutas, claves, órdenes, anfitriones. Son la prueba más barata y a menudo la mejor del propósito |

Fíjate en lo que muestra este PE. Sus importaciones son solo el tiempo de ejecución de C y funciones básicas del sistema, porque el simulador no llama a nada peligroso. Su propósito se ve solo en las cadenas: un lugar de persistencia, una orden de tarea programada, una ruta de volcado, un nombre de anfitrión y un nombre de proceso.

### Indicadores estáticos

Un **indicador** es un hecho observable que apunta a un programa, una familia o un comportamiento. Los indicadores estáticos salen del archivo sin ejecutarlo. Tipos útiles:

| Tipo | Ejemplos | Precaución |
|---|---|---|
| Hashes | SHA-256 del archivo | Se rompen con cualquier cambio del archivo |
| Cadenas | Rutas del registro, líneas de órdenes, nombres de archivo, nombres de anfitrión, nombres de mutex, nombres de tubería, cadenas de agente de usuario | Las cadenas comunes (las claves `Run` aparecen en herramientas de documentación) necesitan el apoyo de otras cadenas |
| Importaciones y exportaciones | Conjuntos de funciones que juntas sugieren una capacidad | El software legítimo usa las mismas funciones |
| Estructura | Nombres y tamaños de secciones, punto de entrada en una sección inusual, entropía alta, un recurso que es un ejecutable | Compiladores e instaladores también producen disposiciones raras |
| Patrones de código | Un bucle de descodificación, una secuencia de llamadas, una tabla de constantes | Hace falta ingeniería inversa para confirmarlos |

Un solo indicador rara vez condena. Varios que encajan entre sí, sí. Anota siempre cuánto respaldo tiene cada indicador y con qué probabilidad aparecería en archivos inocentes.

### Indicadores de comportamiento

Un **indicador de comportamiento** sale de observar lo que hace el programa en el laboratorio aislado. Mira el sistema de archivos (qué se crea, cambia o borra, y dónde), los procesos (qué se inicia, con qué línea de órdenes), el registro en Windows (claves y valores escritos), la red (intentos de conexión, aunque fallen, y los nombres consultados) y la memoria (otros procesos abiertos, memoria reservada en ellos). En Linux, `strace -f` muestra las llamadas al sistema; en Windows, los monitores de procesos, archivos, registro y red hacen el mismo trabajo. El simulador imprime su comportamiento directamente:

```bash
make run
```

```text
[sim] simulator: no action is performed, the steps below are only printed
[sim] would read the computer name, the user name and the OS version
[sim] would list the running processes
[sim] would open the key HKCU\Software\Microsoft\Windows\CurrentVersion\Run
[sim] would set the value "Updater" to "C:\Users\Public\upd\updsvc.exe"
[sim] would run: schtasks /create /tn "GoogleUpdateTaskMachineQC" /tr "C:\Users\Public\upd\updsvc.exe" /sc daily /st 09:00
[sim] would open the process explorer.exe with write access
[sim] would allocate 4096 bytes of executable memory in it
[sim] would write the payload into that memory
[sim] would start a remote thread at the payload
[sim] would open the access token of a system process
[sim] would duplicate the token as a primary token
[sim] would start C:\Users\Public\upd\updsvc.exe with the duplicated token
[sim] would send an HTTP GET to http://updates.contoso-cdn.test:8443/v2/check?id=<host id> every 60 seconds
[sim] would write the reply to C:\Users\Public\upd\updsvc.exe
```

El orden es informativo. El orden de las acciones de una muestra real también se ve en el código: en esta compilación, `main` llama a seis funciones una tras otra, y los nombres las delatan (una muestra real sin símbolos no tendría nombres, pero sí el mismo orden y los mismos argumentos de cadena):

```text
00000000004013da <main>:
  4013e2:  mov    edi,0x402300
  4013e7:  call   puts                      ; the banner
  4013ec:  call   sim_discover
  4013f1:  call   sim_persist_runkey
  4013f6:  call   sim_persist_task
  4013fb:  call   sim_inject
  401400:  call   sim_token
  401405:  call   sim_beacon
```

### Correspondencia con ATT&CK

MITRE ATT&CK es un catálogo público de las técnicas que se sabe que usan los atacantes. Las organiza por **táctica**, el objetivo (persistencia, evasión de defensas, mando y control), y da a cada **técnica** un número, como T1547, con **subtécnicas** debajo, como T1547.001. Asociar tus observaciones a estos números da a todos el mismo vocabulario para informes, detecciones y preguntas de cobertura. Asocia comportamientos, no herramientas: pregunta qué hace el programa y busca la técnica que lo describe; después lee la descripción oficial para confirmar que encaja y ver cómo se detecta la técnica.

| Observación | Técnica | Táctica |
|---|---|---|
| Un valor escrito bajo una clave `Run` del registro que inicia un programa al iniciar sesión | T1547.001 Registry Run Keys / Startup Folder | Persistencia |
| Una tarea programada creada con `schtasks` o con las interfaces del programador de tareas | T1053.005 Scheduled Task | Persistencia, ejecución |
| Un programa que escribe en la memoria de otro proceso e inicia allí un hilo | T1055 Process Injection (T1055.001 DLL injection, T1055.002 portable executable injection, T1055.003 thread execution hijacking, T1055.004 asynchronous procedure call, T1055.012 process hollowing) | Evasión de defensas, escalada de privilegios |
| Un token duplicado o tomado de otro proceso y usado para iniciar un proceso | T1134 Access Token Manipulation (T1134.001 token impersonation or theft, T1134.002 create process with token) | Evasión de defensas, escalada de privilegios |
| Un programa que hace que un programa de confianza cargue su propia biblioteca | T1574 Hijack Execution Flow (T1574.001 DLL search order hijacking, T1574.002 DLL side-loading) | Persistencia, escalada de privilegios, evasión de defensas |
| Leer el nombre del equipo, el usuario y la versión del sistema operativo | T1082 System Information Discovery | Descubrimiento |
| Enumerar los procesos en ejecución | T1057 Process Discovery | Descubrimiento |
| Peticiones HTTP a un servidor a intervalos | T1071.001 Application Layer Protocol: Web Protocols | Mando y control |
| Descargar un archivo para ejecutarlo más tarde | T1105 Ingress Tool Transfer | Mando y control |
| Órdenes ejecutadas a través del intérprete de órdenes | T1059.003 Windows Command Shell | Ejecución |
| Cadenas o código que se codifican y se descodifican en tiempo de ejecución | T1027 Obfuscated Files or Information, T1140 Deobfuscate/Decode Files or Information | Evasión de defensas |

Para el simulador, la salida anterior se asocia a T1082 y T1057 (las líneas de descubrimiento), T1547.001 (la clave y el valor), T1053.005 (la orden `schtasks`), T1055 (abrir un proceso, reservar memoria en él, escribir en ella, iniciar un hilo remoto: la familia de inyección descrita en la tabla siguiente), T1134 (las líneas del token) y T1071.001 (la petición HTTP periódica). Escribe la correspondencia en una tabla junto a la prueba de cada línea. Si un comportamiento encaja con varias técnicas, indica la más cercana y explica por qué.

### Patrones de llamadas a la API tras las técnicas

En Windows, un programa llega al sistema mediante funciones de la API, y cada técnica necesita un grupo característico de ellas. Conocer los grupos permite reconocer la técnica en una lista de importaciones, en el orden de las llamadas en el desensamblado o en el registro de un monitor. Son patrones para reconocer y detectar. El software legítimo, como instaladores, depuradores, herramientas de seguridad y utilidades del sistema, usa muchas de las mismas funciones, así que un patrón es un motivo para mirar más de cerca, nunca un veredicto.

| Técnica | Llamadas características, en el orden habitual | Qué más buscar |
|---|---|---|
| Clave Run del registro (T1547.001) | `RegCreateKeyEx` o `RegOpenKeyEx`, y luego `RegSetValueEx` con una ruta como dato | La ruta de la clave (`...\CurrentVersion\Run`, `RunOnce`) y una ruta a un archivo en una carpeta donde el usuario puede escribir |
| Tarea programada (T1053.005) | `CreateProcess` o `ShellExecute` para `schtasks /create`, o `CoCreateInstance` para las interfaces del programador de tareas | La línea de órdenes con `/tr`, `/sc` y `/tn`, y un nombre de tarea que imita un producto conocido |
| Inyección de DLL (T1055.001) | `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory`, `CreateRemoteThread` (la dirección de inicio suele ser la de `LoadLibrary`) | El nombre del proceso objetivo y una ruta a una biblioteca en la memoria escrita |
| Inyección de código (T1055.002) | Las mismas tres primeras llamadas, seguidas de un hilo cuya dirección de inicio está dentro de la memoria reservada | Permisos de ejecución (`PAGE_EXECUTE_READWRITE`) en la reserva |
| Secuestro de hilo (T1055.003) | `OpenThread`, `SuspendThread`, `GetThreadContext`, `SetThreadContext`, `ResumeThread` | Un puntero de instrucción cambiado en el contexto |
| Inyección por APC (T1055.004) | `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory`, `QueueUserAPC` | Un hilo en estado alertable |
| Vaciado de proceso (T1055.012) | `CreateProcess` con el indicador de suspendido, un desmapeo de la imagen original (`NtUnmapViewOfSection`), `WriteProcessMemory`, `SetThreadContext`, `ResumeThread` | Un proceso cuya imagen en disco difiere de lo que se ejecuta |
| Manipulación de tokens (T1134) | `OpenProcessToken`, `DuplicateTokenEx`, y luego `ImpersonateLoggedOnUser`, `SetThreadToken` o `CreateProcessWithTokenW`; `AdjustTokenPrivileges` para activar un privilegio | La petición de un privilegio de depuración o de suplantación |
| Secuestro del orden de búsqueda de DLL y carga lateral (T1574.001, T1574.002) | `LoadLibrary` con un nombre de biblioteca sin ruta, un ejecutable iniciado desde una carpeta que también contiene una biblioteca con un nombre esperado | Una biblioteca nueva escrita junto a un ejecutable de confianza en una carpeta donde el usuario puede escribir |

En Linux, las familias análogas usan otras llamadas (`ptrace` y `process_vm_writev` para escribir en otro proceso, `LD_PRELOAD` y las rutas de búsqueda de bibliotecas para el secuestro, entradas de cron y unidades de systemd para la persistencia), y el método es el mismo: agrupar las llamadas, ordenarlas y encontrar la prueba en las cadenas y los argumentos.

### Notas de detección

Una **nota de detección** dice a quienes vigilan los sistemas qué buscar. Para cada comportamiento, anota qué se observa y dónde, por qué es sospechoso, qué actividad legítima se le parece y a qué técnica corresponde. Un breve ejemplo para el simulador:

```text
Behavior:     A value is added under HKCU\Software\Microsoft\Windows\CurrentVersion\Run
              that points to a program in C:\Users\Public.
Technique:    T1547.001 Registry Run Keys / Startup Folder
Where to see: Registry auditing and endpoint monitoring events for registry value
              writes, the process that made the write, the file at the path.
Why suspicious: Run values normally point to program folders; a path in a public,
              user-writable folder is unusual.
False positives: Installers of user-level applications also add Run values. Check
              the signer and the folder.
Response:     Collect the file, hash it, check where it came from, remove the value
              after the evidence is saved.
```

Fuentes habituales de eventos para estas notas: eventos de creación de procesos (con la línea de órdenes completa), eventos de escritura en el registro, eventos de creación de tareas del programador de tareas, creación de hilos remotos y eventos de acceso a procesos de los monitores de punto final, y eventos de creación de archivos. Nombra la fuente en la nota, porque una detección que nadie puede ver no vale nada.

### Escribir una regla YARA

YARA compara archivos con patrones. Una regla tiene un nombre, metadatos, cadenas y una condición:

```text
rule name
{
    meta:      description, author, references
    strings:   $a = "text"  $b = { 6A 40 68 ?? ?? }  $c = /regex/
    condition: boolean expression over the strings and the file
}
```

El archivo `examples/19_sim.yar` tiene dos versiones de una regla para el simulador. La primera es fácil de escribir:

```text
rule sim_updater_v1_weak
{
    strings:
        $run = "CurrentVersion\\Run"
    condition:
        $run
}
```

Coincide con el simulador, y también con el programa auxiliar inofensivo (`19_helper.c`), cuyo único delito es que su texto de ayuda menciona la misma ruta del registro. Probar una regla con archivos limpios es tan importante como probarla con la muestra:

```bash
for f in build/19_sim build/19_sim.exe build/19_helper build/19_helper.exe /bin/ls; do yara 19_sim.yar $f; done
```

```text
sim_updater_v1_weak build/19_sim
sim_updater_v2 build/19_sim
sim_updater_v1_weak build/19_sim.exe
sim_updater_v2 build/19_sim.exe
sim_updater_v1_weak build/19_helper
sim_updater_v1_weak build/19_helper.exe
```

La versión 1 salta con los dos programas auxiliares, lo que es un falso positivo. La versión 2 exige varias cadenas distintivas a la vez y una comprobación del tipo de archivo:

```text
rule sim_updater_v2
{
    meta:
        description = "Harmless updater simulator from module 19"
        technique = "T1547.001, T1053.005, T1055, T1134, T1071.001"
    strings:
        $runkey = "CurrentVersion\\Run"
        $task   = "schtasks /create /tn" ascii
        $drop   = "\\Users\\Public\\upd\\updsvc.exe" ascii
        $host   = "updates.contoso-cdn.test" ascii
        $tag    = "[sim] would" ascii
    condition:
        (uint32(0) == 0x464c457f or uint16(0) == 0x5a4d) and
        3 of ($runkey, $task, $drop, $host) and $tag
}
```

Coincide solo con las dos compilaciones del simulador. La opción `-s` muestra qué cadenas coincidieron y dónde, que es como se depura una regla:

```bash
yara -s 19_sim.yar build/19_sim
```

```text
sim_updater_v1_weak build/19_sim
0x2150:$run: CurrentVersion\Run
sim_updater_v2 build/19_sim
0x2150:$runkey: CurrentVersion\Run
0x21ad:$task: schtasks /create /tn
0x216a:$drop: \Users\Public\upd\updsvc.exe
0x2071:$host: updates.contoso-cdn.test
0x2004:$tag: [sim] would
```

Pautas para las reglas:

- Busca lo que es distintivo: rutas inusuales, nombres, líneas de órdenes, constantes y secuencias cortas de código con comodines. Evita las cadenas que tiene todo programa de ese tipo.
- Combina cadenas con `N of`, `all of` o `and`, y añade una comprobación del tipo de archivo con `uint16(0) == 0x5a4d` (las letras MZ) para PE o `uint32(0) == 0x464c457f` para ELF.
- Añade `ascii`, `wide` o ambos cuando el archivo pueda guardar el texto de cualquiera de las dos formas.
- Prueba con una colección de archivos limpios y con varias muestras. Anota para qué sirve cada regla.
- Una regla basada en cadenas deja de funcionar cuando el autor cambia las cadenas. Prefiere varios rasgos independientes a uno solo.
- Pon los números de técnica en los metadatos para que la regla pueda encontrarse por cobertura.

## Pruébalo

Ejecuta los comandos del laboratorio desde `modules/19-analyzing-suspicious-programs/examples`.

1. Compila con `make`. Ejecuta el script de triaje sobre `build/19_sim`, sobre `build/19_sim.exe` y sobre `/bin/ls`. Compara las importaciones, la entropía y las cadenas.
2. Ejecuta `strace -f -c build/19_sim` y lee el resumen de llamadas al sistema. ¿Qué llamadas esperarías de un programa que de verdad se hiciera persistente y se conectara?
3. Haz una tabla a partir de `make run`: una fila por cada línea `[sim]` con el ID de técnica, la táctica y la prueba. Comprueba cada ID con la descripción de la técnica en el sitio de ATT&CK.
4. Escribe una regla YARA que coincida con `19_sim` y `19_sim.exe` pero no con `19_helper`, usando cadenas distintas de las de `19_sim.yar`. Pruébala con `yara -s`.
5. Escribe una nota de detección para el comportamiento de la tarea programada en el formato anterior.
6. Explica por qué una regla que coincida con las cadenas `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory` y `CreateRemoteThread` también coincidiría con algunos programas legítimos, y qué podrías añadir a la condición para hacerla más específica.

## Errores frecuentes

- **Ejecutar primero.** No ejecutes nada antes de las comprobaciones baratas, y ejecútalo solo en el laboratorio aislado.
- **Fiarse de la etiqueta.** Un nombre de archivo, un icono o una firma no prueban lo que hace el programa.
- **Leer un indicador como una prueba.** Una ruta del registro en una cadena es documentación en un archivo y persistencia en otro. Busca un grupo.
- **Asociar herramientas en vez de comportamientos.** La pregunta es qué hace el programa, no cómo se llama.
- **Escribir una regla que solo encaja con la muestra.** Una regla basada en un hash o en una sola cadena se rompe en la siguiente compilación. Pruébala en cuanto a falsos positivos y robustez.
- **Saltarse las notas.** Sin órdenes, horas y hashes, nadie puede repetir ni comprobar los hallazgos.

## Resumen

- Analiza los programas desconocidos solo en un entorno aislado, sin red y con instantánea, y verifica incluso un simulador inofensivo desde fuera con un rastreo de llamadas al sistema.
- El triaje reúne hashes, tipo, entropía, importaciones y cadenas sin ejecutar el archivo y decide hasta dónde profundizar.
- Los indicadores estáticos salen del archivo y los de comportamiento de observarlo en ejecución; varios que encajan entre sí valen más que uno.
- ATT&CK da un vocabulario común: asocia cada comportamiento observado a una táctica y una técnica, con la prueba.
- Cada técnica tiene un grupo característico de llamadas a la API, que es un motivo para mirar más de cerca y nunca un veredicto.
- Las notas de detección y las reglas YARA convierten los hallazgos en algo que otros pueden usar, y ambas deben probarse con archivos limpios.

## Próximos pasos

- Haz el [cuestionario](quiz.es.md).
- Continúa con el módulo 20, leer código ofuscado.
- Ejercicios que usan este módulo: [Ejercicio 19](../../exercises/19-triage-desk/README.es.md).
