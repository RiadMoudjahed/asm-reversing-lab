<!-- source: README.md @ sha256:6b81fa35936a -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 08. La pile, les procédures et les conventions d'appel

Durée : environ 3 heures. Prérequis : [07. Chaînes et instructions de chaîne](../07-strings/README.fr.md)

## Pourquoi c'est important

Presque tout programme est construit à partir de fonctions qui s'appellent les unes les autres, et la pile est l'endroit où vit cette structure : adresses de retour, arguments, variables locales et registres sauvegardés. Une convention d'appel est l'accord qui permet à du code écrit séparément de fonctionner ensemble. Quand vous faites de la rétro-ingénierie d'un binaire, reconnaître la convention vous dit où se trouve chaque argument et qui nettoie, et reconnaître un cadre de pile vous dit la taille des variables d'une fonction. Quand vous écrivez de l'assembleur qui dialogue avec du C, se tromper de convention est la cause de plantage la plus courante.

## Notions

### La pile

La pile est une région de mémoire adressée par `rsp` (`esp` en code 32 bits). Elle croît vers les adresses basses : empiler une valeur fait descendre `rsp`. Quatre instructions sont définies à partir d'elle :

| Instruction | Équivalent |
|---|---|
| `push x` | `sub rsp, 8` puis ranger `x` à `[rsp]` |
| `pop x` | Charger `x` depuis `[rsp]` puis `add rsp, 8` |
| `call target` | Empiler l'adresse de l'instruction suivante, puis sauter à `target` |
| `ret` | Dépiler une adresse dans `rip` |

`ret` fait confiance à ce qui se trouve au sommet de la pile. Si une fonction laisse `rsp` pointer ailleurs que sur son adresse de retour, elle saute au mauvais endroit.

### Cadres de pile

Une fonction qui a besoin de variables locales leur réserve de la place sur la pile. La méthode classique garde un second registre, `rbp`, comme point de repère fixe :

```nasm-fragment
sum_locals:
    push    rbp                         ; save the caller's frame pointer
    mov     rbp, rsp                    ; this function's frame pointer
    sub     rsp, 32                     ; room for four 8-byte locals
    mov     [rbp - 8], rdi              ; locals live below rbp
    ; ...
    leave                               ; mov rsp, rbp ; pop rbp
    ret
```

Le `rbp` sauvegardé et l'adresse de retour se trouvent à `[rbp]` et `[rbp + 8]`. Les variables locales ont des décalages négatifs par rapport à `rbp`, et les arguments passés sur la pile des décalages positifs. Avec l'optimisation, les compilateurs abandonnent souvent le pointeur de cadre et adressent tout par rapport à `rsp`. Les deux formes sont des versions complètes de la même idée, et [08_frames.asm](examples/08_frames.asm) contient des exemples qui fonctionnent.

### Conventions d'appel

Une convention décide où vont les arguments, où revient le résultat, qui retire les arguments de la pile et quels registres une fonction doit laisser inchangés. Voici les cinq que vous rencontrerez :

| | cdecl | stdcall | fastcall | System V AMD64 | Microsoft x64 |
|---|---|---|---|---|---|
| Bits | 32 | 32 | 32 | 64 | 64 |
| Système habituel | Linux, Windows | API Windows | Windows | Linux, macOS, BSD | Windows |
| Arguments entiers | Pile, de droite à gauche | Pile, de droite à gauche | `ecx`, `edx`, puis la pile | `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9`, puis la pile | `rcx`, `rdx`, `r8`, `r9`, puis la pile |
| Qui retire les arguments de la pile | L'appelant | L'appelé (`ret N`) | L'appelé (`ret N`) | L'appelant | L'appelant |
| Résultat | `eax` | `eax` | `eax` | `rax` | `rax` |
| Préservés par l'appelé | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` | `ebx`, `esi`, `edi`, `ebp` | `rbx`, `rbp`, `r12` à `r15` | `rbx`, `rbp`, `rdi`, `rsi`, `r12` à `r15` |
| Règles supplémentaires | Aucune | Aucune | Aucune | Zone rouge de 128 octets | 32 octets d'espace d'ombre |

