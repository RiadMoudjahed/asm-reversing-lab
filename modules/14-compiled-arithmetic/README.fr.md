<!-- source: README.md @ sha256:d0e94170d0f4 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 14. Arithmétique compilée

Durée : environ 2,5 heures. Prérequis : [13. Reconnaître les structures de code](../13-code-structures/README.fr.md)

## Pourquoi c'est important

Les compilateurs optimiseurs émettent rarement l'instruction que l'on attend pour un opérateur arithmétique. Une multiplication devient un `lea`, une division devient une multiplication et un décalage, une comparaison devient une astuce d'indicateurs sans saut. Ces séquences ressemblent à du bruit tant qu'on ne connaît pas les idiomes ; une fois qu'on les connaît, chacune se lit comme une seule opération, et les constantes qu'elles contiennent révèlent les nombres d'origine. Ce module enseigne les idiomes, montre comment retrouver la constante d'origine à partir de la constante compilée et explique comment distinguer l'arithmétique signée de la non signée.

## Concepts

### Comment les listings de ce module ont été produits

Le source C est `examples/14_arith.c`. Le `Makefile` le compile avec GCC en `-O0` et `-O2` et écrit de l'assembleur en syntaxe Intel dans `build/14_O0.s` et `build/14_O2.s`, avec les mêmes options qu'au module 13. Les listings sont ces fichiers privés des directives de l'assembleur. Tout le code est de l'arithmétique 32 bits `int` et `unsigned` en code x86-64 System V : le premier argument est dans `edi`, le deuxième dans `esi` et le résultat dans `eax`.

```bash
make
```

### L'optimisation change l'arithmétique

En `-O0`, le compilateur suit de près le source, mais même alors il n'émet jamais d'instruction lente là où une séquence rapide existe.

```text
times7:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    mov    edx, DWORD PTR [rbp-4]
    mov    eax, edx
    sal    eax, 3
    sub    eax, edx
    pop    rbp
    ret
```

`x * 7` vaut `(x << 3) - x` : `sal eax, 3` est un décalage à gauche de 3, qui multiplie par 8, et le `sub` retire un `x`. En `-O2`, la même fonction perd son cadre et une instruction :

```text
times7:
    lea    eax, [0+rdi*8]
    sub    eax, edi
    ret
```

`lea eax, [0+rdi*8]` calcule `8 * x` et `sub` soustrait `x`. Dans les deux versions, vous reconnaissez la même forme : la multiplication par une constante qui vaut une unité de moins qu'une puissance de deux est un décalage et une soustraction.

### `lea` comme arithmétique

`lea` (load effective address) calcule l'expression d'adresse `base + index*scale + displacement` et range le résultat **sans lire la mémoire** et sans modifier les indicateurs. Le facteur peut valoir 1, 2, 4 ou 8. Les compilateurs l'utilisent comme un petit additionneur et multiplicateur rapide.

```text
times5:
    lea    eax, [rdi+rdi*4]
    ret
```

```text
madd:
    lea    eax, [rsi+3+rdi*8]
    ret
```

`times5` vaut `x + 4*x`. `madd` calcule `a * 8 + b + 3` en une seule instruction : `rdi` multiplié par 8, plus `rsi`, plus 3. Chaque fois que vous voyez `lea` avec une paire de registres et un facteur, et que l'« adresse » ne sert jamais à accéder à la mémoire, c'est de l'arithmétique.

| Forme de `lea` | Valeur calculée |
|---|---|
| `lea eax, [rdi+rdi*2]` | `3 * x` |
| `lea eax, [rdi+rdi*4]` | `5 * x` |
| `lea eax, [rdi+rdi*8]` | `9 * x` |
| `lea eax, [rdi*8+0]` | `8 * x` |
| `lea eax, [rsi+rdi*8+3]` | `8 * a + b + 3` |

### Multiplication par des constantes

