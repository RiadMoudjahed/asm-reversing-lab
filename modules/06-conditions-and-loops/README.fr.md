<!-- source: README.md @ sha256:38fd580b0f74 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 06. Conditions, branchements et boucles

Durée : environ 2 heures. Prérequis : [05. Arithmétique et logique](../05-arithmetic-and-logic/README.fr.md)

## Pourquoi c'est important

L'assembleur n'a ni `if`, ni `while`, ni `for`. Chaque décision est une comparaison qui positionne des indicateurs, suivie d'un saut qui les lit. Une fois que vous savez écrire ces schémas à la main, vous pouvez les reconnaître dans du code compilé et retrouver les conditions, les bornes et la structure des boucles d'un programme dont vous n'avez pas le source. L'erreur la plus courante, choisir un saut signé là où le code est non signé, est aussi l'une des lectures erronées les plus fréquentes en rétro-ingénierie.

## Notions

### Comparer sans rien modifier

| Instruction | Calcule | Conserve |
|---|---|---|
| `cmp a, b` | `a - b` | Uniquement les indicateurs |
| `test a, b` | `a AND b` | Uniquement les indicateurs |

`cmp` est une soustraction dont le résultat est jeté. `test` est un `and` dont le résultat est jeté. `test rax, rax` est la manière habituelle de demander si `rax` est nul ou négatif, car le résultat de `rax AND rax` est `rax` lui-même.

### Sauts conditionnels

`jmp label` saute toujours. Un saut conditionnel ne saute que lorsque sa condition est vraie ; sinon l'exécution continue avec l'instruction suivante. Après `cmp a, b`, les noms des sauts se lisent comme une comparaison de `a` avec `b` :

| Condition | Signé | Non signé |
|---|---|---|
| égal | `je` (`jz`) | `je` (`jz`) |
| différent | `jne` (`jnz`) | `jne` (`jnz`) |
| inférieur | `jl` | `jb` (« below ») |
| inférieur ou égal | `jle` | `jbe` |
| supérieur | `jg` | `ja` (« above ») |
| supérieur ou égal | `jge` | `jae` |

L'égalité ne dépend pas de l'interprétation ; l'ordre en dépend. `jl`, `jle`, `jg` et `jge` lisent SF et OF (et ZF), qui décrivent un résultat signé. `jb`, `jbe`, `ja` et `jae` lisent CF (et ZF), qui décrivent un résultat non signé. Le module 02 a expliqué pourquoi : CF est l'indicateur de débordement non signé et OF celui du signé. Certains sauts lisent un seul indicateur : `js` et `jns` (signe), `jo` et `jno` (débordement), `jc` et `jnc` (retenue ; identiques à `jb` et `jae`), `jp` et `jnp` (parité).

### Fixer ou choisir une valeur sans sauter

| Instruction | Effet |
|---|---|
| `setcc r8` | Met un registre de 8 bits à 1 si la condition est vraie, sinon à 0 (`setg`, `setl`, `sete`, `setz`, `setc`, etc.) |
| `cmovcc r, source` | Copie la source dans le registre seulement si la condition est vraie (`cmovl`, `cmovge`, `cmovb`, etc.) |

Les deux lisent les indicateurs exactement comme un saut conditionnel, et aucune ne les modifie. `setcc` n'écrit que 8 bits : le reste du registre garde son ancienne valeur ; mettez d'abord le registre à zéro, et faites-le avant la comparaison, car `xor` modifie les indicateurs. Ces instructions évitent un branchement, ce que les compilateurs apprécient pour les conditions courtes.

### Construire les structures habituelles

**if / else.** Testez, sautez par-dessus le bloc à ignorer, et sautez par-dessus la partie else à la fin du premier bloc :

```nasm-fragment
    cmp     rdi, rsi
    jle     .else               ; condition false: go to the else part
    ; ... then part ...
    jmp     .end
.else:
    ; ... else part ...
.end:
```

**while.** Le test vient d'abord : le corps peut donc ne jamais s'exécuter :

```nasm-fragment
.while:
    cmp     rcx, rdi
    jg      .done               ; condition false: leave the loop
    ; ... body ...
    jmp     .while
.done:
```

**do-while.** Le test vient en dernier : le corps s'exécute donc au moins une fois. C'est la forme voulue pour compter les chiffres d'un nombre, puisque 0 a un chiffre :

```nasm-fragment
.again:
    ; ... body ...
    test    rax, rax
    jnz     .again              ; condition true: go around again
```

**for.** Une initialisation, un test et un pas, ce qui est une boucle while dont le pas se trouve à la fin du corps. Des versions complètes de tout cela se trouvent dans [06_control.asm](examples/06_control.asm).

### L'instruction `loop`

`loop label` retire 1 de `rcx` et saute si le résultat n'est pas nul. Elle paraît pratique, mais sur la plupart des processeurs elle est plus lente que `dec rcx` suivi de `jnz`, et les compilateurs ne l'émettent pas. Elle comporte aussi un piège : si `rcx` vaut 0 à l'entrée, elle soustrait d'abord, obtient une énorme valeur non nulle et s'exécute 2^64 fois.

### Le point de vue de la rétro-ingénierie

Les compilateurs ne vous rendent pas vos boucles sous la forme où vous les avez écrites. Exécutez `make compiled` dans le dossier des exemples. Ce sont de vrais extraits :

