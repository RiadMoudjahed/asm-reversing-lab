<!-- source: README.md @ sha256:637089ae7b48 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 03. Registre comptable

Level 1. Durée : environ 1 heure. Architecture : x86-64 Linux. Modules : [02. Le processeur et ses registres](../../modules/02-cpu-and-registers/README.fr.md), [04. Déplacer des données et modes d'adressage](../../modules/04-moving-data/README.fr.md), [05. Arithmétique et logique](../../modules/05-arithmetic-and-logic/README.fr.md)

## Scénario

Un outil de comptabilité exécute un court calcul avant de clôturer les comptes. Les auditeurs ne se fient pas au rapport de l'outil lui-même et veulent savoir ce que dit le processeur en quatre points du calcul : les valeurs des registres et l'état des indicateurs. Ils veulent ensuite une routine réutilisable qui rend compte des indicateurs de n'importe quelle addition ou soustraction.

## Objectifs

- Parcourir un programme pas à pas dans un débogueur et lire registres et indicateurs en des points choisis.
- Relier chaque indicateur à l'instruction qui l'a positionné.
- Capturer les indicateurs d'une opération en assembleur, sans instruction de comparaison.

## Partie 1 : Reverse

Le binaire est `bin/ledger`. Il comporte quatre points marqués, les symboles `mark1`, `mark2`, `mark3` et `mark4`. Un point d'arrêt sur une étiquette s'arrête avant l'exécution de l'instruction située à cette étiquette : vous voyez donc le résultat de l'instruction juste au-dessus.

```bash
gdb -q bin/ledger
```

```text
break mark1
break mark2
break mark3
break mark4
run
info registers rax rcx rdx eflags
continue
```

GDB affiche par leur nom les indicateurs positionnés, par exemple `[ CF PF SF IF ]`. Un indicateur absent de la liste vaut 0.

Répondez à ces questions :

1. À `mark1` : combien vaut `rax`, en décimal ?
2. À `mark2` : lesquels des indicateurs CF, ZF, SF et OF sont positionnés ?
3. À `mark3` : lesquels de CF, ZF, SF et OF sont positionnés, et combien vaut `rcx` ?
4. À `mark4` : lesquels de CF, ZF, SF et OF sont positionnés ?

Écrivez chaque groupe d'indicateurs sous forme de quatre chiffres dans l'ordre CF, ZF, SF, OF, avec 1 pour positionné et 0 pour à zéro. Construisez le drapeau à partir de vos réponses :

```text
ARL{ledger_<rax at mark1>_<flags at mark2>_<flags at mark3>_<flags at mark4>}
```

Par exemple, si `rax` valait 5 et que les trois groupes étaient `0000`, `0001` et `0010`, le drapeau serait `ARL{ledger_5_0000_0001_0010}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour implémenter deux fonctions. Elles utilisent la convention d'appel System V : arguments dans `rdi`, `rsi` et `rdx`, résultat dans `rax`.

```c
#include <stdint.h>

uint32_t ledger_add(int64_t a, int64_t b, int64_t *out);   // *out = a + b
uint32_t ledger_sub(int64_t a, int64_t b, int64_t *out);   // *out = a - b
```

Chaque fonction range le résultat dans `*out` et renvoie les indicateurs de cette opération sous forme de masque de bits :

| Bit | Indicateur |
|---|---|
| 0 | CF |
| 1 | ZF |
| 2 | SF |
| 3 | OF |

Tous les autres bits valent 0. Pour `ledger_sub`, CF est l'indicateur de retenue tel que le processeur le positionne pour une soustraction.

## Règles

- N'utilisez ni `cmp` ni `test`. Le statut doit provenir des indicateurs de l'addition ou de la soustraction elle-même.
- Rangez le résultat dans `*out` dans tous les cas.

## Vérifier votre travail

```bash
make check EX=03 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte de la partie 1 et de la partie 2 séparément. Sans `--flag`, il ne vérifie que la partie 2.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
