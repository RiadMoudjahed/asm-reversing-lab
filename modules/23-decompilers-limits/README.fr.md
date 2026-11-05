<!-- source: README.md @ sha256:51047c3ba2a8 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 23. Les décompilateurs et leurs limites

Durée : environ 1,5 heure. Prérequis : [22. Code managé : .NET](../22-managed-code-dotnet/README.fr.md)

## Pourquoi c'est important

Un décompilateur est l'outil le plus utile de la rétro-ingénierie, et celui auquel on croit le plus souvent trop. Il transforme le code machine en quelque chose qui ressemble à du C, avec des boucles, des conditions et des variables nommées, et pour la plupart des fonctions le résultat est assez proche de la vérité pour être lu vite. Pour certaines, il est faux d'une façon qui ne se voit pas : un nombre de mauvaise largeur, une comparaison de mauvais signe, une condition dont les opérateurs se groupent autrement que ne le suggère le listing. Une décision prise sur une telle ligne est une mauvaise décision, et la sortie paraît aussi sûre quand elle est fausse que quand elle est juste. Ce module explique comment fonctionne un décompilateur, pour que vous puissiez prévoir où il échoue, montre les échecs typiques sur du code compilé réel et enseigne l'habitude qui rend l'usage des décompilateurs sûr : traiter la sortie comme une hypothèse sur le code, et la vérifier avec le désassemblage et avec le programme en cours d'exécution. Les exemples utilisent un petit décompilateur écrit pour la leçon, afin que chaque étape puisse être montrée, et de petites fonctions dont la forme compilée est facile à mal lire.

## Concepts

### Ce que fait un décompilateur

Un décompilateur ne défait pas le compilateur. Le compilateur a jeté les noms, les types, les commentaires, la structure des boucles et le choix des constructions du source ; le décompilateur construit un programme plausible qui se compilerait avec le même comportement. Tous les décompilateurs passent par les mêmes étapes :

| Étape | Ce qu'elle fait | Ce qui peut mal tourner |
|---|---|---|
| Désassemblage et graphe de flot de contrôle | Trouve les instructions d'une fonction et les regroupe en blocs de base reliés par des sauts | Une mauvaise limite de fonction, une table de sauts non trouvée, du code atteint seulement par un saut indirect |
| Élévation (lifting) | Transforme chaque instruction en une opération sur des variables ; les registres deviennent des variables et les indicateurs deviennent des conditions | Une instruction à l'effet inhabituel est modélisée de façon approximative |
| Analyse de flot de données | Suit les valeurs : supprime les affectations que personne ne lit, fond les valeurs temporaires dans des expressions, retrouve les variables locales à partir des emplacements de pile | Une valeur utilisée d'une manière que le modèle ne voit pas est abandonnée ou mal fusionnée |
| Récupération des types | Choisit une largeur, un signe et un type pointeur ou nombre pour chaque variable d'après son usage | Les indices sont minces ou contradictoires et on devine |
| Structuration | Reconstruit `if`, `while`, `for` et `switch` à partir du graphe ; ce qui ne peut être reconstruit reste en `goto` | Le compilateur a remodelé les boucles et aucune structure ne convient |
| Sortie | Nomme les variables et affiche les expressions avec le moins de parenthèses et de conversions qui préservent le sens | Des conversions et des parenthèses sont omises ou mal placées dans le texte affiché |

La leçon suit ces étapes avec `examples/23_lift.py`, un décompilateur pour une poignée d'instructions x86-64. Il est volontairement petit ; il ne comprend que ce dont les exemples ont besoin et s'arrête avec un message devant tout le reste. Un vrai décompilateur (Ghidra, que vous avez utilisé dans le module 11) fait le même travail pour tout le jeu d'instructions, avec beaucoup plus d'analyse. Le but est de voir chaque étape.

### Les fonctions utilisées ici