Une multiplication par une petite constante est réécrite en décalages, additions et `lea`. Le résultat doit être le même que celui de la vraie multiplication, y compris quand il déborde sur 32 bits.

```text
times10:
    lea    eax, [rdi+rdi*4]
    add    eax, eax
    ret
```

```text
times_neg3:
    lea    edx, [0+rdi*4]
    mov    eax, edi
    sub    eax, edx
    ret
```

`times10` vaut `(x + 4*x) * 2` : un `lea` pour 5x et un `add eax, eax` pour doubler. `times_neg3` calcule `x - 4*x`, soit `-3 * x`, avec un `lea` et un `sub`. Pour une constante comme 100, le compilateur juge qu'une multiplication coûte moins cher :

```text
times100:
    imul    eax, edi, 100
    ret
```

`imul eax, edi, 100` est la multiplication à trois opérandes : destination, source, immédiat. Une multiplication par une constante sans décalages ni `lea` autour n'est qu'une multiplication.

| Constante | Séquence typique |
|---|---|
| 2, 4, 8, ... | `shl` (ou `lea` avec un facteur) |
| 3, 5, 9 | `lea eax, [rdi+rdi*2]`, `*4`, `*8` |
| 2^n - 1 (7, 15, 31) | `shl` et `sub` |
| 2^n + 1 (17, 33) | `shl` et `add` |
| 6, 10, 12, ... | `lea` ou `shl` suivi de `add` ou `lea` |
| -1, -3 | `neg`, ou `sub` à partir d'une valeur mise à l'échelle |
| Grande ou irrégulière | `imul reg, reg, constant` |

### Division et modulo par des puissances de deux

La division non signée par 8 est un décalage à droite, et le reste non signé est un masque :

```text
udiv8:
    mov    eax, edi
    shr    eax, 3
    ret
```

La division signée est plus subtile, car C arrondit vers zéro alors que `sar` arrondit vers moins l'infini. Pour un `x` négatif, `x / 2` et `x >> 1` diffèrent d'une unité quand `x` est impair. Le compilateur corrige cela en ajoutant un biais aux entrées négatives avant le décalage.

```text
sdiv2:
    mov    eax, edi
    shr    eax, 31
    add    eax, edi
    sar    eax
    ret
```

`shr eax, 31` extrait le bit de signe (1 pour les valeurs négatives, sinon 0), `add` l'ajoute à `x`, et `sar eax` décale d'un cran vers la droite. Pour `x = -3`, les étapes donnent `-3 + 1 = -2`, puis `-2 >> 1 = -1`, ce que C renvoie pour `-3 / 2`. Cet idiome de quatre instructions est la signature de la division signée par 2. Pour des puissances de deux plus grandes, le biais est `2^k - 1` :

```text
sdiv8:
    test    edi, edi
    lea    eax, [rdi+7]
    cmovns    eax, edi
    sar    eax, 3
    ret
```

`lea eax, [rdi+7]` vaut `x + 7` ; `test` et `cmovns` laissent `x` inchangé quand il n'est pas négatif. Ensuite `sar eax, 3` décale. Le reste utilise le même biais :

```text
smod8:
    mov    edx, edi
    sar    edx, 31
    shr    edx, 29
    lea    eax, [rdi+rdx]
    and    eax, 7
    sub    eax, edx
    ret
```

Les trois premières instructions construisent le biais (`7` pour un `x` négatif, `0` sinon), `add` et `and eax, 7` réduisent la valeur biaisée, et `sub` retire le biais. La signature est `sar 31`, `shr 29`, puis un `and` avec 7. Un reste signé par une puissance de deux n'est jamais un simple `and`.

### Division par d'autres constantes : multiplication par un nombre magique

Une instruction de division est lente ; pour un diviseur constant, le compilateur la remplace donc par une multiplication par un **nombre magique** précalculé et un décalage. L'idée est de calculer `x / d` comme `x * M >> k`, où `M` vaut environ `2^k / d`, arrondi par excès. La division non signée par 10 le montre :

