<!-- source: README.md @ sha256:d6d12a704b76 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Übung 08. Visitenkarte

Level 3. Zeitaufwand: etwa 3 Stunden. Architektur: x86 und x86-64 Linux. Module: [08. Der Stapel, Prozeduren und Aufrufkonventionen](../../modules/08-stack-and-calls/README.de.md)

## Szenario

Ein Hersteller hat zwei kleine Testprogramme, `callcard32` und `callcard64`, zusammen mit einer Header-Datei ausgeliefert, die verloren gegangen ist. Der Header nannte die Aufrufkonvention jeder Funktion, und das Integrationsteam muss diese Funktionen nun aus anderem Code aufrufen. Ohne den Header muss die Konvention aus dem Maschinencode gelesen werden. Danach ist ein Ersatz für drei der Funktionen zu schreiben, mit denselben Konventionen, damit bestehende Aufrufer weiter funktionieren.

## Ziele

- cdecl, stdcall und fastcall in 32-Bit-Code daran unterscheiden, wie Argumente gelesen werden und wie der Stapel nach dem Aufruf aufgeräumt wird.
- System V AMD64 von Microsoft x64 in 64-Bit-Code an den Argumentregistern, am Schattenbereich (shadow space) und an der Stapelanpassung des Aufrufers unterscheiden.
- Funktionen schreiben, deren Stapelwirkung zu ihrer Konvention passt, sodass ein für diese Konvention geschriebener Aufrufer im Gleichgewicht bleibt.

## Teil 1: Reverse

Die Binärdateien sind `bin/callcard32` (32 Bit) und `bin/callcard64` (64 Bit). Jede führt einen Selbsttest ihrer Funktionen aus und gibt `callcard32: ok` beziehungsweise `callcard64: ok` aus. Die Funktionen sind die globalen Symbole `f1` bis `f4` in `callcard32` und `g1` und `g2` in `callcard64`.

```bash
bin/callcard32
bin/callcard64
objdump -d -M intel bin/callcard32
objdump -d -M intel bin/callcard64
```

Jede Funktion verwendet eine von fünf Konventionen. Weisen Sie jeder Funktion einen Buchstaben zu:

| Buchstabe | Konvention |
|---|---|
| `c` | cdecl: Argumente auf dem Stapel, der Aufrufer entfernt sie |
| `s` | stdcall: Argumente auf dem Stapel, die aufgerufene Funktion entfernt sie |
| `f` | fastcall (32 Bit): erste zwei Argumente in `ecx` und `edx`, der Rest auf dem Stapel, die aufgerufene Funktion entfernt diesen |
| `v` | System V AMD64: erste Argumente in `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9` |
| `m` | Microsoft x64: erste Argumente in `rcx`, `rdx`, `r8`, `r9`, mit 32 Byte Schattenbereich |

Bilden Sie das Flag aus den sechs Buchstaben, in der Reihenfolge `f1`, `f2`, `f3`, `f4`, `g1`, `g2`:

```text
ARL{calling_<f1><f2><f3><f4><g1><g2>}
```

Wären die Buchstaben zum Beispiel `c`, `c`, `s`, `f`, `v` und `m`, lautete das Flag `ARL{calling_ccsfvm}`.

## Teil 2: Assemble

Bearbeiten Sie `starter/solution.asm`, um drei Funktionen in 32-Bit-Code zu implementieren. Jede muss dasselbe Ergebnis berechnen wie die Funktion an derselben Position in Teil 1 und dieselbe Aufrufkonvention wie diese Funktion verwenden:

- `cc_f1` entspricht `f1`.
- `cc_f2` entspricht `f2`.
- `cc_f3` entspricht `f3`.

Alle drei nehmen drei `int`-Argumente und liefern einen `int`. Die Tests rufen sie über Prototypen auf, die jede Konvention deklarieren:

```c
int __attribute__((stdcall))  cc_f1(int a, int b, int c);
int                           cc_f2(int a, int b, int c);   /* cdecl */
int __attribute__((fastcall)) cc_f3(int a, int b, int c);
```

Lesen Sie die Formel jeder Funktion aus ihrem Code in `callcard32` ab und achten Sie darauf, welches Argument wo liegt.

## Regeln

- Das Testprogramm vergleicht vor und nach jedem Aufruf den Stapelzeiger. Eine Funktion, die zu wenige oder zu viele Bytes entfernt, fällt durch, auch wenn ihr Ergebnis stimmt.
- Erhalten Sie `ebx`, `esi`, `edi` und `ebp`.
- Verwenden Sie kein `call`.
- Ergebnisse laufen in 32 Bit über; die Tests enthalten sehr große und sehr kleine Werte.

## Arbeit prüfen

```bash
make check EX=08 ARGS='--flag ARL{your_answer_here}'
```

Das Prüfprogramm meldet Teil 1 und Teil 2 getrennt. Ohne `--flag` prüft es nur Teil 2.

## Hinweise

Öffnen Sie [HINTS.md](HINTS.de.md) jeweils eine Stufe.
