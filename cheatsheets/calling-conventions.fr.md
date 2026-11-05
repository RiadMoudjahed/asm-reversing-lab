<!-- source: calling-conventions.md @ sha256:5e066f9fba0a -->
[English](calling-conventions.fr.md) | [العربية](calling-conventions.ar.md) | [Deutsch](calling-conventions.de.md) | [Español](calling-conventions.es.md) | [Français](calling-conventions.fr.md)

# Conventions d'appel

## 32 bits

| | cdecl | stdcall | fastcall |
|---|---|---|---|
| Arguments | Pile, de droite à gauche | Pile, de droite à gauche | Les deux premiers entiers dans `ecx` et `edx`, le reste sur la pile |
| Qui retire les arguments de la pile | Appelant | Appelé, avec `ret N` | Appelé, avec `ret N` |
| Résultat | `eax` | `eax` | `eax` |
| Sauvegardé par l'appelé | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` |

## 64 bits

| | System V AMD64 (Linux, macOS) | Microsoft x64 (Windows) |
|---|---|---|
| Arguments entiers et pointeurs | `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9` | `rcx`, `rdx`, `r8`, `r9` |
| Arguments à virgule flottante | `xmm0` à `xmm7` | `xmm0` à `xmm3` |
| Arguments supplémentaires | Pile | Pile, au-dessus de l'espace d'ombre |
| Résultat | `rax` (`xmm0` pour la virgule flottante) | `rax` (`xmm0` pour la virgule flottante) |
| Espace d'ombre | Aucune | 32 octets réservés par l'appelant |
| Zone rouge | 128 octets sous `rsp` pour les fonctions feuilles | Aucune |
| Appels variadiques | `al` contient le nombre de registres vectoriels utilisés | Les arguments à virgule flottante sont aussi copiés dans des registres entiers |
| Sauvegardé par l'appelé | `rbx`, `rbp`, `r12` à `r15` | `rbx`, `rbp`, `rdi`, `rsi`, `r12` à `r15`, `xmm6` à `xmm15` |

## Alignement de la pile

En code 64 bits, `rsp` doit être un multiple de 16 au moment de chaque `call`. À la première instruction d'une fonction, il vaut 8 modulo 16, car l'adresse de retour vient d'être empilée.

## Reconnaître une convention dans un binaire

- Un `ret` suivi d'un nombre : stdcall ou fastcall.
- `ecx` ou `edx` lus avant d'être écrits, en code 32 bits : fastcall.
- Un appelant qui exécute `add esp, N` après un appel : cdecl.
- `rdi` et `rsi` lus en premier : System V. `rcx` et `rdx` lus en premier, avec des écritures dans `[rsp + 8]` et au-dessus : Microsoft x64.
- Un appelant qui exécute `sub rsp, 0x28` (ou un multiple de 8 plus grand qui conserve l'alignement) avant un appel : espace d'ombre de Microsoft x64.