`examples/23_cases.c` contient sept fonctions courtes, compilées avec `-O2` dans une bibliothèque partagée. Chacune est facile à mal lire :

| Fonction | Ce qu'elle fait | Le piège |
|---|---|---|
| `div10` | Division non signée par 10 | Le compilateur utilise une multiplication par une constante et un décalage |
| `widen` | Multiplie deux nombres de 32 bits en un résultat de 64 bits | La largeur de la multiplication |
| `sat_add` | Additionne deux nombres signés et sature en cas de débordement | Le débordement est un indicateur, pas une comparaison |
| `is_pow2` | Vrai pour 1, 2, 4, 8, ... | Une condition faite d'un test, d'une soustraction et d'un indicateur |
| `sum_to` | Somme des nombres de 0 à n-1, dans une boucle | L'optimiseur a changé la forme de la boucle |
| `sign_byte` | Lit un octet comme un nombre signé | Extension de signe contre extension par des zéros |
| `count_lt` | Compte les éléments plus petits qu'une valeur | Une comparaison et un décompte sans branchement |

Compilez les exemples et lancez la vérification (expliquée plus loin) et une fonction avec l'outil :

```bash
make
make run
```

### Étape 1 : le graphe

La première étape découpe les instructions en blocs de base et note où va le contrôle. Un bloc commence à la première instruction de la fonction, à chaque cible de saut et après chaque saut ; il se termine à un saut ou à un retour. Un arc de retour, un arc d'un bloc vers un bloc qui le domine (qui se trouve sur tous les chemins de l'entrée au bloc), signale une boucle. Pour `count_lt` :

```bash
python3 23_lift.py build/libcases.so count_lt --stage cfg
```

```text
== stage 1: control-flow graph of count_lt ==
block L_11b0:
    11b0:  test esi, esi
    11b2:  jle 0x11d8
    -> taken L_11d8, fall-through L_11b4
block L_11b4:
    11b4:  movsxd rsi, esi
    11b7:  xor eax, eax
    11b9:  lea rsi, [rdi + rsi*4]
    -> L_11c0
block L_11c0:
    11c0:  xor ecx, ecx
    11c2:  cmp edx, dword ptr [rdi]
    11c4:  setg cl
    11c7:  add rdi, 4
    11cb:  add eax, ecx
    11cd:  cmp rsi, rdi
    11d0:  jne 0x11c0
    -> taken L_11c0, fall-through L_11d2
block L_11d2:
    11d2:  ret
    -> return
block L_11d8:
    11d8:  xor eax, eax
    11da:  ret
    -> return
back edge L_11c0 -> L_11c0: L_11c0 is a loop header
```

Cinq blocs et une boucle : le bloc d'entrée teste le compteur (`jle` saute tout quand il est nul ou négatif), le deuxième prépare la boucle (le pointeur de fin et la somme), le troisième est la boucle et saute vers lui-même, et deux blocs retournent. Tout cela vient des seuls sauts, et c'est déjà là qu'un décompilateur peut se tromper : une table de sauts est un saut dont les cibles ne sont pas dans l'instruction, et si le décompilateur ne trouve pas la table, il voit une fonction qui se termine au milieu.

### Étape 2 : élévation (lifting)

L'élévation donne à chaque instruction un sens d'affectation. Il faut deux choses que la machine n'a pas : une variable pour chaque registre à chaque point, et des conditions à la place des indicateurs. Une comparaison et un saut conditionnel sont des instructions séparées reliées par le registre des indicateurs ; le décompilateur les fusionne. Dans `count_lt` :

```text
cmp    edx, dword ptr [rdi]
setg   cl
```

devient `rcx = (i32)rdx > (i32)(*(u32*)(rdi))`, parce que `setg` teste la condition « supérieur, signé » de la comparaison précédente. Ce dont un lecteur a besoin, c'est de la condition, pas de l'indicateur. L'outil le fait à l'intérieur d'un bloc en suivant une expression pour chaque registre et la dernière opération qui a positionné les indicateurs, et il substitue les expressions les unes dans les autres. Le résultat pour toute la fonction :

