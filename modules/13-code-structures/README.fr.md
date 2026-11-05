<!-- source: README.md @ sha256:83466ca39b04 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 13. Reconnaître les structures de code

Durée : environ 3 heures. Prérequis : [12. Formats exécutables : ELF et PE](../12-executable-formats/README.fr.md)

## Pourquoi c'est important

Un compilateur transforme boucles, conditions et appels de fonctions en un petit ensemble de motifs d'instructions. Une fois les motifs reconnus, une page d'assembleur devient quelques lignes d'intention, et vous lisez le code à la vitesse de la sortie du compilateur plutôt qu'instruction par instruction. Ce module enseigne les formes : entrée et sortie de fonction, variables locales, `if`, logique à court-circuit, toutes les formes courantes de boucle, `switch`, mise en ligne et appels terminaux. Il compare le même code C compilé sans optimisation et avec, parce que vous rencontrerez les deux.

## Concepts

### Comment les listings de ce module ont été produits

Le source C est `examples/13_structs.c`. Le `Makefile` le compile avec GCC en `-O0`, `-O1` et `-O2` et écrit de l'assembleur en syntaxe Intel dans `build/13_O0.s`, `build/13_O1.s` et `build/13_O2.s`. Les listings ci-dessous sont ces fichiers privés des directives de l'assembleur. Les étiquettes comme `.L11` sont les noms que le compilateur donne aux cibles de saut ; dans un binaire, ce sont de simples adresses et vous devez trouver les cibles vous-même. Tout le code est x86-64 System V, compilé avec `-fcf-protection=none -fno-stack-protector` pour qu'aucune instruction supplémentaire ne masque la structure.

```bash
make
```

### Entrée et sortie d'une fonction

**Sans optimisation**, chaque fonction construit un cadre avec le pointeur de cadre `rbp`. Les arguments arrivent dans des registres mais sont copiés dans le cadre, et chaque variable vit en mémoire.

```text
sum3:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-20], edi
    mov    DWORD PTR [rbp-24], esi
    mov    DWORD PTR [rbp-28], edx
    mov    edx, DWORD PTR [rbp-20]
    mov    eax, DWORD PTR [rbp-24]
    add    eax, edx
    mov    DWORD PTR [rbp-4], eax
    mov    edx, DWORD PTR [rbp-4]
    mov    eax, DWORD PTR [rbp-28]
    add    eax, edx
    pop    rbp
    ret
```

Les deux premières instructions, `push rbp` et `mov rbp, rsp`, sont le **prologue**. Elles sont compensées à la fin par `pop rbp` (ou `leave`) et `ret`, l'**épilogue**. Les trois instructions `mov DWORD PTR [rbp-N], reg` sauvegardent les arguments `a`, `b` et `c` dans le cadre. La variable `total` est en `[rbp-4]`. Les lectures et écritures en mémoire expliquent la longueur de ce code.

**Avec optimisation**, la même fonction n'a plus besoin de cadre du tout :

```text
sum3:
    add    edi, esi
    lea    eax, [rdi+rdx]
    ret
```

Les arguments restent dans `edi`, `esi` et `edx`, le résultat est construit avec `add` et `lea`, et il n'y a pas de prologue. Une fonction qui ne fait aucun appel et n'a pas besoin de mémoire pour des variables locales est une **fonction feuille**, et de telles fonctions n'ont souvent ni prologue ni épilogue.

**Une fonction qui en appelle d'autres** doit préserver des valeurs au travers des appels. En `-O2`, GCC n'utilise pas `rbp` comme pointeur de cadre ; il sauvegarde les registres à préserver par l'appelé dont il a besoin et ajuste `rsp` :

```text
two_calls:
    push    rbp
    push    rbx
    mov    ebx, edi
    sub    rsp, 8
    call    act_a
    lea    edi, [rbx+1]
    mov    ebp, eax
    call    act_b
    add    rsp, 8
    add    eax, ebp
    pop    rbx
    pop    rbp
    ret
```

Lisez ceci comme le cadre que vous avez reconstitué à l'exercice 09 : deux `push` pour `rbp` et `rbx`, un `sub rsp, 8` qui garde `rsp` aligné aux appels, et les mêmes registres dépilés dans l'ordre inverse à la fin. Ici `rbx` contient `v` et `rbp` contient le premier résultat pendant le second appel. Le `sub rsp, 8` n'a aucune variable locale à abriter ; il n'existe que pour l'alignement.

