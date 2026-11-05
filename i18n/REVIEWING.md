# Reviewing a translation

This guide is for native speakers who check one language of the course. Machine checks already cover spelling (Hunspell), structure (headings, tables, lists, code blocks and inline code match the English page) and terminology (`i18n/terminology.csv`). What remains needs a person: grammar, natural wording and technical accuracy.

## What to check

1. **Meaning.** The page says what the English page says. Nothing is added, nothing is dropped.
2. **Natural wording.** A professional reader would not stumble. Prefer the established term in your language; where none exists, the glossary term applies.
3. **Terminology.** Terms match `i18n/terminology.csv` and the page glossary. The English term appears in parentheses on first use.
4. **Technical accuracy.** Statements about registers, flags, calling conventions and stack layout are correct, not just fluent.
5. **Register and tone.** Formal address as set in `i18n/STYLE.md`; no slang, no jokes, no exclamation marks.
6. **Right-to-left pages (Arabic).** Prose reads correctly inside the right-to-left wrapper; code, tables of instructions and inline code stay left-to-right and readable.

## What not to change

- Code blocks, inline code, file names, flags, command lines and program output.
- Link targets. The build tools rewrite them.
- The structure of the page. If the structure seems wrong, fix the English page first; the translation follows.

## How to report or fix

- Small fixes: edit the translated file directly.
- Changes to a term: edit `i18n/terminology.csv` for all languages, then update the pages that use it.
- Meaning problems in the English source: open an issue; do not diverge the translation.

## When a page is done

After your fixes, run:

```bash
make i18n
make lint
```

Priority pages for the first review: `README`, `ETHICS`, modules 00 to 03, then the exercise READMEs.

## Finding terminology gaps

`make terminology` prints the terms of `i18n/terminology.csv` that an English page uses but a translation does not (an inflected ending is allowed). The list is advisory: many entries are a translator's valid rephrasing, and some English words (`flag`, `branch`, `jump`) have an ordinary meaning besides the technical one. Use it as a work list for the consistency pass.

## Known terminology variants to settle

The terminology table is the reference. A scan of all translated pages against it finds these deliberate or unsettled differences; a reviewer should pick one form per language and apply it everywhere.

- German: `Malware` (used in most pages) against `Schadsoftware` (the table). `ausführbare Datei` appears inflected (`ausführbaren Datei`), which is expected.
- French: `logiciel malveillant` appears in the plural form on some pages, which is expected.
- Arabic: for `callee` the older module pages use `المستدعى`, while later pages use `الدالة المستدعاة`; for `prologue` and `epilogue` the later pages use the short forms `المقدمة` and `الخاتمة`; `disassembler` and `decompiler` appear as `مفكك التجميع` and `مفكك الترجمة`; `jump` appears as `قفزة` (older pages) and `قفز` (newer pages).
- Arabic: the challenge flag is `علم` and the processor flag is `راية`; both are intended.
- Spanish and French: `bandera` and `drapeau` mean the challenge flag, `indicador` and `indicateur` the processor flag; both are intended.
