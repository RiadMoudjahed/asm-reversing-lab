<!-- source: README.md @ sha256:26989c484734 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 09. Was Reverse Engineering ist

Zeitaufwand: etwa 1 Stunde. Voraussetzungen: [08. Der Stapel, Prozeduren und Aufrufkonventionen](../08-stack-and-calls/README.de.md)

## Warum das wichtig ist

Die Module 00 bis 08 haben Ihnen das Schreiben von Assembler beigebracht. Reverse Engineering ist dieselbe Fähigkeit in der Gegenrichtung: Sie beginnen mit einem fertigen Programm und ermitteln, was es tut und wie. Vor den Werkzeugen und Formaten der nächsten Module brauchen Sie eine klare Definition, eine Reihe legitimer Einsatzzwecke, die Grenzen, die Sie einhalten müssen, und eine wiederholbare Arbeitsweise, damit Sie sich in einer Binärdatei nicht verlieren.

## Konzepte

### Definition

**Reverse Engineering** bedeutet, Entwurf, Verhalten oder Daten eines Produkts aus dem fertigen Produkt selbst zu erschließen, ohne die ursprünglichen Entwurfsunterlagen. Bei Software ist das Produkt meist eine ausführbare Datei, eine Bibliothek oder ein Firmware-Abbild, und das Ergebnis ist Wissen: die Beschreibung eines Dateiformats, die Logik einer Funktion, die Liste der Aktionen, die ein Programm ausführt.

Es ist nicht dasselbe, wie das Programm neu zu schreiben. Das Ziel ist Verständnis. Neu zu schreiben ist manchmal ein späterer Schritt, wie in Übung 08 und Übung 09, wo Sie Funktionen neu implementieren, die Sie zuvor verstanden haben.

### Legitime Einsatzzwecke

| Zweck | Beispiel |
|---|---|
| Interoperabilität | Ein undokumentiertes Dateiformat dokumentieren, damit ein anderes Werkzeug es lesen kann |
| Sicherheitsprüfung | Eine Closed-Source-Komponente auf Speichersicherheitsfehler prüfen |
| Malware-Analyse | Herausfinden, was eine feindliche Probe tut, damit Verteidiger sie erkennen und entfernen können |
| Verlorenes Wissen wiederherstellen | Ein Werkzeug verstehen, dessen Quelltext verloren ist |
| Fehlersuche | Herausfinden, warum ein Absturz in Code auftritt, den Sie nicht mit Symbolen neu bauen können |
| Lernen | Sehen, was ein Compiler aus dem eigenen Code macht |

### Grenzen

Dieser Kurs lehrt Analyse und Verständnis. Er lehrt Folgendes nicht, und seine Übungen verlangen es nie:

- Exploits, Shellcode oder Malware entwickeln;
- Kopierschutz, Lizenzierung oder digitale Rechteverwaltung umgehen;
- jede Tätigkeit gegen Systeme oder Software, die zu untersuchen Sie kein Recht haben.

Bevor Sie ein Programm analysieren, das nicht Ihnen gehört, prüfen Sie drei Dinge: wem es gehört, was die Lizenz oder der Vertrag zur Analyse sagt und was das Gesetz in Ihrem Land sagt. Interoperabilität und Sicherheitsforschung genießen an manchen Orten besonderen rechtlichen Schutz und an anderen keinen. Die Regeln unterscheiden sich zwischen den Ländern und ändern sich im Lauf der Zeit; dieser Kurs gibt daher keine Rechtsberatung. Fragen Sie im Zweifel zuerst eine qualifizierte Person. Jede Binärdatei in diesem Repository wird aus Quelltext gebaut, der im Repository enthalten ist, die Arbeit an den Übungen ist also immer erlaubt. Siehe [ETHICS.md](../../ETHICS.de.md).

### Zwei Ebenen des Reversing