Un registre que l'appelé doit préserver est **sauvegardé par l'appelé (callee-saved)** : une fonction qui veut l'utiliser sauvegarde d'abord l'ancienne valeur et la restaure avant de revenir. Les autres sont **sauvegardés par l'appelant (caller-saved)** : une fonction peut les écraser librement, et un appelant qui a besoin d'une valeur à travers un appel doit donc la garder ailleurs. Les valeurs de retour en virgule flottante vont dans `xmm0`, et les arguments en virgule flottante utilisent des registres `xmm` dans les conventions 64 bits.

### Les trois conventions 32 bits

[08_conv32.asm](examples/08_conv32.asm) écrit une même fonction de trois façons. Les seules différences visibles sont l'endroit où les arguments sont lus et la manière dont la fonction revient :

```nasm-fragment
add3_cdecl:     ; a, b, c at [esp+4], [esp+8], [esp+12]
    ; ...
    ret                                 ; the caller removes the arguments

add3_stdcall:   ; the same locations
    ; ...
    ret     12                          ; the callee removes 12 bytes

add3_fastcall:  ; a in ecx, b in edx, c at [esp+4]
    ; ...
    ret     4                           ; the callee removes its 4 bytes
```

`ret N` trahit stdcall et fastcall : elle revient puis ajoute `N` à `esp`. Si l'appelant et l'appelé ne s'accordent pas sur qui nettoie, `esp` dérive après chaque appel.

### Les deux conventions 64 bits

**System V** (Linux, macOS, BSD) passe les six premiers arguments entiers dans `rdi`, `rsi`, `rdx`, `rcx`, `r8` et `r9`. Les arguments suivants sont sur la pile : à la première instruction de la fonction, le septième se trouve à `[rsp + 8]` (juste au-dessus de l'adresse de retour) et le huitième à `[rsp + 16]`. Une fonction feuille (qui ne fait aucun appel) peut aussi utiliser la **zone rouge (red zone)**, les 128 octets sous `rsp`, sans ajuster `rsp`.

**Microsoft x64** passe les quatre premiers dans `rcx`, `rdx`, `r8` et `r9`. L'appelant doit toujours réserver **32 octets d'espace d'ombre (shadow space)** au-dessus de l'adresse de retour, même pour des fonctions à moins d'arguments, et l'appelé peut s'en servir pour ranger les arguments passés par registre. Il n'y a pas de zone rouge. [08_win64.asm](examples/08_win64.asm) montre une fonction et un appelant qui réserve l'espace d'ombre.

### Alignement de la pile

Les deux conventions 64 bits exigent que `rsp` soit un multiple de 16 au moment d'un `call`. Comme l'appel lui-même empile 8 octets, à la première instruction d'une fonction `rsp + 8` est un multiple de 16. Dans une fonction qui fait des appels, vous maintenez cela en faisant en sorte que la quantité totale soustraite soit un multiple impair de 8 : un seul `push` (comme le fait `factorial` avec `rbx`) ou un `sub rsp, 8` (comme le fait `length_of`) rétablit l'alignement. La raison est que certaines instructions, comme les écritures SSE alignées que le code des bibliothèques utilise pour sauvegarder les registres à virgule flottante, provoquent une faute sur une adresse mal alignée.

### Appeler du C depuis l'assembleur, et inversement

Pour appeler une fonction de la bibliothèque C, déclarez-la avec `extern`, placez les arguments dans les registres de la convention, gardez `rsp` aligné, puis appelez-la avec `call`. Pour une fonction variadique comme `printf`, `al` doit contenir le nombre de registres vectoriels utilisés par les arguments ; mettez-le à 0 avec `xor eax, eax` quand il n'y en a aucun. [08_printf.asm](examples/08_printf.asm) est un programme complet :

```nasm-fragment
main:
    sub     rsp, 8                      ; realign: main was entered with rsp + 8 aligned
    lea     rdi, [fmt]
    mov     esi, 42
    xor     eax, eax                    ; no vector registers used
    call    printf wrt ..plt
```

Dans l'autre sens, une fonction que C doit appeler est marquée `global`, et C la déclare avec un prototype ordinaire. Le Makefile lie les fichiers objets assemblés au programme C avec `gcc`.

### Macros et plusieurs fichiers source

Les macros NASM écrivent à votre place du code répétitif. [08_frame.inc](examples/08_frame.inc) définit `FUNC_ENTER` et `FUNC_LEAVE`, et [08_macros.asm](examples/08_macros.asm) inclut le fichier avec `%include` et les utilise. Chaque fichier `.asm` est assemblé séparément en un fichier objet, et l'éditeur de liens les combine : `global` exporte un symbole et `extern` en importe un.

### Le point de vue de la rétro-ingénierie

Exécutez `make compiled` dans le dossier des exemples pour comparer la même fonction compilée sans et avec optimisation. Ce sont de vrais extraits :

```text
== gcc -O0
<with_locals>:
    push   rbp
    mov    rbp,rsp
    sub    rsp,0x40
    mov    QWORD PTR [rbp-0x38],rdi          ; arguments are stored in the frame
    mov    QWORD PTR [rbp-0x40],rsi
    mov    rax,QWORD PTR fs:0x28             ; stack protector: a canary value
    mov    QWORD PTR [rbp-0x8],rax
    ...
    call   <helper>
    ...
    sub    rdx,QWORD PTR fs:0x28             ; the canary is checked before returning
    leave
    ret

== gcc -O2
<with_locals>:
    lea    rax,[rdi+rsi*1]
    push   rbx                               ; a callee-saved register is used, so it is saved
    mov    rbx,rsi
    imul   rdi,rsi
    add    rdi,rax
    call   <helper>
    add    rax,rbx
    pop    rbx                               ; and restored
    ret
```

Ce qu'on peut en lire :

- **Prologue et épilogue.** `push rbp` et `mov rbp, rsp` au début, `leave` et `ret` à la fin, signalent un cadre classique. Le nombre de `sub rsp, N` est la taille des variables locales plus le remplissage.
- **Pas de pointeur de cadre.** Dans le code optimisé, le cadre disparaît. Il reste un `push` d'un registre sauvegardé par l'appelé au début et le `pop` correspondant avant `ret`. Le tableau de quatre variables locales disparaît entièrement, gardé dans des registres.
- **Un canari.** Un chargement depuis `fs:0x28` au début et une comparaison avec lui avant l'épilogue constituent une protection de pile. C'est un dispositif de sécurité, pas une partie de la logique du programme.
- **La convention.** Des arguments lus dans `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9` signifient System V. Des lectures de `rcx`, `rdx`, `r8`, `r9` suivies d'écritures dans `[rsp + 8]`, `[rsp + 16]` et ainsi de suite (espace d'ombre) signifient Microsoft x64. `ret N` signifie stdcall ou fastcall, et `ecx` et `edx` lus avant d'être écrits signifient fastcall.
- **Récursion et pile.** Chaque niveau empile une adresse de retour et souvent un registre sauvegardé : une fonction récursive laisse donc un motif répété sur la pile.

