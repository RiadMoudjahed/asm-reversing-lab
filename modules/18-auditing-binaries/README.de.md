<!-- source: README.md @ sha256:2cc754f9080c -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 18. Binärdateien auf Fehler prüfen

Zeitaufwand: etwa 2,5 Stunden. Voraussetzungen: [17. Dateiformate entschlüsseln](../17-deciphering-file-formats/README.de.md)

## Warum das wichtig ist

Die meisten schweren Sicherheitslücken in nativem Code sind Speichersicherheitsfehler: Ein Programm kopiert, indiziert oder gibt Speicher auf eine Weise frei, die sein Autor nicht beabsichtigt hat. Wer kompilierten Code lesen kann, findet einen solchen Fehler auch in einer Komponente ohne Quelltext, schreibt einen Bericht, mit dem der Eigentümer arbeiten kann, und schlägt eine Korrektur vor, die sich testen lässt. Dieses Modul lehrt diese Arbeit aus der Sicht des Verteidigers. Es endet bei Diagnose und Reparatur. Es lehrt nicht, einen Fehler in einen Angriff zu verwandeln, und der Kurs verlangt das nie. Prüfen Sie nur Code, der Ihnen gehört oder den Sie untersuchen dürfen, und lesen Sie vorher [die Ethik-Erklärung](../../ETHICS.md).

## Konzepte

### Die Methode: von Quellen zu Senken

Ein Speichersicherheitsfehler braucht zwei Dinge: einen Wert, den das Programm nicht kontrolliert (aus einer Datei, einem Netzwerkpaket, einem Argument oder einem Feld eines Datensatzes gelesen), und eine Stelle, an der dieser Wert bestimmt, wie viel Speicher berührt wird. Das erste ist eine **Quelle**, das zweite eine **Senke**. Die Prüfung verfolgt jede Quelle zu jeder Senke und betrachtet die Prüfungen dazwischen.

| Senke | Frage |
|---|---|
| Ein Kopiervorgang (`memcpy`, `strcpy`, eine Schleife, die Bytes speichert) | Was begrenzt die Zahl der Bytes? Wird die Grenze mit der Größe des Ziels verglichen, und mit der richtigen Größe? |
| Ein Index in ein Array | Wird er zuerst mit der Länge verglichen, und ist der Vergleich vorzeichenbehaftet oder vorzeichenlos? |
| Eine Speicherreservierung (`malloc`, `calloc`, `realloc`) | Wie wird die Größe berechnet? Kann die Rechnung überlaufen? Verwendet späterer Code dieselben Zahlen? |
| Ein Aufruf von `free` | Wird der Zeiger danach auf irgendeinem Pfad verwendet? Wird er zweimal freigegeben? |
| Ein Aufruf einer Bibliotheksfunktion mit einem Format oder einer Zeichenkette | Nimmt sie eine Größe? Ist die Größe richtig? |

Suchen Sie in einem kompilierten Programm zuerst die Senken: Durchsuchen Sie das Disassembly nach Aufrufen der Kopier-, Reservierungs- und Freigabefunktionen und nach Speichervorgängen mit skaliertem Index. Gehen Sie dann vom Größenoperanden (`rdx` bei einem Aufruf von `memcpy`, `rdi` bei einem Aufruf von `malloc`) rückwärts bis zu der Stelle, von der der Wert stammt, und lesen Sie jeden Vergleich auf dem Weg.

### Der Übungscode

Die Datei `examples/18_bugs.c` hat sechs kleine Funktionen mit je einem Fehler einer anderen Klasse. `examples/18_fixed.c` hat dieselben Funktionen repariert. Die Listings dieser Lektion stammen aus einem Build von `18_bugs.c` mit `-O1`, `-fno-builtin`, `-mstringop-strategy=libcall` und `-D_FORTIFY_SOURCE=0`, wodurch die Bibliotheksaufrufe sichtbar bleiben, wie in vielen echten Binärdateien. Zur Bestätigung baut `make` die Demoprogramme auch mit AddressSanitizer, einer Compilerfunktion, die ein Programm beim ersten fehlerhaften Speicherzugriff anhält und einen Bericht ausgibt. Sie können es verwenden, wenn Sie den Quelltext haben; bei einer Binärdatei, die Sie nicht neu bauen können, verwenden Sie den Debugger.