```bash
python3 23_lift.py build/libcases.so count_lt --stage lift
```

```text
== stage 2: lifted statements (every register that a block changes) ==
L_11b0:
    if ((i32)rsi <= 0) goto L_11d8; else goto L_11b4;
L_11b4:
    rsi = rdi + 4 * (i64)(i32)(u32)rsi;
    rax = 0;
    goto L_11c0;
L_11c0:
    flag = rsi != (rdi + 4);
    rcx = (i32)rdx > (i32)(*(u32*)(rdi));
    rax = (u32)((u32)rax + ((i32)rdx > (i32)(*(u32*)(rdi))));
    rdi = rdi + 4;
    if (flag) goto L_11c0; else goto L_11d2;
L_11d2:
    return rax;
L_11d8:
    rax = 0;
    return rax;
(7 assignments)
```

Regardez le bloc de la boucle. Quatre instructions de l'original (`xor`, `cmp`, `setg`, `add`) sont devenues une seule affectation à `rax`, et la comparaison avec le pointeur de fin est conservée dans une variable `flag`, parce que le bloc modifie aussi `rdi` et que la condition doit utiliser l'ancienne valeur. Un décompilateur affronte ce problème à chaque bloc, et une erreur dans l'ordre d'évaluation est l'une des façons dont un listing devient faux.

### Étape 3 : analyse de flot de données

Le code élevé affecte tous les registres qu'un bloc modifie. La plupart de ces valeurs ne sont jamais relues. L'analyse de vivacité détermine lesquelles le sont : un registre est vivant en un point si un chemin à partir de là le lit avant de l'écrire. Une affectation à un registre qui n'est pas vivant ensuite est supprimée. La même passe indique au décompilateur quels registres sont des arguments (lus avant toute écriture) et lequel contient le résultat (lu au retour).

```bash
python3 23_lift.py build/libcases.so count_lt --stage prune
```

```text
== stage 3: after removing assignments that nothing reads ==
L_11b0:
    if ((i32)rsi <= 0) goto L_11d8; else goto L_11b4;
L_11b4:
    rsi = rdi + 4 * (i64)(i32)(u32)rsi;
    rax = 0;
    goto L_11c0;
L_11c0:
    flag = rsi != (rdi + 4);
    rax = (u32)((u32)rax + ((i32)rdx > (i32)(*(u32*)(rdi))));
    rdi = rdi + 4;
    if (flag) goto L_11c0; else goto L_11d2;
L_11d2:
    return rax;
L_11d8:
    rax = 0;
    return rax;
(6 assignments)
```

L'affectation à `rcx` a disparu : c'était une valeur temporaire que la boucle écrivait puis intégrait à `rax`. Les vrais décompilateurs font beaucoup plus ici, notamment retrouver les variables locales que le compilateur gardait dans des emplacements de pile, propager les constantes et décider quelles valeurs reçoivent un nom et lesquelles restent dans une expression. Chacune de ces décisions est un endroit où le listing peut différer du code sans que cela se voie dans le texte affiché.

### Étape 4 : types et structure

Les types sont la partie qui est devinée. La machine connaît les largeurs (la taille de registre ou de mémoire de chaque accès) et presque rien d'autre. Qu'une valeur de 32 bits soit signée ou non, un nombre ou un pointeur, découle de son usage : un `movsx` ou une comparaison signée (`jl`, `setg`) indique le signé, un `movzx` ou une comparaison non signée (`jb`, `seta`) indique le non signé, une valeur ajoutée à un indice mis à l'échelle est une adresse. L'outil le fait implicitement en choisissant la conversion de chaque expression d'après l'instruction qui l'a produite : `(i32)` pour une comparaison signée, `(u32)` pour une non signée. Un vrai décompilateur doit faire un choix par variable, d'après tous les usages, et quand ils se contredisent il en choisit un.

