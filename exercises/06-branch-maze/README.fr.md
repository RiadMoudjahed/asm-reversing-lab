<!-- source: README.md @ sha256:846929be6ef4 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 06. Labyrinthe de branchements

Level 2. Durée : environ 1,5 heure. Architecture : x86-64 Linux. Modules : [05. Arithmétique et logique](../../modules/05-arithmetic-and-logic/README.fr.md), [06. Conditions, branchements et boucles](../../modules/06-conditions-and-loops/README.fr.md)

## Scénario

Un contrôleur de porte accepte trois nombres entiers et ouvre le passage ou répond « wall ». Personne n'a conservé les règles. Le programme ne dit jamais quel contrôle vous avez échoué : procéder par essais et erreurs ne mène nulle part, les règles doivent être lues dans le code. Une fois qu'elles sont connues, un nouveau composant a besoin de la même décision en assembleur.

## Objectifs

- Lire une chaîne de comparaisons et de sauts conditionnels et retrouver la règle derrière chacun.
- Distinguer les sauts signés des sauts non signés dans le désassemblage.
- Reproduire la logique de décision avec seulement `cmp`, `test` et des sauts conditionnels.

## Partie 1 : Reverse

Le binaire est `bin/maze`. Il prend trois entiers sur la ligne de commande.

```bash
bin/maze 1 2 3
objdump -d -M intel bin/maze
```

Le programme affiche `wall` lorsqu'un contrôle échoue et `path open` lorsqu'ils réussissent tous. Plusieurs triplets sont acceptés. Trouvez le triplet accepté qui est le plus petit lorsqu'on compare d'abord A, puis B, puis C (celui qui a le plus petit A ; parmi ceux-là, le plus petit B ; parmi ceux-là, le plus petit C).

Construisez le drapeau à partir de ce triplet. Écrivez un nombre négatif avec la lettre `m` à la place du signe moins : -7 devient `m7` :

```text
ARL{maze_<A>_<B>_<C>}
```

Par exemple, le triplet 5, -3, -4 donnerait `ARL{maze_5_m3_m4}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour implémenter une fonction selon la convention d'appel System V : `a` arrive dans `edi`, `b` dans `esi` et `c` dans `edx`, et le résultat repart dans `eax`.

```c
#include <stdint.h>

int32_t maze_ok(int32_t a, int32_t b, int32_t c);
```

`maze_ok` renvoie 1 exactement lorsque `bin/maze` affiche `path open` pour les trois mêmes nombres, et 0 sinon. Les tests comprennent des valeurs limites et des valeurs proches de chaque borne : chaque règle doit donc être exacte.

## Règles

- N'utilisez que `cmp`, `test` et des sauts conditionnels pour les décisions. N'utilisez ni `cmov` ni `setcc`.
- Traitez `a`, `b` et `c` comme des entiers signés de 32 bits.

## Vérifier votre travail

```bash
make check EX=06 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte de la partie 1 et de la partie 2 séparément. Sans `--flag`, il ne vérifie que la partie 2.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