```bash
make
make run
```

### Fehler 1: eine Länge, die niemand prüft

```text
0000000000000000 <set_name>:
   0:  push   rbp
   1:  push   rbx
   2:  sub    rsp,0x8
   6:  mov    rbx,rdi                      ; rbx = dst
   9:  movzx  ebp,BYTE PTR [rsi]           ; length = first byte of the packet, 0 to 255
   c:  add    rsi,0x1                      ; source = packet + 1
  10:  mov    rdx,rbp                      ; size for memcpy
  13:  call   memcpy
  18:  mov    BYTE PTR [rbx+rbp*1],0x0    ; terminator written at dst[length]
  1c:  add    rsp,0x8
  20:  pop    rbx
  21:  pop    rbp
  22:  ret
```

Die an `memcpy` übergebene Größe ist ein aus der Eingabe gelesenes, mit Nullen erweitertes Byte, und zwischen dem Laden und dem Aufruf gibt es keinen Vergleich. Die Funktion bekommt die Größe des Ziels gar nicht, sodass in ihr keine richtige Prüfung möglich ist: Die Funktion selbst ist der Mangel, weil ihrer Schnittstelle die nötige Information fehlt. Das Schreiben des Abschlusszeichens bei `dst[length]` ist ein zweiter Schreibvorgang mit demselben Problem.

Der Aufrufer in der Demo hat einen Puffer von 16 Bytes und eine Länge von 40. Im Debugger sehen Sie die Lage, bevor das Kopieren geschieht:

```text
Breakpoint 1, set_name (dst=0x7ffdef7e5010 "", pkt=0x14a982a0 "(", 'A' <repeats 40 times>) at 18_bugs.c:13
13        unsigned len = pkt[0];
0x14a982a0:    40
#1  0x0000000000401244 in main (argc=2, argv=0x7ffdef7e5168) at 18_demo_bug.c:20
20        set_name(dst, pkt);
$1 = 16
$2 = 40 '('
```

Die Befehle waren `break set_name`, `run name`, `x/ub $rsi`, `up`, `print sizeof dst` und `print pkt[0]` und danach `kill`. Das Programm wurde vor dem fehlerhaften Kopieren angehalten, und die Zahlen genügen für den Bericht: Das Ziel fasst 16 Bytes, und die Länge ist 40. Mit dem Sanitizer-Build nennt der Bericht dieselbe Zeile:

```text
ERROR: AddressSanitizer: stack-buffer-overflow on address ADDR at pc ADDR bp ADDR sp ADDR
WRITE of size 40 at ADDR thread T0
    #1 ADDR in set_name 18_bugs.c:14
    #2 ADDR in main 18_demo_bug.c:20
```

Korrektur: Die Funktion muss die Kapazität des Ziels und die Länge des Pakets erhalten und Eingaben ablehnen, die nicht hineinpassen. `18_fixed.c` hat `set_name(dst, cap, pkt, pkt_len)`, das eine Länge ablehnt, die größer als der Rest des Pakets oder nicht kleiner als die Kapazität ist (das letzte Byte gehört dem Abschlusszeichen).

### Fehler 2: um eins daneben

```text
0000000000000023 <copy_label>:
  ...
  30:  mov    rdi,rdx                      ; rdi = src
  33:  call   strlen                       ; rax = n
  38:  cmp    r12,rax                      ; cap against n
  3b:  jb     56 <copy_label+0x33>         ; cap < n: return -1
  3d:  lea    rdx,[rax+0x1]                ; size = n + 1
  41:  mov    rsi,rbx
  44:  mov    rdi,rbp
  47:  call   memcpy                       ; copies n + 1 bytes
```