```text
<max_signed>:     cmp    rsi,rdi
                  mov    rax,rdi
                  cmovge rax,rsi
                  ret

<sum_to>:         test   rdi,rdi
                  jle    <skip>               ; n <= 0: do not enter the loop
                  add    rdi,0x1
                  mov    eax,0x1
                  mov    edx,0x0
                  add    rdx,rax              ; the loop body
                  add    rax,0x1
                  cmp    rax,rdi
                  jne    <body>               ; the test is at the bottom
                  mov    rax,rdx
                  ret

<sum_array>:      test   rsi,rsi
                  jle    <skip>
                  mov    rax,rdi
                  lea    rsi,[rdi+rsi*4]      ; end pointer
                  mov    edx,0x0
                  movsxd rcx,DWORD PTR [rax]  ; the body walks a pointer, no index
                  add    rdx,rcx
                  add    rax,0x4
                  cmp    rax,rsi
                  jne    <body>
```

Ce qu'on peut en lire :

- **Opérandes échangés.** `max_signed` compare `rsi` à `rdi` et utilise `cmovge` : « si b >= a, prendre b ». La condition est réécrite, mais une condition signée (`ge`) vous apprend que le type est signé. `max_unsigned` donne le même code avec `cmovae`.
- **Inversion de boucle.** Une garde (`test` et `jle`) décide si l'on entre dans la boucle, et le vrai test se trouve en bas avec un saut en arrière. Une boucle avec un saut conditionnel en arrière est la signature de `for` et de `while` dans du code optimisé.
- **Compteurs réécrits.** `sum_to` fait courir un compteur jusqu'à `n + 1` avec `jne`, et `sum_array` n'a aucun indice : il parcourt un pointeur jusqu'à un pointeur de fin. La boucle est équivalente à la vôtre, pas identique.
- **Le saut révèle le type.** `jl` ou `jg` signifie signé ; `jb` ou `ja` signifie non signé.

## À vous de jouer

Construisez les exemples et exécutez-les :

```bash
cd modules/06-conditions-and-loops/examples
make run
```

Sortie attendue :

```text
max_signed(-1, 1)        = 1
max_unsigned(-1, 1)      = 18446744073709551615
sign_of(-7), (0), (12)   = -1 0 1
sign_branchless same     = -1 0 1
sum_to(10), sum_to(0)    = 55 0
count_digits(0), (9), (12345) = 1 1 5
sum_array({4,-9,20,1}, 4) = 16
is_power_of_two(64), (96), (0) = 1 0 0
```

Les deux premières lignes comparent les mêmes bits, -1 et 1, et donnent des réponses différentes : en signé, -1 est plus petit ; en non signé, -1 est la plus grande valeur qui existe. C'est la différence entre `cmovl` et `cmovb`.

Modifiez ensuite quelque chose et prédisez le résultat avant de reconstruire :

1. Dans `sum_to`, changez `jg .done` en `jge .done`. Que renvoie `sum_to(10)` ?
2. Dans `max_unsigned`, changez `cmovb` en `cmovl`. Quel appel change de réponse ?

## Pièges courants

- **Un saut signé sur des données non signées, ou l'inverse.** `jl` sur une longueur non signée se comporte mal dès que le bit de poids fort est positionné. Décidez d'abord du type, puis choisissez `jl` ou `jb`.
- **L'ordre des opérandes de `cmp`.** `cmp a, b` suivi de `jg` saute lorsque `a` est supérieur à `b`. Lu à l'envers, il donne la condition opposée.
- **Modifier les indicateurs entre la comparaison et le saut.** Un `add` ou un `xor` intercalé remplace les indicateurs. `mov` et `lea` sont sans danger.
- **Mettre un registre à zéro après la comparaison.** `xor eax, eax` entre `cmp` et `setg al` détruit les indicateurs. Mettez à zéro d'abord, comparez ensuite.
- **Erreur d'une unité.** `jg` et `jge` diffèrent d'un tour de boucle. Testez la première et la dernière valeur.
- **Oublier le pas.** Une boucle qui n'incrémente jamais son compteur ne se termine jamais.
- **`loop` avec `rcx` égal à 0.** Elle s'exécute 2^64 fois.

## Résumé

- `cmp` et `test` positionnent des indicateurs et jettent le résultat ; les sauts conditionnels, `setcc` et `cmovcc` lisent les indicateurs.
- Les comparaisons signées utilisent `jl`, `jle`, `jg`, `jge` ; les non signées `jb`, `jbe`, `ja`, `jae` ; l'égalité est la même pour les deux.
- `if`, `while`, `do-while` et `for` sont tous une comparaison et un saut ; la position du test décide du nombre d'exécutions du corps.
- Les compilateurs placent le test en bas des boucles, réécrivent les compteurs en pointeurs et remplacent les branchements courts par `cmov` ou `setcc`.
- La condition du saut dans du code compilé vous dit si le type d'origine était signé ou non signé.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 07, chaînes et instructions de chaîne.
- Exercices qui utilisent ce module : [Exercice 05](../../exercises/05-ticket-counter/README.fr.md), [Exercice 06](../../exercises/06-branch-maze/README.fr.md), [Exercice 07](../../exercises/07-string-sweeper/README.fr.md).