```text
udiv10:
    mov    eax, edi
    mov    edx, 3435973837
    imul    rax, rdx
    shr    rax, 35
    ret
```

`3435973837` vaut `0xCCCCCCCD`. Le `imul` 64 bits multiplie `x` par le nombre magique sans débordement, et `shr rax, 35` retire 35 bits de partie fractionnaire. Vérifiez les nombres : `3435973837 * 10 = 34359738370`, alors que `2^35 = 34359738368`. Le nombre magique est `2^35 / 10` arrondi par excès, et le décalage de 35 est l'exposant.

**Retrouver le diviseur à partir du code est une division :** diviseur = `2^shift / magic`, arrondi à l'entier le plus proche. Le script `examples/14_magic.py` le fait et compare le résultat à une vraie division sur 200 000 entrées :

```bash
python3 14_magic.py 3435973837 35
```

```text
2**35 / 3435973837 = 9.9999999994  ->  divisor 10
checked 200007 inputs, 0 differences
```

La division signée par 10 utilise un autre nombre magique et une correction pour les entrées négatives :

```text
sdiv10:
    movsx    rax, edi
    sar    edi, 31
    imul    rax, rax, 1717986919
    sar    rax, 34
    sub    eax, edi
    ret
```

`movsx rax, edi` étend `x` avec signe sur 64 bits, et `sar edi, 31` transforme le bit de signe en 0 ou -1. Le produit est décalé de 34 vers la droite, et soustraire le signe (0 ou -1) ajoute 1 pour les entrées négatives, ce qui transforme l'arrondi vers le bas en troncature vers zéro :

```bash
python3 14_magic.py 1717986919 34 signed
```

```text
2**34 / 1717986919 = 9.9999999965  ->  divisor 10
checked 200007 inputs, 0 differences
```

Certains diviseurs, dont 7, exigent un nombre magique de 33 bits, qui ne tient pas dans une multiplication 32 bits. Le compilateur utilise alors une séquence plus longue :

```text
udiv7:
    mov    eax, edi
    imul    rax, rax, 613566757
    shr    rax, 32
    sub    edi, eax
    shr    edi
    add    eax, edi
    shr    eax, 2
    ret
```

Le premier `imul` et `shr rax, 32` donnent la moitié haute du produit. Les étapes `sub / shr / add` rajoutent le bit manquant, et le `shr eax, 2` final est le reste du décalage. Le script auxiliaire gère cette forme « avec correction » : le nombre magique est `613566757 + 2^32` et le décalage total est `32 + 2 + 1` :

```bash
python3 14_magic.py 613566757 2 fixup
```

```text
2**35 / 4908534053 = 6.9999999994  ->  divisor 7
checked 200007 inputs, 0 differences
```

Un reste par une constante se calcule comme `x - (x / d) * d`. Pour 10, le quotient vient de la séquence magique, la multiplication par 10 est `lea` et `add`, et un `sub` conclut :

```text
umod10:
    mov    eax, edi
    mov    edx, 3435973837
    imul    rax, rdx
    shr    rax, 35
    lea    edx, [rax+rax*4]
    mov    eax, edi
    add    edx, edx
    sub    eax, edx
    ret
```

Quand le diviseur n'est pas une constante, le compilateur ne peut rien précalculer et utilise l'instruction réelle. L'extension du dividende est la signature du caractère signé :

```text
sdiv_var:
    mov    eax, edi
    cdq
    idiv    esi
    ret
```

```text
udiv_var:
    mov    eax, edi
    xor    edx, edx
    div    esi
    ret
```

La division signée étend `eax` avec signe dans `edx` par `cdq` et utilise `idiv`. La division non signée met `edx` à zéro avec `xor edx, edx` et utilise `div`. Les deux laissent le quotient dans `eax` et le reste dans `edx`.

