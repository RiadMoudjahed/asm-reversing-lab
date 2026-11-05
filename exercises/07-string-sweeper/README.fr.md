<!-- source: README.md @ sha256:7927481bd356 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 07. Balayeur de chaînes

Level 2. Durée : environ 2 heures. Architecture : x86-64 Linux. Modules : [04. Déplacer des données et modes d'adressage](../../modules/04-moving-data/README.fr.md), [06. Conditions, branchements et boucles](../../modules/06-conditions-and-loops/README.fr.md), [07. Chaînes et instructions de chaîne](../../modules/07-strings/README.fr.md)

## Scénario

Un outil de diagnostic appelé `sweeper` embarque sa propre minuscule bibliothèque : six routines de mémoire et de chaînes, sans bibliothèque C derrière. Les routines ont des noms sans signification, et les mainteneurs doivent savoir laquelle est laquelle avant de pouvoir les documenter ou les remplacer. Ensuite, un nouveau composant a besoin de quatre de ces mêmes routines, écrites de zéro.

## Objectifs

- Reconnaître des routines de mémoire et de chaînes aux instructions de chaîne et aux registres qu'elles utilisent.
- Distinguer une copie vers l'avant d'une copie sûre en cas de chevauchement, et une longueur d'une recherche.
- Écrire `strlen`, `memchr`, `memcpy` et `memmove` avec les instructions de chaîne, en gardant l'indicateur de direction correct.

## Partie 1 : Reverse

Le binaire est `bin/sweeper`. Il exécute un auto-test de six routines et affiche `sweeper: all routines behave`. Les routines sont les symboles globaux `fn1` à `fn6`.

```bash
bin/sweeper
objdump -d -M intel bin/sweeper
```

Chaque routine implémente l'une de ces fonctions de la bibliothèque C : `strlen`, `memchr`, `memcpy`, `memmove`, `memset` et `memcmp`. Déterminez quel numéro `fn` correspond à quelle fonction.

Construisez le drapeau à partir des six numéros, dans cet ordre de fonctions :

```text
ARL{sweeper_<strlen>_<memchr>_<memcpy>_<memmove>_<memset>_<memcmp>}
```

Par exemple, si `strlen` était `fn3`, `memchr` `fn1`, `memcpy` `fn2`, `memmove` `fn4`, `memset` `fn5` et `memcmp` `fn6`, le drapeau serait `ARL{sweeper_3_1_2_4_5_6}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour implémenter quatre des routines selon la convention d'appel System V : arguments dans `rdi`, `rsi` et `rdx`, résultat dans `rax`.

```c
#include <stddef.h>

size_t my_strlen(const char *s);                      // bytes before the first zero byte
void  *my_memchr(const void *s, int c, size_t n);     // first byte equal to (unsigned char) c
                                                      // among the n bytes, or NULL
void  *my_memcpy(void *d, const void *s, size_t n);   // copy n bytes, no overlap; returns d
void  *my_memmove(void *d, const void *s, size_t n);  // copy n bytes, overlap allowed; returns d
```

## Règles

- Utilisez les instructions de chaîne : le vérificateur exige au moins un `rep movs` et un `repnz scas` dans votre code.
- N'utilisez pas `call`.
- L'indicateur de direction est à zéro lorsque votre fonction est appelée. Il doit être à zéro au retour.
- Ne lisez et n'écrivez que les octets qui appartiennent à l'opération. Les tests placent chaque bloc juste avant une page inaccessible : toucher un octet au-delà fait planter le test.
- Traitez correctement le cas `n` égal à 0.

## Vérifier votre travail

```bash
make check EX=07 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte de la partie 1 et de la partie 2 séparément. Sans `--flag`, il ne vérifie que la partie 2.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