**Reversing auf Systemebene** betrachtet, wie sich ein Programm von außen verhält: welche Dateien es öffnet, welche Netzwerkverbindungen es aufbaut, welche Systemaufrufe es absetzt, was es auf die Platte schreibt. Das Programm wird als Black Box behandelt und beobachtet.

**Reversing auf Codeebene** liest die Befehle des Programms, um genau zu erfahren, wie es rechnet. Es ist langsamer und genauer. Echte Untersuchungen wechseln zwischen beiden: Die Außensicht sagt, wo man hinsehen muss, und die Codesicht sagt, warum.

### Statische und dynamische Analyse

| | Statische Analyse | Dynamische Analyse |
|---|---|---|
| Was Sie tun | Die Datei lesen, ohne sie auszuführen | Das Programm ausführen und beobachten |
| Werkzeuge | `file`, `strings`, `readelf`, `objdump`, ein Disassembler, ein Dekompilierer | Ein Debugger, `strace`, `ltrace` |
| Stärken | Sicher; sieht den gesamten Code, auch Pfade, die nie laufen | Zeigt echte Werte; entpackt Daten, die das Programm selbst dekodiert |
| Schwächen | Sieht keine zur Laufzeit berechneten Werte; verschleierter Code ist schwer lesbar | Sieht nur die tatsächlich durchlaufenen Pfade; unbekannten Code auszuführen erfordert eine sichere Umgebung |

Keine ist besser. Ein guter Ablauf nutzt die statische Analyse, um eine Frage zu bilden, und die dynamische Analyse, um sie zu beantworten, und kehrt dann mit den neuen Fakten zur statischen Analyse zurück.

### Ein wiederholbarer Ablauf

1. **Triage.** Identifizieren Sie die Datei, ohne sie auszuführen: Typ, Architektur, Größe, Hash, Zeichenketten, importierte Funktionen, Abschnitte. Das dauert Minuten und beantwortet die Frage oft oder zeigt, wo man suchen muss.
2. **Beobachten.** Führen Sie das Programm an einem sicheren Ort aus und halten Sie fest, was es von außen tut: Ausgabe, Systemaufrufe, Dateien.
3. **Eine Frage bilden.** Notieren Sie eine konkrete Sache, die Sie wissen müssen, zum Beispiel „Wo wird die Eingabe geprüft?“ oder „Wie ist dieser Datensatz aufgebaut?“.
4. **Den Code finden.** Nehmen Sie Zeichenketten, Importe und Systemaufrufe als Ausgangspunkte und folgen Sie den Querverweisen zum Code, auf den es ankommt.
5. **Lesen und benennen.** Lesen Sie die Befehle, ermitteln Sie, was jede Funktion tut, und geben Sie ihr einen aussagekräftigen Namen. Namen sind das wichtigste Ergebnis der Arbeit.
6. **Die Idee prüfen.** Prüfen Sie Ihr Verständnis mit einem Debugger, indem Sie den Code mit gewählten Eingaben schrittweise ausführen, oder indem Sie die Funktion neu implementieren und die Ergebnisse vergleichen.
7. **Berichten.** Halten Sie fest, was Sie gefunden haben, woher Sie es wissen und was unbekannt bleibt. Trennen Sie Fakten von Vermutungen.

Machen Sie unterwegs Notizen. In einer großen Binärdatei lässt das Gedächtnis schnell nach, und eine Notiz wie „Die Funktion bei 0x401136 kopiert den Kopfbereich in einen Puffer von 32 Bytes“ spart am nächsten Tag eine Stunde.

### Ein sicherer Ort zum Ausführen von Programmen

Führen Sie unbekannte Programme nur in einer Umgebung aus, die Sie wegwerfen können. Für diesen Kurs genügt der Lab-Container, weil alle Binärdateien aus diesem Repository stammen. Für echte Proben verwenden Sie eine virtuelle Maschine ohne Netzwerk und mit einem Snapshot, zu dem Sie zurückkehren können. Führen Sie niemals eine unbekannte Probe auf dem Rechner aus, der Ihre Dateien enthält.