| Séquence | Opération |
|---|---|
| `shr reg, k` | Division non signée par `2^k` |
| `and reg, 2^k - 1` | Reste non signé modulo `2^k` |
| `shr 31 / add / sar` ou `lea / cmovns / sar` | Division signée par une puissance de deux |
| `imul` par une grande constante impaire, puis `shr` | Division par une constante (non signée) |
| La même chose plus `sar 31` et un `sub` | Division par une constante (signée) |
| `cdq`, puis `idiv` | Division signée par une variable |
| `xor edx, edx`, puis `div` | Division non signée par une variable |

### Code sans saut

Plusieurs idiomes transforment une comparaison en valeur sans aucun saut. Les indicateurs de condition sont l'entrée, et `setcc`, `cmov` et `sbb` produisent la sortie.

```text
is_zero:
    xor    eax, eax
    test    edi, edi
    sete    al
    ret
```

`is_zero` est `x == 0` : `test` positionne l'indicateur de zéro, et `sete al` le range sous forme de 0 ou 1 (le `xor eax, eax` qui précède efface le reste du registre, car `setcc` n'écrit qu'un octet).

```text
absval:
    mov    eax, edi
    neg    eax
    cmovs    eax, edi
    ret
```

`absval` met `x` à l'opposé dans `eax`. Si le résultat est négatif (`cmovs` teste l'indicateur de signe issu de `neg`), `x` était positif et `eax` est remplacé par l'original. Le résultat est la valeur absolue.

```text
sign:
    xor    eax, eax
    test    edi, edi
    setg    al
    shr    edi, 31
    sub    eax, edi
    ret
```

`sign` renvoie `(x > 0) - (x < 0)` : `setg al` donne 1 pour les valeurs positives, `shr edi, 31` donne 1 pour les valeurs négatives, et la soustraction produit -1, 0 ou 1.

```text
mask_lt:
    cmp    edi, esi
    sbb    eax, eax
    ret
```

`mask_lt` renvoie `-(a < b)`, soit un mot dont tous les bits valent 1 quand `a < b` et zéro sinon. `cmp edi, esi` positionne l'indicateur de retenue quand `edi` est inférieur à `esi` (non signé), et `sbb eax, eax` calcule `eax - eax - CF`, soit `-CF`. La paire `cmp / sbb reg, reg` crée un masque à partir d'une comparaison ; c'est aussi un composant courant du code à temps constant.

| Idiome | Signification |
|---|---|
| `xor eax, eax` / `test` / `sete al` | `x == 0` sous forme de 0 ou 1 |
| `cmp` / `setl` ou `setb` | Une comparaison sous forme de 0 ou 1 |
| `neg` / `cmovs` | Valeur absolue |
| `cmp reg, reg` / `sbb reg, reg` | Un masque de uns si le premier est inférieur au second (non signé), sinon 0 |
| `cmp` / `cmovge` ou `cmovnb` | Maximum de deux valeurs |

### Signé ou non signé

La machine ne conserve pas le caractère signé d'une valeur. C'est une propriété des instructions qui l'utilisent, ce qui en fait l'un des renseignements les plus utiles que vous puissiez retrouver dans du code compilé.

```text
less_s:
    xor    eax, eax
    cmp    edi, esi
    setl    al
    ret
```

```text
less_u:
    xor    eax, eax
    cmp    edi, esi
    setb    al
    ret
```

Les deux fonctions renvoient `a < b`. La seule différence est `setl` contre `setb`, et cette différence sépare `int` de `unsigned`.

| | Signé | Non signé |
|---|---|---|
| Sauts après une comparaison | `jl`, `jle`, `jg`, `jge` | `jb`, `jbe`, `ja`, `jae` |
| Positionnement depuis une comparaison | `setl`, `setg`, ... | `setb`, `seta`, ... |
| Déplacement conditionnel | `cmovl`, `cmovge`, ... | `cmovb`, `cmovnb` (aussi `cmovae`), ... |
| Décalage à droite | `sar` | `shr` |
| Extension à 64 bits | `movsx` ou `movsxd` | `mov` vers un registre 32 bits |
| Division | `cdq` / `idiv` | `xor edx, edx` / `div` |

