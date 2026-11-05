<!-- source: README.md @ sha256:c29540243c82 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 05. Kartenzähler

Level 2. Zeitaufwand: etwa 2 Stunden. Architektur: x86 (32 Bit) Linux. Module: [01. Datendarstellung](../../modules/01-data-representation/README.de.md), [03. Ihr erstes Programm](../../modules/03-first-program/README.de.md), [05. Arithmetik und Logik](../../modules/05-arithmetic-and-logic/README.de.md), [06. Bedingungen, Verzweigungen und Schleifen](../../modules/06-conditions-and-loops/README.de.md)

## Szenario

Ein Kartenbüro addiert die Verkäufe zweier Schalter mit einem alten 32-Bit-Werkzeug namens `ticket_counter`. Die Zahlen werden als Text eingegeben und können länger sein, als ein 32-Bit-Register fassen kann: Eine 12-stellige Zahl ist größer als 2^32. Das Büro wechselt auf ein neues System und braucht das Verhalten des Werkzeugs genau dokumentiert, auch den Umgang mit fehlerhaften Eingaben, bevor es ersetzt werden kann.

## Ziele

- Die Regeln eines Programms aus seiner Disassemblierung und aus Experimenten ermitteln.
- Den ASCII-Zifferntest und den Exit-Status in 32-Bit-Code lesen.
- Als Text gegebene Zahlen Ziffer für Ziffer mit Übertrag addieren, ohne sie in Ganzzahlen umzuwandeln.

## Teil 1: Reverse

Die Binärdatei ist `bin/ticket_counter`. Sie liest zwei Zeilen von der Standardeingabe und gibt ein Ergebnis aus.

```bash
printf '12\n34\n' | bin/ticket_counter ; echo "exit status: $?"
objdump -d -M intel bin/ticket_counter
strace bin/ticket_counter
```

Experimentieren Sie mit den Eingaben und lesen Sie den Code, um diese Fragen zu beantworten:

1. Wie viele Ziffern hat die längste Eingabe, die das Programm akzeptiert?
2. Welchen Exit-Status liefert es bei einer ungültigen Eingabe?
3. Welchen Exit-Status liefert es, wenn die Summe zu groß zum Ausgeben ist?
4. Was gibt es für die Eingaben `000` und `0000` aus? (Prüfen Sie Ihre Antwort durch Ausführen.)

Bilden Sie das Flag aus Ihren Antworten:

```text
ARL{ticket_<longest input>_<status for invalid input>_<status for a sum that is too large>}
```

Wäre die längste Eingabe zum Beispiel 8 Ziffern lang, der erste Status 3 und der zweite Status 4, lautete das Flag `ARL{ticket_8_3_4}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um ein vollständiges 32-Bit-Programm zu schreiben, das sich genau wie `bin/ticket_counter` verhält: derselbe Text auf der Standardausgabe und derselbe Exit-Status für jede Eingabe. Bauen Sie es im Ordner `starter` und probieren Sie es aus:

```bash
make run
```

Das Prüfprogramm führt Ihr Programm mit 80 Eingaben aus: gewöhnliche Summen, Überträge durch jede Ziffer, führende Nullen, Eingaben genau an und knapp hinter jeder Grenze sowie fehlerhafte Eingaben verschiedener Art.

## Regeln

- Schreiben Sie 32-Bit-Code, der die Schnittstelle `int 0x80` und keine C-Bibliothek verwendet.
- Wandeln Sie die Zahlen nicht in ein einzelnes Register um. Eine 12-stellige Zahl passt nicht in 32 Bit, addieren Sie also Ziffer für Ziffer.
- Ihr Programm darf `ticket_counter` nicht ausführen.

## Arbeit prüfen

```bash
make check EX=05 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) Stufe für Stufe.
