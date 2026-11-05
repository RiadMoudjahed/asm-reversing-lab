<!-- source: README.md @ sha256:0c07042e0978 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 20. Projet de synthèse : Tiny VM

Niveau 5. Durée : environ 8 heures. Architecture : x86-64 Linux. Modules : [13. Reconnaître les structures de code](../../modules/13-code-structures/README.fr.md), [15. Retrouver les données](../../modules/15-recovering-data/README.fr.md), [21. Machines virtuelles et bytecode](../../modules/21-virtual-machines-bytecode/README.fr.md)

## Scénario

Un vieil outil de maintenance exécute de petits programmes sur une machine virtuelle que personne n'a documentée. Le fabricant a disparu et le source est perdu. Il reste l'interpréteur sans symboles `tinyvm` et un programme d'exemple, `sample.tvm`. Votre travail consiste à documenter la machine assez bien pour travailler avec elle : noter son jeu d'instructions, construire en assembleur les pièces manquantes d'un interpréteur de remplacement et écrire pour elle un nouveau programme avec un assembleur piloté par vos notes. Si vos notes sont fausses quelque part, un maillon de cette chaîne casse.

## Objectifs

- Faire la rétro-ingénierie d'un interpréteur de bytecode : trouver l'état, la répartition, la table des codes opération et le comportement de chaque instruction, y compris les indicateurs, les masques et les cas d'erreur.
- Écrire une spécification complète du jeu d'instructions sous une forme qu'un outil peut utiliser.
- Implémenter en NASM huit gestionnaires d'instructions qui se comportent exactement comme les originaux, sur une structure d'état que vous lisez dans le binaire.
- Écrire pour la machine un programme de bytecode qui produit une sortie donnée dans une limite de pas.

## Partie 1 : Reverse

Les fichiers sont `bin/tinyvm` et `bin/sample.tvm`. Exécutez l'exemple pour voir ce que fait la machine :

```bash
bin/tinyvm bin/sample.tvm
```