Les indicateurs portent les deux interprétations à la fois : l'indicateur de retenue signale un emprunt non signé, tandis que les indicateurs de signe et de débordement signalent une comparaison signée. Le saut ou le `setcc` qui suit choisit celle qui importe au programme. Comparez les deux façons d'étendre une valeur 32 bits à 64 bits :

```text
extend_s:
    movsx    rax, edi
    ret
```

```text
extend_u:
    mov    eax, edi
    ret
```

Pour l'`int` signé, `movsx rax, edi` recopie le bit de signe vers le haut. Pour l'`unsigned`, un simple `mov eax, edi` suffit, car sur x86-64 toute écriture dans un registre 32 bits met à zéro les 32 bits supérieurs du registre complet.

## Essayez

1. Exécutez `make` et ouvrez `build/14_O2.s`. Pour chaque fonction de `14_arith.c`, dites ce qu'elle calcule avant de la lire, puis vérifiez.
2. Exécutez `python3 14_magic.py` avec les constantes de `umod10` et `smod10`. Donnent-elles le même diviseur que `udiv10` et `sdiv10` ?
3. Écrivez dans un nouveau fichier une fonction C `int times9(int x)` et une fonction `unsigned udiv3(unsigned x)`, compilez avec `-O2 -S -masm=intel` et identifiez l'idiome de chacune.
4. Trouvez dans la sortie du compilateur le nombre magique pour diviser par 3, 5 et 100 (non signé) et retrouvez chaque diviseur avec le script.
5. Expliquez, sans compiler, pourquoi `x % 8` pour un `int` signé ne peut pas être un simple `and`.

## Pièges courants

- **Prendre `lea` pour un accès mémoire.** `lea` ne touche jamais la mémoire. Vérifiez si le résultat sert d'adresse ou de nombre.
- **Manquer le biais de signe.** Un `sar` seul n'est pas une division signée. Cherchez le biais ajouté, ou `cmovns`.
- **Oublier la multiplication 64 bits.** La multiplication magique a besoin des bits hauts du produit ; elle se fait donc dans un registre 64 bits même si les valeurs sont sur 32 bits.
- **Lire un décalage de 32 ou plus comme une erreur.** `shr rax, 32` extrait la moitié haute d'un produit 64 bits.
- **Appeler booléen tout résultat de `setcc`.** `sete al` peut alimenter une expression arithmétique, comme dans la fonction `sign`.

## Résumé

- `lea` est un additionneur et multiplicateur rapide par 2, 3, 4, 5, 8 et 9 qui ne touche ni aux indicateurs ni à la mémoire.
- La multiplication par une constante consiste en décalages, additions et `lea` ; un simple `imul reg, reg, constant` signale une constante qui se décompose mal.
- La division non signée par une puissance de deux est `shr` ; la division signée ajoute d'abord un biais. Les restes utilisent `and`, avec un biais pour les valeurs signées.
- La division par une constante est la multiplication par `2^k / d` arrondi par excès, puis un décalage ; le diviseur est `2^k` divisé par le nombre magique.
- `setcc`, `cmov` et `sbb` transforment des comparaisons en valeurs sans sauts.
- Le choix de `jl` ou `jb`, `sar` ou `shr`, `movsx` ou `mov`, et `idiv` ou `div` indique si les valeurs sont signées.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 15, la récupération des données.
- Aide-mémoire : [Idiomes du compilateur](../../cheatsheets/compiler-idioms.fr.md).
- Exercices qui utilisent ce module : [Exercice 13](../../exercises/13-magic-numbers/README.fr.md).
