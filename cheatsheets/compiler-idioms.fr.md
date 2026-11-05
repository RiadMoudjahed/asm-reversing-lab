<!-- source: compiler-idioms.md @ sha256:dbf265daea9e -->
[English](compiler-idioms.md) | [العربية](compiler-idioms.ar.md) | [Deutsch](compiler-idioms.de.md) | [Español](compiler-idioms.es.md) | [Français](compiler-idioms.fr.md)

# Idiomes du compilateur

Ce que produisent les compilateurs optimiseurs pour les constructions C courantes, et comment le relire. Leçons : [13. Reconnaître les structures de code](../modules/13-code-structures/README.fr.md), [14. Arithmétique compilée](../modules/14-compiled-arithmetic/README.fr.md), [15. Récupérer les données](../modules/15-recovering-data/README.fr.md). Les exemples sont du code 32 bits avec `int` et `unsigned` pour x86-64 System V, avec le premier argument dans `edi`.

## Forme d'une fonction

| Motif | Ce qu'il vous apprend |
|---|---|
| `push rbp; mov rbp, rsp` ... `pop rbp` ou `leave` | Un pointeur de cadre : code non optimisé, ou zone de pile de taille variable |
| Instructions push de `rbx`, `rbp`, `r12` à `r15` au début | Valeurs qui vivent à travers les appels |
| `sub rsp, N` | Place pour les variables locales, les temporaires sauvegardés ou l'alignement |
| Pas de prologue, se termine par `ret` | Une fonction feuille |
| Se termine par un `jmp` vers une autre fonction | Un appel terminal : la fonction appelée retourne à l'appelant de l'appelant |
| Un corps de fonction copié dans l'appelant | Expansion en ligne ; cherchez un appel qui manque |

## Flux de contrôle

| Indice | Source |
|---|---|
| `cmp` / `jcc` au-dessus d'un bloc | `if` |
| Deux blocs avec un `jmp` au-dessus du second | `if` / `else` |
| Plusieurs `jcc` vers une même étiquette "false", le succès continue en séquence | Conditions reliées par `&&` |
| `jcc` vers une étiquette "true", le dernier test saute ailleurs si false | Conditions reliées par l'opérateur OU |
| `jmp test` au début, test en bas avec un `jcc` vers l'arrière | `for` / `while`, non optimisé |
| Une garde qui saute la boucle, le corps, puis un `jcc` vers l'arrière | `for` / `while`, optimisé |
| Le corps d'abord, seulement un `jcc` vers l'arrière | `do` / `while` |
| `jmp` inconditionnel vers l'arrière avec une sortie dans le corps | Boucle infinie |

## Switch

| Indice | Ce que c'est |
|---|---|
| Plusieurs `cmp reg, constant / je` à la suite | Chaîne de comparaisons |
| `cmp reg, N / ja default` puis un `jmp` indirect par `[reg*8 + table]` | Table de sauts de N + 1 entrées |
| `cmp reg, N / ja default` puis un chargement depuis un tableau | Table de correspondance |
| `cmp` avec une constante médiane, puis `je` et `jg`, répétés | Recherche binaire |

## Multiplication par une constante

| Constante | Code typique |
|---|---|
| 2, 4, 8 | `shl` ou `lea eax, [rdi*8]` |
| 3, 5, 9 | `lea eax, [rdi+rdi*2]`, `[rdi+rdi*4]`, `[rdi+rdi*8]` |
| 2^n - 1 (7, 15, 31) | `shl`, puis `sub` de l'original |
| 2^n + 1 (17, 33) | `shl`, puis `add` de l'original |
| 6, 10, 12 | `lea`, puis `add` ou `shl` |
| -1, -3 | `neg`, ou `sub` d'une valeur multipliée |
| Grande ou irrégulière | `imul reg, reg, constant` |

`lea` calcule `base + index*scale + displacement` (facteur 1, 2, 4 ou 8) sans lire la mémoire ni modifier les indicateurs. Un `lea` dont le résultat n'est jamais utilisé comme adresse est de l'arithmétique.

## Division et reste par une constante

| Séquence | Opération |
|---|---|
| `shr reg, k` | Division non signée par 2^k |
| `and reg, 2^k - 1` | Reste non signé modulo 2^k |
| `shr 31`, `add`, `sar` ou `lea`, `test`, `cmovns`, `sar` | Division signée par une puissance de deux (ajoute un biais de 2^k - 1 aux valeurs négatives) |
| `sar 31`, `shr 32-k`, `add`, `and`, `sub` | Reste signé par une puissance de deux (jamais un simple `and`) |
| `imul` par une grande constante impaire, puis `shr` | Division non signée par une constante |
| La même plus `sar 31` et un `sub` | Division signée par une constante |
| `sub`, `shr`, `add`, `shr` après la multiplication haute | Division par une constante dont le nombre magique demande 33 bits (par exemple 7) |
| `x - (x / d) * d` avec les séquences ci-dessus | Reste par une constante |
| `cdq`, `idiv` | Division signée par une variable |
| `xor edx, edx`, `div` | Division non signée par une variable |

Retrouver le diviseur : diviseur = 2^shift / magic, arrondi à l'entier le plus proche. Le script `modules/14-compiled-arithmetic/examples/14_magic.py` le fait et le vérifie.

## Code sans branchement

| Séquence | Signification |
|---|---|
| `xor eax, eax` / `test` / `sete al` | `x == 0` comme 0 ou 1 |
| `cmp` / `setl` ou `setb` | Une comparaison comme 0 ou 1 |
| `neg` / `cmovs` | Valeur absolue |
| `cmp a, b` / `sbb reg, reg` | Que des uns si `a < b` non signé, sinon 0 |
| `cmp` / `cmovge` ou `cmovnb` | Maximum de deux valeurs |

## Signé ou non signé

| | Signé | Non signé |
|---|---|---|
| Saut après une comparaison | `jl`, `jle`, `jg`, `jge` | `jb`, `jbe`, `ja`, `jae` |
| Positionnement à partir d'une comparaison | `setl`, `setg` | `setb`, `seta` |
| Déplacement conditionnel | `cmovl`, `cmovge` | `cmovb`, `cmovae` |
| Décalage à droite | `sar` | `shr` |
| Extension à 64 bits | `movsx` ou `movsxd` | `mov` vers un registre 32 bits |
| Division | `cdq` / `idiv` | `xor edx, edx` / `div` |

## Données

| Indice | Signification |
|---|---|
| Plusieurs accès à des décalages fixes depuis un pointeur | Une structure ; les décalages sont ses champs |
| `[base + index*scale]` | Un tableau d'éléments de taille `scale` |
| Un accès avec un déplacement constant et un indice | Un tableau dans une structure |
| Un appel à l'allocateur avec une taille constante | La taille d'une structure |
| Un pointeur rangé à un décalage d'une structure, chargé et suivi dans une boucle | Une liste chaînée ou un arbre |
| Un rangement d'une adresse constante dans le premier mot d'un nouvel objet | Un pointeur de vtable (C++) : l'adresse pointe après les informations de type |
| `call [rax + 8*n]` après le chargement de `rax` depuis `[rdi]` | Un appel virtuel, emplacement `n` |
