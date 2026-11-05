<!-- source: README.md @ sha256:c6f208d5cb3f -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 07. Chaînes et instructions de chaîne

Durée : environ 2 heures. Prérequis : [06. Conditions, branchements et boucles](../06-conditions-and-loops/README.fr.md)

## Pourquoi c'est important

Copier, remplir, comparer et parcourir des blocs de mémoire figure parmi les opérations les plus courantes de n'importe quel programme, et x86 dispose d'instructions dédiées pour cela. Vous rencontrerez ces instructions dans de l'assembleur écrit à la main, dans du code compilé optimisé en taille et dans des logiciels anciens ; reconnaître la fonction de bibliothèque que représente une instruction `rep` est une compétence de base en rétro-ingénierie. Écrire vous-même `strlen`, `memcpy` et `memmove` montre aussi comment le processeur traite la mémoire élément par élément.

## Notions

### Trois façons de stocker une chaîne

| Type | Disposition | Longueur |
|---|---|---|
| Terminée (C) | Des caractères suivis d'un octet nul | Trouvée en parcourant jusqu'au zéro |
| Préfixée par la longueur | Un octet de longueur, puis les caractères | Lue dans le premier octet |
| Pointeur et longueur | L'adresse et le nombre sont conservés séparément | Toujours connue |

L'assembleur stocke ce que vous écrivez ; la convention est la vôtre. [07_prefixed.asm](examples/07_prefixed.asm) lit une chaîne préfixée par sa longueur avec `lodsb` et l'affiche. Une chaîne terminée par zéro peut être parcourue avec `scasb`, et un bloc de longueur connue avec `movsb`, `stosb` ou `cmpsb`.

### Les instructions de chaîne

| Instruction | Effet | Registres utilisés |
|---|---|---|
| `movs` | Copie l'élément situé à `[rsi]` vers `[rdi]` | `rsi`, `rdi` |
| `lods` | Charge l'élément situé à `[rsi]` dans `al`, `ax`, `eax` ou `rax` | `rsi`, l'accumulateur |
| `stos` | Range `al`, `ax`, `eax` ou `rax` à `[rdi]` | `rdi`, l'accumulateur |
| `scas` | Compare l'accumulateur à l'élément situé à `[rdi]` | `rdi`, l'accumulateur |
| `cmps` | Compare l'élément situé à `[rsi]` à celui situé à `[rdi]` | `rsi`, `rdi` |

Chacune existe en quatre tailles : un suffixe `b`, `w`, `d` ou `q` sélectionne 1, 2, 4 ou 8 octets (`movsb`, `movsw`, `movsd`, `movsq`). Après chaque exécution, les pointeurs utilisés avancent de la taille de l'élément. Les registres sont fixes : vous ne les nommez pas, vous les chargez d'abord.

### L'indicateur de direction

Le fait que les pointeurs montent ou descendent dépend de l'**indicateur de direction** (DF). `cld` le met à zéro et ils vont vers les adresses hautes ; `std` le positionne et ils vont vers les adresses basses. L'ABI System V exige que DF soit à zéro à l'entrée d'une fonction et à son retour : une routine qui le positionne avec `std` doit donc le rétablir avec `cld` avant `ret`.

### Répéter : les préfixes `rep`

Un préfixe placé devant une instruction de chaîne la fait répéter avec le compte contenu dans `rcx` :

| Préfixe | Répète | Partenaire habituel |
|---|---|---|
| `rep` | `rcx` fois | `movs`, `stos`, `lods` |
| `repe` ou `repz` | Tant que `rcx` > 0 et que la dernière comparaison était égale (ZF = 1) | `cmps`, `scas` |
| `repne` ou `repnz` | Tant que `rcx` > 0 et que la dernière comparaison était différente (ZF = 0) | `scas`, `cmps` |

Chaque passage décrémente `rcx`. Si `rcx` vaut 0 à l'entrée, l'instruction ne fait rien et laisse les indicateurs en l'état : testez donc d'abord si le compte est nul lorsque le résultat dépend des indicateurs.

### Les routines classiques

Les versions complètes se trouvent dans [07_strings.asm](examples/07_strings.asm). Leur forme :

```nasm-fragment
my_strlen:
    xor     eax, eax                    ; al = 0, the byte to find
    mov     rcx, -1                     ; effectively no limit
    repne scasb                         ; scan until [rdi] equals al
    not     rcx                         ; rcx counted down from -1
    lea     rax, [rcx - 1]              ; bytes scanned, minus the terminator
    ret

my_memcpy:
    mov     rax, rdi                    ; the return value
    mov     rcx, rdx                    ; the count
    rep movsb                           ; copy rcx bytes from [rsi] to [rdi]
    ret
```

Remplir, c'est `rep stosb` avec l'octet dans `al` ; comparer, c'est `repe cmpsb` ; chercher, c'est `repne scasb` avec l'octet recherché dans `al`.

### Copies qui se chevauchent

Une copie vers l'avant tourne mal lorsque la destination se trouve à l'intérieur de la source et au-dessus d'elle : la copie écrase des octets qu'elle n'a pas encore lus. `memmove` règle cela en vérifiant d'abord la direction. Si la destination est en dessous de la source, ou commence après la fin de la source, elle copie vers l'avant. Sinon, elle positionne `std`, part du dernier octet des deux blocs, copie vers l'arrière et remet l'indicateur à zéro. Avec le tampon `abcdefgh`, déplacer 6 octets de 2 positions vers le haut donne `ababcdef` ; de 2 positions vers le bas donne `cdefghgh`.

### Le point de vue de la rétro-ingénierie

