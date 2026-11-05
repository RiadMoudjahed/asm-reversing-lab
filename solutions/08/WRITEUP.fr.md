<!-- source: WRITEUP.md @ sha256:8b7f8b149302 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 08. Carte de visite

Le drapeau est `ARL{calling_scfcmv}` : `f1` est stdcall, `f2` est cdecl, `f3` est fastcall, `f4` est cdecl, `g1` est Microsoft x64 et `g2` est System V AMD64. Chaque convention laisse une signature à deux endroits : la façon dont la fonction lit ses arguments et la façon dont la pile est équilibrée après l'appel.

## Partie 1 : Reverse

### Les fonctions 32 bits

```text
f1:  mov eax,[esp+4]       f2:  mov eax,[esp+0xc]
     lea eax,[eax+eax*2]        imul eax,eax,0x64
     mov edx,[esp+8]            mov edx,[esp+8]
     lea eax,[eax+edx*2]        lea edx,[edx+edx*4]
     add eax,[esp+0xc]          lea eax,[eax+edx*2]
     ret 0xc                    add eax,[esp+4]
                                ret

f3:  lea eax,[ecx+ecx*4]   f4:  mov eax,[esp+4]
     sub eax,edx                imul eax,[esp+8]
     add eax,[esp+4]            ret
     ret 4
```

| Fonction | Indice | Convention |
|---|---|---|
| `f1` | Trois arguments sur la pile en `[esp+4]`, `[esp+8]`, `[esp+0xc]` ; se termine par `ret 0xc`, donc l'appelé les retire ; l'appelant ne touche pas `esp` après l'appel | stdcall |
| `f2` | Trois arguments sur la pile ; `ret` simple ; l'appelant exécute `add esp,0xc` après l'appel | cdecl |
| `f3` | Lit `ecx` et `edx` sans les écrire d'abord ; un argument sur la pile en `[esp+4]` ; se termine par `ret 4` ; l'appelant charge `ecx` et `edx` et empile une valeur | fastcall |
| `f4` | Deux arguments sur la pile ; `ret` simple ; l'appelant exécute `add esp,8` après l'appel | cdecl |

Les appelants dans `_start` confirment chaque lecture. Après `call f1` il n'y a aucun ajustement de pile, après `call f2` il y a `add esp,0xc`, avant `call f3` un seul `push` est exécuté avec deux chargements de registres, et après `call f4` il y a `add esp,8`. Trois des fonctions ne prennent que des arguments sur la pile, donc l'emplacement des arguments ne suffit pas à séparer stdcall de cdecl ; le `ret` est l'instruction décisive.

Les formules, lues dans le code : `f1(a, b, c) = 3a + 2b + c`, `f2(a, b, c) = a + 10b + 100c` et `f3(a, b, c) = 5a - b + c`, avec `a` dans `ecx` et `b` dans `edx`.

### Les fonctions 64 bits

```text
g1:  mov [rsp+8],rcx        g2:  lea rax,[rsi+rsi]
     mov [rsp+0x10],rdx          add rax,rdi
     mov rax,[rsp+8]             lea rdx,[rdx+rdx*2]
     imul rax,rax,7              add rax,rdx
     add rax,[rsp+0x10]          ret
     add rax,r8
     ret
```

| Fonction | Indice | Convention |
|---|---|---|
| `g1` | Les arguments arrivent dans `rcx`, `rdx` et `r8` ; la fonction range `rcx` et `rdx` dans les 32 octets au-dessus de son adresse de retour (l'espace d'ombre réservé par l'appelant) ; l'appelant exécute `sub rsp,0x28` avant l'appel | Microsoft x64 |
| `g2` | Les arguments arrivent dans `rdi`, `rsi` et `rdx` ; pas d'espace d'ombre, pas d'ajustement de pile chez l'appelant | System V AMD64 |

Dans l'ordre `f1`, `f2`, `f3`, `f4`, `g1`, `g2`, les lettres sont `s`, `c`, `f`, `c`, `m`, `v`.

Le drapeau est `ARL{calling_scfcmv}`.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Les points qui comptent :

- **`cc_f1` (stdcall).** Les trois arguments sont en `[esp+4]`, `[esp+8]` et `[esp+12]`. Le résultat est `3a + 2b + c`, calculé avec `lea` et un `add`. La fonction se termine par `ret 12`.
- **`cc_f2` (cdecl).** Le résultat est `a + 10b + 100c`. La fonction se termine par un `ret` simple, car l'appelant retire les arguments.
- **`cc_f3` (fastcall).** `a` est dans `ecx` et `b` dans `edx` ; seul `c` est sur la pile, en `[esp+4]`. Le résultat est `5a - b + c`. La fonction se termine par `ret 4`.

Toute l'arithmétique déborde sur 32 bits, ce que `lea`, `add` et `imul` font sans code supplémentaire. Aucun registre à préserver par l'appelé n'est touché, donc rien à sauvegarder.

## Erreurs qui coûtent du temps

- **Utiliser `ret` là où un nombre est nécessaire.** Le résultat est correct, mais le pointeur de pile diffère après l'appel ; le vérificateur signale `stack` pour chaque cas.
- **Utiliser `ret N` dans la fonction cdecl.** L'appelant retire alors les arguments une seconde fois ; le pointeur de pile se déplace et le programme se termine en général par le signal 11.
- **Oublier `ret 4` dans la fonction fastcall.** L'argument empilé reste sur la pile ; le vérificateur signale `stack`.
- **Lire les arguments de fastcall sur la pile.** Seul `c` s'y trouve. Si `a` et `b` sont pris aux mauvais emplacements, 18 des 27 vecteurs de test échouent.
- **Intervertir `b` et `c` dans la formule cdecl.** Les poids 10 et 100 ne sont pas symétriques, donc seules des entrées avec `b` et `c` différents le révèlent.