La structuration transforme le graphe en `if` et en boucles. L'outil reconnaît une conditionnelle qui se sépare et se rejoint, une conditionnelle qui saute du code et une boucle dont le test est à la fin ; tout le reste, il le laisse en `goto` avec une étiquette.

```bash
python3 23_lift.py build/libcases.so count_lt --stage struct
```

```text
== stage 4: structured ==
result count_lt(...)
{
    if ((i32)rsi <= 0) {
        rax = 0;
        return rax;
    } else {
        rsi = rdi + 4 * (i64)(i32)(u32)rsi;
        rax = 0;
        do {
            flag = rsi != (rdi + 4);
            rax = (u32)((u32)rax + ((i32)rdx > (i32)(*(u32*)(rdi))));
            rdi = rdi + 4;
        } while (flag);
        return rax;
    }
}
(goto statements left: 0)
```

C'est une fonction qu'un lecteur comprend tout de suite : si le compteur n'est pas positif, renvoyer 0 ; sinon, parcourir le tableau du début au pointeur de fin, ajouter un pour chaque élément plus petit que la valeur de `rdx`, et renvoyer la somme. Les noms des variables sont ceux des registres. Un vrai décompilateur nomme les arguments d'après la convention d'appel et les variables locales d'après leur type.

### Où la sortie est fausse ou trompeuse

Les cas suivants viennent de la bibliothèque d'exemple. Chacun a la même forme : ce que fait le code machine, comment un listing peut tromper et ce qu'il faut vérifier. La sortie de `23_lift.py` pour la fonction est montrée, car elle rend explicite le comportement de la machine.

#### Division par une constante

```bash
objdump -d -M intel --no-show-raw-insn build/libcases.so | sed -n '/<div10>:/,/ret/p'
```

```text
0000000000001100 <div10>:
    1100:    mov    eax,edi
    1102:    mov    edx,0xcccccccd
    1107:    imul   rax,rdx
    110b:    shr    rax,0x23
    110f:    ret
```

Il n'y a pas de division. `0xcccccccd` est l'approximation sur 32 bits de 2 puissance 35 divisé par 10, et le produit décalé de 35 vers la droite donne le quotient pour toute entrée de 32 bits. Un décompilateur peut afficher la multiplication et le décalage, ce qui est fidèle et difficile à lire, ou reconnaître le motif et afficher `x / 10`. Les deux sont justes. L'erreur vient du lecteur : la largeur et le signe de la division sont dans les instructions autour de la multiplication (`mov eax,edi` étend avec des zéros ; une division signée par 10 a une autre séquence avec une correction de signe), et une lecture `(int)x / 10` est fausse pour tout `x` à partir de 2 puissance 31.

#### Largeur d'une multiplication

```text
== stage 4: structured ==
result widen(...)
{
    rax = (u64)(u32)rdi * (u64)(u32)rsi;
    return rax;
}
(goto statements left: 0)
```

Les deux arguments sont étendus par des zéros à 64 bits et multipliés comme des nombres de 64 bits (`imul rax, rsi`). En C, cela s'écrit `(unsigned long)a * b`. Écrit `(unsigned long)(a * b)`, ce qui paraît identique au premier coup d'œil, la multiplication serait faite sur 32 bits et perdrait la moitié haute. L'emplacement de la conversion fait toute la différence, et un listing imprimé peut la placer mal ou l'omettre.

#### Le débordement est un indicateur

```bash
python3 23_lift.py build/libcases.so sat_add --stage struct
```

```text
== stage 4: structured ==
result sat_add(...)
{
    flag = overflow_add_i32((u32)rsi, (u32)rdi);
    rsi = (u32)((u32)rsi + (u32)rdi);
    if (flag) {
        rax = (u32)(((u32)rdi >> 0x1f) + 0x7fffffff);
        return rax;
    } else {
        rax = (u32)rsi;
        return rax;
    }
}
(goto statements left: 0)
```