Der Code lehnt die Zeichenkette nur ab, wenn sie länger als die Kapazität ist. Eine Zeichenkette mit genau `cap` Zeichen besteht die Prüfung, und das Kopieren schreibt dann `cap + 1` Bytes: die Zeichen und das Abschlusszeichen. Der Hinweis ist der Unterschied zwischen der verglichenen Größe (`n`) und der kopierten (`n + 1`, das `lea rdx,[rax+0x1]`). Hat eine Kopiergröße eine addierte Konstante, muss die Prüfung davor dieselbe Konstante einschließen. Der Sanitizer meldet einen Schreibvorgang der Größe 9 in einen 8-Byte-Puffer. In der reparierten Version ist der Sprung nach `cmp rax,r12` ein `jae`: ablehnen, wenn `n >= cap`.

### Fehler 3: Ganzzahlüberlauf vor einer Reservierung

```text
000000000000005d <make_table>:
  65:  mov    r13d,edi                     ; count
  68:  mov    r12d,esi                     ; elem
  6b:  imul   edi,esi                      ; total = count * elem, in 32 bits
  6e:  call   malloc                       ; malloc(total), rdi zero-extended
  ...
  8b:  mov    rdx,r12                      ; the loop uses elem ...
  93:  call   memset
  98:  add    ebx,0x1
  9e:  cmp    r13d,ebx                     ; ... for count iterations
  a1:  jne    8b
```

Die Multiplikation ist ein 32-Bit-`imul` zweier Werte aus der Eingabe, und das Ergebnis geht direkt an `malloc`. Die Schleife, die die Tabelle füllt, verwendet `count` und `elem` getrennt und sieht nie die reservierte Größe an. Läuft das Produkt über (eine Anzahl von 0x10000001 mit einer Elementgröße von 16 ergibt 16), ist die Reservierung winzig, und die Schleife schreibt weit darüber hinaus. Das Muster, nach dem man sucht, ist ein arithmetischer Befehl auf aus der Eingabe abgeleiteten Werten, dessen Ergebnis eine Reservierungsgröße ist, wobei dieselben Eingaben später für eine Schleife oder einen Index wieder verwendet werden. Der Sanitizer-Bericht ist ein Schreibvorgang der Größe 16 hinter dem Ende eines 16-Byte-Heap-Bereichs. Die Reparatur berechnet die Größe in 64 Bit, was für 32-Bit-Eingaben nicht überlaufen kann, und wendet vor der Reservierung eine Richtliniengrenze an. `__builtin_mul_overflow` leistet dasselbe, wenn die Operanden schon 64 Bit breit sind.

### Fehler 4: vorzeichenbehaftet und vorzeichenlos

```text
00000000000000af <copy_block>:
  af:  cmp    edx,0x40                     ; len against 64
  b2:  jg     c3 <copy_block+0x14>         ; signed: greater than 64 is refused
  b4:  push   rbx
  b5:  mov    ebx,edx
  b7:  movsxd rdx,edx                      ; sign-extend len to 64 bits
  ba:  call   memcpy                       ; size = rdx
```

Zwei Befehle zusammen bilden das Muster: ein vorzeichenbehafteter Vergleich (`jg`), der nur das obere Ende schützt, und eine Vorzeichenerweiterung (`movsxd`), wenn der Wert zur 64-Bit-Größe verbreitert wird. Ein negatives `len` besteht den Vergleich, wird zu einer riesigen vorzeichenlosen Zahl vorzeichenerweitert und erreicht `memcpy` als Größe. Der Sanitizer nennt das `negative-size-param`. In der reparierten Version erzeugt der Compiler einen einzigen vorzeichenlosen Vergleich (`cmp edx,0x40` und dann `ja`), weil der Quelltext auch `len < 0` ablehnt und ein negativer Wert als große vorzeichenlose Zahl gelesen wird. Merken Sie sich die Regel: Eine Größe ist vorzeichenlos, und eine Prüfung der Obergrenze bei einem vorzeichenbehafteten Wert ist ohne Prüfung der Untergrenze unvollständig.

### Fehler 5: unsichere Bibliotheksaufrufe

