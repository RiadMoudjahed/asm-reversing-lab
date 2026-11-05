<!-- source: quiz.md @ sha256:62ae4799ebaf -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 09. Was Reverse Engineering ist

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Was ist das Ziel von Reverse Engineering, und wie unterscheidet es sich vom Neuschreiben eines Programms?**

<details>
<summary>Antwort</summary>

Das Ziel ist Verständnis: aus dem Programm selbst zu erschließen, wie es funktioniert. Neuschreiben erzeugt neuen Code; es kann aus dem Verständnis folgen, ist aber nicht das Ziel.

</details>

**2. Nennen Sie drei legitime Einsatzzwecke von Reverse Engineering.**

<details>
<summary>Antwort</summary>

Beliebige drei von: Interoperabilität, Sicherheitsprüfung, Malware-Analyse, Wiederherstellung verlorenen Wissens, Fehlersuche ohne Quelltext, Lernen, wie ein Compiler Code übersetzt.

</details>

**3. Welche drei Dinge sollten Sie prüfen, bevor Sie ein Programm analysieren, das nicht Ihnen gehört?**

<details>
<summary>Antwort</summary>

Wem es gehört, was seine Lizenz oder sein Vertrag zur Analyse sagt und was das Gesetz in Ihrem Land sagt.

</details>

**4. Nennen Sie eine Stärke und eine Schwäche der statischen Analyse.**

<details>
<summary>Antwort</summary>

Stärke: Sie ist sicher und sieht den gesamten Code, auch Pfade, die nie laufen. Schwäche: Sie sieht keine Werte, die zur Laufzeit berechnet werden, und verschleierter Code ist schwer lesbar.

</details>

**5. Warum führt man dynamische Analyse nie auf dem Rechner aus, der Ihre persönlichen Dateien enthält?**

<details>
<summary>Antwort</summary>

Weil dabei Code läuft, den Sie noch nicht vollständig verstehen. Verwenden Sie eine Wegwerfumgebung, etwa den Lab-Container dieses Kurses oder für echte Proben eine virtuelle Maschine ohne Netzwerk und mit Snapshot.

</details>

**6. Bringen Sie diese Schritte in die richtige Reihenfolge: lesen und benennen, Triage, berichten, beobachten, die Idee prüfen, eine Frage bilden, den Code finden.**

<details>
<summary>Antwort</summary>

Triage, beobachten, eine Frage bilden, den Code finden, lesen und benennen, die Idee prüfen, berichten.

</details>

**7. Nach der Triage einer Datei sehen Sie `statically linked, not stripped` und die Zeichenkette `NASM`. Was können Sie daraus schließen, und was nicht?**

<details>
<summary>Antwort</summary>

Sie können schließen, dass die Datei keine Abhängigkeiten von gemeinsam genutzten Bibliotheken hat, dass die Symbolnamen noch vorhanden sind und dass sie wahrscheinlich mit NASM assembliert wurde. Was das Programm tut, können Sie nicht schließen; Zeichenketten und Kopfbereiche sind Hinweise, die Codeanalyse oder Beobachtung bestätigen muss.

</details>
