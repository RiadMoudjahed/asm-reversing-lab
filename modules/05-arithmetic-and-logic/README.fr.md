<!-- source: README.md @ sha256:6ef34d637b50 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 05. Arithmétique et logique

Durée : environ 2,5 heures. Prérequis : [04. Déplacer des données et modes d'adressage](../04-moving-data/README.fr.md)

## Pourquoi c'est important

Les instructions arithmétiques font deux choses à la fois : elles calculent un résultat et le décrivent dans les indicateurs. Chaque comparaison, chaque contrôle de débordement et chaque saut conditionnel du module suivant dépendent de ces indicateurs. Quand vous lisez du code compilé, vous devez aussi reconnaître ce que calcule réellement une courte suite de décalages, d'additions et de masques.

## Notions

### Addition et soustraction

| Instruction | Effet | Indicateurs |
|---|---|---|
| `add a, b` | `a = a + b` | CF, ZF, SF, OF, PF, AF |
| `sub a, b` | `a = a - b` | CF (emprunt), ZF, SF, OF, PF, AF |
| `inc a`, `dec a` | `a = a + 1`, `a = a - 1` | Tous sauf CF, qui reste inchangé |
| `neg a` | `a = 0 - a` | CF vaut 1 sauf si `a` valait 0 |

Le processeur ne sait pas si vos nombres sont signés ou non. Il positionne les indicateurs des deux interprétations et vous lisez le bon : **CF** pour le débordement non signé (une retenue sortant d'une addition, un emprunt dans une soustraction) et **OF** pour le débordement signé. `add rax, 1` sur `0x7FFFFFFFFFFFFFFF` positionne OF, car le résultat signé passe du plus grand nombre positif au plus négatif, mais pas CF. La même instruction sur `0xFFFFFFFFFFFFFFFF` positionne CF et ZF, et pas OF.

### Chaînes de retenue

`adc a, b` ajoute `b` et l'indicateur de retenue ; `sbb a, b` soustrait `b` et l'emprunt. Elles permettent de construire une arithmétique plus large qu'un registre. Additionner deux nombres de 128 bits, c'est un `add` sur les moitiés basses suivi d'un `adc` sur les moitiés hautes :

```nasm-fragment
    mov     rax, [rsi]
    add     [rdi], rax                  ; low halves: may set CF
    mov     rax, [rsi + 8]
    adc     [rdi + 8], rax              ; high halves plus the carry
```

### Multiplication

| Forme | Signification |
|---|---|
| `mul b` | Non signée : `rdx:rax = rax * b`. Le résultat de 128 bits occupe deux registres. |
| `imul b` | Signée, même disposition que `mul`. |
| `imul a, b` | Signée, `a = a * b`, tronquée à la taille du registre. |
| `imul a, b, constant` | Signée, `a = b * constant`, tronquée. |

`mul` écrit `rdx` dans le cadre de son résultat. Si `rdx` contenait un pointeur dont vous avez encore besoin, sauvegardez-le d'abord ; l'exemple `mul_wide` le fait. Pour les formes tronquantes d'`imul`, CF et OF sont positionnés lorsque le résultat complet n'a pas tenu dans le registre.

### Division

`div b` et `idiv b` divisent la valeur de double largeur `rdx:rax` par `b`. Le quotient va dans `rax` et le reste dans `rdx`.

- Avant `idiv`, placez dans `rdx` l'extension de signe de `rax` avec `cqo` (`cdq` pour les valeurs de 32 bits). Sans cela, `rdx` contient n'importe quoi et le résultat est faux.
- Avant `div`, mettez `rdx` à zéro (`xor edx, edx`).
- Diviser par zéro, ou obtenir un quotient trop grand pour `rax`, provoque une faute du processeur que Linux remet au programme sous la forme du signal `SIGFPE`.

`idiv` arrondit le quotient vers zéro ; le reste prend donc le signe du dividende : `-17 / 5` donne un quotient de -3 et un reste de -2.

### Logique

| Instruction | Effet |
|---|---|
| `and a, b` | Conserve les bits qui valent 1 dans les deux. Sert à sélectionner des bits avec un masque. |
| `or a, b` | Positionne les bits qui valent 1 dans l'un ou l'autre. Sert à forcer des bits à 1. |
| `xor a, b` | Inverse les bits là où `b` vaut 1. `xor eax, eax` met `eax` à 0 en deux octets. |
| `not a` | Inverse chaque bit. Aucun indicateur ne change. |

`and`, `or` et `xor` mettent CF et OF à zéro et positionnent ZF, SF et PF d'après le résultat.

### Décalages et rotations

| Instruction | Effet | Utilisation |
|---|---|---|
| `shl a, n` | Décalage à gauche, rempli de 0 | Multiplier par 2^n |
| `shr a, n` | Décalage à droite, rempli de 0 (logique) | Diviser sans signe par 2^n |
| `sar a, n` | Décalage à droite en recopiant le bit de signe (arithmétique) | Diviser avec signe par 2^n, en arrondissant vers le bas |
| `rol a, n`, `ror a, n` | Rotation : les bits qui sortent d'un côté rentrent de l'autre | Échange d'octets, hachage |

Le compte `n` est une constante ou le registre `cl`. CF reçoit le dernier bit décalé hors du registre. `sar` arrondit vers moins l'infini alors qu'`idiv` arrondit vers zéro : `-17 >> 2` vaut donc -5, mais `-17 / 4` vaut -4.

### Arithmétique décimale

Le module 01 a présenté le BCD compacté, où `0x38` signifie le nombre 38. Additionner `0x38` et `0x45` en binaire donne `0x7D`, qui n'est pas un octet BCD valide. Les instructions d'ajustement décimal (`daa`, `das`, `aaa`, `aas`, `aam`, `aad`) corrigent le résultat après une addition, une soustraction, une multiplication ou une division. Elles n'existent qu'en code 32 bits. En mode 64 bits, NASM les rejette avec `error: instruction not supported in 64-bit mode` et les compilateurs ne les produisent jamais. [05_bcd32.asm](examples/05_bcd32.asm) additionne 38 et 45 avec `daa` et se termine avec `0x83`, qui est le BCD compacté de 83.

### Le point de vue de la rétro-ingénierie

Les compilateurs choisissent l'instruction la moins coûteuse pour chaque opération arithmétique. Exécutez `make compiled` dans le dossier des exemples :

```text
<times8>:        lea    rax,[rdi*8+0x0]
<div8_unsigned>: mov    rax,rdi
                 shr    rax,0x3
<shift_signed>:  mov    rax,rdi
                 sar    rax,0x2
<divide_vars>:   mov    rax,rdi
                 cqo
                 idiv   rsi
<remainder_vars>: mov    rax,rdi
                 cqo
                 idiv   rsi
                 mov    rax,rdx
```

Lues à l'envers : une multiplication par 8 n'utilise jamais `imul` ; c'est une échelle ou un `shl`. Un `shr` de 3 est une division non signée par 8, et un `sar` de 2 est une division signée par 4 pour un type dont vous savez qu'il est signé. Un `cqo` suivi d'un `idiv` est une division signée par une variable, et la même suite terminée par `mov rax, rdx` donne le reste. Le choix entre `shr` et `sar`, ou entre `div` et `idiv`, vous dit si le type d'origine était non signé ou signé. Le module 14 traite les réécritures plus élaborées que les compilateurs utilisent pour diviser par des constantes.

## À vous de jouer

Construisez les exemples et exécutez-les :

```bash
cd modules/05-arithmetic-and-logic/examples
make run
```

Sortie attendue :

```text
add_overflows(INT64_MAX, 1)      = 1  (sum = -9223372036854775808)
add_overflows(5, 7)              = 0
add_carries(UINT64_MAX, 1)       = 1
add_carries(INT64_MAX, 1)        = 0
mul_wide(2^32, 3*2^32)           = hi 3, lo 0
divide(-17, 5)                   = quotient -3, remainder -2
sar_by(-16, 2)                   = -4
shr_by(-16, 2)                   = 4611686018427387900
add128(2^64 - 1, 1)              = high 1, low 0
negate(10)                       = -10
combine_bits(0xAB)               = 0x50
05_bcd32 exit status: 131
```

Comparez chaque ligne avec la théorie. Les lignes 1 et 4 additionnent les mêmes opérandes : OF signale un débordement, CF non. `sar_by` et `shr_by` décalent les mêmes bits et donnent -4 et un énorme nombre positif. `05_bcd32` se termine avec 131, soit `0x83`.

Parcourez ensuite `add_overflows` pas à pas dans le débogueur et observez les indicateurs :

```bash
gdb -q build/05_main
```

```text
break add_overflows
run
stepi
p $eflags
```

Avant d'appuyer sur Entrée, prédisez quels indicateurs sont positionnés après le `add`.

## Pièges courants

- **Sauter `cqo` avant `idiv`.** `rdx` contient encore une ancienne valeur : la division utilise donc une moitié haute quelconque. Le résultat est faux, ou le processus meurt avec `SIGFPE`.
- **Perdre un pointeur dans `rdx`.** `mul` et `div` écrivent `rdx`. Sauvegardez d'abord dans un autre registre ce dont vous avez besoin.
- **Diviser par zéro.** `div` et `idiv` provoquent une faute avec un diviseur nul. Dans du code compilé, le contrôle se trouve généralement avant l'instruction.
- **`sar` là où `shr` est voulu, et inversement.** `sar` sur une valeur non signée conserve à 1 un bit de poids fort valant 1 ; `shr` sur un nombre négatif en fait un grand nombre positif.
- **Lire CF après `inc` ou `dec`.** Elles le laissent intact. Utilisez `add` ou `sub` lorsque vous avez besoin de la retenue.
- **Lire un indicateur après une instruction qui l'a écrasé.** Presque toute instruction arithmétique positionne plusieurs indicateurs. `mov` et `lea` n'en positionnent aucun, ce qui explique que `mov [rdx], rdi` puisse se trouver entre `add` et `seto`.
- **Utiliser des instructions d'ajustement BCD en code 64 bits.** Elles n'y existent pas.

## Résumé

- `add`, `sub` et leurs semblables calculent un résultat et positionnent CF (débordement non signé), OF (débordement signé), ZF et SF.
- `adc` et `sbb` enchaînent les opérations par l'indicateur de retenue pour construire une arithmétique plus large.
- `mul` et `div` travaillent sur `rdx:rax` ; fixez `rdx` avec `cqo` avant `idiv` et avec `xor` avant `div`.
- `and`, `or`, `xor` et `not` opèrent bit à bit ; les décalages multiplient et divisent par des puissances de deux, avec `sar` pour les valeurs signées.
- Les compilateurs choisissent `shr` ou `sar`, et `div` ou `idiv`, d'après le signe du type, ce que vous pouvez relire dans le code.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 06, conditions, branchements et boucles.
- Exercices qui utilisent ce module : [Exercice 03](../../exercises/03-register-ledger/README.fr.md), [Exercice 05](../../exercises/05-ticket-counter/README.fr.md), [Exercice 06](../../exercises/06-branch-maze/README.fr.md), [Exercice 13](../../exercises/13-magic-numbers/README.fr.md).