```text
00000000000000c9 <greet>:
  c9:  push   rbx
  ca:  sub    rsp,0x20                     ; a 32-byte frame; the buffer is 24 bytes of it
  ce:  mov    rdx,rdi                      ; the user string
  d1:  mov    rbx,rsi
  d4:  mov    esi,0x0                      ; the format "hello, %s" (a relocation)
  d9:  mov    rdi,rsp                      ; destination: a buffer on the stack
  dc:  mov    eax,0x0
  e1:  call   sprintf                      ; no size anywhere
  e6:  mov    rsi,rsp
  e9:  mov    rdi,rbx
  ec:  call   strcpy                       ; and the copy to the caller's buffer is unbounded too
```

Manche Bibliotheksfunktionen lassen sich nicht sicher verwenden, weil sie die Größe des Ziels nicht nehmen: `strcpy`, `strcat`, `sprintf`, `vsprintf`, `gets` und `scanf` mit einer `%s`-Umwandlung ohne Breite. Andere nehmen eine Größe und werden trotzdem falsch benutzt: `strncpy` fügt kein Abschlusszeichen an, wenn die Quelle so lang wie die Grenze ist, und `memcpy` nimmt eine Größe, die der Aufrufer prüfen muss. Ein Aufruf ist unsicher, wenn das Ziel ein Puffer fester Größe ist (ein Bereich des Stack-Frames oder eine globale Variable, hier `rdi = rsp`), die Quelle von der Eingabe abhängt und dazwischen kein Vergleich mit der Zielgröße liegt. Viele Distributionen kompilieren mit Härtung (fortification), die solche Aufrufe durch `__sprintf_chk` und ähnliche Funktionen ersetzt, die die Zielgröße als zusätzliches Argument erhalten (der `-O1`-Build mit Standardoptionen zeigt `__sprintf_chk` mit der Größe 0x18). Das macht aus dem stillen Überschreiben einen Abbruch, die Wirkung ist also milder, aber der Fehler ist noch da und verdient weiterhin einen Bericht. Eine `_chk`-Funktion in einer Binärdatei zeigt, dass der Code mit dem Schutz kompiliert wurde; die einfache Funktion zeigt, dass er es nicht wurde. Die Reparatur verwendet `snprintf` mit der Kapazität und prüft das Ergebnis.

### Fehler 6: Verwendung nach der Freigabe

```text
00000000000000f7 <finish>:
  f7:  push   rbx
  f8:  mov    rbx,rdi                      ; rbx = the item
  fb:  test   esi,esi
  fd:  je     103 <finish+0xc>             ; ok == 0: go to the free
  ff:  mov    eax,DWORD PTR [rbx]          ; read it->id
 101:  pop    rbx
 102:  ret
 103:  call   free                          ; free(rbx)
 108:  jmp    ff <finish+0x8>              ; and then continue at the read through rbx
```

Das Muster einer Verwendung nach der Freigabe (use after free) im Disassembly ist ein Pfad, der durch ein `call free` auf einen Zeiger geht und dann einen Befehl erreicht, der denselben Zeiger wieder dereferenziert, hier durch den Sprung zurück nach `ff`. Verfolgen Sie das Register oder den Stack-Slot, der das Argument von `free` hielt: Wird er nach dem Aufruf auf irgendeinem Pfad gelesen, benutzt der Code freigegebenen Speicher. Ein zweites `call free` auf denselben Wert ist eine doppelte Freigabe. Der Sanitizer meldet `heap-use-after-free` mit einem Lesen der Größe 4. In der reparierten Version liest `18_fixed.c` das Feld vor der Freigabe. Eine weitere gute Gewohnheit ist, nach der Freigabe einen Nullzeiger zu speichern, damit eine spätere Verwendung sofort scheitert.

### Einen Fund bestätigen, ohne Schaden anzurichten

Ein Fund ist mit Belegen nützlicher. Die sicheren Formen von Belegen sind:

