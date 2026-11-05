<!-- source: quiz.md @ sha256:d39f86564105 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 16. Undokumentierte Schnittstellen

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Eine Funktion liest `edi`, `rsi` und `edx`, bevor sie sie schreibt, und endet mit `mov eax, ...`. Was lässt sich über ihre Signatur höchstens sagen?**

<details>
<summary>Antwort</summary>

Drei Parameter: ein 32-Bit-Wert, ein 64-Bit-Wert (ein Zeiger oder ein `size_t`) und ein 32-Bit-Wert, in dieser Reihenfolge, und ein 32-Bit-Ergebnis. Ob die 32-Bit-Werte vorzeichenbehaftet sind, hängt davon ab, wie sie verglichen oder erweitert werden, und ob `rsi` ein Zeiger ist, davon, ob er dereferenziert wird.

</details>

**2. Der Code setzt `rax` auf seinen Fehlerpfaden mit `mov rax, 0xffffffffffffffff`. Was sagt Ihnen das?**

<details>
<summary>Antwort</summary>

Das Ergebnis ist ein vorzeichenbehafteter 64-Bit-Wert, etwa `long`, `ssize_t` oder eine zeigergroße Ganzzahl, und -1 ist ein Fehlercode. Ein 32-Bit-`int` würde `mov eax, -1` oder `or eax, 0xffffffff` verwenden.

</details>

**3. Woran erkennen Sie einen Ausgabeparameter?**

<details>
<summary>Antwort</summary>

Ein Zeigerargument, über das die Funktion nur schreibt: Speichervorgänge nach `[reg]` oder `[reg+N]` und vor den Speichervorgängen keine Ladevorgänge daraus. Der Aufrufer übergibt die Adresse einer Variablen und liest danach dort den Wert.

</details>

**4. Eine Bibliotheksfunktion gibt einen Zeiger von `calloc` zurück. Der Aufrufer liest oder schreibt nie darüber und reicht ihn nur an andere Funktionen der Bibliothek weiter, von denen eine mit `free` endet. Was ist dieser Zeiger, und welche Verwendungsregeln folgen daraus?**

<details>
<summary>Antwort</summary>

Ein opakes Handle. Jedes Handle, das eine öffnende Funktion zurückgibt, muss genau einmal geschlossen werden, darf nach dem Schließen nicht mehr verwendet werden, und ein Null-Ergebnis der öffnenden Funktion muss geprüft werden.

</details>

**5. `subsd xmm1, xmm0` und `mulsd xmm1, xmm2` erscheinen, und `xmm0` enthält das Ergebnis. Welche Register sind Argumente, und welchen Typ haben sie?**

<details>
<summary>Antwort</summary>

`xmm0`, `xmm1` und `xmm2` werden gelesen, bevor sie geschrieben werden, es gibt also drei `double`-Argumente (`sd` bedeutet skalar, doppelt genau; `ss` wäre `float`), und der Rückgabetyp ist `double`. Ganzzahlregister sind nicht beteiligt.

</details>

**6. Sie haben eine Hypothese mit drei Eingaben getestet, und alle stimmten. Ist sie bestätigt?**

<details>
<summary>Antwort</summary>

Noch nicht. Übereinstimmung bei typischen Eingaben ist ein schwacher Beleg. Testen Sie die Grenzen (leere oder einelementige Eingabe, null, größte und kleinste Werte, ein Puffer, der gerade passt, und einer, der ein Byte zu klein ist) und vergleichen Sie mit einem unabhängigen Modell der Regel, das vor dem Lauf geschrieben wurde.

</details>

**7. Warum gibt das Testprogramm die drei Werte von `bb_next` in drei getrennten Anweisungen aus?**

<details>
<summary>Antwort</summary>

C legt die Reihenfolge nicht fest, in der die Argumente eines Aufrufs ausgewertet werden. In einem einzigen `printf` mit drei Aufrufen können die Werte in beliebiger Reihenfolge erscheinen, und die Ausgabe würde nicht mehr dem entsprechen, was die Bibliothek tut.

</details>

**8. Eine Funktion liest `[rdi]`, bevor sie prüft, dass ihr Anzahlargument nicht null ist. Was fügt das der Schnittstelle hinzu?**

<details>
<summary>Antwort</summary>

Eine Verwendungsregel: Die Anzahl muss mindestens 1 sein, sonst liest die Funktion Speicher, der ihr nicht gehört. Eine Signatur kann das nicht ausdrücken, es gehört also in den Kommentar des Headers.

</details>