Ne vous attendez pas à ce que les compilateurs émettent `rep` partout. Exécutez `make compiled` dans le dossier des exemples pour voir les deux niveaux d'optimisation des mêmes fonctions C. Les lignes importantes :

```text
== gcc -O1
<clear_struct>:   ...  rep stos QWORD PTR es:[rdi],rax
<copy_struct>:    sixteen movdqu/movups pairs (SSE)
<copy_var>:       call to memcpy
<length>:         call to strlen

== gcc -Os
<clear_struct>:   mov  ecx,0x40
                  xor  eax,eax
                  rep stos DWORD PTR es:[rdi],eax
<copy_struct>:    mov  ecx,0x40
                  rep movs DWORD PTR es:[rdi],DWORD PTR ds:[rsi]
<copy_var>:       mov  rcx,rdx
                  rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]
```

Ce qu'on peut en lire :

- **Optimisé pour la vitesse (`-O1`).** Les opérations de taille variable deviennent des appels à la bibliothèque C, et les petites copies de taille fixe des déplacements SSE. Un `call` vers `memcpy`, `strlen` ou `memset` est la forme compilée de ces fonctions C.
- **Optimisé pour la taille (`-Os`).** Le compilateur utilise des instructions `rep` parce qu'elles sont courtes.
- **Compte fois taille de l'élément.** `mov ecx, 0x40` avec `rep stos DWORD` met à zéro 0x40 fois 4 = 256 octets. La taille de l'élément dans l'instruction et la valeur de `ecx` donnent ensemble la taille du bloc.
- **Ce que disent les registres.** `rep stos` avec `eax` ou `rax` nul est `memset(p, 0, n)`. `rep movs` est `memcpy`. `repne scas` avec `al` nul est `strlen`. `repe cmps` est `memcmp`.
- **`std` avant `rep movs`.** Une copie vers l'arrière : un `memmove` avec chevauchement, ou une routine qui travaille depuis la fin d'un tampon.

## À vous de jouer

Construisez les exemples et exécutez-les :

```bash
cd modules/07-strings/examples
make run
```

Sortie attendue :

```text
my_strlen("assembly"), ("")  = 8 0
my_memchr("assembly", 'm', 8) = offset 4
my_memchr("assembly", 'z', 8) = NULL
my_memset(buf, 'x', 6)        = xxxxxx..
my_memcpy(buf, "assembly", 8) = assembly
my_memcmp: abc vs abd, abc, abd vs abc = negative zero positive
memmove up by 2 (overlap)      = ababcdef
memmove down by 2 (overlap)    = cdefghgh
direction flag after the calls = 0
Hello
```

La dernière ligne provient de l'exemple à longueur préfixée. Les deux lignes `memmove` déplacent les mêmes octets de 2 positions vers le haut et de 2 vers le bas dans un même tampon ; seule la première exige une copie vers l'arrière. L'indicateur de direction vaut 0 après les appels, comme l'exige l'ABI.

Modifiez ensuite quelque chose et prédisez le résultat avant de reconstruire :

1. Dans `my_memmove`, supprimez `std`, `cld` et les deux lignes `lea` du cas vers l'arrière, de sorte que chaque appel copie vers l'avant. Laquelle des deux lignes `memmove` change, et pourquoi ?
2. Dans `my_strlen`, remplacez `mov rcx, -1` par `mov rcx, 4`. Que renvoie `my_strlen("assembly")` ?

## Pièges courants

- **Oublier `cld` après `std`.** La prochaine instruction de chaîne de l'appelant s'exécute alors à l'envers. L'indicateur de direction doit être à zéro au retour.
- **Lire ZF après un compte nul.** Avec `rcx` égal à 0, une instruction `repe` ou `repne` ne fait rien et les indicateurs gardent leurs anciennes valeurs : un test trouvé ou non trouvé n'a alors aucun sens. Vérifiez d'abord si le compte est nul.
- **Intervertir source et destination.** `movs` copie de `[rsi]` vers `[rdi]` ; `cmps` compare `[rsi]` à `[rdi]`. Pour `memcmp(a, b)`, le pointeur `a` est dans `rdi` : la différence d'octets à renvoyer est donc `[rdi]` moins `[rsi]`. L'inverse renverse le signe de chaque résultat.
- **Erreur d'une unité dans `strlen`.** Après `repne scasb`, le pointeur se trouve un cran après l'octet nul, et `not rcx` compte aussi le zéro. Retranchez un.
- **Copier vers l'avant sur un chevauchement.** Un `rep movsb` vers l'avant dont la destination commence à l'intérieur de la source duplique un motif au lieu de déplacer les données.
- **Lire au-delà de la fin.** Une routine qui utilise `scasq` ou `movsq` pour aller plus vite lit des mots entiers et peut toucher de la mémoire au-delà de la chaîne.

## Résumé

- Les instructions de chaîne travaillent par `rsi`, `rdi`, `rcx` et l'accumulateur, et déplacent les pointeurs de la taille de l'élément.
- `rep`, `repe` et `repne` les répètent ; un compte nul ne fait rien et laisse les indicateurs intacts.
- L'indicateur de direction choisit l'avant ou l'arrière ; remettez-le à zéro avec `cld` avant de revenir.
- `strlen`, `memchr`, `memset`, `memcpy`, `memcmp` et `memmove` tiennent chacune en quelques instructions ; `memmove` exige une copie vers l'arrière en cas de chevauchement.
- Dans du code compilé, les instructions `rep` apparaissent surtout dans les builds optimisés en taille ; un `call` vers la bibliothèque est sinon la forme courante.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 08, la pile, les procédures et les conventions d'appel.
- Exercices qui utilisent ce module : [Exercice 07](../../exercises/07-string-sweeper/README.fr.md).
