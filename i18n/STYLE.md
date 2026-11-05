# Translation style guide

Applies to every translated page. English is the source. Read this together with
`terminology.csv`, which fixes the translation of each technical term.

## All languages

- Translate meaning, not words. Keep headings, lists, tables and `<details>` blocks in the same
  order and number as the English page so pages stay aligned.
- Never translate: code, mnemonics, register names, directives, command lines, file names,
  paths, exercise flags (`ARL{...}`), program output, addresses and hexadecimal values.
- Link to the same-language sibling when it exists (`README.fr.md`). Links to English-only
  documents (`i18n/status.md`) stay as they are.
- On first use in a page, follow a translated technical term with the English term in
  parentheses, unless the CSV shows the languages share the same word.
- Keep the professional tone of the source: no emoji, no decorative symbols, no exclamation
  marks, no humor that depends on the English wording.
- Level names stay `Level 1` to `Level 5` in every language so exercise references match.
- Start each file with the two header comments (`source` hash and `review` state) before the
  language bar.

## Arabic (ar)

- Modern Standard Arabic.
- Wrap prose in `<div dir="rtl">` and `</div>` with a blank line after the opening tag and
  before the closing tag. Close the div before a fenced code block and reopen it after, so code
  stays left-to-right.
- Use Western digits (0 to 9) for all numbers, addresses and offsets.
- Keep technical Latin-script names (Docker, GDB, NASM, x86-64) unchanged inside Arabic text.
- Use the Arabic comma and question mark in prose; use ASCII punctuation inside code spans.

## German (de)

- Formal address with `Sie`.
- Follow standard German compound-noun rules; hyphenate when a compound would be unreadable
  (`Little-Endian`, `Dispatch-Schleife`).
- Use decimal point and number formats of the source in code; use German formats only in prose.

## Spanish (es)

- Neutral international Spanish, informal `tú`.
- Prefer terms understood in both Spain and Latin America; the CSV records the choice
  (for example `ordenador` is avoided in favor of `equipo` when a general term is needed).
- Use inverted question and exclamation marks as normal Spanish requires.

## French (fr)

- Formal address with `vous`.
- Use French typography: a non-breaking space before `:`, `;`, `?` and `!` where the source
  uses them in prose, and guillemets for quotations.
- Prefer the established French term (`boutisme`, `pile`, `registre`); use the English term only
  where the CSV says the community does.

## Review

Translations are drafted with AI assistance. A native-speaker reviewer reads the whole page
against the English source and fixes it in place. See `i18n/REVIEWING.md`.