La machine prend un argument, un fichier de programme. Le fichier a un en-tête de 8 octets puis le code et les données : les quatre octets `TVM1`, la longueur du code comme nombre de 16 bits, la longueur des données comme nombre de 16 bits (les deux octet de poids faible d'abord), le code et les données. Les données sont copiées au début des 256 octets de mémoire de la machine avant l'exécution du programme. Le nombre de pas que la machine affiche sur la sortie d'erreur standard est le nombre d'instructions exécutées.

La machine a vingt instructions. Leurs noms et leurs formes d'opérandes sont fixés, parce que vos outils en ont besoin :

| Nom | Opérandes |
|---|---|
| `movi` | un registre et un nombre de 0 à 65535 |
| `mov`, `add`, `sub`, `mul`, `xor`, `and`, `cmp` | deux registres |
| `shl`, `shr` | un registre et un compteur de 0 à 15 |
| `jmp`, `jz`, `jnz`, `call` | un décalage dans le code |
| `ret`, `halt` | aucun |
| `ldb` | un registre et un registre utilisé comme adresse : `ldb r1, [r2]` |
| `stb` | un registre utilisé comme adresse et un registre : `stb [r1], r2` |
| `out`, `outc` | un registre |

Tout le reste doit venir du binaire : quel octet de code opération correspond à quelle instruction, combien d'octets chaque instruction occupe, quel quartet contient le premier registre, l'ordre des octets des nombres de 16 bits et ce que fait chaque instruction aux registres, aux indicateurs, à la mémoire et à la pile d'appels.

Retrouvez ce qui suit :

1. Remplissez `starter/isa.json`, un fichier qui décrit le jeu d'instructions. Pour chaque instruction, donnez l'octet de code opération (sous forme de chaîne comme `"0x2b"`) et la longueur totale en octets. Donnez `register_a`, le quartet de l'octet qui suit le code opération qui contient le premier opérande registre (`"high"` ou `"low"`), et `immediate`, l'ordre des octets des nombres de 16 bits (`"little"` ou `"big"`).
2. Trois nombres pour le flag : `N`, le nombre de codes opération valides ; `C`, l'octet de code opération de `call` en décimal ; et `I`, le nombre d'instructions de `bin/sample.tvm`.

```text
ARL{tinyvm_<N>_<C>_<I>}
```

Les valeurs 12, 100 et 30 donneraient par exemple `ARL{tinyvm_12_100_30}`.

Écrivez un désassembleur pour la machine afin de trouver `I` et de vérifier vos notes ; la méthode est celle du module 21, et l'exemple est le programme sur lequel le tester. Le corrigé détaillé de `solutions/20/` contient un désassembleur modèle pour vous comparer ensuite.

## Partie 2 : Assemble

La partie 2 comporte deux morceaux. Les deux dépendent de ce que vous avez trouvé dans la partie 1.

### Les gestionnaires

Un interpréteur de remplacement appelle une fonction pour chaque instruction. Les douze autres gestionnaires sont écrits ; vous écrivez ces huit-ci, dans `starter/solution.asm` :

```c-fragment
void h_mul(struct vm *m, const unsigned char *ins);
void h_shl(struct vm *m, const unsigned char *ins);
void h_shr(struct vm *m, const unsigned char *ins);
void h_cmp(struct vm *m, const unsigned char *ins);
void h_call(struct vm *m, const unsigned char *ins);
void h_ret(struct vm *m, const unsigned char *ins);
void h_ldb(struct vm *m, const unsigned char *ins);
void h_stb(struct vm *m, const unsigned char *ins);
```

`m` pointe vers l'état de la machine, décrit dans `starter/vm.h`, qui reprend la disposition de la structure du binaire : huit registres de 32 bits, le compteur ordinal, l'indicateur de zéro, l'indicateur de retenue, un code de faute, un indicateur d'arrêt, la profondeur de la pile d'appels, la pile d'appels de seize adresses de retour de 16 bits et 256 octets de mémoire. `ins` pointe vers les octets de l'instruction, en commençant par l'octet de code opération, et a exactement la longueur de cette instruction. Quand un gestionnaire est appelé, le compteur ordinal a déjà été avancé au-delà de l'instruction.

Chaque gestionnaire doit laisser l'état exactement comme le fait l'instruction dans `tinyvm`, y compris les indicateurs, quelles instructions laissent un indicateur intact, l'effet des numéros de registre supérieurs à 7 et ce qui se passe aux limites. Deux cas sont donnés : `call` avec une pile d'appels pleine met le code de faute à 3 et l'indicateur d'arrêt à 1 et ne change rien d'autre, et `ret` avec une pile d'appels vide met le code de faute à 2 et l'indicateur d'arrêt à 1 et ne change rien d'autre. Tout le reste du comportement est à trouver.

Les tests construisent de nombreux états, des plus ordinaires aux plus extrêmes, exécutent un gestionnaire sur chacun et comparent l'état complet obtenu avec celui de l'original.

### Le programme

Écrivez dans `starter/program.tvs` un programme de bytecode qui range les douze premiers nombres de Fibonacci (1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144) dans la mémoire de la machine sous forme d'octets, un à chaque adresse de 0 à 11, puis les affiche avec `out` du dernier au premier, un par ligne, de sorte que la sortie soit :

```text
144
89
55
34
21
13
8
5
3
2
1
1
```

Le programme doit utiliser `call` et `ret` (l'affichage est un sous-programme), `stb` (pour ranger les nombres) et `ldb` (pour les relire), et il doit se terminer par `halt` en au plus 400 pas. Assemblez-le avec l'assembleur de l'exercice, qui est piloté par votre `isa.json`, et exécutez-le sur la machine d'origine :

```bash
python3 starter/tvasm.py --isa starter/isa.json starter/program.tvs starter/program.tvm
bin/tinyvm starter/program.tvm
```

Le format source de l'assembleur est décrit en tête de `starter/tvasm.py`. Si vos notes sont fausses, le programme assemblé ne s'exécutera pas correctement sur la machine d'origine.

## Règles

- Ne lisez aucun octet en dehors de l'état ou de l'instruction dans un gestionnaire et n'écrivez pas en dehors de l'état. Les tests placent les deux juste avant une page non mappée, et un accès au-delà de la fin termine l'exécution des tests par un signal.
- Préservez les registres que l'appelé doit conserver : `rbx`, `rbp` et `r12` à `r15`.
- Gardez la pile alignée à chaque `call`, si vous en faites un.
- Les fichiers vérifiés sont `isa.json`, `solution.asm` et `program.tvs` de `starter/`. Ne modifiez pas `tvasm.py`.

## Vérifier votre travail

```bash
make check EX=20 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte séparément de la partie 1 et de la partie 2. La partie 1 est réussie quand le flag est correct et que `starter/isa.json` est correct ; la sortie nomme chaque instruction dont l'entrée est fausse. La partie 2 est réussie quand tous les tests des gestionnaires passent et que le programme affiche la bonne sortie dans les limites. Pour un test de gestionnaire en échec, le vérificateur liste les champs de l'état qui diffèrent, avec leurs valeurs avant l'instruction, la valeur attendue et votre valeur. Sans `--flag`, il saute la partie 1.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
