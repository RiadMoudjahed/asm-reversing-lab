<!-- source: WRITEUP.md @ sha256:019af3edf455 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 07. Balayeur de chaînes

Le drapeau est `ARL{sweeper_2_5_1_3_6_4}` : `fn2` est `strlen`, `fn5` est `memchr`, `fn1` est `memcpy`, `fn3` est `memmove`, `fn6` est `memset` et `fn4` est `memcmp`. Chaque routine s'identifie par son instruction de chaîne, son préfixe et les registres qu'elle prépare avant.

## Partie 1 : Reverse

### Les six routines

```text
fn1:  mov rax,rdi            fn2:  xor eax,eax
      mov rcx,rdx                  mov rcx,-1
      shr rcx,3                    repnz scas al,[rdi]
      rep movs QWORD               not rcx
      mov ecx,edx                  lea rax,[rcx-1]
      and ecx,7                    ret
      rep movs BYTE
      ret

fn3:  mov rax,rdi            fn4:  mov rcx,rdx
      mov rcx,rdx                  xor eax,eax
      cmp rdi,rsi                  repz cmps [rsi],[rdi]
      jbe .forward                 je .same
      lea r8,[rsi+rdx]             movzx eax,[rdi-1]
      cmp rdi,r8                   movzx ecx,[rsi-1]
      jae .forward                 sub eax,ecx
      std                          ret
      lea rsi,[rsi+rdx-1]
      lea rdi,[rdi+rdx-1]    fn5:  mov eax,esi
      rep movs BYTE                mov rcx,rdx
      cld                          test rcx,rcx
      ret                          je .none
.forward: rep movs BYTE            repnz scas al,[rdi]
      ret                          jne .none
                                   lea rax,[rdi-1]
fn6:  mov r8,rdi                   ret
      movzx eax,sil
      mov rcx,rdx
      rep stos BYTE
      mov rax,r8
      ret
```

### Identifier chacune d'elles

| Routine | Indice | Fonction |
|---|---|---|
| `fn1` | `rep movs` copie `rcx` quadruples mots puis les octets restants ; aucune gestion de la direction ; renvoie `rdi` | `memcpy` |
| `fn2` | `al` est nul et `rcx` vaut -1 ; `repnz scas` balaye jusqu'à l'octet nul ; `not rcx` et la soustraction de un transforment le compte en longueur | `strlen` |
| `fn3` | Compare les adresses et, lorsque la destination est au-dessus de la source et la chevauche, exécute `std`, copie depuis le dernier octet et rétablit `cld` | `memmove` |
| `fn4` | `repz cmps` s'arrête à la première différence ; les deux octets différents sont soustraits pour donner le résultat | `memcmp` |
| `fn5` | `al` vient du deuxième argument et `rcx` du troisième ; `repnz scas` s'arrête sur une correspondance ; renvoie `rdi - 1` ou zéro | `memchr` |
| `fn6` | `al` prend l'octet du deuxième argument et `rep stos` le range `rcx` fois | `memset` |

Les deux routines qui utilisent `repnz scas` diffèrent par ce avec quoi elles comparent et par ce qu'elles renvoient : `fn2` cherche un zéro et renvoie un compte, `fn5` cherche un octet donné par l'appelant et renvoie un pointeur. Les deux copies diffèrent par la gestion de la direction : seule `fn3` a `std` et `cld`. Dans l'ordre strlen, memchr, memcpy, memmove, memset, memcmp, les numéros sont 2, 5, 1, 3, 6 et 4.

Le drapeau est `ARL{sweeper_2_5_1_3_6_4}`.

## Partie 2 : Assemble

La solution de référence est `solution.asm`, avec les mêmes formes que `fn2`, `fn5`, `fn1` (simplifiée en copie octet par octet) et `fn3`. Les points qui comptent :

- **`my_strlen`.** `al` vaut 0, `rcx` vaut -1, `repnz scasb`, puis `not rcx` et soustraire un.
- **`my_memchr`.** Vérifiez si `n` vaut 0 avant le balayage, car avec `rcx` à 0 le balayage ne fait rien et l'indicateur de zéro garde la valeur qu'il avait. Après le balayage, l'indicateur de zéro dit si une correspondance a été trouvée.
- **`my_memcpy`.** Sauvegardez `rdi` pour la valeur de retour, mettez `n` dans `rcx` et exécutez `rep movsb`.
- **`my_memmove`.** Une copie vers l'avant est sûre lorsque la destination n'est pas au-dessus de la source, ou lorsqu'elle commence à la fin de la source ou après. Sinon, copiez vers l'arrière avec `std` et mettez l'indicateur à zéro avant de revenir.

## Erreurs qui font perdre du temps

- **Toujours copier vers l'avant dans `my_memmove`.** Les cas de chevauchement sortent faux ; 8 des 74 cas de test échouent.
- **Laisser l'indicateur de direction positionné.** Le vérificateur signale `BAD direction flag left set`. La prochaine instruction de chaîne de l'appelant s'exécuterait à l'envers.
- **Copier trop d'octets.** Écrire au-delà de la fin du bloc heurte la page inaccessible et le test plante avec le signal 11.
- **Omettre le contrôle de `n` égal à 0 dans `my_memchr`.** Le test appelle la routine avec l'indicateur de zéro positionné et `n` égal à 0 ; une routine qui se fie à l'ancien indicateur renvoie un mauvais pointeur et échoue sur ce cas.
- **Renvoyer le pointeur un octet trop loin.** Après une correspondance, `rdi` se trouve déjà un octet après l'octet trouvé ; 11 des 74 cas échouent.
- **Compter le terminateur dans `my_strlen`.** Le résultat est trop grand d'une unité ; 16 cas échouent, à commencer par la chaîne vide.
- **Appeler une autre routine.** `call` est interdit ; gardez chaque routine autonome.
