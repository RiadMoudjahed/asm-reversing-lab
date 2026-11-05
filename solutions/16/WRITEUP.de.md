<!-- source: WRITEUP.md @ sha256:c04d588def8e -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Lösungsweg: Übung 16. Stille Bibliothek

Das Flag lautet `ARL{silent_16_8_5_13_7}`: Der Zustandsblock ist 16 Bytes groß, der 64-Bit-Bytezähler liegt bei Offset 8, die Einspeisefunktion rotiert um 5 Bits, die Funktion, die den Digest liest, rotiert um 13 Bits, und die Packfunktion schreibt für die vier Ganzzahlen 7 Bytes.

## Teil 1: Reverse

### Die Exporte

```text
$ nm -D --defined-only bin/libsilent.so
0000000000001141 T sl_feed
0000000000001173 T sl_final
000000000000117f T sl_free
000000000000119a T sl_live
0000000000001119 T sl_new
00000000000011a1 T sl_pack
```

Die Namen deuten auf eine Gruppe (`sl_new`, `sl_feed`, `sl_final`, `sl_free`) und zwei Funktionen außerhalb davon. Die Gruppe wird an ihrem Code geprüft, nicht ihren Namen geglaubt.

### Der Zustand: `sl_new`

```text
1119:  push   rbx
111a:  mov    ebx,edi                       ; seed, 32 bits
111c:  mov    esi,0x10
111f:  mov    edi,0x1                       ; calloc(1, 16)
1126:  call   calloc@plt
112b:  test   rax,rax
112e:  je     113f                          ; null stays null
1130:  xor    ebx,0x811c9dc5
1136:  mov    DWORD PTR [rax],ebx           ; first field = seed xor constant
1138:  add    DWORD PTR [rip+0x2edd],0x1    ; a global counter goes up
113f:  pop    rbx
1140:  ret
```

Die Funktion nimmt einen 32-Bit-Wert, reserviert einen mit Nullen gefüllten Block von 16 Bytes, speichert den Seed xor 0x811c9dc5 in den ersten vier Bytes und erhöht eine globale Variable. Also ist `S` gleich 16. Die globale Variable wird von `sl_live` gelesen (`mov eax, DWORD PTR [rip+...]`, `ret`) und von `sl_free` verringert, die ihr Argument nach einem Nulltest an `free` weitergibt. Beide zusammen zählen die offenen Zustände.

### Die Einspeisefunktion: `sl_feed`

```text
1141:  test   rdx,rdx
1144:  je     116e                          ; no bytes: skip the loop
1146:  mov    r8,rsi
1149:  add    rsi,rdx                       ; end of the buffer
114c:  movzx  eax,BYTE PTR [r8]             ; next byte
1150:  xor    eax,DWORD PTR [rdi]           ; ... xor first field
1152:  imul   eax,eax,0x1000193
1158:  mov    DWORD PTR [rdi],eax           ; first field = (first xor byte) * 16777619
115a:  mov    ecx,DWORD PTR [rdi+0x4]
115d:  rol    ecx,0x5                       ; second field rotated left by 5
1160:  add    eax,ecx
1162:  mov    DWORD PTR [rdi+0x4],eax       ; second field = rol(second, 5) + first
1165:  add    r8,0x1
1169:  cmp    r8,rsi
116c:  jne    114c
116e:  add    QWORD PTR [rdi+0x8],rdx       ; the 64-bit count grows by n
1172:  ret
```

Die Signatur hat drei Argumente: den Zustand in `rdi`, den Puffer in `rsi` (gelesen mit `movzx ... BYTE PTR`) und die Länge in `rdx`. Der Zustand hat zwei 32-Bit-Felder bei den Offsets 0 und 4, die sich bei jedem Byte ändern, und ein 64-Bit-Feld bei Offset 8, das einmal nach der Schleife um die Gesamtlänge wächst. Also ist `C` gleich 8 und `F` gleich 5. Der Zustand ist 4 + 4 + 8 = 16 Bytes groß, was zur Reservierung passt.

### Der Digest: `sl_final`

```text
1173:  mov    eax,DWORD PTR [rdi+0x4]
1176:  rol    eax,0xd                       ; second field rotated left by 13
1179:  xor    eax,DWORD PTR [rdi+0x8]       ; xor the low 32 bits of the count
117c:  xor    eax,DWORD PTR [rdi]           ; xor the first field
117e:  ret
```

Die Funktion liest den Zustand, ohne ihn zu ändern, und gibt einen 32-Bit-Wert zurück: `unsigned sl_final(const struct ctx *)`. Also ist `G` gleich 13. Die beiden Rotationsbeträge stehen in verschiedenen Funktionen, was die Falle der Frage ist.

### Die Packfunktion: `sl_pack`

