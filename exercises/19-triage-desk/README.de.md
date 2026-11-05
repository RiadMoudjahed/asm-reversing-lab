<!-- source: README.md @ sha256:091712ebb014 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 19. Triage-Schreibtisch

Level 5. Zeitaufwand: etwa 5 Stunden. Architektur: x86-64 Linux und Windows. Module: [19. Verdächtige Programme analysieren](../../modules/19-analyzing-suspicious-programs/README.de.md), [20. Verschleierten Code lesen](../../modules/20-reading-obfuscated-code/README.de.md)

## Szenario

Das Sicherheitsteam einer kleinen Firma hat nach einem Alarm auf einem Arbeitsplatzrechner ein Programm namens `dispatch` erhalten, dazu eine Windows-Fassung desselben Programms, `dispatch.exe`. Niemand weiß, was es tut. Es muss noch vor Feierabend bewertet werden: Was tut es, wie kann das Team weitere Kopien finden, und wie kann das Team seine Konfiguration lesen, ohne etwas Riskantes auszuführen? Das Programm dieser Übung ist ein eigens für den Kurs geschriebener Simulator. Es gibt nur aus, was es tun würde, und führt nichts aus. Sie schreiben die Triage-Notizen und die Erkennungsregel und anschließend die Routine, die seine Konfiguration dekodiert.

## Ziele

- Ein unbekanntes Programm in fester Reihenfolge bewerten: Identifikation, Zeichenketten, Importe, Verhalten und Dekodierung versteckter Daten.
- Eine kodierte Konfiguration finden, die Dekodierregel aus dem Code gewinnen und jeden Eintrag dekodieren.
- Beobachtetes Verhalten ATT&CK-Technikkennungen zuordnen und angeben, was die Belege stützen.
- Eine YARA-Regel schreiben, die das Beispiel findet und bei Programmen, die ihm nur ähneln, nicht anschlägt.
- Den Dekodierer in Assembler neu umsetzen, damit er auf Konfigurationen anderer Proben angewandt werden kann.

## Teil 1: Reverse

Die Dateien sind `bin/dispatch` (Linux) und `bin/dispatch.exe` (Windows). Sie geben einige Protokollzeilen aus und beenden sich. Sie auszuführen ist ungefährlich, und dort sollten Sie beginnen, aber die Protokollzeilen enthalten nur Platzhalter für die interessanten Daten: Die Daten selbst liegen kodiert vor und werden nur im Speicher dekodiert.

```bash
bin/dispatch
file bin/dispatch bin/dispatch.exe
strings -n 6 bin/dispatch
```

Arbeiten Sie die Triage aus Modul 19 und die Dekodiermethoden aus Modul 20 durch. Finden Sie die Tabelle mit den kodierten Einträgen, gewinnen Sie die Dekodierregel aus dem Code und dekodieren Sie alle Einträge. Bereiten Sie dann diese Werte und Dateien vor:

1. `N`: die Anzahl der Einträge der kodierten Konfiguration, dezimal.
2. `L`: die Länge des längsten dekodierten Eintrags in Bytes, dezimal.
3. Die ATT&CK-Technikkennungen von drei Verhaltensweisen, jeweils als vollständige Kennung der Technik oder Untertechnik (ein `T`, vier Ziffern, ein Punkt und drei Ziffern), im Flag aber ohne das `T` und den Punkt: das Anlegen eines Dienstes, das Ablegen einer Bibliothek neben einem vertrauenswürdigen Programm, damit das Programm sie lädt, und die Nutzung von DNS-Namen als Träger für Daten.
4. Eine YARA-Regel in `starter/rule.yar`. Sie muss auf `bin/dispatch` und auf `bin/dispatch.exe` zutreffen und auf keines der anderen Programme, die das Prüfprogramm ausprobiert: drei Programme, die dieselben Themen harmlos erwähnen (ein Hilfetext, ein Probelauf, ein Aktualisierer, der dieselbe Dekodierroutine für harmlose Texte benutzt) in beiden Formaten sowie ein Systemprogramm. Eine Regel, die bei einem davon anschlägt, würde die Analysten mit Fehlalarmen überfluten.

Das Flag verbindet die ersten drei Werte in dieser Reihenfolge: `N`, `L`, dann die drei Kennungen (Dienst, Side-Loading, DNS) ohne das `T` und den Punkt.

```text
ARL{triage_<N>_<L>_<service>_<sideload>_<dns>}
```

Die Werte 3 und 20 und die Kennungen T1234.567, T1234.568 und T1234.569 ergäben zum Beispiel `ARL{triage_3_20_1234567_1234568_1234569}`.

Die Protokollzeilen beschreiben zwei weitere Verhaltensweisen, die Sie in Ihren Notizen zuordnen sollten, obwohl das Flag sie nicht verlangt: wie das Programm etwas über das System um sich herum erfährt und was es mit den Spuren macht, die es hinterlässt. Das Schreiben der Notizen gehört zur Übung. Das Prüfprogramm liest sie nicht; der Lösungsweg in `solutions/19/` enthält ein Muster, das Sie danach zum Vergleich nutzen können.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um eine Funktion in der System-V-Aufrufkonvention zu implementieren:

```c-fragment
long decode_record(const unsigned char *rec, size_t avail, char *out, size_t cap);
```

`rec` zeigt auf einen kodierten Eintrag in dem Aufbau, den Sie in Teil 1 gefunden haben, und `avail` ist die Anzahl der Bytes, die Sie ab `rec` lesen dürfen. Die Funktion schreibt den dekodierten Text, gefolgt von einem Nullbyte, nach `out` und gibt die Länge des Textes zurück. Die Regeln, in der Reihenfolge ihrer Anwendung:

- Ist `avail` kleiner als 2 oder kleiner als 2 plus die Länge, die im ersten Byte des Eintrags steht, liegt der Eintrag nicht vollständig in den lesbaren Bytes: -1 zurückgeben und nichts schreiben.
- Ist `cap` kleiner als die Länge des Textes plus eins, passt die Ausgabe nicht: -2 zurückgeben und nichts schreiben.
- Andernfalls den Eintrag genau wie der Simulator dekodieren, den Text und das abschließende Nullbyte schreiben und die Länge des Textes zurückgeben. Ein Eintrag der Länge 0 ergibt einen leeren Text, das Nullbyte und den Rückgabewert 0.

Die Tests enthalten die Einträge des Beispiels, dieselben Einträge an jeder Stelle abgeschnitten, Einträge mit Randwerten für Länge und Seed sowie zufällige Einträge. Sie vergleichen Rückgabewert und Ausgabe mit einer Referenz.

## Regeln

- Lesen Sie nie ein Byte bei oder hinter `rec + avail`. Die Tests legen den Eintrag direkt vor eine nicht abgebildete Seite, und ein Lesen über das Ende hinaus beendet den Testlauf mit einem Signal.
- Schreiben Sie nie über `cap` Bytes von `out` hinaus, und schreiben Sie nichts, wenn Sie einen Fehler zurückgeben.
- Sichern Sie die Register, die der Aufgerufene erhalten muss: `rbx`, `rbp` und `r12` bis `r15`.
- Halten Sie den Stack bei jedem `call` ausgerichtet.

## Arbeit prüfen

```bash
make check EX=19 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Teil 1 gilt nur dann als bestanden, wenn das Flag richtig ist und die YARA-Regel in `starter/rule.yar` richtig ist; die Ausgabe des Prüfprogramms nennt jedes Programm, bei dem die Regel die falsche Antwort gibt. Ohne `--flag` prüft es nur Teil 2.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) jeweils eine Stufe.
