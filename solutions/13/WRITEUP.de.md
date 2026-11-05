<!-- source: WRITEUP.md @ sha256:abe06d06e270 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 13. Magische Zahlen

Das Flag lautet `ARL{magic_12_9_60_11_5_3}`: Die Punktzahl ist `(u / 12) * 5 + (u % 9) * 3 - (x / 60) + (x % 11) * 4`, wobei `u` das als vorzeichenlos gelesene `x` ist; die ersten beiden Operationen sind vorzeichenlos und die letzten beiden vorzeichenbehaftet.

## Teil 1: Reverse

### Der Code

```bash
objdump -d -M intel --no-show-raw-insn bin/magic
```

```text
0000000000401190 <mix>:
  401190:    mov    edx,edi
  401192:    mov    ecx,0xaaaaaaab
  401197:    mov    rax,rdx
  40119a:    imul   rcx,rdx
  40119e:    imul   rdx,rdx,0x38e38e39
  4011a5:    mov    edi,eax
  4011a7:    sar    edi,0x1f
  4011aa:    shr    rcx,0x23
  4011ae:    shr    rdx,0x21
  4011b2:    lea    esi,[rcx+rcx*4]
  4011b5:    lea    ecx,[rdx+rdx*8]
  4011b8:    mov    edx,eax
  4011ba:    sub    edx,ecx
  4011bc:    lea    edx,[rdx+rdx*2]
  4011bf:    add    esi,edx
  4011c1:    movsxd rdx,eax
  4011c4:    imul   rcx,rdx,0xffffffff88888889
  4011cb:    imul   rdx,rdx,0x2e8ba2e9
  4011d2:    shr    rcx,0x20
  4011d6:    add    ecx,eax
  4011d8:    sar    rdx,0x21
  4011dc:    sar    ecx,0x5
  4011df:    sub    edx,edi
  4011e1:    sub    ecx,edi
  4011e3:    sub    esi,ecx
  4011e5:    lea    ecx,[rdx+rdx*4]
  4011e8:    lea    edx,[rdx+rcx*2]
  4011eb:    sub    eax,edx
  4011ed:    lea    eax,[rsi+rax*4]
  4011f0:    ret
```

Der Compiler hat die vier Berechnungen ineinander verschränkt; lesen Sie sie daher, indem Sie jeder Konstante folgen.

### Die vier Divisionen

Jede Division ist ein `imul` mit einer großen Konstante, gefolgt von einer oder mehreren Rechtsverschiebungen. Der Divisor ist jeweils `2^shift / constant`, gerundet. Das Skript aus Modul 14 rechnet und prüft:

```bash
python3 14_magic.py 0xaaaaaaab 35
python3 14_magic.py 0x38e38e39 33
python3 14_magic.py 0x88888889 37 signed
python3 14_magic.py 0x2e8ba2e9 33 signed
```

```text
2**35 / 2863311531 = 11.9999999986  ->  divisor 12
checked 200007 inputs, 0 differences
2**33 / 954437177 = 8.9999999990  ->  divisor 9
checked 200007 inputs, 0 differences
2**37 / 2290649225 = 59.9999999878  ->  divisor 60
checked 200007 inputs, 0 differences
2**33 / 780903145 = 10.9999999962  ->  divisor 11
checked 200007 inputs, 0 differences
```

| Division | Anzeichen | Divisor |
|---|---|---|
| Vorzeichenloser Quotient | `0xaaaaaaab` (ein 32-Bit-Wert in `ecx`, die Multiplikation ist also vorzeichenlos) und `shr rcx, 0x23`, also 35 | 12 |
| Vorzeichenloser Rest | `0x38e38e39` und `shr rdx, 0x21`, also 33; der Quotient wird dann mit `lea ecx,[rdx+rdx*8]` mit 9 multipliziert und von der Eingabe abgezogen | 9 |
| Vorzeichenbehafteter Quotient | `0xffffffff88888889` (die 32-Bit-Konstante 0x88888889 mit gesetztem höchstem Bit, daher vorzeichenerweitert dargestellt), `shr rcx, 0x20`, das `add ecx, eax`, das `x` wieder addiert, und `sar ecx, 5`; die Gesamtverschiebung ist 32 + 5 = 37 | 60 |
| Vorzeichenbehafteter Rest | `movsxd`, `0x2e8ba2e9` und `sar rdx, 0x21`, also 33; der Quotient wird mit zwei `lea`-Befehlen mit 11 multipliziert und von `x` abgezogen | 11 |

