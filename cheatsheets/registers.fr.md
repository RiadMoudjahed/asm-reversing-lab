<!-- source: registers.md @ sha256:430608b4d7d1 -->
[English](registers.md) | [العربية](registers.ar.md) | [Deutsch](registers.de.md) | [Español](registers.es.md) | [Français](registers.fr.md)

# Registres x86 et x86-64

## Registres généraux

Écrire dans un registre 32 bits en mode 64 bits met à zéro les 32 bits supérieurs du registre complet. Écrire dans un registre 8 ou 16 bits laisse les autres bits inchangés.

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
| `r8` à `r15` | `r8d` à `r15d` | `r8w` à `r15w` | `r8b` à `r15b` | aucun |

Les registres `r8` à `r15` et les noms 8 bits `sil`, `dil`, `bpl` et `spl` n'existent qu'en mode 64 bits.

## Registres spéciaux

| Registre | Rôle |
|---|---|
| `rip` / `eip` | Adresse de l'instruction suivante. Modifié par les sauts, `call` et `ret`. En code 64 bits, il peut servir de base d'adressage, comme dans `[rel label]`. |
| `rsp` / `esp` | Pointeur de pile. Désigne l'élément empilé le plus récemment. |
| `rbp` / `ebp` | Pointeur de cadre par convention ; un registre général dans le code optimisé. |
| `rflags` / `eflags` | Indicateurs d'état et de contrôle (voir ci-dessous). |

## Indicateurs

| Indicateur | Nom | Positionné quand |
|---|---|---|
| `CF` | Retenue | une opération non signée a produit une retenue ou un emprunt |
| `ZF` | Zéro | le résultat est nul |
| `SF` | Signe | le bit de poids fort du résultat est à 1 |
| `OF` | Débordement | une opération signée a débordé |
| `PF` | Parité | l'octet de poids faible du résultat contient un nombre pair de bits à 1 |
| `AF` | Ajustement | une retenue ou un emprunt a franchi le bit 3 (utilisé par les instructions BCD) |
| `DF` | Direction | les instructions de chaîne reculent ; on le met à zéro avec `cld` |

## Qui doit préserver quoi

| Registre | System V AMD64 | Microsoft x64 | 32 bits cdecl, stdcall, fastcall |
|---|---|---|---|
| `rax` | Sauvegardé par l'appelant, résultat | Sauvegardé par l'appelant, résultat | Sauvegardé par l'appelant, résultat dans `eax` |
| `rbx` | Sauvegardé par l'appelé | Sauvegardé par l'appelé | Sauvegardé par l'appelé |
| `rcx`, `rdx` | Sauvegardé par l'appelant | Sauvegardé par l'appelant | Sauvegardé par l'appelant |
| `rsi`, `rdi` | Sauvegardé par l'appelant | Sauvegardé par l'appelé | Sauvegardé par l'appelé |
| `rbp` | Sauvegardé par l'appelé | Sauvegardé par l'appelé | Sauvegardé par l'appelé |
| `r8` à `r11` | Sauvegardé par l'appelant | Sauvegardé par l'appelant | non disponible |
| `r12` à `r15` | Sauvegardé par l'appelé | Sauvegardé par l'appelé | non disponible |

Les registres d'arguments figurent dans [Conventions d'appel](calling-conventions.fr.md).