## Ausprobieren

Führen Sie die Triage des kleinsten Programms im Kurs durch, ohne es auszuführen. Die Befehle laufen in `modules/03-first-program/examples`, nachdem `make` die Beispiele gebaut hat.

```bash
file build/03_hello64
strings -n 4 build/03_hello64 | head -6
readelf -h build/03_hello64 | grep -E "Type|Entry"
```

Tatsächliche Ausgabe:

```text
build/03_hello64: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, with debug_info, not stripped
Hello, assembly!
03_hello64.asm
/lab/modules/03-first-program/examples/
NASM 2.16.01
03_hello64.asm
YYuY0Y/
  Type:                              EXEC (Executable file)
  Entry point address:               0x401000
```

Ohne irgendetwas auszuführen, wissen Sie: Dies ist ein 64-Bit-Linux-Programm für x86-64 ohne Bibliotheken, es wurde in Assembler mit NASM geschrieben (die Herstellerzeichenkette und der Name der Quelldatei stehen in der Datei, ebenso das Build-Verzeichnis), die Symbole wurden nicht entfernt, und es gibt eine Begrüßung aus. Das ist Triage.

Beobachten Sie es nun von außen:

```bash
strace -q build/03_hello64
```

```text
execve("build/03_hello64", ["build/03_hello64"], 0x7ffcb16ebed8 /* 15 vars */) = 0
write(1, "Hello, assembly!\n", 17Hello, assembly!
)      = 17
exit(0)                                 = ?
+++ exited with 0 +++
```

Die Systemaufrufe bestätigen, was die Zeichenketten nahelegten: ein `write` von 17 Bytes auf die Standardausgabe, dann `exit`. Die eigene Ausgabe des Programms ist in den Trace gemischt, weil beide auf dem Terminal landen. Das ist Reversing auf Systemebene; die Befehle zu lesen wäre Reversing auf Codeebene.

Schreiben Sie in drei Zeilen auf, was Sie über diese Datei wissen und woher Sie es wissen. Das ist die Gewohnheit, die Sie aufbauen sollen.

## Häufige Fehler

- **Im Disassembler beginnen.** Ohne Triage lesen Sie Code, ohne zu wissen, wofür das Programm gedacht ist. Verbringen Sie die ersten Minuten mit der Außensicht.
- **Alles lesen.** Eine Binärdatei kann Tausende Funktionen enthalten. Folgen Sie der Frage, nicht der Reihenfolge in der Datei.
- **Namen vertrauen.** Symbolnamen und Zeichenketten können falsch oder absichtlich irreführend sein. Behandeln Sie sie als Hinweise, die bestätigt werden müssen.
- **Unbekannten Code auf dem eigenen Rechner ausführen.** Verwenden Sie eine Wegwerfumgebung.
- **Fakten und Vermutungen in Notizen mischen.** Kennzeichnen Sie, was Sie geprüft und was Sie angenommen haben.

## Zusammenfassung

- Reverse Engineering ermittelt aus dem Programm selbst, wie es funktioniert; das Ziel ist Verständnis.
- Legitime Einsatzzwecke sind Interoperabilität, Sicherheitsprüfung, Malware-Analyse, die Wiederherstellung verlorenen Wissens und Lernen. Prüfen Sie Eigentum, Lizenz und Recht, bevor Sie Software analysieren, die nicht Ihnen gehört.
- Statische Analyse liest die Datei, dynamische Analyse führt sie aus. Gute Arbeit wechselt zwischen beiden.
- Ein Ablauf aus Triage, Beobachtung, Frage, Code finden, Benennen, Prüfen und Berichten hält die Arbeit fokussiert.
- Führen Sie unbekannte Programme nur in einer Wegwerfumgebung aus.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 10, Betriebssystemgrundlagen für Reverser.
