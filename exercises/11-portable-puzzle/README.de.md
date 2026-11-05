<!-- source: README.md @ sha256:306f7c5d94e2 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 11. Portable-Rätsel

Level 3. Zeitaufwand: etwa 4 Stunden. Architektur: x86-64 Windows (unter Wine). Module: [08. Der Stapel, Prozeduren und Aufrufkonventionen](../../modules/08-stack-and-calls/README.de.md), [10. Grundlagen von Betriebssystemen für Reverser](../../modules/10-os-for-reversers/README.de.md), [12. Ausführbare Formate: ELF und PE](../../modules/12-executable-formats/README.de.md)

## Szenario

Ein Windows-Konsolenwerkzeug, `puzzle.exe`, trifft ohne Dokumentation ein. Ein Paketierungsteam braucht einige Angaben aus seinen Headern, und es möchte außerdem eine kleine Hilfsbibliothek, in Assembler geschrieben, damit sie keine Laufzeitabhängigkeiten hat, die herausfindet, wo eine gegebene Adresse eines geladenen PE-Images in der Datei auf der Platte liegt. Zuerst lesen Sie die Header selbst. Dann schreiben Sie die Hilfsbibliothek.

## Ziele

- Die Header, die Abschnittstabelle und das Importverzeichnis einer PE32+-Datei auslesen.
- Eine relative virtuelle Adresse (RVA) in einen Dateioffset umrechnen.
- Windows-x64-Funktionen schreiben, die die Microsoft-Aufrufkonvention einhalten: Argumentregister, Schattenbereich, Stapelausrichtung und nicht flüchtige Register.

## Teil 1: Reverse

Die Binärdatei ist `bin/puzzle.exe`. Es ist ein Konsolenprogramm, das eine Zeile ausgibt; Sie können es unter Wine ausführen.

```bash
wine bin/puzzle.exe
x86_64-w64-mingw32-objdump -p bin/puzzle.exe
python3 ../../modules/12-executable-formats/examples/12_pe_dump.py bin/puzzle.exe
```

Ermitteln Sie diese sechs Werte:

1. `S`: die Anzahl der Abschnitte, dezimal.
2. `I`: die Image-Basis, in Kleinbuchstaben hexadezimal ohne `0x`.
3. `R`: die RVA des Einstiegspunkts, in Kleinbuchstaben hexadezimal ohne `0x`.
4. `O`: der Dateioffset des Bytes bei RVA `0x12348`, in Kleinbuchstaben hexadezimal ohne `0x`.
5. `D`: die Anzahl der Bibliotheken im Importverzeichnis, dezimal.
6. `K`: die Anzahl der aus `KERNEL32.dll` importierten Funktionen, dezimal.

```text
ARL{portable_<S>_<I>_<R>_<O>_<D>_<K>}
```

Die Werte 5, 140000000, 1410, 7a30, 2 und 9 ergäben zum Beispiel `ARL{portable_5_140000000_1410_7a30_2_9}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um zwei Funktionen in der Microsoft-x64-Aufrufkonvention zu implementieren.

```c
#include <stdint.h>

uint32_t pe_rva_to_offset(const void *sections, uint32_t count, uint32_t rva);
uint32_t pe_entry_offset(const void *file);

/* Provided by the tests. */
void log_rva(uint32_t rva);
```

- `pe_rva_to_offset` erhält einen Zeiger auf eine Abschnittstabelle mit `count` Einträgen und eine RVA. Es gibt den Dateioffset dieser RVA zurück oder `0xFFFFFFFF`, wenn kein Abschnitt die RVA in seinen Rohdaten enthält. Ein Abschnitt enthält die RVAs von seiner virtuellen Adresse bis ausschließlich seiner virtuellen Adresse plus seiner Größe der Rohdaten.
- `pe_entry_offset` erhält einen Zeiger auf den Anfang einer PE-Datei im Speicher. Beginnt die Datei nicht mit `MZ` oder beginnen die NT-Header nicht mit `PE\0\0`, gibt es `0xFFFFFFFF` zurück. Andernfalls liest es die RVA des Einstiegspunkts aus dem optionalen Header und ruft `log_rva` einmal mit dieser RVA auf. Danach gibt es den Dateioffset des Einstiegspunkts zurück, berechnet mit der Abschnittstabelle, die auf den optionalen Header folgt. Liegt der Einstiegspunkt in keinem Abschnitt, gibt es `0xFFFFFFFF` zurück.

Sie brauchen das Layout der Header und der Abschnittstabelle; in Teil 1 finden Sie es.

## Regeln

- Befolgen Sie die Microsoft-x64-Konvention: Argumente in `rcx`, `rdx`, `r8` und `r9`; 32 Bytes Schattenbereich bei jedem Aufruf; `rsp` bei jedem `call` ein Vielfaches von 16.
- Erhalten Sie `rbx`, `rbp`, `rdi`, `rsi` und `r12` bis `r15`. Beachten Sie, dass `rdi` und `rsi` unter Windows nicht flüchtig sind.
- `log_rva` zerstört jedes flüchtige Register und schreibt in seinen Schattenbereich. Halten Sie, was Sie über den Aufruf hinweg brauchen, in nicht flüchtigen Registern oder in Ihrem eigenen Stapelrahmen.
- Rufen Sie keine andere Funktion auf.
- Ergebnisse haben 32 Bit und laufen als vorzeichenlose Zahlen über.

## Arbeit prüfen

```bash
make check EX=11 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm baut ein Testprogramm mit dem MinGW-Cross-Compiler, bindet es mit Ihrer Objektdatei und führt es unter Wine aus. Es meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2. Jeder Test in Teil 2 gibt ein Ergebnis aus, gefolgt von `ok` oder einer Liste dessen, was schiefging: `align`, `regs` oder `calls`.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) jeweils eine Stufe.
