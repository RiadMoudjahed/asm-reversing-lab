<!-- source: README.md @ sha256:7be1abb7c629 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 13. Nombres magiques

Level 3. Durée : environ 3 heures. Architecture : x86-64 Linux. Modules : [05. Arithmétique et logique](../../modules/05-arithmetic-and-logic/README.fr.md), [14. Arithmétique compilée](../../modules/14-compiled-arithmetic/README.fr.md)

## Scénario

Une routine de notation, `mix`, a été compilée avec optimisation et son source est perdu. Elle prend un entier 32 bits et renvoie un score, et elle ne contient aucune instruction de division : chaque division et chaque reste a été remplacé par une multiplication par une constante. Un mainteneur a besoin de la formule exacte, parce que le même score doit être calculé sur une plateforme sans multiplication 64 bits ni instruction de division. Vous retrouvez d'abord la formule. Vous réécrivez ensuite la routine, sans `div` ni `idiv`.

## Objectifs

- Reconnaître la forme multiplication-décalage de la division et du reste, signés et non signés.
- Retrouver le diviseur caché derrière chaque constante magique.
- Reconstruire une division par une constante à partir d'une multiplication, de décalages et d'une correction de signe.

## Partie 1 : Reverse

Le binaire est `bin/magic`. Il lit des entiers sur l'entrée standard et affiche le score de chacun. La fonction qui calcule le score est `mix`.

```bash
printf '100\n-100\n' | bin/magic
objdump -d -M intel bin/magic
```

Le script `14_magic.py` du module 14 transforme une constante magique et son décalage en diviseur et le vérifie.

Le score combine quatre opérations sur l'entrée `x` : un quotient non signé, un reste non signé, un quotient signé et un reste signé, chacun par une constante différente. Les deux opérations non signées travaillent sur `x` lu comme un nombre non signé. Les résultats sont ensuite combinés avec de petits facteurs.

Trouvez ces six valeurs :

1. `A` : le diviseur du quotient non signé.
2. `B` : le diviseur du reste non signé.
3. `C` : le diviseur du quotient signé.
4. `D` : le diviseur du reste signé.
5. `K` : le facteur qui multiplie le quotient non signé dans la somme.
6. `M` : le facteur qui multiplie le reste non signé dans la somme.

```text
ARL{magic_<A>_<B>_<C>_<D>_<K>_<M>}
```

Par exemple, les valeurs 5, 3, 25, 7, 2 et 6 donneraient `ARL{magic_5_3_25_7_2_6}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour implémenter une fonction dans la convention d'appel System V :

```c
#include <stdint.h>

int32_t mix_asm(int32_t x);   // x in edi, result in eax
```

`mix_asm` doit renvoyer exactement ce que `mix` renvoie dans `bin/magic`, pour toute valeur 32 bits de `x`, y compris les valeurs négatives et les extrêmes de la plage.

## Règles

- N'utilisez ni `div` ni `idiv` ; le vérificateur les rejette. Utilisez multiplication, décalages, `lea` et instructions semblables.
- N'utilisez pas `call`.
- L'arithmétique déborde sur 32 bits exactement comme dans le binaire.
- Les quotients et restes signés arrondissent vers zéro, comme en C.

## Vérifier votre travail

```bash
make check EX=13 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte séparément de la Partie 1 et de la Partie 2. Sans `--flag`, il ne vérifie que la Partie 2.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