`jo` branche sur l'indicateur de débordement, positionné par le `add` précédent. Un décompilateur peut l'afficher comme un appel à un prédicat de débordement, comme le fait l'outil, comme une condition écrite sur les signes des opérandes et du résultat ou, quand il ne modélise pas l'indicateur, comme une comparaison qui n'est pas équivalente. La valeur de saturation vient du signe du premier argument : `(x >> 31) + 0x7fffffff` vaut `0x7fffffff` pour un `x` non négatif et `0x80000000` pour un nombre négatif. Une lecture qui ne traite que le débordement de deux nombres positifs rate la moitié de la fonction.

#### Une condition faite de plusieurs étapes

```bash
python3 23_lift.py build/libcases.so is_pow2 --stage struct
```

```text
== stage 4: structured ==
result is_pow2(...)
{
    rax = 0;
    if ((u32)rdi != 0) {
        rax = ((u32)(rdi - 1) & (u32)rdi) == 0;
    }
    return rax;
}
(goto statements left: 0)
```

Le résultat est un booléen : `lea eax,[rdi-1]` calcule `x - 1`, `test eax,edi` calcule `(x - 1) & x` et positionne l'indicateur de zéro, et `sete al` transforme l'indicateur en 0 ou 1. En C, cela se lit `x != 0 && (x & (x - 1)) == 0`. L'erreur courante d'une lecture écrite à la main est de supprimer les parenthèses : en C la comparaison `==` lie plus fort que `&`, donc `x & (x - 1) == 0` signifie `x & ((x - 1) == 0)`, qui est une autre fonction. Un décompilateur affiche les parenthèses, mais quand vous réécrivez ou simplifiez la sortie, ce sont les premières à partir.

#### Une boucle remodelée par l'optimiseur

```bash
python3 23_lift.py build/libcases.so sum_to --stage struct
```

```text
== stage 4: structured ==
result sum_to(...)
{
    if ((u32)rdi == 0) {
        rax = 0;
        return rax;
    } else {
        rax = 0;
        rdx = 0;
        if (((u8)rdi & 1) == 0) {
            L_1180:
            do {
                flag = (u32)rdi != (u32)((u32)rax + 2);
                rdx = (u32)(rdx + 2 * rax + 1);
                rax = (u32)((u32)rax + 2);
            } while (flag);
        } else {
            rax = 1;
            if ((u32)rdi != 1) {
                goto L_1180;
            }
        }
        rax = (u32)rdx;
        return rax;
    }
}
(goto statements left: 1)
```

Le source est une boucle qui additionne les nombres de 0 à n-1 un par un. La boucle compilée additionne deux nombres par itération, et une étape supplémentaire avant elle traite un compteur impair ; l'entrée saute au milieu de la structure. L'outil ne peut pas donner à celle-ci une structure propre : un `goto` vers la boucle subsiste, avec une étiquette à l'intérieur du `if`. Les vrais décompilateurs rencontrent la même forme et peuvent afficher la boucle deux fois, afficher un `goto`, ou afficher une boucle correcte mais qui ne ressemble pas au source. Lue à partir du graphe, la réponse apparaît : la boucle ajoute `2 * i + 1` à la somme et fait avancer `i` de 2, ce qui est la somme de la paire consécutive `i` et `i + 1` ; pour un compteur impair, l'étape avant la boucle commence les paires à 1, parce que le premier terme vaut 0.

#### Signe et extension

```bash
python3 23_lift.py build/libcases.so sign_byte --stage struct
```

```text
== stage 4: structured ==
result sign_byte(...)
{
    rax = (i32)(i8)*(u8*)(rdi);
    return rax;
}
(goto statements left: 0)
```

