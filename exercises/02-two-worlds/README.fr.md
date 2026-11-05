<!-- source: README.md @ sha256:ff34d51c3293 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 02. Deux mondes

Level 1. Durée : environ 1 heure. Architecture : x86 et x86-64 Linux. Modules : [02. Le processeur et ses registres](../../modules/02-cpu-and-registers/README.fr.md), [03. Votre premier programme](../../modules/03-first-program/README.fr.md)

## Scénario

Une équipe maintient deux versions du même petit utilitaire : l'une pour d'anciens systèmes 32 bits et l'autre pour des systèmes 64 bits. Personne ne se souvient en quoi les deux versions diffèrent au plus bas niveau. Votre mission est de documenter la différence à partir des binaires, puis d'utiliser ce savoir pour faire passer un troisième programme 32 bits ancien en 64 bits.

## Objectifs

- Comparer la façon dont les interfaces Linux 32 bits et 64 bits entrent dans le noyau.
- Identifier les numéros d'appel système et les registres d'arguments dans le désassemblage.
- Porter un programme de l'interface 32 bits vers l'interface 64 bits.

## Partie 1 : Reverse

Les binaires sont `bin/two_worlds32` et `bin/two_worlds64`. Ils se comportent de la même façon. Ils diffèrent par la manière dont ils sont écrits.

```bash
file bin/two_worlds32 bin/two_worlds64
objdump -d -M intel bin/two_worlds32
objdump -d -M intel bin/two_worlds64
strace ./bin/two_worlds32
strace ./bin/two_worlds64
```

Utilisez l'[aide-mémoire des appels système Linux](../../cheatsheets/syscalls-linux.fr.md) pour retrouver les numéros. Répondez à ces questions :

1. Quels trois numéros d'appel système chaque binaire charge-t-il dans `eax`, dans l'ordre ? (Les nombres du listing sont en hexadécimal.)
2. Quelle instruction entre dans le noyau dans chaque binaire ?
3. Dans chaque binaire, quel registre porte le premier argument de `write` ?

Construisez le drapeau à partir de l'appel qui n'est pas `write`, et de `exit` :

```text
ARL{worlds_<sync in 32-bit>_<sync in 64-bit>_<exit in 32-bit>_<exit in 64-bit>}
```

## Partie 2 : Assemble

`starter/solution.asm` est un ancien programme 32 bits. Il affiche une ligne, appelle `sync` et se termine avec le code 5. Modifiez-le pour qu'il devienne un programme 64 bits au comportement identique :

- La même sortie sur la sortie standard.
- Les trois mêmes appels système, dans le même ordre.
- Le même code de sortie.

Compilez-le et exécutez-le depuis le dossier `starter` :

```bash
make run
```

## Règles

- Le résultat doit s'assembler avec `nasm -f elf64` et se lier avec `ld` ordinaire.
- N'utilisez pas `int 0x80`. Utilisez l'instruction `syscall` 64 bits.
- Conservez les trois appels, y compris `sync`. Le vérificateur surveille les appels effectués par le programme.

## Vérifier votre travail

```bash
make check EX=02 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte de la partie 1 et de la partie 2 séparément. Sans `--flag`, il ne vérifie que la partie 2.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
