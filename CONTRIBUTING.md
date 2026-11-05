<div align="center">
  
[English](CONTRIBUTING.md) | [العربية](CONTRIBUTING.ar.md) | [Deutsch](CONTRIBUTING.de.md) | [Español](CONTRIBUTING.es.md) | [Français](CONTRIBUTING.fr.md)
</div>  

# Contributing

Thank you for helping people learn. This page explains how to contribute well.

## Ways to contribute

- Report a technical mistake, an unclear explanation or a broken example.
- Improve or review a translation. Native-speaker review is especially valuable.
- Suggest an improvement to a hint or a write-up.
- Fix typos and formatting.

For new modules or exercises, open an issue first so that the scope can be agreed before you
start.

## Ground rules

1. **Original work only.** Write your own explanations, examples and challenges. Do not
   copy or closely paraphrase any book, course, blog or video, and do not name commercial
   works as sources. Official specifications may be cited.
2. **Stay in scope.** No exploit development, shellcode, malware development, or content on
   defeating copy protection or licensing. Challenge programs are built from source in this
   repository; no third-party binaries.
3. **No spoilers.** Flags and full solutions belong only in `solutions/`. Never post them in
   issues or discussions.
4. **Professional presentation.** No emoji or decorative symbols. Clear, direct language.
5. **Everything runs.** Code blocks tagged `nasm`, `c` or `bash` must work in the lab image.

## Workflow

1. Fork the repository and create a topic branch.
2. Make your change, following the ground rules above and the style of the existing pages.
3. Run `make all-checks` inside the lab. All checks must pass.
4. Open a pull request and complete the checklist in the template.

## Translations

English files are the source. A translation lives next to the English file with a language
suffix: `README.fr.md`, `README.de.md`, `README.es.md`, `README.ar.md`.

- Start the file with `<!-- source: FILE.md @ sha256:<12 hex> -->`, using the first 12
  characters of `sha256sum FILE.md`.
- Do not translate code, mnemonics, register names, commands, file names or flags.
- Use the terms in `i18n/terminology.csv`. Add a row when you need a new term.
- Follow `i18n/STYLE.md` for tone and typography in each language.
- Run `make i18n` to see which pages are missing or out of date.

## Reporting a security or safety concern

If you believe a page or program in this repository could cause harm, contact the maintainers
privately instead of opening a public issue.
