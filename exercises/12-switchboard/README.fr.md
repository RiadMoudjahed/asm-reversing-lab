<!-- source: README.md @ sha256:9b1e5ce5522d -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 12. Standard

Level 3. Durée : environ 3 heures. Architecture : x86-64 Linux. Modules : [13. Reconnaître les structures de code](../../modules/13-code-structures/README.fr.md)

## Scénario

Un petit processeur de commandes, `switchboard`, lit des paires de nombres, une commande et une valeur, et affiche un résultat. Son source est perdu. Un mainteneur doit documenter exactement quelles commandes existent, lesquelles partagent du code et ce qui se passe pour tout autre numéro de commande. Il veut ensuite un remplacement écrit en assembleur et organisé de la même façon, avec une table de sauts, pour qu'il soit aussi rapide que l'original quel que soit le numéro de commande.

## Objectifs

- Reconnaître un `switch` compilé en table de sauts et lire sa vérification de bornes, son ajustement d'indice et sa table.
- Retrouver le code qu'atteint chaque numéro de commande, y compris le code partagé, la continuation (fall-through) et le cas par défaut.
- Écrire en NASM un répartiteur fondé sur une table de sauts.

## Partie 1 : Reverse

Le binaire est `bin/switchboard`. Il lit sur l'entrée standard des lignes de la forme `command value` et affiche un résultat par ligne. La fonction qui fait le travail est `dispatch`.

```bash
printf '7 10\n3 2\n' | bin/switchboard
objdump -d -M intel bin/switchboard
objdump -s -j .rodata bin/switchboard
```

Trouvez ces six valeurs :

1. `N` : le nombre d'entrées de la table de sauts, en décimal.
2. `L` : le plus petit numéro de commande que gère la table de sauts, en décimal.
3. `B` : le nombre d'adresses de code différentes dans la table, sans compter l'adresse qui gère les commandes sans cas, en décimal.
4. `G` : le nombre d'entrées de la table qui pointent vers le code des commandes sans cas, en décimal.
5. `A` : l'adresse que contient la table pour la commande 7, en hexadécimal minuscule sans `0x`.
6. `S` : le numéro de commande, autre que 6, dont l'entrée de la table contient la même adresse que l'entrée de la commande 6, en décimal.

```text
ARL{switch_<N>_<L>_<B>_<G>_<A>_<S>}
```

Par exemple, les valeurs 8, 1, 6, 3, 401234 et 4 donneraient `ARL{switch_8_1_6_3_401234_4}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour implémenter une fonction dans la convention d'appel System V :

```c
#include <stdint.h>

int32_t run_cmd(int32_t cmd, uint32_t acc);   // cmd in edi, acc in esi, result in eax
```

`run_cmd` doit renvoyer exactement ce que `dispatch` renvoie dans `bin/switchboard`, pour tout numéro de commande sur 32 bits (y compris négatif et très grand) et toute valeur sur 32 bits. Les tests les comprennent tous.

## Règles

- Répartissez par une table de sauts : un `jmp` indirect qui lit sa cible dans une table d'adresses indexée avec un facteur 8.
- Vérifiez la plage de la commande par une comparaison non signée avant d'utiliser la table, afin que toute commande hors de la table aille au cas par défaut.
- Utilisez au plus deux instructions de comparaison (`cmp`) dans toute la routine.
- N'utilisez pas `call`.
- Plusieurs entrées de la table peuvent pointer vers le même code, et un cas peut se poursuivre dans le code du suivant.

## Vérifier votre travail

```bash
make check EX=12 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte séparément de la Partie 1 et de la Partie 2. Sans `--flag`, il ne vérifie que la Partie 2. En plus des résultats, il contrôle les règles ci-dessus sur les instructions de votre fichier objet.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
