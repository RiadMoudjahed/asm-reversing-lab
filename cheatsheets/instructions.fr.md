<!-- source: instructions.md @ sha256:34a18bb7f3bc -->
[English](instructions.md) | [العربية](instructions.ar.md) | [Deutsch](instructions.de.md) | [Español](instructions.es.md) | [Français](instructions.fr.md)

# Instructions courantes

Syntaxe Intel : la destination vient en premier. Colonne des indicateurs : quels indicateurs l'instruction modifie.

## Déplacement de données

| Instruction | Effet | Indicateurs |
|---|---|---|
| `mov d, s` | Copie `s` dans `d` | aucun |
| `movzx d, s` | Copie avec extension par zéros | aucun |
| `movsx d, s` | Copie avec extension de signe | aucun |
| `lea d, [m]` | Charge l'adresse `m`, sans lire la mémoire | aucun |
| `xchg a, b` | Échange `a` et `b` | aucun |
| `push s` | Diminue `rsp` et range `s` | aucun |
| `pop d` | Charge `d` depuis la pile et augmente `rsp` | aucun |

## Arithmétique et logique

| Instruction | Effet | Indicateurs |
|---|---|---|
| `add d, s` / `sub d, s` | Addition et soustraction | tous les indicateurs arithmétiques |
| `adc d, s` / `sbb d, s` | Addition ou soustraction avec retenue | tous les indicateurs arithmétiques |
| `inc d` / `dec d` | Ajoute ou retranche 1 | tous sauf `CF` |
| `neg d` | Négation en complément à deux | tous les indicateurs arithmétiques |
| `imul d, s` | Multiplication signée, tronquée | `CF`, `OF` |
| `mul s` / `div s` | Multiplication ou division non signée avec `rax` et `rdx` | variable |
| `idiv s` | Division signée avec `rax` et `rdx` | indéfinis |
| `and`, `or`, `xor`, `not` | Opérations bit à bit (`not` ne modifie pas les indicateurs) | `ZF`, `SF`, `PF` ; `CF` et `OF` à zéro |
| `shl`, `shr`, `sar` | Décalage à gauche, à droite logique, à droite arithmétique | `CF`, `ZF`, `SF`, `PF` |
| `rol`, `ror` | Rotation | `CF`, `OF` |
| `cmp a, b` | Calcule `a - b`, ne garde que les indicateurs | tous les indicateurs arithmétiques |
| `test a, b` | Calcule `a & b`, ne garde que les indicateurs | `ZF`, `SF`, `PF` |

## Contrôle de flux

| Instruction | Effet |
|---|---|
| `jmp t` | Saute vers `t` |
| `je` / `jne` | Saute si égal / différent (`ZF`) |
| `ja` / `jb` / `jae` / `jbe` | Saute si supérieur / inférieur, non signé (`CF`, `ZF`) |
| `jg` / `jl` / `jge` / `jle` | Saute si supérieur / inférieur, signé (`SF`, `OF`, `ZF`) |
| `js` / `jns` | Saute si le signe est à 1 / à 0 |
| `call t` | Empile l'adresse de retour, saute vers `t` |
| `ret` / `ret N` | Dépile l'adresse de retour dans `rip` ; `ret N` retire aussi `N` octets |
| `loop t` | Diminue `rcx`, saute si non nul |

## Instructions de chaîne

| Instruction | Effet |
|---|---|
| `movs`, `stos`, `lods` | Copie, range `al`/`ax`/`eax`/`rax`, charge dans celui-ci |
| `cmps`, `scas` | Compare deux opérandes mémoire, compare la mémoire avec `al`/`ax`/`eax`/`rax` |
| `rep`, `repe`, `repne` | Répète tant que `rcx` est non nul (et que `ZF` est à 1 ou à 0, pour `repe` et `repne`) |
| `cld` / `std` | Met à 0 ou à 1 l'indicateur de direction |

## Système

| Instruction | Effet |
|---|---|
| `syscall` | Appel système 64 bits |
| `int 0x80` | Appel système Linux 32 bits |
| `nop` | Ne fait rien |
| `leave` | `mov rsp, rbp` puis `pop rbp` |
