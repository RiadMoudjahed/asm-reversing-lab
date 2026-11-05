<!-- source: README.md @ sha256:708c94a5dc1d -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 15. Inventaire chaîné

Level 4. Durée : environ 3 heures. Architecture : x86-64 Linux. Modules : [15. Récupérer les données](../../modules/15-recovering-data/README.fr.md)

## Scénario

Un outil d'entrepôt, `inventory`, garde ses chiffres de stock dans une structure de données chaînée en mémoire. Ses symboles ont été supprimés et le source est perdu. Un second outil, écrit en assembleur, doit ajouter des nombres à la même sorte de structure et la parcourir, et le banc de test lit les nœuds que construit votre code ; la disposition doit donc correspondre exactement. Vous retrouvez d'abord la disposition des nœuds et les règles d'insertion. Vous écrivez ensuite les deux fonctions.

## Objectifs

- Reconnaître un arbre dans du code compilé : l'allocation, les champs du nœud et le parcours.
- Retrouver les décalages, les largeurs et les rôles des champs à partir des instructions qui les utilisent.
- Écrire en assembleur le même code de structure de données, y compris le cas où un lien est encore vide.

## Partie 1 : Reverse

Le binaire est `bin/inventory`. Il lit des commandes sur l'entrée standard, une par ligne.

```bash
printf 'i 5\ni 9\ni 5\nf 5\nf 7\nd\n' | bin/inventory
objdump -d -M intel bin/inventory
```

La commande `i KEY` range une clé (un nombre signé sur 32 bits) et affiche `ok`. La commande `f KEY` affiche deux nombres pour une clé rangée, ou `none`. La commande `d` affiche toute la structure sous forme de texte. Le programme alloue un bloc de mémoire pour chaque nœud, et tous les nœuds contiennent les mêmes champs.

Trouvez ces six valeurs :

1. `S` : la taille en octets que le programme demande pour un nœud, en décimal.
2. `K` : le décalage en octets de la clé dans un nœud, en décimal.
3. `T` : le décalage en octets du compteur de 32 bits qui augmente à chaque commande `i` dont le parcours passe par le nœud, en décimal.
4. `C` : le décalage en octets du compteur qui n'augmente que lorsque la même clé est rangée de nouveau, en décimal.
5. `G` : le décalage en octets du pointeur que suit le parcours lorsque la nouvelle clé est plus grande que la clé du nœud, en décimal.
6. `W` : le premier nombre qu'affiche `f 7` après que la commande `i 7` a été donnée 65540 fois de suite, en décimal.

```bash
for i in $(seq 65540); do echo 'i 7'; done | { cat; echo 'f 7'; } | bin/inventory | tail -1
```

```text
ARL{inventory_<S>_<K>_<T>_<C>_<G>_<W>}
```

Par exemple, les valeurs 40, 8, 12, 16, 24 et 3 donneraient `ARL{inventory_40_8_12_16_24_3}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour implémenter deux fonctions dans la convention d'appel System V :

```c
struct node *inv_insert(struct node *root, int key);
struct node *inv_find(struct node *root, int key);

/* Provided by the tests. */
struct node *inv_alloc(void);
```

- `inv_insert` fait ce que fait la commande `i` de `bin/inventory`. `root` est le pointeur vers le premier nœud, ou 0 pour une structure vide. Elle renvoie le pointeur vers le premier nœud après l'insertion. Chaque modification que le programme apporte aux nœuds existants et chaque champ d'un nouveau nœud doivent correspondre.
- `inv_find` renvoie le pointeur vers le nœud qui contient `key`, ou 0 s'il n'y en a aucun.
- `inv_alloc` renvoie un bloc de mémoire de la taille de nœud que vous avez trouvée, rempli de zéros. Appelez-la une fois pour chaque nouveau nœud et jamais autrement.

Les tests partent d'une structure vide, exécutent une suite d'insertions et de recherches, et affichent ce que la bibliothèque lit dans vos nœuds. Ils comparent chaque ligne avec la sortie que `bin/inventory` produit pour les mêmes commandes.

## Règles

- Utilisez la disposition et les règles du programme ; les tests lisent vos nœuds à travers elles.
- N'écrivez que dans les champs des nœuds. Les tests vérifient que rien ne change au-delà de la taille du nœud et que les octets inutilisés à l'intérieur d'un nœud restent à zéro.
- Préservez les registres à sauvegarder par l'appelé `rbx`, `rbp` et `r12` à `r15`.
- Gardez la pile alignée à chaque `call`.
- N'appelez rien d'autre que `inv_alloc`.

## Vérifier votre travail

```bash
make check EX=15 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte séparément de la Partie 1 et de la Partie 2. Sans `--flag`, il ne vérifie que la Partie 2. Chaque résultat d'un test de la Partie 2 est suivi de `ok` ou d'une liste de ce qui s'est mal passé : `find`, `alloc`, `overrun`, `align` ou `regs`.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
