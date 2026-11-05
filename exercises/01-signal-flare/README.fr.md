<!-- source: README.md @ sha256:09da60cefe79 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 01. Fusée de détresse

Level 1. Durée : environ 45 minutes. Architecture : x86-64 Linux. Modules : [00. Mise en place du laboratoire](../../modules/00-setup/README.fr.md), [03. Votre premier programme](../../modules/03-first-program/README.fr.md)

## Scénario

Un petit programme de balise appelé `signal_flare` annonce son lancement puis se termine. Son code source a été perdu, et les mainteneurs doivent savoir exactement ce qu'il fait pour pouvoir le reconstruire. Vous avez le binaire et rien d'autre.

## Objectifs

- Lire les appels système d'un programme avec `strace` et `objdump`.
- Déterminer le numéro, les arguments et le résultat de chaque appel.
- Reconstruire le programme en assembleur avec des appels système bruts et sans bibliothèque C.

## Partie 1 : Reverse

Le binaire est `bin/signal_flare`. Exécutez ces commandes depuis le dossier de l'exercice, dans le laboratoire :

```bash
file bin/signal_flare
strace ./bin/signal_flare
objdump -d -M intel bin/signal_flare
```

`strace` liste chaque appel système effectué par le programme. `objdump -d -M intel` affiche les instructions en syntaxe Intel ; le code qui vous intéresse commence à l'étiquette `_start`. L'[aide-mémoire des appels système Linux](../../cheatsheets/syscalls-linux.fr.md) indique ce que signifie chaque numéro d'appel et quel registre porte chaque argument.

Répondez à ces questions :

1. Quels numéros d'appel système le programme utilise-t-il, dans l'ordre ?
2. Pour chaque `write`, quel descripteur de fichier utilise-t-il et combien d'octets envoie-t-il ?
3. Quel code de sortie le programme renvoie-t-il ?

Construisez le drapeau à partir de vos réponses :

```text
ARL{flare_<bytes written to descriptor 1>_<bytes written to descriptor 2>_<exit status>}
```

Par exemple, si les nombres étaient 10, 20 et 3, le drapeau serait `ARL{flare_10_20_3}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour que votre programme se comporte exactement comme `signal_flare` :

- Les mêmes octets sur la sortie standard.
- Les mêmes octets sur la sortie d'erreur standard.
- Le même code de sortie.

Compilez-le et exécutez-le depuis le dossier `starter` :

```bash
make run
```

## Règles

- N'utilisez que les appels système `write` et `exit`. Ne liez pas la bibliothèque C.
- Écrivez vous-même le texte et les appels. Votre programme ne doit pas lancer `signal_flare`.
- Définissez les longueurs des textes avec NASM (`$ - label`), sans les compter à la main.

## Vérifier votre travail

```bash
make check EX=01 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte de la partie 1 et de la partie 2 séparément. Sans `--flag`, il ne vérifie que la partie 2.

## Indices

Ouvrez [HINTS.fr.md](HINTS.fr.md) un niveau à la fois.
