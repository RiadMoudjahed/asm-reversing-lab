<!-- source: README.md @ sha256:f458fd954ca9 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 02. Le processeur et ses registres

Durée : environ 90 minutes. Prérequis : [01. Représentation des données](../01-data-representation/README.fr.md)

## Pourquoi c'est important

Les registres sont l'endroit où un processeur fait son travail, et les indicateurs sont le moyen par lequel une instruction transmet une information à la suivante. Chaque vue du débogueur, chaque désassemblage et chaque exercice de ce cours s'exprime en leurs termes. Si vous savez lire un tableau de registres d'un coup d'œil, vous pouvez suivre ce que fait n'importe quel code.

## Notions

### Le cycle de recherche, décodage et exécution

Un processeur répète trois étapes. Il **recherche** l'instruction à l'adresse contenue dans le pointeur d'instruction, la **décode** et l'**exécute**. Puis le pointeur d'instruction passe à l'instruction suivante, sauf si l'instruction était un saut, un appel ou un retour qui l'a placé ailleurs.

### Registres généraux

Le mode 64 bits dispose de seize registres généraux de 64 bits chacun. Huit d'entre eux portent des noms hérités de processeurs plus anciens, et chacun peut s'utiliser en quatre largeurs :

| 64 bits | 32 bits | 16 bits | 8 bits bas | 8 bits haut |
|---|---|---|---|---|
| `rax` | `eax` | `ax` | `al` | `ah` |
| `rbx` | `ebx` | `bx` | `bl` | `bh` |
| `rcx` | `ecx` | `cx` | `cl` | `ch` |
| `rdx` | `edx` | `dx` | `dl` | `dh` |
| `rsi` | `esi` | `si` | `sil` | aucun |
| `rdi` | `edi` | `di` | `dil` | aucun |
| `rbp` | `ebp` | `bp` | `bpl` | aucun |
| `rsp` | `esp` | `sp` | `spl` | aucun |

Les huit autres sont numérotés : de `r8` à `r15`. Leurs vues plus étroites sont `r8d` (32 bits), `r8w` (16 bits) et `r8b` (8 bits). En code 32 bits, seuls les huit premiers existent, en 32, 16 et 8 bits.

Certains registres ont des rôles conventionnels, que les modules suivants expliquent : `rsp` est le pointeur de pile, `rbp` est souvent le pointeur de cadre, et `rax` porte les résultats de fonctions et les numéros d'appel système.

### Écrire dans une vue étroite

Les noms étroits sont des fenêtres sur un seul registre, pas des registres distincts. Ce qui arrive aux autres bits dépend de la largeur dans laquelle vous écrivez :

- Écrire dans une vue **8 bits ou 16 bits** ne modifie que ces bits. Le reste est conservé.
- Écrire dans une vue **32 bits** modifie les 32 bits bas et **met à zéro les 32 bits hauts**.

Vous pouvez l'observer dans [02_registers.asm](examples/02_registers.asm). La valeur de `rax` après chaque instruction, lue dans le débogueur :

| Instruction | `rax` ensuite |
|---|---|
| `mov rax, 0x1122334455667788` | `0x1122334455667788` |
| `mov al, 0xFF` | `0x11223344556677ff` |
| `mov ah, 0xEE` | `0x112233445566eeff` |
| `mov ax, 0xDDCC` | `0x112233445566ddcc` |
| `mov eax, 0xAABBCCDD` | `0x00000000aabbccdd` |
| `mov rax, -1` | `0xffffffffffffffff` |
| `mov eax, eax` | `0x00000000ffffffff` |

La dernière ligne est une expression délibérée : `mov eax, eax` étend `eax` par des zéros jusqu'à `rax`. Vous la rencontrerez dans du code compilé, où elle signifie « traiter cette valeur 32 bits comme un 64 bits non signé ».

### Le pointeur d'instruction

`rip` contient l'adresse de la prochaine instruction à exécuter. On n'y écrit pas avec `mov` ; ce sont les sauts, les appels et les retours qui le modifient. En code 32 bits, c'est `eip`.

### Le registre d'indicateurs

`rflags` (ou `eflags`) contient des indicateurs d'un bit que le processeur positionne comme effet de bord de l'arithmétique et des comparaisons. Les instructions suivantes les lisent. Ceux dont vous avez d'abord besoin :

