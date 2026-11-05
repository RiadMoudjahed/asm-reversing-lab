<!-- source: README.md @ sha256:686f6efb0253 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 04. Déplacer des données et modes d'adressage

Durée : environ 2 heures. Prérequis : [03. Votre premier programme](../03-first-program/README.fr.md)

## Pourquoi c'est important

La plupart des instructions d'un programme déplacent des données : de la mémoire vers un registre, d'un registre vers la mémoire, d'un registre vers un autre. La façon dont une instruction désigne un emplacement mémoire, son mode d'adressage, est ce qui donne aux tableaux, aux structures et aux variables locales leur aspect en assembleur. Quand vous lisez du code compilé, l'expression entre crochets d'une instruction vous dit ce que le programme d'origine indexait, et avec quelle taille d'élément.

## Notions

### L'instruction `mov`

`mov destination, source` copie la source dans la destination. La source n'est pas modifiée. Il existe cinq formes :

| Forme | Exemple | Signification |
|---|---|---|
| registre, immédiat | `mov eax, 5` | Mettre une constante dans un registre |
| registre, registre | `mov rbx, rax` | Copier un registre dans un autre |
| registre, mémoire | `mov eax, [rdi]` | Charger depuis la mémoire |
| mémoire, registre | `mov [rdi], eax` | Écrire en mémoire |
| mémoire, immédiat | `mov dword [rdi], 5` | Écrire une constante en mémoire |

Il n'existe pas de forme mémoire vers mémoire. Pour copier entre deux emplacements mémoire, passez par un registre. Les deux opérandes doivent avoir la même taille, et cette taille doit être connue : un registre la fixe, mais une constante écrite en mémoire ne la fixe pas, vous devez donc préciser la largeur avec `byte`, `word`, `dword` ou `qword`.

### Opérandes mémoire

Les crochets signifient « la mémoire à cette adresse ». Sans crochets, vous obtenez la valeur de l'expression elle-même. La forme générale d'un opérande mémoire est :

```text
[ base + index * scale + displacement ]
```

- **base** et **index** sont des registres. L'un ou l'autre peut être omis.
- **scale** vaut 1, 2, 4 ou 8 et multiplie l'index. Il correspond à la taille d'élément d'un tableau d'octets, de mots, de doubles mots ou de quadruples mots.
- **displacement** est une constante, comme le décalage d'un champ dans une structure.

Usages typiques :

| Opérande | Signification habituelle |
|---|---|
| `[rdi]` | La mémoire sur laquelle pointe un pointeur |
| `[rdi + 8]` | Un champ au décalage 8 d'une structure |
| `[rdi + rsi*4]` | L'élément `rsi` d'un tableau de valeurs de 4 octets |
| `[rdi + rsi + 8]` | Un champ au décalage 8 d'un élément dont le décalage est déjà dans `rsi` |
| `[table + 8]` | Une variable globale, adressée relativement à `rip` en code 64 bits |

### Taille de l'opérande

La taille d'un accès mémoire provient du registre concerné (`eax` fait 4 octets, `al` en fait 1) ou d'un mot-clé (`byte`, `word`, `dword`, `qword`). Un chargement 32 bits dans `eax` met aussi à zéro la moitié haute de `rax`, comme l'a montré le module 02.

### Chargements avec élargissement

Le module 01 a expliqué l'extension par des zéros et l'extension de signe. Trois instructions les appliquent au chargement :

| Instruction | Effet | Utilisation |
|---|---|---|
| `movzx eax, byte [rdi]` | Extension par des zéros depuis 8 ou 16 bits | Valeurs non signées |
| `movsx eax, byte [rdi]` | Extension de signe depuis 8 ou 16 bits | Valeurs signées |
| `movsxd rax, dword [rdi]` | Extension de signe de 32 à 64 bits | Une valeur signée de 32 bits utilisée comme valeur de 64 bits |

Il n'existe pas de `movzx` de 32 à 64 bits, car un `mov` 32 bits ordinaire met déjà à zéro la moitié haute.

### `lea` : calculer une adresse

`lea rax, [rdi + rdi*4 + 7]` calcule l'expression d'adresse et range le résultat dans `rax`. Elle ne lit jamais la mémoire. Les compilateurs s'en servent comme d'un moyen rapide de faire de l'arithmétique : celle-ci calcule `5 * rdi + 7` en une seule instruction.

### `xchg`

`xchg a, b` échange deux opérandes. Avec un opérande mémoire, elle est lente parce que le processeur verrouille automatiquement la mémoire ; le code ordinaire échange donc avec `mov` en passant par des registres.

### Adressage relatif à `rip`

En code 64 bits, une référence à une variable globale comme `[table + 8]` est codée comme un décalage par rapport au pointeur d'instruction lorsque vous écrivez `default rel` en tête de fichier. Cela permet de charger le programme à n'importe quelle adresse. Vous verrez `[rip + 0x...]` partout dans le désassemblage 64 bits. En code 32 bits, l'adresse est au contraire une constante absolue, comme vous l'avez vu à l'exercice 02.