## À vous de jouer

Construisez les exemples et exécutez-les :

```bash
cd modules/08-stack-and-calls/examples
make run
```

Sortie attendue :

```text
== System V (64-bit)
sum3(1, 2, 3)                = 6
sum_locals(5)                = 50
factorial(10)                = 3628800
weighted8(1,1,1,1,1,1,1,1)   = 36
weighted8(1,2,3,4,5,6,7,8)   = 204
apply_twice(add_ten, 1)      = 21
length_of("stack frame")     = 11
macro_demo(14)               = 43
== assembly calling printf
The answer is 42
== 32-bit conventions
cdecl:    add3(1, 2, 3) = 123, stack balanced
stdcall:  add3(1, 2, 3) = 123, stack balanced
fastcall: add3(1, 2, 3) = 123, stack balanced
== Microsoft x64 under Wine
ms_sum4(9, 8, 7, 6)  = 9876
ms_call_sum4()       = 1234
```

Les trois lignes 32 bits renvoient la même valeur avec une pile équilibrée, bien que les fonctions nettoient différemment. `weighted8(1,2,3,4,5,6,7,8)` vaut 1 + 4 + 9 + 16 + 25 + 36 + 49 + 64 = 204, ce qui prouve que les septième et huitième arguments sont lus aux bons emplacements de la pile.

Observez maintenant la pile pendant la récursion. Lancez le débogueur et arrêtez-vous au quatrième appel de `factorial` :

```bash
gdb -q build/08_main
```

```text
break factorial
run
continue 3
p $rdi
x/8gx $rsp
```