| Indicateur | Nom | Positionné quand |
|---|---|---|
| CF | retenue (carry) | Une opération non signée a débordé ou emprunté |
| ZF | zéro (zero) | Le résultat est nul |
| SF | signe (sign) | Le bit de poids fort du résultat vaut 1 (négatif si signé) |
| OF | débordement (overflow) | Une opération signée a débordé |
| PF | parité (parity) | L'octet de poids faible du résultat a un nombre pair de bits à 1 |
| AF | ajustement (adjust) | Une retenue est passée du bit 3 au bit 4 (utilisé par l'arithmétique décimale) |
| DF | direction (direction) | Contrôle le sens des instructions de chaîne (module 07) |

CF et OF répondent à la même question pour des interprétations différentes. Additionner 255 et 1 dans un octet donne 0 et positionne CF, un débordement non signé ; lu comme signé, c'est -1 + 1 = 0, donc OF reste à zéro. Additionner 127 et 1 donne `0x80` : c'est 128 non signé sans retenue (CF=0), mais -128 signé, donc OF est positionné.

Vous pouvez voir chaque cas dans [02_flags.asm](examples/02_flags.asm). Le débogueur affiche les indicateurs après chaque paire d'étapes :

| Instructions | Indicateurs positionnés ensuite |
|---|---|
| `mov al, 255` puis `add al, 1` | CF, PF, AF, ZF |
| `mov al, 127` puis `add al, 1` | AF, SF, OF |
| `mov al, 5` puis `sub al, 5` | PF, ZF |
| `mov al, 3` puis `cmp al, 5` | CF, AF, SF |

`IF` apparaît aussi à chaque ligne ; c'est l'indicateur d'interruption, que vous pouvez ignorer. Notez que `cmp al, 5` calcule 3 - 5 et jette le résultat en ne gardant que les indicateurs : CF signale que la soustraction a emprunté (3 est inférieur à 5 en non signé) et SF signale un résultat négatif.

### Registres de segment

`cs`, `ss`, `ds`, `es`, `fs` et `gs` viennent de la mémoire segmentée des processeurs 16 bits. Dans les programmes modernes à mémoire plate, les quatre premiers n'ont pas de rôle pratique. Sous Linux et Windows, `fs` ou `gs` pointe vers des données propres à chaque thread, ce que vous verrez dans certains désassemblages.

### Le point de vue de la rétro-ingénierie

Un débogueur affiche tout cet ensemble de registres après chaque pas : le lire vite est donc la compétence essentielle. GEF affiche les noms des indicateurs, les majuscules signifiant positionné, par exemple `[ZERO carry PARITY adjust sign ...]` après `xor edi, edi`. Quand vous voyez `[... CARRY ...]` après un `cmp`, l'opérande de gauche était inférieur à celui de droite en non signé.

## À vous de jouer

Construisez les exemples et parcourez-les pas à pas dans le débogueur :

```bash
cd modules/02-cpu-and-registers/examples
make
gdb -q build/02_registers
```

À l'invite, tapez ces commandes, en répétant `stepi` et `p/x $rax` pour chaque instruction :

```text
break _start
run
stepi
p/x $rax
```

Comparez chaque valeur avec le tableau ci-dessus. Faites ensuite de même pour les indicateurs dans `build/02_flags` :

```text
break _start
run
stepi 2
p $eflags
```

Prédisez les indicateurs de chaque paire d'instructions avant d'appuyer sur Entrée, puis vérifiez.

## Pièges courants

- **S'attendre à ce qu'une écriture 32 bits conserve la moitié haute.** Elle la met à zéro. Une écriture 8 bits ou 16 bits la conserve.
- **Lire CF comme un « débordement » pour des nombres signés.** Le débordement signé, c'est OF.
- **Utiliser `ah` avec `sil`, `dil`, `bpl` ou `spl`.** On ne peut pas combiner les registres d'octet haut avec des instructions qui nécessitent les registres d'octet plus récents.
- **Supposer qu'un indicateur survit.** Presque toute instruction arithmétique réécrit plusieurs indicateurs. Lisez un indicateur immédiatement après l'instruction qui le positionne.

## Résumé

- Seize registres de 64 bits ; `rax` contient `eax`, qui contient `ax`, qui contient `al` et `ah`.
- Les écritures 8 bits et 16 bits conservent les autres bits ; les écritures 32 bits mettent à zéro les 32 bits hauts.
- `rip` est la prochaine instruction ; les sauts, les appels et les retours le modifient.
- CF signale le débordement non signé, OF le débordement signé, ZF le zéro, SF le signe.
- Un débogueur montre tout cela après chaque pas.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec [03. Votre premier programme](../03-first-program/README.fr.md).
- Exercices qui utilisent ce module : [Exercice 02](../../exercises/02-two-worlds/README.fr.md), [Exercice 03](../../exercises/03-register-ledger/README.fr.md).