Der vorzeichenbehaftete Quotient zeigt eine Variante, die die Lektion nicht ausdrücklich behandelt. Die magische Zahl für 60 passt nicht in einen positiven 32-Bit-Wert; der Compiler multipliziert deshalb mit dem als negativ gelesenen 32-Bit-Wert und addiert nach dem Bilden der oberen Hälfte `x` wieder hinzu: `(x * (M - 2^32)) >> 32` plus `x` ergibt `(x * M) >> 32`. Das Skript verwendet `M` als positiv gelesen, also den Wert, den die Divisorformel braucht.

Das `sar edi, 0x1f` weit oben berechnet einmal das Vorzeichen von `x` (0 oder -1). Beide vorzeichenbehafteten Quotienten ziehen es ab (`sub edx, edi` und `sub ecx, edi`); das ist die Korrektur für negative Eingaben. Die vorzeichenlosen Terme beginnen mit `mov edx, edi` und `mov rax, rdx`, die `x` nullerweitern; die vorzeichenbehafteten Terme verwenden `movsxd`.

### Die Faktoren

Nachdem der vorzeichenlose Quotient in `rcx` berechnet ist, multipliziert `lea esi, [rcx+rcx*4]` ihn mit 5, also ist `K` gleich 5. Nach dem Rest `u - q * 9` multipliziert `lea edx, [rdx+rdx*2]` ihn mit 3, also ist `M` gleich 3. Die beiden Ergebnisse werden addiert (`add esi, edx`). Danach wird der vorzeichenbehaftete Quotient abgezogen (`sub esi, ecx`), und der vorzeichenbehaftete Rest wird im abschließenden `lea eax, [rsi+rax*4]` mit 4 multipliziert und addiert.

Die Formel lautet `(u / 12) * 5 + (u % 9) * 3 - (x / 60) + (x % 11) * 4`, und das Flag lautet `ARL{magic_12_9_60_11_5_3}`.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Worauf es ankommt:

- **Vorzeichenloser Quotient.** `mov eax, edi` erweitert `x` mit Nullen nach `rax`; ein 64-Bit-`imul` mit `0xAAAAAAAB` liefert das vollständige Produkt, und `shr rax, 35` ist der Quotient. `lea r8d, [rax+rax*4]` multipliziert ihn mit 5.
- **Vorzeichenloser Rest.** Der Quotient durch 9 entsteht aus `0x38E38E39` und einer Verschiebung um 33. Der Rest ist `u` minus Quotient mal 9 (`lea eax, [rax+rax*8]`), und `lea edx, [rdx+rdx*2]` multipliziert ihn mit 3.
- **Vorzeichenbehafteter Quotient.** `movsxd rax, edi` erweitert mit Vorzeichen. Die Konstante `0x88888889` wird mit `mov edx, 0x88888889` geladen, was mit Nullen erweitert und sie damit positiv hält, und das 64-Bit-Produkt wird um 37 nach rechts geschoben. Weil `sar` zur negativen Unendlichkeit hin rundet, wird bei negativen Eingaben 1 addiert, was `sub edx, ecx` mit `ecx = x >> 31` erledigt.
- **Vorzeichenbehafteter Rest.** Dieselbe Vorzeichenkorrektur gilt für den Quotienten `(x * 0x2E8BA2E9) >> 33`, und `q * 11` ist `q + q*5*2`, mit zwei `lea`-Befehlen berechnet.
- **Zusammenfassen.** Das letzte `lea` addiert `d * 4` zur laufenden Summe, die schon die ersten drei Terme enthält.

## Fehler, die Zeit kosten

- **Keine Vorzeichenkorrektur.** Ohne das Abziehen von `x >> 31` ist ein vorzeichenbehafteter Quotient bei negativen Eingaben betragsmäßig um eins zu klein: Die Punktzahl weicht um 1 ab, wenn die Korrektur bei `x / 60` fehlt, und um 44 (das Vierfache des Divisors 11), wenn sie bei `x % 11` fehlt.
- **Eine falsche Verschiebung.** `shr rax, 34` statt 35 ändert den Quotienten für jede Eingabe ab 6.
- **Die vorzeichenbehafteten Terme nullerweitern.** `mov eax, edi` statt `movsxd` liefert für jede negative Eingabe falsche Ergebnisse.
- **Die Konstante als negativ lesen.** Der Fall 60 braucht die Konstante als positive 2290649225, wenn Sie `x` nicht wieder addieren.
- **`div` verwenden.** Das Prüfprogramm lehnt es auch bei richtigen Ergebnissen ab.
