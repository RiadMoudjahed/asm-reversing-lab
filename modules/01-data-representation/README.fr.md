<!-- source: README.md @ sha256:dcf22bed5b5c -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 01. Représentation des données

Durée : environ 90 minutes. Prérequis : [00. Mise en place du laboratoire](../00-setup/README.fr.md)

## Pourquoi c'est important

Un processeur stocke tout sous forme de bits et ne sait pas si un octet est un nombre, une lettre ou une partie d'adresse. C'est vous qui décidez de ce que signifient les bits, tout comme chaque programme que vous analyserez. La plupart des erreurs de débutant en assembleur et en rétro-ingénierie (un signe faux, une valeur inversée, un caractère manquant) viennent d'une mauvaise lecture des mêmes bits.

## Notions

### Bits, octets et tailles

Un **bit** vaut 0 ou 1. Un **octet** fait 8 bits. Les unités plus grandes se comptent en octets :

| Nom | Octets | Bits | Plage non signée |
|---|---|---|---|
| octet | 1 | 8 | 0 à 255 |
| mot (word) | 2 | 16 | 0 à 65535 |
| double mot (dword) | 4 | 32 | 0 à 4294967295 |
| quadruple mot (qword) | 8 | 64 | 0 à 2^64 - 1 |

Ces noms viennent des premiers processeurs 16 bits et restent identiques en code 32 bits et 64 bits : un « mot » vaut donc toujours 2 octets sur x86.

### Binaire et hexadécimal

Le binaire est exact mais long. L'**hexadécimal** (base 16) écrit quatre bits sous la forme d'un chiffre, si bien qu'un octet fait exactement deux chiffres hexadécimaux. NASM écrit l'hexadécimal `0x1F` ou `1Fh` ; ce cours utilise `0x`.

Pour convertir, regroupez les chiffres binaires par quatre à partir de la droite :

```text
1111 0110  ->  F 6  ->  0xF6
```

### Entiers non signés et signés

Le motif de bits `0xF6` vaut 246 lorsqu'il est lu comme non signé. Les ordinateurs stockent les nombres négatifs en **complément à deux** : pour opposer un nombre, on inverse tous ses bits et on ajoute un.

```text
 10 = 0000 1010
~10 = 1111 0101
+1  = 1111 0110  = 0xF6  = -10
```

Le même octet `0xF6` vaut donc 246 non signé et -10 signé. Le processeur ne retient pas lequel vous aviez en tête ; ce sont les instructions que vous choisirez ensuite (comme les sauts signés par opposition aux sauts non signés du module 06) qui portent cette signification. Les plages signées sont asymétriques : les valeurs signées sur 8 bits vont de -128 à 127, et celles sur 32 bits de -2147483648 à 2147483647. Le bit de poids fort indique le signe.

### Extension de signe et extension par des zéros

Élargir une valeur à davantage de bits peut se faire de deux façons, et le choix compte :

- L'**extension par des zéros** remplit les nouveaux bits de poids fort avec 0. Elle convient aux valeurs non signées : `0xF6` (246) devient `0x000000F6` (246).
- L'**extension de signe** copie le bit de poids fort dans les nouveaux bits. Elle convient aux valeurs signées : `0xF6` (-10) devient `0xFFFFFFF6` (-10).

Utiliser la mauvaise change le nombre. Le module 04 présente les instructions `movzx` et `movsx`, qui réalisent chacune l'une d'elles.

### Boutisme

Une valeur de plusieurs octets doit être écrite en mémoire octet par octet. x86 est **petit-boutiste** (little-endian) : l'octet de poids faible va à l'adresse la plus basse. La valeur `0x11223344` est stockée ainsi :

```text
address:   +0  +1  +2  +3
bytes:     44  33  22  11
```

Quand vous lisez un vidage hexadécimal, inversez les octets pour obtenir la valeur : `44 33 22 11` est `0x11223344`. Les protocoles réseau utilisent souvent le **gros-boutisme** (big-endian, octet de poids fort en premier), vous rencontrerez donc les deux.