`rdi` vaut 7. La pile, de `rsp` vers le haut, contient ce motif (les adresses de retour sur votre machine peuvent différer, la structure non) :

```text
0x...e8:  0x0000000000401305  0x0000000000000009
0x...f8:  0x0000000000401305  0x000000000000000a
0x...08:  0x0000000000401305  0x00007ffd...48
0x...18:  0x00000000004011aa  ...
```

Lisez-le par paires. La première valeur d'une paire est une adresse de retour dans `factorial` (l'instruction qui suit le `call` récursif), et la seconde est le `rbx` que l'appelant a sauvegardé avec `push rbx` : 9 puis 10, qui sont les valeurs de `n` dans les deux cadres au-dessus. La troisième paire contient le `rbx` qui existait avant le début de `factorial(10)`, et la dernière ligne commence par l'adresse de retour dans `main`. Toute la chaîne d'appels est visible en mémoire.

Modifiez ensuite quelque chose et prédisez le résultat avant de reconstruire :

1. Dans `factorial`, supprimez `push rbx` et `pop rbx` et utilisez `rdi` après l'appel. Pourquoi le résultat devient-il faux ?
2. Dans `08_printf.asm`, supprimez `sub rsp, 8` et `add rsp, 8`. Le programme plante-t-il ? (La section suivante explique ce que vous voyez.)

## Pièges courants

- **Une pile mal alignée.** Retirer le réalignement de `08_printf.asm` ne le fait pas planter, car `printf` sans argument vectoriel n'utilise jamais d'écriture alignée. La même erreur avec un argument à virgule flottante (`al` égal à 1) provoque une erreur de segmentation dans `printf`, à un `movaps`. Un mauvais alignement peut rester dormant longtemps : alignez chaque appel.
- **Désaccord sur le nettoyage.** Appeler une fonction stdcall comme si elle était cdecl, ou l'inverse, laisse `esp` faux après chaque appel. Le programme peut sembler fonctionner jusqu'à ce que la pile ait assez dérivé pour que cela compte.
- **Oublier de sauvegarder un registre sauvegardé par l'appelé.** Si une fonction modifie `rbx` sans le restaurer, les données de son appelant changent silencieusement.
- **Garder une valeur dans un registre sauvegardé par l'appelant à travers un appel.** `rax`, `rcx`, `rdx`, `rsi`, `rdi` et `r8` à `r11` peuvent être modifiés par n'importe quelle fonction que vous appelez.
- **Utiliser la zone rouge dans une fonction qui fait des appels.** L'appel empile une adresse de retour dans la zone rouge. Seules les fonctions feuilles peuvent l'utiliser, et seulement en System V.
- **Mauvais décalages pour les arguments de pile.** À l'entrée, le septième argument est à `[rsp + 8]`, pas à `[rsp]`, car l'adresse de retour est au sommet.
- **Oublier l'espace d'ombre sous Windows.** L'appelé peut y écrire. Un appelant qui ne l'a pas réservé voit sa pile écrasée.
- **Oublier `al` pour un appel variadique.** Mettez-le au nombre de registres vectoriels utilisés, ou à 0.

## Résumé

- La pile croît vers le bas ; `call` empile une adresse de retour et `ret` la dépile, et un cadre contient des variables locales, des registres sauvegardés et le `rbp` sauvegardé.
- Une convention d'appel fixe où vont les arguments et les résultats, qui nettoie la pile et quels registres doivent être préservés.
- cdecl, stdcall et fastcall diffèrent par le placement des arguments et par qui les retire (`ret N`) ; System V utilise six registres entiers et une zone rouge ; Microsoft x64 utilise quatre registres et 32 octets d'espace d'ombre.
- Gardez `rsp` multiple de 16 à chaque `call` ; utilisez un `push` ou un `sub rsp, 8` pour y parvenir.
- Un prologue, un épilogue, un canari, un motif de push et de pop et les registres dans lesquels un argument est lu identifient la convention et le cadre dans du code compilé.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 09, ce qu'est la rétro-ingénierie.
- Exercices qui utilisent ce module : [Exercice 08](../../exercises/08-calling-card/README.fr.md), [Exercice 09](../../exercises/09-stack-archaeology/README.fr.md), [Exercice 11](../../exercises/11-portable-puzzle/README.fr.md).