| Motif | Ce que cela vous apprend |
|---|---|
| `push rbp; mov rbp, rsp` ... `pop rbp` ou `leave` | Un pointeur de cadre ; probablement du code non optimisé ou une fonction avec une zone de pile de taille variable |
| Sauvegarde de `rbx`, `rbp`, `r12` à `r15` au début | Des valeurs qui vivent au travers des appels |
| `sub rsp, N` | De la place pour les variables locales, des temporaires sauvegardés ou l'alignement |
| Pas de prologue, se termine par `ret` | Une fonction feuille |
| Se termine par `jmp` vers une autre fonction | Un appel terminal (voir ci-dessous) |

### Variables locales et paramètres

En `-O0`, un paramètre est rangé une fois à l'entrée puis relu en mémoire à chaque fois, et une variable locale est un décalage négatif par rapport à `rbp` : `[rbp-4]`, `[rbp-8]`. Les variables reçoivent des emplacements distincts dans l'ordre où le compilateur les alloue. Dans `sum3`, `a` est en `[rbp-20]`, `b` en `[rbp-24]` et `c` en `[rbp-28]` (en hexadécimal `-0x14`, `-0x18` et `-0x1c`, comme `objdump` les écrit), et `total` est en `[rbp-4]`.

À partir de `-O1`, la plupart des variables vivent dans des registres et n'ont aucune adresse. Une variable n'obtient un emplacement mémoire que si son adresse est prise, si c'est un tableau ou une structure, ou si le compilateur manque de registres. Quand vous voyez un emplacement de pile en `-O2`, demandez-vous pourquoi : c'est souvent un tableau, une structure ou une valeur dont l'adresse a été passée à une autre fonction, comme à l'exercice 09.

### If et else

Une condition se compile en une comparaison suivie d'un saut conditionnel. Le saut **évite** le code exécuté quand la condition est vraie ; la condition du saut est donc l'**opposé** de la condition C.

```text
sign_text:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    cmp    DWORD PTR [rbp-4], 0
    jle    .L7
    mov    eax, 1
    jmp    .L8
.L7:
    cmp    DWORD PTR [rbp-4], 0
    jns    .L9
    mov    eax, -1
    jmp    .L8
.L9:
    mov    eax, 0
.L8:
    pop    rbp
    ret
```

Voici `sign_text` : `if (x > 0) return 1; else if (x < 0) return -1; else return 0;`. Le premier test est `x > 0`, mais le saut est `jle`, qui évite le bloc `return 1` quand la condition est fausse. De même, `x < 0` est testé avec `jns`. Chaque `jmp .L` vers l'épilogue est un `return`. La chaîne `cmp / jcc / body / jmp end` se répète pour chaque branche.

Le code optimisé évite souvent complètement la branche. Deux fonctions en `-O2` :

```text
clamp_low:
    cmp    edi, esi
    mov    eax, esi
    cmovge    eax, edi
    ret
```

```text
pick:
    mov    edx, edi
    mov    eax, esi
    sub    edx, esi
    sub    eax, edi
    cmp    edi, esi
    cmovg    eax, edx
    ret
```

`clamp_low` renvoie `x` ou `low`, le plus grand des deux : `cmp` positionne les indicateurs et `cmovge` copie une valeur dans le résultat seulement si la condition est vraie. Un **déplacement conditionnel** (`cmov`) est la façon dont le compilateur écrit `a ? b : c` sans saut quand les deux côtés sont peu coûteux et sûrs à calculer. Dans `pick`, `a - b` et `b - a` sont d'abord calculés tous les deux, et `cmovg` en choisit un. Si vous voyez deux résultats arithmétiques calculés côte à côte et un `cmov`, vous avez affaire à une expression ternaire ou à un petit `if`.

### Logique à court-circuit

C évalue `a && b` de gauche à droite et n'évalue pas `b` quand `a` est faux. Le code compilé le montre par une comparaison et un saut séparés pour chaque opérande, tous sautant au même endroit.

```text
in_range:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    mov    DWORD PTR [rbp-8], esi
    mov    DWORD PTR [rbp-12], edx
    mov    eax, DWORD PTR [rbp-4]
    cmp    eax, DWORD PTR [rbp-8]
    jl    .L15
    mov    eax, DWORD PTR [rbp-4]
    cmp    eax, DWORD PTR [rbp-12]
    jg    .L15
    mov    eax, 1
    jmp    .L17
.L15:
    mov    eax, 0
.L17:
    pop    rbp
    ret
```

