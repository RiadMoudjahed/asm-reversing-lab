<!-- source: calling-conventions.md @ sha256:5e066f9fba0a -->
[English](calling-conventions.de.md) | [العربية](calling-conventions.ar.md) | [Deutsch](calling-conventions.de.md) | [Español](calling-conventions.es.md) | [Français](calling-conventions.fr.md)

# Aufrufkonventionen

## 32 Bit

| | cdecl | stdcall | fastcall |
|---|---|---|---|
| Argumente | Stapel, von rechts nach links | Stapel, von rechts nach links | Die ersten zwei Ganzzahlen in `ecx` und `edx`, der Rest auf dem Stapel |
| Wer die Stapelargumente entfernt | Aufrufer | Aufgerufener, mit `ret N` | Aufgerufener, mit `ret N` |
| Ergebnis | `eax` | `eax` | `eax` |
| Vom Aufgerufenen gesichert | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` |

## 64 Bit

| | System V AMD64 (Linux, macOS) | Microsoft x64 (Windows) |
|---|---|---|
| Ganzzahl- und Zeigerargumente | `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9` | `rcx`, `rdx`, `r8`, `r9` |
| Gleitkommaargumente | `xmm0` bis `xmm7` | `xmm0` bis `xmm3` |
| Weitere Argumente | Stapel | Stapel, oberhalb des Schattenbereichs |
| Ergebnis | `rax` (`xmm0` für Gleitkomma) | `rax` (`xmm0` für Gleitkomma) |
| Schattenbereich | Keine | 32 Bytes, vom Aufrufer reserviert |
| Red Zone | 128 Bytes unter `rsp` für Blattfunktionen | Keine |
| Aufrufe mit variabler Argumentzahl | `al` enthält die Zahl der verwendeten Vektorregister | Gleitkommaargumente werden zusätzlich in Ganzzahlregister kopiert |
| Vom Aufgerufenen gesichert | `rbx`, `rbp`, `r12` bis `r15` | `rbx`, `rbp`, `rdi`, `rsi`, `r12` bis `r15`, `xmm6` bis `xmm15` |

## Stapelausrichtung

`rsp` muss im 64-Bit-Code im Moment jedes `call` ein Vielfaches von 16 sein. Beim ersten Befehl einer Funktion ist es 8 modulo 16, weil die Rücksprungadresse gerade abgelegt wurde.

## Eine Konvention in einer Binärdatei erkennen

- Ein `ret` mit einer Zahl: stdcall oder fastcall.
- `ecx` oder `edx` werden in 32-Bit-Code gelesen, bevor sie geschrieben werden: fastcall.
- Ein Aufrufer, der nach einem Aufruf `add esp, N` ausführt: cdecl.
- Zuerst gelesene `rdi` und `rsi`: System V. Zuerst gelesene `rcx` und `rdx`, mit Schreibzugriffen nach `[rsp + 8]` und darüber: Microsoft x64.
- Ein Aufrufer, der vor einem Aufruf `sub rsp, 0x28` ausführt (oder ein größeres Vielfaches von 8, das die Ausrichtung erhält): Schattenbereich von Microsoft x64.