```text
11f0:  lea    edx,[rcx+rcx*1]               ; v + v
11f3:  sar    ecx,0x1f                      ; 0 or -1 from the sign of v
11f6:  xor    edx,ecx                       ; zigzag value
11f8:  cmp    edx,0x7f
11fb:  jbe    11d0                          ; fits in one byte
...
1218:  add    rax,0x1
121c:  mov    ecx,edx
121e:  or     ecx,0xffffff80                ; low 7 bits with bit 7 set
1221:  mov    BYTE PTR [rdi+rax*1-0x1],cl
1225:  shr    edx,0x7
1228:  cmp    edx,0x7f
122b:  ja     1218
```

Vier Argumente werden gelesen: die Ganzzahlen in `rdi` (mit einem 4-Byte-Faktor gelesen), die Anzahl in `rsi`, der Ausgabepuffer in `rdx` (nach `rdi` verschoben) und die Kapazität in `rcx` (nach `r11` verschoben). Die Fehlerpfade setzen `mov eax,0xffffffff`: ein 32-Bit-Wert -1, der Rückgabetyp ist also `int`. Jeder Wert wird zuerst mit `(v + v) xor (v >> 31)` und einem arithmetischen Schieben abgebildet, sodass kleine Beträge beider Vorzeichen kleine Zahlen ergeben. Die abgebildete Zahl wird dann in Siebenergruppen geschrieben, niedrige Bits zuerst, mit gesetztem Bit 7 bei jedem Byte außer dem letzten. Vor dem Schreiben eines Wertes addiert der Code dessen Länge zu den bisher geschriebenen Bytes und vergleicht die Summe mit der Kapazität (`cmp r11,rsi` / `jb` zum Fehlerpfad); die Prüfung erfolgt pro Wert, ein Wert passt also ganz hinein oder wird nicht geschrieben, und die Werte davor bleiben im Puffer.

Zu `B`: 0 wird auf 0 abgebildet (ein Byte), -1 auf 1 (ein Byte), 300 auf 600 (zwei Bytes, da 600 über 127 und unter 16384 liegt) und -70000 auf 139999 (drei Bytes, da es über 16383 und unter 2097152 liegt). Die Summe ist 1 + 1 + 2 + 3 = 7. Ein kurzes C-Programm, das die Bibliothek aufruft, bestätigt es: Der Ausgabebereich enthält `00 01 d8 04 df c5 08`.

## Teil 2: Assemble

Die Referenzlösung ist `solution.asm`. Die wichtigen Punkte:

- **`digest_buffer`.** Vier Aufrufe: `sl_new(seed)`, `sl_feed(handle, p, n)`, `sl_final(handle)`, `sl_free(handle)`. Das Handle und die Pufferargumente liegen in vom Aufgerufenen zu sichernden Registern, weil sie Aufrufe überdauern müssen, und der Digest wird vor `sl_free` in ein solches Register verschoben, damit das Ergebnis auch diesen Aufruf überlebt. Drei Pushes und die Rücksprungadresse halten `rsp` ausgerichtet.
- **`pack_values`.** Sie braucht weder Aufrufe noch gesicherte Register. Für jeden Wert berechnet sie die Zickzack-Zahl, zählt die nötigen Bytes, vergleicht die Summe mit der Kapazität und gibt entweder -1 zurück oder schreibt die Bytes. Der Kapazitätstest verwendet einen vorzeichenlosen Vergleich; die Länge wird berechnet, bevor etwas geschrieben wird, sodass ein Wert, der nicht hineinpasst, die Ausgabe unberührt lässt.
- **Der Zickzack-Schritt.** Das arithmetische Schieben `sar r10d, 31` macht aus dem Vorzeichen 0 oder -1, und das Exklusiv-Oder kehrt bei einem negativen verdoppelten Wert alle Bits um.

## Fehler, die Zeit kosten

- **`sl_free` vergessen.** Die Ergebnisse stimmen, und das Prüfprogramm meldet `handle`.
- **Das Handle zweimal freigeben.** Der Zähler der Bibliothek wird negativ, oder das Programm bricht in `free` ab.
- **Ein logisches Schieben im Zickzack-Schritt.** Positive Werte funktionieren. Negative Werte werden falsch kodiert.
- **Die Kapazität pro Byte prüfen.** Der scheiternde Wert wird teilweise geschrieben, und der Ausgabebereich weicht von dem der Bibliothek ab.
- **Ein Off-by-one im Kapazitätstest.** Ein Wert, der genau hineinpasst, wird abgelehnt, oder es wird ein Byte zu viel geschrieben.
- **Die Packfunktion als `long` deklarieren.** Der Fehlerpfad setzt `eax` auf den 32-Bit-Wert minus eins, das Ergebnis ist also ein `int`; ein falscher Prototyp in Ihrem Testprogramm verbirgt das.
- **Den Digest über `sl_free` hinweg verlieren.** `rax` wird vom Aufruf überschrieben; der Digest muss vorher gesichert werden.