`in_range` est `x >= lo && x <= hi`. Le premier test `x < lo` (l'opposé de `x >= lo`) saute vers `.L15`, le bloc « le résultat est 0 » ; le second test `x > hi` saute vers le même bloc. Ce n'est que si aucun saut n'est pris que le code atteint `mov eax, 1`. Deux sauts conditionnels vers la même étiquette « faux » signifient `&&`.

```text
either:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    mov    DWORD PTR [rbp-8], esi
    cmp    DWORD PTR [rbp-4], 0
    je    .L19
    cmp    DWORD PTR [rbp-8], 0
    jne    .L20
.L19:
    mov    eax, 1
    jmp    .L22
.L20:
    mov    eax, 0
.L22:
    pop    rbp
    ret
```

Pour `a == 0 || b == 0`, le motif est différent : le premier saut va vers le bloc **vrai** (`je .L19`), et le dernier test saute vers le bloc faux. Un saut conditionnel vers l'étiquette « vrai » suivi d'un test qui tombe dans « vrai » signifie `||`. En résumé :

| Forme | Source |
|---|---|
| Plusieurs `jcc` vers une étiquette « faux », le succès continue en séquence | Une chaîne de conditions reliées par `&&` |
| `jcc` vers une étiquette « vrai », puis le dernier test saute ailleurs si faux | Des conditions reliées par l'opérateur OU |

En `-O2`, le compilateur peut supprimer les sauts et combiner les résultats avec `setcc` et `and`/`or`. C'est permis quand le second opérande n'a pas d'effet de bord et ne peut pas provoquer de faute.

```text
in_range:
    mov    eax, edx
    cmp    edi, esi
    setge    dl
    cmp    edi, eax
    setle    al
    movzx    eax, al
    and    eax, edx
    ret
```

### Boucles

Toute boucle a une initialisation, un test, un corps et une mise à jour, et un saut conditionnel va vers l'arrière. **Un saut conditionnel vers l'arrière est la signature d'une boucle.**

En `-O0`, une boucle `for` est disposée avec le test en bas :

```text
count_up:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-20], edi
    mov    DWORD PTR [rbp-4], 0
    mov    DWORD PTR [rbp-8], 0
    jmp    .L24
.L25:
    mov    eax, DWORD PTR [rbp-8]
    add    DWORD PTR [rbp-4], eax
    add    DWORD PTR [rbp-8], 1
.L24:
    mov    eax, DWORD PTR [rbp-8]
    cmp    eax, DWORD PTR [rbp-20]
    jl    .L25
    mov    eax, DWORD PTR [rbp-4]
    pop    rbp
    ret
```

Le code met d'abord `total` et `i` à zéro (`[rbp-4]` et `[rbp-8]`), puis saute au test en `.L24`. Le corps est en `.L25` ; il ajoute `i` à `total` et incrémente `i`. Le test compare `i` à `n` (en `[rbp-20]`) et revient en arrière avec `jl .L25` tant que `i < n`. Sauter d'abord au test explique le `jmp` par-dessus le corps au début.

En `-O1`, la même fonction est **pivotée** :

```text
count_up:
    test    edi, edi
    jle    .L14
    mov    eax, 0
    mov    edx, 0
.L13:
    add    edx, eax
    add    eax, 1
    cmp    edi, eax
    jne    .L13
.L11:
    mov    eax, edx
    ret
.L14:
    mov    edx, 0
    jmp    .L11
```

Le compilateur vérifie une fois si la boucle s'exécute (`test edi, edi / jle`), puis place le test en bas du corps, de sorte que chaque itération coûte un saut conditionnel. `edx` est `total` et `eax` est `i`. Notez que le compteur va de 0 vers le haut et que le test de sortie est `cmp edi, eax / jne` : le compilateur savait que la boucle s'arrête quand `i` atteint `n`, donc il utilise `jne` plutôt que `jl`. Les boucles optimisées peuvent avoir une condition de sortie différente de celle que vous lisez dans le source.

Une boucle `while` sur une chaîne ressemble à ceci en `-O1` :

```text
count_while:
    lea    rcx, [rdi+1]
    cmp    BYTE PTR [rdi], 0
    je    .L19
    mov    rax, rcx
.L18:
    mov    rdx, rax
    add    rax, 1
    cmp    BYTE PTR [rax-1], 0
    jne    .L18
    sub    edx, ecx
    lea    eax, [rdx+1]
    ret
.L19:
    mov    eax, 0
    ret
```

Le pointeur `rax` parcourt la chaîne et la boucle s'arrête quand l'octet qu'il vient de lire est nul (`cmp BYTE PTR [rax-1], 0 / jne`). Le nombre n'est pas un compteur séparé : après la boucle, le compilateur calcule la longueur comme la différence entre le pointeur de fin et le pointeur de début (`sub edx, ecx` puis `lea eax, [rdx+1]`). Chaque fois qu'un compteur est remplacé par de l'arithmétique de pointeurs, le compilateur applique une **réduction de force** (strength reduction) à la boucle.

Une boucle `do { } while` exécute le corps au moins une fois ; elle n'a donc pas de garde avant la première itération :

```text
do_once:
    mov    eax, 0
.L22:
    mov    edx, edi
    mov    ecx, edi
    shr    ecx, 31
    add    edi, ecx
    sar    edi
    add    eax, 1
    cmp    edx, 1
    jg    .L22
    ret
```

Le corps est exécuté d'abord, et le saut vers l'arrière `jg .L22` est le seul test. Dans le corps, `n / 2` pour une valeur signée est l'idiome de quatre instructions `mov / shr 31 / add / sar` : le bit de signe est ajouté avant le décalage pour que la division arrondisse vers zéro. Le module suivant explique pourquoi.

| Forme de boucle | Disposition |
|---|---|
| `for`/`while`, non optimisé | `jmp test` au début, corps, mise à jour, test en bas avec un `jcc` vers l'arrière |
| `for`/`while`, optimisé | Une garde qui saute la boucle, puis le corps, puis le test avec un `jcc` vers l'arrière |
| `do`/`while` | Le corps d'abord ; le seul test est le `jcc` vers l'arrière |
| Boucle infinie | Un `jmp` inconditionnel vers l'arrière avec une sortie dans le corps |

### Switch

Un `switch` peut se compiler de quatre façons, selon les valeurs des cas et le niveau d'optimisation. Apprenez à les distinguer par les instructions qui précèdent les sauts.

**Une chaîne de comparaisons** pour quelques cas éparpillés :

```text
sparse:
    mov    eax, 2
    cmp    edi, 400
    je    .L35
    mov    eax, 3
    cmp    edi, 9000
    je    .L35
    xor    eax, eax
    cmp    edi, 3
    sete    al
.L35:
    ret
```

Les deux premières paires `cmp` et `je` testent les valeurs de cas 400 et 9000, et le dernier cas (3) devient un `sete`. Avec une poignée de cas, une chaîne coûte aussi peu que n'importe quoi d'autre. En `-O0`, le même code utilise des paires `cmp / je` dans un ordre différent, mais la forme est la même.

**Une table de sauts** pour des valeurs de cas denses, chacune faisant quelque chose de différent :

```text
dispatch:
    mov    eax, edi
    mov    edi, esi
    cmp    eax, 4
    ja    .L44
    jmp    [QWORD PTR .L46[0+rax*8]]
.L46:
    .quad    .L50
    .quad    .L49
    .quad    .L48
    .quad    .L47
    .quad    .L45
.L47:
    jmp    act_d
.L45:
    jmp    act_e
.L50:
    jmp    act_a
.L49:
    jmp    act_b
.L48:
    jmp    act_c
.L44:
    mov    eax, -1
    ret
```

`cmp eax, 4 / ja .L44` est la **vérification de bornes** : si la valeur dépasse le plus grand cas, on va à `default`. L'instruction suivante est le `jmp [QWORD PTR .L46[0+rax*8]]` indirect, qui lit une adresse de 8 octets dans une table et saute par elle. La table `.L46` se trouve dans `.rodata` et contient cinq adresses, une par cas. Les cibles sont ici `jmp act_a` et ainsi de suite : chaque cas est un appel terminal. Dans un binaire, vous voyez `cmp reg, N / ja default / jmp qword ptr [reg*8 + table]`. Pour retrouver les cas, lisez la table dans le fichier : les entrées sont les cibles des cas dans l'ordre, et `N` + 1 est le nombre d'entrées.

En `-O0`, le même motif apparaît avec un `mov rax, QWORD PTR .L37[0+rax*8]` et `jmp rax` :

```text
dense:
    push    rbp
    mov    rbp, rsp
    mov    DWORD PTR [rbp-4], edi
    cmp    DWORD PTR [rbp-4], 5
    ja    .L35
    mov    eax, DWORD PTR [rbp-4]
    mov    rax, QWORD PTR .L37[0+rax*8]
    jmp    rax
.L37:
    .quad    .L42
    .quad    .L41
    .quad    .L40
    .quad    .L39
    .quad    .L38
    .quad    .L36
.L42:
    mov    eax, 10
    jmp    .L43
.L41:
    mov    eax, 20
    jmp    .L43
.L40:
    mov    eax, 35
    jmp    .L43
.L39:
    mov    eax, 41
    jmp    .L43
.L38:
    mov    eax, 52
    jmp    .L43
.L36:
    mov    eax, 63
    jmp    .L43
.L35:
    mov    eax, -1
.L43:
    pop    rbp
    ret
```

**Une table de correspondance**, quand chaque cas ne fait que renvoyer une constante :

```text
dense:
    mov    eax, -1
    cmp    edi, 5
    ja    .L32
    mov    edi, edi
    mov    eax, DWORD PTR CSWTCH.13[0+rdi*4]
.L32:
    ret
```

L'optimiseur a vu que `dense` renvoie une constante pour chaque valeur ; il a donc remplacé tout le `switch` par un tableau de résultats (`CSWTCH.13`) et un chargement. Il y a une vérification de bornes, mais pas de saut indirect. Vous trouverez le tableau dans `.rodata` : 10, 20, 35, 41, 52, 63.

**Une recherche dichotomique** pour de nombreux cas éparpillés :

```text
many:
    cmp    edi, 400
    je    .L52
    jg    .L53
    cmp    edi, 77
    je    .L54
    jg    .L55
    cmp    edi, 3
    je    .L56
    cmp    edi, 40
    jne    .L51
    mov    edi, 40
    jmp    act_b
.L53:
    cmp    edi, 2000
    je    .L60
    cmp    edi, 9000
    je    .L61
    cmp    edi, 900
    jne    .L51
    mov    edi, 901
    jmp    act_a
.L56:
    mov    edi, 3
    jmp    act_a
.L61:
    mov    edi, 9001
    jmp    act_c
.L55:
    cmp    edi, 150
    jne    .L51
    mov    edi, 150
    jmp    act_d
.L52:
    mov    edi, 400
    jmp    act_e
.L54:
    mov    edi, 77
    jmp    act_c
.L60:
    mov    edi, 2001
    jmp    act_b
.L51:
    xor    eax, eax
    ret
```

Le compilateur compare avec la valeur médiane (400), puis se branche en dessous ou au-dessus, en divisant par deux les candidats à chaque étape. Un arbre d'instructions `cmp / je / jg` dont les constantes sont triées et descendent sur plusieurs niveaux est un switch compilé en recherche dichotomique. Triez les constantes pour obtenir la liste des cas.

| Indice | Ce que c'est |
|---|---|
| Plusieurs `cmp reg, constant / je` à la suite | Une chaîne de comparaisons |
| `cmp reg, N / ja default` puis un `jmp` indirect via `[reg*8 + table]` | Une table de sauts à N + 1 entrées |
| `cmp reg, N / ja default` puis un chargement depuis un tableau | Une table de correspondance |
| `cmp` avec une constante médiane suivi de `je` et `jg`, répété | Une recherche dichotomique |

### Fonctions mises en ligne

Un compilateur optimiseur peut coller le corps d'une petite fonction dans ses appelants. L'appel, les arguments et le retour disparaissent, et le code se mêle à ce qui l'entoure.

```text
uses_helper:
    push    rbp
    mov    rbp, rsp
    push    rbx
    sub    rsp, 8
    mov    DWORD PTR [rbp-12], edi
    mov    eax, DWORD PTR [rbp-12]
    mov    edi, eax
    call    helper
    mov    ebx, eax
    mov    eax, DWORD PTR [rbp-12]
    add    eax, 1
    mov    edi, eax
    call    helper
    add    eax, ebx
    mov    rbx, QWORD PTR [rbp-8]
    leave
    ret
```

En `-O0`, `uses_helper` appelle `helper` deux fois. En `-O2`, les appels ont disparu :

```text
uses_helper:
    lea    eax, [rdi+rdi*2]
    lea    eax, [rax+5+rax]
    ret
```

L'expression `helper(v) + helper(v + 1)`, avec `helper(x) = x * 3 + 1`, a été simplifiée algébriquement en `6v + 5` et calculée avec deux instructions `lea`. Aucun appel ne subsiste, et aucune fonction nommée `helper` n'existe dans la sortie. Signes de mise en ligne : le même motif d'instructions répété dans plusieurs fonctions, et une fonction qui fait un travail relevant clairement d'une routine distincte. La tâche du rétro-ingénieur est de trouver la forme, de la nommer et de passer à la suite.

### Appels terminaux

Quand une fonction se termine par un appel dont elle renvoie le résultat tel quel, un compilateur optimiseur peut remplacer `call` et `ret` par un `jmp` :

```text
tail:
    add    edi, 1
    jmp    forwards
```

`tail` incrémente son argument et **saute** vers `forwards` ; `forwards` reviendra directement à l'appelant de `tail`. Voir un `jmp` vers une autre fonction à la fin d'une fonction, plutôt que vers une étiquette à l'intérieur, signifie un appel terminal. Dans la table de sauts ci-dessus, chaque cas se termine ainsi. Les appels terminaux cachent l'appelant dans les traces de pile et dans le graphe d'appels, si bien qu'une fonction peut sembler n'avoir aucun appelant alors qu'elle est utilisée.

## Essayez

1. Exécutez `make` et ouvrez `build/13_O0.s` et `build/13_O2.s`. Pour chaque fonction de `13_structs.c`, prédisez la forme avant de lire le listing, puis comparez.
2. Compilez `sparse` et `dense` avec `-O1`. Quelle stratégie chacune utilise-t-elle ?
3. Ajoutez à `13_structs.c` une fonction `int max3(int a, int b, int c)`, reconstruisez et trouvez les instructions `cmp` et `cmov` en `-O2`.
4. Dans `build/13_O2.s`, trouvez la table `.L46` utilisée par `dispatch` et associez chaque entrée au cas qu'elle sert.
5. Compilez `modules/08-stack-and-calls/examples/08_compiled.c` avec `gcc -O0 -S -masm=intel` et avec `-O2`, et classez le prologue et les variables locales de chaque version.

## Pièges courants

- **Lire la condition du saut comme la condition du source.** Le compilateur saute généralement par-dessus le code exécuté quand la condition est vraie ; le saut teste donc l'opposé.
- **Confondre sauts signés et non signés.** `jg`, `jl`, `jge`, `jle` sont signés ; `ja`, `jb`, `jae`, `jbe` sont non signés. Le choix indique le type de la variable.
- **Chercher la borne de boucle du source.** Une boucle optimisée peut décompter, compter par un pointeur ou se terminer sur un autre test que celui du source.
- **Supposer que chaque appel est dans le source.** La mise en ligne supprime des appels ; les compilateurs ajoutent aussi des appels à des routines auxiliaires.
- **Traiter un `jmp` vers une fonction comme un saut à l'intérieur de celle-ci.** C'est un appel terminal.

## Résumé

- Un prologue `push rbp; mov rbp, rsp` signifie un pointeur de cadre ; le code optimisé n'en a généralement pas et ne sauvegarde que les registres dont il a besoin.
- Un saut conditionnel évite le code exécuté quand la condition est vraie. `cmov` et `setcc` remplacent de petites branches.
- `&&` et `||` apparaissent comme plusieurs sauts conditionnels vers une étiquette commune ; le code optimisé peut les transformer en `setcc` et en logique.
- Un saut conditionnel vers l'arrière est une boucle. Les boucles optimisées sont pivotées, peuvent utiliser des pointeurs plutôt que des compteurs et peuvent sortir sur un autre test.
- Un `switch` est une chaîne de comparaisons, une table de sauts, une table de correspondance ou une recherche dichotomique ; les instructions avant le saut indiquent laquelle.
- La mise en ligne supprime des appels, et les appels terminaux transforment `call; ret` en `jmp`.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 14, l'arithmétique compilée.
- Aide-mémoire : [Idiomes du compilateur](../../cheatsheets/compiler-idioms.fr.md).
- Exercices qui utilisent ce module : [Exercice 09](../../exercises/09-stack-archaeology/README.fr.md), [Exercice 12](../../exercises/12-switchboard/README.fr.md), [Exercice 20](../../exercises/20-capstone-tiny-vm/README.fr.md).
