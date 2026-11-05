<!-- source: CONTRIBUTING.md @ sha256:1e4e4a084d48 -->
<div align="center">
  
[English](CONTRIBUTING.md) | [العربية](CONTRIBUTING.ar.md) | [Deutsch](CONTRIBUTING.de.md) | [Español](CONTRIBUTING.es.md) | [Français](CONTRIBUTING.fr.md)
</div>  

# Mitwirken

Danke, dass Sie Menschen beim Lernen helfen. Diese Seite erklärt, wie Sie gut mitwirken.

## Möglichkeiten mitzuwirken

- Einen fachlichen Fehler, eine unklare Erklärung oder ein defektes Beispiel melden.
- Eine Übersetzung verbessern oder prüfen. Die Prüfung durch Muttersprachler ist besonders wertvoll.
- Eine Verbesserung für einen Hinweis oder eine Lösungsbeschreibung vorschlagen.
- Tippfehler und Formatierung korrigieren.

Eröffnen Sie für neue Module oder Übungen zuerst ein Issue, damit der Umfang abgestimmt werden kann, bevor Sie beginnen.

## Grundregeln

1. **Nur eigene Arbeit.** Schreiben Sie Ihre eigenen Erklärungen, Beispiele und Aufgaben. Kopieren oder paraphrasieren Sie kein Buch, keinen Kurs, keinen Blog und kein Video eng, und nennen Sie keine kommerziellen Werke als Quellen. Offizielle Spezifikationen dürfen zitiert werden.
2. **Im Rahmen bleiben.** Keine Exploit-Entwicklung, kein Shellcode, keine Entwicklung von Schadsoftware und keine Inhalte zur Umgehung von Kopierschutz oder Lizenzierung. Aufgabenprogramme werden in diesem Repository aus dem Quelltext gebaut; keine Binärdateien Dritter.
3. **Keine Spoiler.** Flags und vollständige Lösungen gehören ausschließlich nach `solutions/`. Veröffentlichen Sie sie nie in Issues oder Diskussionen.
4. **Professionelle Darstellung.** Keine Emojis und keine dekorativen Symbole. Klare, direkte Sprache.
5. **Alles läuft.** Codeblöcke mit den Tags `nasm`, `c` oder `bash` müssen im Lab-Image funktionieren.

## Ablauf

1. Forken Sie das Repository und legen Sie einen Themenzweig an.
2. Nehmen Sie Ihre Änderung vor und folgen Sie dabei den Grundregeln oben und dem Stil der vorhandenen Seiten.
3. Führen Sie `make all-checks` im Lab aus. Alle Prüfungen müssen bestehen.
4. Öffnen Sie einen Pull Request und füllen Sie die Checkliste der Vorlage aus.

## Übersetzungen

Die englischen Dateien sind die Quelle. Eine Übersetzung liegt neben der englischen Datei und trägt ein Sprachsuffix: `README.fr.md`, `README.de.md`, `README.es.md`, `README.ar.md`.

- Beginnen Sie die Datei mit `<!-- source: FILE.md @ sha256:<12 hex> -->` (die ersten 12 Zeichen von `sha256sum FILE.md`).
- Übersetzen Sie keinen Code, keine Mnemonics, Registernamen, Befehle, Dateinamen oder Flags.
- Verwenden Sie die Begriffe aus `i18n/terminology.csv`. Fügen Sie eine Zeile hinzu, wenn Sie einen neuen Begriff brauchen.
- Beachten Sie `i18n/STYLE.md` für Ton und Typografie der jeweiligen Sprache.
- Mit `make i18n` sehen Sie, welche Seiten fehlen oder veraltet sind.

## Sicherheits- oder Gefahrenhinweise melden

Wenn Sie glauben, dass eine Seite oder ein Programm in diesem Repository Schaden anrichten könnte, wenden Sie sich bitte vertraulich an die Maintainer, statt ein öffentliches Issue zu eröffnen.