### Une remarque sur l'appel de ces fonctions

Les exemples sont des fonctions appelées depuis un programme C. En attendant que le module 08 explique la convention d'appel, admettez deux faits : les deux premiers arguments entiers arrivent dans `rdi` et `rsi`, et le résultat repart dans `rax`. `ret` rend la main à l'appelant.

### Le point de vue de la rétro-ingénierie

Comparez les fonctions de [04_addressing.asm](examples/04_addressing.asm) avec ce qu'un compilateur produit pour du C équivalent dans [04_compiled.c](examples/04_compiled.c). Exécutez `make compiled` dans le dossier des exemples :

```text
<get_dword>:
  movsxd rax,DWORD PTR [rdi+rsi*4]
  ret
<load_u8>:
  movzx  eax,BYTE PTR [rdi]
  ret
<load_s8>:
  movsx  eax,BYTE PTR [rdi]
  ret
<times5plus7>:
  lea    rax,[rdi+rdi*4+0x7]
  ret
<item_price>:
  shl    rsi,0x4
  mov    rax,QWORD PTR [rsi+rdi*1+0x8]
  ret
```

La ressemblance n'est pas un hasard. Le C `a[i]` sur un tableau d'`int` devient `[rdi+rsi*4]` ; `*p` sur un octet non signé devient `movzx` et sur un octet signé `movsx` ; `x * 5 + 7` devient un seul `lea` ; et `v[i].price` devient `shl rsi, 4` (la structure fait 16 octets) plus un décalage de 8 (le décalage du champ). Lire l'expression entre crochets à l'envers permet de retrouver la structure de données : une échelle de 4 signifie des éléments de 4 octets, un décalage constant signifie un champ, un décalage de bits de 4 signifie des enregistrements de 16 octets.

## À vous de jouer

Construisez les exemples et exécutez-les :

```bash
cd modules/04-moving-data/examples
make run
```

Sortie attendue :

```text
get_dword(numbers, 3)    = -40
load_u8(0xF6)            = 246
load_s8(0xF6)            = -10
load_le16(34 12)         = 0x1234
times5plus7(6)           = 37
item_price(items, 1)     = 975
after set_second         = 10 0 30 -40
after swap_ints          = 2 1
table_second()           = 222
```

Associez chaque ligne à la fonction qui l'a produite. `load_u8` et `load_s8` lisent le même octet et donnent 246 et -10, ce qui est la leçon du module 01 en action. `load_le16` lit les octets `34 12` et renvoie `0x1234` : l'octet de poids faible vient en premier.

Modifiez ensuite quelque chose et prédisez le résultat avant de reconstruire :

1. Dans `get_dword`, changez l'échelle de 4 à 8. Que lit maintenant `get_dword(numbers, 1)` ?
2. Dans `times5plus7`, changez l'instruction en `lea rax, [rdi + rdi*2 + 1]`. Que renvoie `times5plus7(6)` ?

## Pièges courants

- **Oublier la taille lors de l'écriture d'une constante.** `mov [rdi], 1` échoue avec `error: operation size not specified`. Écrivez `mov dword [rdi], 1`.
- **Mémoire vers mémoire.** `mov [rdi], [rsi]` échoue avec `error: invalid combination of opcode and operands`. Passez par un registre.
- **Oublier les crochets.** `mov eax, x` charge l'adresse de `x` ; `mov eax, [x]` charge la valeur qui y est stockée.
- **S'attendre à ce que `lea` lise la mémoire.** Elle ne fait que calculer une adresse. `lea rax, [rdi + 8]` et `mov rax, [rdi + 8]` font des choses différentes.
- **`movzx` de 32 à 64 bits.** Elle n'existe pas et NASM la rejette avec `error: invalid combination of opcode and operands`. Utilisez `mov eax, eax` ou n'importe quelle écriture 32 bits.
- **Utiliser la mauvaise extension.** `movzx` sur un octet signé transforme -10 en 246.

## Résumé

- `mov` copie ; il n'existe pas de forme mémoire vers mémoire, et une constante écrite en mémoire demande un mot-clé de taille.
- Un opérande mémoire s'écrit `[base + index*scale + displacement]` ; les crochets signifient « la mémoire à ».
- `movzx` et `movsx` étendent par des zéros ou par le signe au chargement ; `movsxd` élargit de 32 à 64 bits.
- `lea` calcule une adresse sans lire la mémoire et sert souvent à faire de l'arithmétique.
- Lire une expression entre crochets à l'envers révèle les tailles d'éléments des tableaux et les décalages des champs des structures.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 05, arithmétique et logique.
- Exercices qui utilisent ce module : [Exercice 03](../../exercises/03-register-ledger/README.fr.md), [Exercice 04](../../exercises/04-byte-order/README.fr.md), [Exercice 07](../../exercises/07-string-sweeper/README.fr.md).