- Führen Sie Ihren eigenen Build des Codes unter einem Sanitizer aus, mit einer Eingabe, die den Pfad auslöst. Das ist der beste Beleg, wenn Sie den Quelltext haben.
- Halten Sie in einem Debugger an der Senke an, bevor sie ausgeführt wird, und lesen Sie die Zahlen: die Größe und die Kapazität. Das braucht weder einen Absturz noch einen zu großen Schreibvorgang.
- Zeigen Sie bei der bloßen Binärdatei den Pfad im Disassembly: die Quelle, den fehlenden oder falschen Vergleich und die Senke.

Bauen Sie keine Eingaben, deren Zweck es ist, die Kontrolle über das Programm zu übernehmen. Für die Prüfung ist es unerheblich, was ein Angreifer als Nächstes tun könnte; wichtig ist, dass das Programm Speicher berührt, den es nicht berühren soll, und dass die Korrektur die Ursache beseitigt.

### Den Bericht schreiben

Einen Bericht liest die Person, die den Fehler beheben muss. Er muss genau und kurz sein. Verwenden Sie jedes Mal dieselben Felder.

```text
Title:      Unchecked packet length in set_name
Class:      CWE-787 out-of-bounds write (CWE-120, copy without checking the size)
Location:   set_name, call to memcpy at offset 0x13 (source line 14)
Severity:   High. The length and the bytes both come from the input, and the
            destination is a fixed buffer.
Cause:      set_name copies the number of bytes given by the first byte of the
            packet (0 to 255) into a destination whose size it does not know.
            No comparison separates the load of the length from the call.
Evidence:   Debugger, stopped before the copy: destination 16 bytes, length 40.
            Sanitizer build: stack-buffer-overflow, WRITE of size 40 at
            18_bugs.c:14.
Impact:     Memory outside the destination is overwritten with bytes from the
            packet. This corrupts adjacent data and can crash the program. What
            is next to the buffer depends on the caller; that was not assessed.
Fix:        Pass the capacity and the packet length. Reject a length larger than
            the rest of the packet or not smaller than the capacity. See
            set_name in 18_fixed.c.
Test:       Packets with lengths 0, 15, 16, 17 and 255 and a one-byte packet.
            The first two succeed, the rest are rejected without a write. Run
            the tests under the sanitizer.
```

Wählen Sie aus der öffentlichen Schwächenliste (CWE) die Klasse, die zur Ursache passt, nicht zum Symptom: 787 für einen Schreibzugriff außerhalb der Grenzen, 125 für einen Lesezugriff, 193 für einen Off-by-one-Fehler, 190 für Ganzzahlüberlauf oder Umlauf, 195 für eine Umwandlung von vorzeichenbehaftet in vorzeichenlos, 416 für die Verwendung nach der Freigabe, 415 für eine doppelte Freigabe und 676 für die Verwendung einer gefährlichen Funktion. Geben Sie die Schwere durch Überlegung an: wer die Eingabe kontrolliert, was geschrieben wird und wo. Sagen Sie, was Sie nicht beurteilt haben. Haben Sie den Fehler in Software gefunden, die jemand anderem gehört, melden Sie ihn dem Eigentümer vertraulich und geben Sie ihm Zeit zur Behebung, bevor Sie öffentlich etwas sagen.

### Die Korrektur prüfen

Eine Korrektur wird wie der Fehler geprüft. Führen Sie dieselbe Eingabe erneut aus und sehen Sie, dass der Bericht verschwunden ist. Führen Sie die Grenzwerte aus: den größten akzeptierten Wert, den kleinsten abgelehnten Wert, null, negativ und den Wert, der überliefe. Das Programm `demo_fix` tut das für die sechs reparierten Funktionen:

```text
name:   -1
name:   0 (alice)
label:  -1
label:  0
table:  refused
block:  -1
greet:  -1
greet:  0 (hello, bob)
finish: 7
```

Jede gefährliche Eingabe wird ohne Speicherzugriff abgelehnt, und die gewöhnliche Eingabe funktioniert weiter. Eine Korrektur, die das Problem nur verschiebt (zum Beispiel ein größerer Puffer), ist keine.

## Ausprobieren

Führen Sie die Lab-Befehle in `modules/18-auditing-binaries/examples` aus.

