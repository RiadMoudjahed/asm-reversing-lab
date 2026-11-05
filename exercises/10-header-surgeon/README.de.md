<!-- source: README.md @ sha256:b4dcee2d886b -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 10. Header-Chirurg

Level 3. Zeitaufwand: etwa 3 Stunden. Architektur: x86-64 Linux. Module: [12. Ausführbare Formate: ELF und PE](../../modules/12-executable-formats/README.de.md)

## Szenario

Ein Lieferant hat ein ELF-Programm namens `surgeon` geliefert, dessen Build-Einstellungen niemand aufgeschrieben hat. Das Deployment-Team braucht Image-Basis, Einstiegspunkt und einige Größenangaben, bevor es das Programm in einen eingeschränkten Starter einbinden kann. Außerdem braucht es eine minimale Testdatei, die dieselbe Image-Basis verwendet, aber keine Bibliotheken und keinen Compiler dahinter hat, damit der Starter ohne Abhängigkeiten geprüft werden kann. Diese Testdatei muss von Hand geschrieben werden, Header für Header.

## Ziele

- Die Header, Programmkopfzeilen, Abschnitte, dynamischen Einträge und Relokationen einer ELF-Datei mit Kommandozeilenwerkzeugen lesen.
- Zwischen virtuellen Adressen und Dateioffsets umrechnen.
- Mit `nasm -f bin` eine gültige ausführbare Datei schreiben, einschließlich der Kongruenzregel zwischen Dateioffsets und virtuellen Adressen.

## Teil 1: Reverse

Die Binärdatei ist `bin/surgeon`. Sie läuft und gibt eine Zeile aus.

```bash
bin/surgeon
readelf -h -l -S -d bin/surgeon
```

Ermitteln Sie diese sechs Werte:

1. `B`: die Image-Basis, die niedrigste virtuelle Adresse eines `PT_LOAD`-Segments. Schreiben Sie sie in Kleinbuchstaben hexadezimal ohne `0x`.
2. `E`: der Dateioffset des Einstiegspunkts, in Kleinbuchstaben hexadezimal ohne `0x`.
3. `P`: die Anzahl der Programmkopfzeilen, dezimal.
4. `M`: wie viele Bytes des les- und schreibbaren `PT_LOAD`-Segments nur im Speicher existieren, also `MemSiz` minus `FileSiz`, dezimal.
5. `V`: die Größe in Bytes, dezimal, des einen Abschnitts, dessen Name kein Standardname eines Compilers oder Linkers ist.
6. `L`: der Name der gemeinsam genutzten Bibliothek außer der C-Bibliothek, ohne das Präfix `lib` und ohne das Suffix `.so` samt Version (für `libz.so.1` schreiben Sie `z`).

```text
ARL{header_<B>_<E>_<P>_<M>_<V>_<L>}
```

Die Werte 400000, 1050, 13, 8, 21 und z ergäben zum Beispiel `ARL{header_400000_1050_13_8_21_z}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, sodass `nasm -f bin` eine vollständige ausführbare Datei schreibt. Es wird nichts ergänzt: Sie schreiben jedes Header-Byte selbst. Das Programm gibt `header surgery ok` gefolgt von einem Zeilenumbruch auf der Standardausgabe aus und endet mit dem Status 0.

Die Datei muss alle folgenden Bedingungen erfüllen:

- Sie ist eine 64-Bit-ELF-Datei in Little-Endian-Darstellung, ausführbar für x86-64.
- Die niedrigste virtuelle Adresse eines `PT_LOAD` ist die Image-Basis `B` von `bin/surgeon`.
- Sie hat ein `PT_LOAD`-Segment mit den Flags Lesen + Ausführen, das die Header und den Code enthält, und ein anderes `PT_LOAD`-Segment mit den Flags Lesen + Schreiben, das die Nachricht enthält. Kein Segment darf zugleich schreibbar und ausführbar sein.
- Für jedes `PT_LOAD`-Segment haben `p_offset` und `p_vaddr` denselben Rest modulo `p_align`.
- Der Einstiegspunkt liegt innerhalb des Segments mit Lesen + Ausführen.
- Ein Programmkopf `PT_GNU_STACK` mit den Flags Lesen + Schreiben kennzeichnet den Stapel als nicht ausführbar.
- Der Nachrichtentext steht im Dateibereich des les- und schreibbaren Segments, und der Code liest ihn von dort.
- Die Datei ist höchstens 1024 Bytes lang.

Das Prüfprogramm liest `B` aus `bin/surgeon` selbst, untersucht dann die Header Ihrer Datei und führt sie aus.

## Regeln

- Verwenden Sie nur `nasm -f bin`: Das Prüfprogramm assembliert Ihre Datei auf diese Weise und führt das Ergebnis direkt aus.
- Verwenden Sie keinen Linker, keine C-Bibliothek und kein dynamisches Laden.
- Abschnittskopfzeilen sind nicht erforderlich.

## Arbeit prüfen

```bash
make check EX=10 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2. Für Teil 2 listet es jede Anforderung auf, gegen die Ihre Datei verstößt.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) jeweils eine Stufe.