`movsx` est une extension de signe ; `movzx` est une extension par des zéros. En C, la première est une conversion vers `signed char` et la seconde une lecture d'un `unsigned char`. Pour l'octet `0x80`, elles donnent -128 et 128. Dans `count_lt`, la même distinction se cache dans l'instruction `setg`, qui teste une comparaison signée, là où `setb` en testerait une non signée. Lire le bon mnémonique est la vérification.

### Vérifier un listing

Traitez chaque fonction décompilée comme une affirmation : « ce C fait ce que fait le binaire ». Trois vérifications, de la moins chère à la plus forte.

**Lire le listing en regard du désassemblage.** Pour chaque ligne qui compte, trouvez les instructions qui l'ont produite et vérifiez ces points :

| Point | Ce qu'il faut regarder dans le désassemblage |
|---|---|
| Largeur | Le nom du registre (`eax` contre `rax`), le mot de taille dans les opérandes mémoire (`byte`, `word`, `dword`, `qword`) et l'endroit où une valeur est élargie |
| Signe | `movsx`, `movsxd`, `sar`, `idiv`, `jl/jg/jle/jge` et `setl/setg` contre `movzx`, `shr`, `div`, `jb/ja/jbe/jae` et `setb/seta` |
| Constantes | Les valeurs immédiates, en hexadécimal ; une constante magique est une division ou un modulo |
| Conditions | Si le branchement est pris quand la condition est vraie ou fausse ; un décompilateur nie des conditions pour les adapter à la structure |
| Bornes de boucle | La première et la dernière valeur du compteur, et la comparaison qui termine la boucle (`<` contre `<=`) |
| Accès mémoire | L'échelle de l'indice (la taille d'élément du tableau), le décalage (le champ de la structure) et la taille de chaque accès |
| Appels | Les arguments, d'après la convention d'appel, et les registres qui survivent à l'appel |
| Valeur de retour | La largeur et le signe de ce qui reste dans `rax` |
| Ordre | Quand deux effets de bord modifient une valeur que l'autre lit, l'ordre dans lequel ils se produisent |

**Tester le listing.** Écrivez le C que dit le listing, compilez-le comme une fonction de même signature et exécutez-le avec l'original sur les mêmes entrées. Les différences sont des erreurs de l'un ou de l'autre. Le script `examples/23_verify.py` le fait pour les sept fonctions : `23_candidates.c` contient, pour chaque fonction, une lecture qui correspond (`_ok`) et une lecture avec une erreur typique (`_bad`).

```bash
python3 23_verify.py build/libcases.so build/libcand.so
```

```text
div10_ok       matches
div10_bad      DIFFERS on 98 of 215 inputs
    first input ('0x80000000',): binary returns 214748364, reading returns 4080218932
widen_ok       matches
widen_bad      DIFFERS on 977 of 1225 inputs
    first input (2, '0x80000000'): binary returns 4294967296, reading returns 0
sat_add_ok     matches
sat_add_bad    DIFFERS on 168 of 841 inputs
    first input (-2147483648, -2147483648): binary returns -2147483648, reading returns 0
is_pow2_ok     matches
is_pow2_bad    DIFFERS on 3 of 215 inputs
    first input (2,): binary returns 1, reading returns 0
sum_to_ok      matches
sum_to_bad     DIFFERS on 14 of 50 inputs
    first input ('0x10001',): binary returns 2147516416, reading returns 32768
sign_byte_ok   matches
sign_byte_bad  DIFFERS on 128 of 256 inputs
    first input (b'\x80',): binary returns -128, reading returns 128
count_lt_ok    matches
count_lt_bad   DIFFERS on 233 of 300 inputs
    first input ([-2, 2, 2147483646, 1], 0): binary returns 1, reading returns 0
```

Les lectures qui correspondent ne produisent aucune différence sur aucune entrée. Chaque lecture fausse diffère sur certaines : `div10_bad` traite le nombre comme signé et échoue pour toute valeur à partir de 2 puissance 31, `widen_bad` multiplie sur 32 bits, `sat_add_bad` ne traite qu'un côté du débordement, `sum_to_bad` déborde avant la division, `sign_byte_bad` étend par des zéros, et `count_lt_bad` compare en non signé et est faux dès qu'un élément ou le seuil est négatif. Le cas qui mérite l'attention est `is_pow2_bad` : il ne diffère que sur 3 des 215 entrées, parce que l'ensemble de tests contient peu de puissances de deux, et 212 entrées concordent avec la lecture fausse. Un test ne vaut que par ses entrées. Incluez les extrêmes (0, 1, les plus grandes et les plus petites valeurs, les valeurs aux limites des largeurs) et les cas particuliers que la logique de la fonction met en évidence (ici, les puissances de deux).

**Vérifier les affirmations du listing sur le programme en cours d'exécution.** Pour les lignes qui décident de la réponse, arrêtez-vous sur la fonction dans le débogueur (module 11), donnez-lui des arguments connus et comparez les registres et la mémoire avec ce que dit le listing. Cette vérification fonctionne même quand le code est écrit à la main, obscurci ou trop compliqué pour être testé isolément.

### Quand se fier plutôt au désassemblage

Préférez le listing des instructions au code décompilé quand :

- La fonction est petite et chaque instruction compte (une somme de contrôle, une routine de comparaison, un tour de chiffrement, une manipulation de bits). La sémantique est dans les opérations, les largeurs et les indicateurs exacts.
- Le décompilateur abandonne, dépasse le temps imparti ou affiche une fonction de quelques lignes qui ne peut pas être juste pour la taille du code.
- Le code a été écrit en assembleur ou utilise des instructions sans équivalent en C (chaînes de retenue avec `adc` et `sbb`, `rep movs` et `rep stos`, instructions vectorielles, `rdtsc`, `cpuid`, appels système).
- Le flot de contrôle est inhabituel : tables de sauts non trouvées, sauts calculés, code qui saute au milieu d'une autre fonction, une machine virtuelle du module 21.
- Le code décompilé contient beaucoup d'instructions `goto` ou des variables qui changent de type ou de sens entre les usages ; la structure est une supposition.
- Un résultat surprend : une fonction qui devrait faire quelque chose ne fait rien, ou une constante n'a pas de sens. Regardez les instructions.

Préférez le décompilateur quand vous avez besoin de la forme générale d'une grande fonction, de la logique autour d'un appel ou d'une carte rapide de ce que fait un programme. Une bonne pratique consiste à utiliser les deux ensemble : lisez la fonction décompilée pour la structure, et lisez les instructions derrière chaque ligne qui décide de quelque chose.

### Un ordre de travail

1. **Lire une fois la fonction décompilée** pour sa forme : les arguments, les boucles, les appels, la valeur de retour.
2. **Marquer les lignes qui décident de quelque chose** : comparaisons, constantes, tailles, indices, la valeur de retour.
3. **Vérifier chaque ligne marquée** avec le désassemblage, à l'aide du tableau de points ci-dessus.
4. **Renommer et retyper** à mesure que vous apprenez, dans le décompilateur, pour que la lecture suivante soit plus proche de la vérité ; puis vérifier de nouveau, car un type modifié change le listing.
5. **Tester l'affirmation** quand cela compte : écrire le C équivalent et exécuter les deux sur des entrées extrêmes et des entrées aléatoires.
6. **Noter ce que vous n'avez pas pu vérifier** et le rapporter comme tel.

## À essayer

Exécutez les commandes du laboratoire depuis `modules/23-decompilers-limits/examples`.

1. Compilez avec `make` et lancez `make run`. Pour chacune des sept fonctions, lisez le désassemblage (`objdump -d -M intel --no-show-raw-insn build/libcases.so`) et écrivez le C que vous pensez y voir. Lancez `23_lift.py` sur la fonction et comparez.
2. Lancez `23_lift.py` avec `--stage cfg`, `lift`, `prune` et `struct` sur `sat_add`, et décrivez en une phrase ce que chaque étape a changé.
3. Ouvrez `build/libcases.so` dans Ghidra sur l'hôte et comparez sa sortie décompilée des sept fonctions avec la sortie de `23_lift.py` et avec le source. Pour chaque différence, décidez s'il s'agit de noms et de style ou de sens.
4. Ajoutez une huitième fonction à `23_cases.c`, par exemple une moyenne de deux nombres non signés écrite `(a + b) / 2` et `a + (b - a) / 2`. Recompilez, lisez le désassemblage des deux et dites laquelle le compilateur peut réduire à une séquence plus courte et pourquoi elles diffèrent pour de grandes entrées.
5. Écrivez dans `23_candidates.c` une lecture fausse de votre cru pour l'une des fonctions, ajoutez-la aux cas de `23_verify.py` et vérifiez que le test la trouve. Faites ensuite diverger la lecture fausse sur très peu d'entrées et voyez combien d'entrées aléatoires le test demande pour la trouver.
6. Remplacez l'argument de `23_lift.py` par une fonction d'un programme d'un module précédent (une fonction d'une bibliothèque que vous avez construite avant). Où l'outil s'arrête-t-il ? Quelle instruction ne connaît-il pas ?