1. Bauen Sie mit `make` und führen Sie `make run` aus. Notieren Sie für jeden der sechs Fälle die Fehlerklasse und die Zeile des Berichts, bevor Sie die Erklärung der Lektion lesen.
2. Disassemblieren Sie `build/18_bugs.o` mit `objdump -dr -M intel`. Finden Sie den Aufruf von `memcpy` in `copy_block` und verfolgen Sie den Ursprung von `rdx`. Welcher Befehl hat es aus `edx` erzeugt?
3. Vergleichen Sie `build/18_bugs.o` und `build/18_fixed.o` mit demselben Befehl. Listen Sie für `copy_label` und `copy_block` die geänderten Befehle auf.
4. Führen Sie `gdb -nx -q build/demo_plain` aus und halten Sie mit den Befehlen der Lektion `set_name` vor seinem Kopieren an und lesen Sie die Zahlen. Fahren Sie mit der zu großen Eingabe nicht über den Haltepunkt hinaus fort.
5. Schreiben Sie einen Bericht im obigen Format für den Fehler in `finish`. Nehmen Sie die Klasse, die Stelle, die Ursache, den Beleg und die Korrektur auf.
6. Durchsuchen Sie ein eigenes Programm (ein kleines, das Sie geschrieben haben) nach Aufrufen von `strcpy`, `sprintf` und `memcpy` und entscheiden Sie jeweils, ob die Zielgröße geprüft wird.

## Häufige Fehler

- **Das Symptom prüfen.** Ein Absturz in einer Zeile stammt oft von einem früheren Fehler, etwa einer falsch berechneten Reservierungsgröße. Verfolgen Sie den Wert zu seiner Quelle zurück.
- **Die falsche Größe vergleichen.** Die Prüfung muss die Größe verwenden, die kopiert wird, einschließlich Abschlusszeichen und Header-Bytes.
- **Einem Typ vertrauen.** Eine als `int` gelesene und später als `size_t` übergebene Größe ändert ihre Bedeutung, wenn sie negativ ist. Finden Sie, wo sich der Typ ändert.
- **Den gehärteten Aufruf für sicher halten.** Eine `_chk`-Funktion macht aus Beschädigung einen Abbruch, aber ein Programm, das bei gültig aussehender Eingabe abbricht, hat weiterhin einen Fehler.
- **Nur eine Stelle beheben.** Dasselbe Muster erscheint meist anderswo. Suchen Sie danach, wenn Sie den ersten Fall behoben haben.
- **Zu viel beweisen.** Der Bericht braucht die Ursache und den Beleg, keine Vorführung dessen, was ein Angreifer tun könnte.

## Zusammenfassung

- Verfolgen Sie Eingabewerte von Quellen zu Senken und lesen Sie jeden Vergleich dazwischen.
- Sechs Muster decken die meisten Funde ab: eine ungeprüfte Länge, ein Off-by-one-Vergleich, ein arithmetischer Überlauf vor einer Reservierung, ein vorzeichenbehafteter Wert als Größe, ein unsicherer Bibliotheksaufruf und eine Verwendung nach der Freigabe.
- Jedes Muster hat im Disassembly ein Kennzeichen: einen Größenoperanden ohne Vergleich, eine verglichene Größe, die von der kopierten abweicht, ein `imul` auf Eingabewerten vor `malloc`, `jg` mit `movsxd`, einen Aufruf ohne Größe und eine Dereferenzierung nach `free`.
- Bestätigen Sie Funde mit einem Sanitizer-Build oder einem Debugger, der vor der Senke angehalten wird, nie mit einer schädlichen Eingabe.
- Ein Bericht nennt Klasse, Stelle, Ursache, Beleg, Auswirkung, Korrektur und Test; eine Korrektur wird mit den Grenzwerten geprüft.

## Nächste Schritte

- Machen Sie das [Quiz](quiz.de.md).
- Weiter mit Modul 19, dem Analysieren verdächtiger Programme.
- Übungen zu diesem Modul: [Übung 18](../../exercises/18-bug-hunt-report/README.de.md).