### Texte

Le texte, ce sont des octets qui suivent une convention. En **ASCII**, un octet est un caractère : `0x48` est `H`, `0x69` est `i`, `0x21` est `!`, `0x0A` est le saut de ligne, et `0x00` sert souvent de terminateur. **UTF-8** étend l'ASCII : les caractères inférieurs à 128 sont le même octet unique, les autres utilisent de deux à quatre octets. Un lecteur qui ne connaît que l'ASCII voit le texte UTF-8 correct pour l'anglais et, pour tout le reste, des octets inexpliqués au-dessus de `0x7F`.

### Chiffres décimaux dans des octets

Deux formats plus anciens stockent des chiffres décimaux et apparaissent encore dans certains formats de données :

- **BCD non compacté** : un chiffre décimal par octet. Le nombre 59 s'écrit `05 09`.
- **BCD compacté** : deux chiffres par octet, un par demi-octet. Le nombre 59 s'écrit `0x59`.

Lire `0x59` comme du binaire donne 89, pas 59 : il faut donc savoir de quel format il s'agit.

### Le point de vue de la rétro-ingénierie

Dans un vidage hexadécimal, vous voyez des octets, pas du sens. Les questions à poser à toute valeur trouvée sont : combien d'octets, signée ou non signée, quel boutisme, et est-ce du texte ? Une valeur de 4 octets `44 33 22 11` dans un programme petit-boutiste est le nombre `0x11223344` ; les mêmes octets lus comme du texte donnent `D3"` suivi d'un caractère de contrôle, ce qui serait une coïncidence. Le contexte, par exemple la façon dont le programme utilise les octets, tranche.

## À vous de jouer

Compilez et exécutez le programme d'exemple. Il se trouve dans [01_representation.c](examples/01_representation.c).

```bash
cd modules/01-data-representation/examples
make run
```

Sortie attendue :

```text
0xF6 as unsigned: 246
0xF6 as signed:   -10
0xF6 as binary:   11110110
-10 by hand:      -10 (0xF6)
sign-extended:    0xFFFFFFF6
zero-extended:    0x000000F6
0x11223344 in memory: 44 33 22 11
"Hi!" bytes:   48 69 21
```

Associez chaque ligne à une section ci-dessus. Vérifiez ensuite vous-même la règle du complément à deux avec Python :

```bash
python3 -c "print(hex(-10 & 0xFF))"
```

Cela affiche `0xf6`. Modifiez maintenant l'exemple : donnez à `word` la valeur `0xDEADBEEF`, recompilez et prédisez les quatre octets avant d'exécuter. La réponse se trouve dans la règle du boutisme ci-dessus.

## Pièges courants

- **Lire un vidage hexadécimal de gauche à droite comme une valeur.** Sur x86, le premier octet est celui de poids faible. Inversez-le.
- **Traiter signé et non signé comme des données différentes.** Ce sont les mêmes bits ; seule votre interprétation diffère.
- **Étendre avec la mauvaise règle.** Étendre par des zéros un nombre négatif en fait un grand nombre positif.
- **Confondre `0x59` et 59.** En BCD compacté ils sont égaux ; en binaire, non.
- **Supposer qu'un caractère est un octet.** Cela n'est vrai que pour l'ASCII.

## Résumé

- Un octet fait 8 bits, un mot 2 octets, un double mot 4 et un quadruple mot 8.
- Le complément à deux représente les nombres négatifs : inversez les bits et ajoutez un.
- L'extension par des zéros convient aux valeurs non signées ; l'extension de signe, aux valeurs signées.
- x86 est petit-boutiste : l'octet de poids faible se trouve à l'adresse la plus basse.
- Les mêmes bits peuvent être un nombre, du texte ou une adresse ; le contexte décide.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec [02. Le processeur et ses registres](../02-cpu-and-registers/README.fr.md).
- Exercices qui utilisent ce module : [Exercice 04](../../exercises/04-byte-order/README.fr.md), [Exercice 05](../../exercises/05-ticket-counter/README.fr.md).