## Pièges courants

- **Croire un listing parce qu'il se lit bien.** Une sortie lisible n'est pas une sortie correcte. Vérifiez les lignes qui décident de quelque chose.
- **Se fier aux types.** Une variable déclarée `int` peut être non signée dans le code, et un pointeur peut être affiché comme un nombre. Lisez les instructions qui utilisent la valeur.
- **Rater une condition niée.** Le décompilateur peut afficher le contraire du branchement pour s'adapter à sa structure. Vérifiez quel côté est pris.
- **Simplifier la sortie et changer le sens.** Supprimer des parenthèses ou des conversions change ce que signifie le C, comme le montre la lecture de `is_pow2`.
- **Tester seulement avec des entrées aléatoires.** Beaucoup d'erreurs apparaissent aux limites et sur des valeurs spéciales. Incluez-les.
- **Corriger la supposition du décompilateur par une autre supposition.** Quand vous retypez une variable, vérifiez de nouveau le résultat avec les instructions.
- **Ignorer ce que le décompilateur n'a pas pu structurer.** Un `goto` dans la sortie indique que le compilateur a remodelé le code, et il mérite un coup d'œil au graphe.

## Résumé

- Un décompilateur construit un programme plausible à partir du code machine par étapes : graphe, élévation, analyse de flot de données, récupération des types, structuration et affichage ; chaque étape peut introduire des erreurs que le texte affiché cache.
- La largeur, le signe et l'emplacement des conversions sont les sources les plus fréquentes de lectures fausses, suivies des indicateurs transformés en conditions, des boucles remodelées et des parenthèses mal placées.
- Vérifiez un listing en le lisant en regard du désassemblage pour chaque ligne qui décide de quelque chose, en testant une lecture en C contre le binaire avec des entrées extrêmes et aléatoires, et en contrôlant les affirmations dans un débogueur.
- Un test ne vaut que par ses entrées ; incluez les limites et les cas particuliers de la logique.
- Utilisez les instructions plutôt que le code décompilé pour les petites routines exactes, l'assembleur écrit à la main, le flot de contrôle inhabituel et partout où le décompilateur échoue ou paraît faux.
- Le décompilateur donne la forme ; le désassemblage donne la vérité ; utilisez les deux.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec l'exercice 20, le projet de synthèse, qui combine le code compilé (module 13), la récupération des données (module 15) et les machines virtuelles (module 21).
