<!-- source: quiz.md @ sha256:f9784c17b4a4 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz: 15. Daten wiederherstellen

Antworten Sie zuerst, öffnen Sie dann die Antwort.

**1. Der Code enthält `mov eax, DWORD PTR [rdi+rsi*4]`. Was wissen Sie über `rdi` und `rsi`?**

<details>
<summary>Antwort</summary>

`rdi` ist die Basis eines Feldes aus 4-Byte-Elementen, wahrscheinlich `int`, und `rsi` ist der Index (hier als 64-Bit-Wert). Der Faktor 4 und das 4-Byte-Laden stimmen in der Elementgröße überein.

</details>

**2. Was verrät `[rdi+4+rsi*8]`, das `[rdi+rsi*8]` nicht verraten würde?**

<details>
<summary>Antwort</summary>

Die Schrittweite 8 ist die Größe eines Elements eines Feldes aus Strukturen, und die 4 ist der Offset eines Feldes innerhalb jedes Elements. Der Code liest das Feld bei Offset 4 des Elements `rsi`.

</details>

**3. Eine Funktion liest von `rdi` die Offsets 0 (4 Bytes), 8 (8 Bytes) und 16 (1 Byte). Was können Sie über das Layout und über die Offsets 4 und 9 bis 15 sagen?**

<details>
<summary>Antwort</summary>

Es ist eine Struktur mit mindestens einem int-großen Feld bei 0, einem 8-Byte-Feld bei 8 und einem Byte bei 16. Die Offsets 4 bis 7 sind wahrscheinlich Auffüllung, weil ein 8-Byte-Feld auf 8 ausgerichtet wird, und 9 bis 15 sind Auffüllung oder Felder, die diese Funktion nicht benutzt. Die Gesamtgröße beträgt wahrscheinlich 24, auf ein Vielfaches von 8 aufgerundet.

</details>

**4. Eine Schleife führt `mov rdi, [rdi+8]` aus und prüft `rdi` gegen null. Welche Datenstruktur ist das?**

<details>
<summary>Antwort</summary>

Eine verkettete Liste: Das Basisregister wird aus dem Knoten nachgeladen, auf den es zeigt, und null beendet die Schleife. Der Zeiger auf den nächsten Knoten liegt bei Offset 8.

</details>

**5. Woran erkennen Sie im Maschinencode eine Suche in einem binären Baum im Unterschied zum Durchlaufen einer Liste?**

<details>
<summary>Antwort</summary>

Eine Baumsuche lädt eines von zwei Zeigerfeldern (zum Beispiel bei den Offsets 8 und 16), ausgewählt durch einen Vergleich des gesuchten Schlüssels mit dem Schlüssel des Knotens. Ein Listendurchlauf lädt immer dasselbe Feld.

</details>

**6. Was ist das erste Feld eines C++-Objekts mit virtuellen Funktionen, und wie nutzt ein virtueller Aufruf es?**

<details>
<summary>Antwort</summary>

Der vptr bei Offset 0. Ein virtueller Aufruf lädt den vptr aus dem Objekt (`mov rax, [rdi]`) und ruft dann eine in der vtable an einem festen Slot gespeicherte Adresse auf (`call [rax+8*slot]`), wobei der Objektzeiger als erstes Argument übergeben wird.

</details>

**7. Zwei Klassen haben vtables, die sich nur in einem Slot unterscheiden. Was sagt Ihnen das?**

<details>
<summary>Antwort</summary>

Eine Klasse ist von der anderen abgeleitet, und der abweichende Slot ist eine virtuelle Funktion, die die abgeleitete Klasse überschreibt. Alle anderen Slots enthalten dieselben Funktionsadressen, weil sie geerbt sind.

</details>
