<!-- source: README.md @ sha256:f31f4f86ea8b -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 15. Récupérer les données

Durée : environ 3 heures. Prérequis : [14. Arithmétique compilée](../14-compiled-arithmetic/README.fr.md)

## Pourquoi c'est important

Le code machine n'a pas de types. Il n'y a ni tableaux, ni structures, ni classes, seulement des adresses, des décalages et des largeurs. Pourtant, le résultat le plus utile d'une séance de rétro-ingénierie est souvent une description des données : ce que contient un enregistrement, comment une liste est chaînée, quelle table pilote une décision. Ce module vous apprend à reconstruire ces types à partir de la façon dont le code utilise la mémoire : le facteur d'un indice donne la taille d'un élément, une constante ajoutée à un pointeur donne un champ, un chargement depuis le début d'un objet donne une table de fonctions. Il se termine par les objets C++, qui ajoutent un champ caché à tout.

## Concepts

### Comment les listings de ce module ont été produits

Le source C est `examples/15_data.c` et le source C++ est `examples/15_cpp.cpp`. Le `Makefile` les compile avec GCC et G++ en `-O1` et écrit de l'assembleur en syntaxe Intel dans `build/15_data.s` et `build/15_cpp.s`. Les listings sont ces fichiers privés des directives de l'assembleur. Tout le code est x86-64 System V ; le premier argument est dans `rdi`, le deuxième dans `rsi` et le résultat dans `eax` ou `rax`.

```bash
make
```

### Globales, locales et le tas

L'endroit où vit une variable détermine comment le code l'atteint.

**Une globale** a une adresse fixe. Dans cet exemple, construit sans code indépendant de la position, le compilateur écrit l'adresse dans l'instruction. Avec du code indépendant de la position, le même accès est relatif à `rip` :

```text
bump:
    mov    eax, DWORD PTR counter[rip]
    add    eax, 1
    mov    DWORD PTR counter[rip], eax
    ret
```

`counter[rip]` est une référence relative à `rip` vers la variable `counter`, qui vit dans la section `.data` parce qu'elle a une valeur initiale. Les globales non initialisées vivent dans `.bss`, les constantes dans `.rodata`. Une séquence de lecture, modification et écriture à la même adresse (charger, ajouter, ranger) correspond à `counter += 1`.

**Une locale** vit sur la pile ou dans un registre. Un tableau local a toujours une adresse, donc il vit en mémoire :

```text
local_sum:
    mov    DWORD PTR [rsp-24], edi
    lea    edx, [rdi+1]
    mov    DWORD PTR [rsp-20], edx
    lea    esi, [rdi+2]
    mov    DWORD PTR [rsp-16], esi
    lea    ecx, [rdi+3]
    mov    DWORD PTR [rsp-12], ecx
    lea    eax, [rdi+rdx]
    add    eax, esi
    add    eax, ecx
    and    edi, 3
    add    eax, DWORD PTR [rsp-24+rdi*4]
    ret
```

La fonction n'a ni pointeur de cadre ni `sub rsp` : elle range les quatre éléments de `buf` de `[rsp-24]` à `[rsp-12]`, sous le pointeur de pile, dans la **zone rouge** (red zone) du module 08. La boucle sur `i` a été entièrement déroulée, donc aucun compteur n'existe ; les éléments sont rangés à des décalages consécutifs espacés de 4 octets. L'accès final `[rsp-24+rdi*4]` est la lecture indexée `buf[n & 3]`.

**Le tas** est de la mémoire fournie par l'allocateur. Un appel à `malloc` avec une constante vous donne la taille de l'objet ; le code qui suit y range des valeurs par le pointeur renvoyé :

```text
push:
    push    rbp
    push    rbx
    sub    rsp, 8
    mov    rbx, rdi
    mov    ebp, esi
    mov    edi, 16
    call    malloc
    test    rax, rax
    je    .L30
    mov    DWORD PTR [rax], ebp
    mov    QWORD PTR [rax+8], rbx
.L28:
    add    rsp, 8
    pop    rbx
    pop    rbp
    ret
.L30:
    mov    rax, rbx
    jmp    .L28
```

`mov edi, 16` est `sizeof(struct node)`. Le pointeur renvoyé est dans `rax`. Les deux rangements suivants, `[rax]` (4 octets) et `[rax+8]` (8 octets), sont les champs `value` et `next`. Le `test rax, rax / je` est la vérification d'échec d'allocation, qui renvoie l'ancien `head`. Un `call malloc` (ou `operator new` en C++) suivi de rangements par le résultat signifie qu'un objet est en cours d'initialisation, et l'argument est sa taille.

### Tableaux et arithmétique de pointeurs

Un élément de tableau se trouve à `base + index * element_size`. Le mode d'adressage x86 `[base + index*scale]` l'exprime directement, et le **facteur** (scale) est la taille de l'élément quand elle vaut 1, 2, 4 ou 8.

```text
get:
    movsx    rdi, edi
    mov    eax, DWORD PTR table[0+rdi*4]
    ret
```

```text
get_local:
    movsx    rsi, esi
    mov    eax, DWORD PTR [rdi+rsi*4]
    ret
```

```text
get_long:
    movsx    rsi, esi
    mov    rax, QWORD PTR [rdi+rsi*8]
    ret
```

```text
get_char:
    movsx    rsi, esi
    movzx    eax, BYTE PTR [rdi+rsi]
    ret
```

Les quatre fonctions lisent `p[i]`, et seul le type de l'élément change l'instruction. `get` accède au tableau global `table` à une adresse fixe avec le facteur 4 (c'est un tableau de `int`). `get_local` prend la base dans `rdi` et multiplie par 4. `get_long` multiplie par 8 et charge 8 octets dans `rax`. `get_char` n'a pas de facteur (facteur 1) et charge un octet avec `movzx`. Le `movsx rsi, esi` du début étend avec signe l'indice 32 bits sur 64 bits. La taille du chargement (`BYTE`, `WORD`, `DWORD`, `QWORD`) et le facteur donnent ensemble le type de l'élément.

| Indice | Signification |
|---|---|
| `[base+index*4]` avec un chargement de 4 octets | Un tableau de `int` (ou `float`, `unsigned`) |
| `[base+index*8]` avec un chargement de 8 octets | Un tableau de `long`, de pointeurs ou de `double` |
| `[base+index]` avec un chargement d'un octet | Un tableau de `char` ou d'octets |
| Une base qui est une adresse fixe | Un tableau global ; sa taille figure dans la table des symboles ou dans la section |

Pour un **tableau à deux dimensions** ou un tableau de structures dont la taille n'est pas 1, 2, 4 ou 8, l'indice doit d'abord être multiplié :

```text
second_row:
    movsx    rsi, esi
    sal    rsi, 4
    add    rdi, rsi
    movsx    rdx, edx
    mov    eax, DWORD PTR [rdi+rdx*4]
    ret
```

`sal rsi, 4` multiplie l'indice de ligne par 16 : chaque ligne de `int m[][4]` compte 4 éléments de 4 octets. La ligne est ajoutée à la base, puis la colonne est multipliée par 4. La taille de ligne 16 est la largeur de la dimension intérieure ; le code ne mentionne jamais le nombre 4 comme dimension. Un tableau de structures fonctionne de la même façon, avec la taille de la structure comme pas :

```text
point_array:
    movsx    rsi, esi
    mov    eax, DWORD PTR [rdi+4+rsi*8]
    ret
```

`[rdi+4+rsi*8]` lit le champ `y` (décalage 4) de l'élément `i` d'un tableau de valeurs `struct point` de 8 octets. Le facteur 8 est la taille de la structure entière et le déplacement 4 est la position du champ en son sein. Les deux constantes portent des informations différentes.

La soustraction de pointeurs divise par la taille de l'élément :

```text
pointer_diff:
    sub    rsi, rdi
    sar    rsi, 2
    mov    eax, esi
    ret
```

`sub rsi, rdi` calcule la distance en octets, et `sar rsi, 2` la divise par 4 : le résultat est un nombre d'éléments `int`. Un décalage après la soustraction de deux pointeurs révèle la taille de l'élément.

### Structures

Une structure est un bloc de mémoire dans lequel chaque champ a un décalage fixe. Le code accède à un champ sous la forme `[pointer + offset]` ; aucune instruction ne mentionne le nom du champ. Vous retrouvez la disposition en collectant chaque `[reg + constant]` utilisé avec le même pointeur de base, avec la largeur de chaque accès.

```text
point_sum:
    mov    eax, DWORD PTR [rdi]
    add    eax, DWORD PTR [rdi+4]
    ret
```

`point_sum` lit les décalages 0 et 4, tous deux de 4 octets : une structure à deux champs `int`. Les compilateurs placent les champs à des adresses adaptées à leur taille, ce qui laisse du **remplissage** (padding) entre certains champs. Un exemple plus grand :

```text
record_id:
    mov    eax, DWORD PTR [rdi+4]
    ret
```

```text
record_flags:
    movzx    eax, WORD PTR [rdi+8]
    ret
```

```text
record_total:
    mov    rax, QWORD PTR [rdi+16]
    ret
```

```text
record_name:
    movsx    rsi, esi
    movzx    eax, BYTE PTR [rsi+24+rdi]
    ret
```

Voici ce que montrent ces fonctions d'accès pour `struct record`, rassemblé :

| Décalage | Largeur | Vu dans | Champ |
|---|---|---|---|
| 0 | 1 | (non lu ici) | `char tag` |
| 4 | 4 | `[rdi+4]`, `DWORD` | `int id` |
| 8 | 2 | `[rdi+8]`, `WORD` | `short flags` |
| 16 | 8 | `[rdi+16]`, `QWORD` | `long total` |
| 24 | 1 par élément | `[rsi+24+rdi]`, `BYTE` | `char name[6]` |

Les trous sont du remplissage. `id` est en 4 et non en 1 parce qu'un champ de 4 octets est aligné sur 4. `total` est en 16 et non en 10 parce qu'un champ de 8 octets est aligné sur 8. La structure se termine en 30 et est arrondie à un multiple de 8 : sa taille est 32. Le rapport du compilateur lui-même concorde (`offsetof` et `sizeof` donnent pour cette structure `tag 0 id 4 flags 8 total 16 name 24 size 32`). La taille totale est visible ailleurs dans le programme : comme argument de `malloc`, comme pas d'un tableau de telles structures ou comme quantité retranchée de `rsp` pour une locale.

Règles pour lire une structure :

- Toute constante ajoutée au même pointeur de base est un décalage de champ.
- La largeur du chargement ou du rangement, et le choix de `movzx` ou `movsx`, donnent le type du champ.
- Un trou entre des décalages est du remplissage ou un champ que ce code n'utilise pas.
- Un accès avec un registre d'indice en `[base + const + idx*scale]` est un tableau à l'intérieur de la structure (`name`) quand le facteur est la taille de l'élément.
- Deux fonctions qui utilisent des décalages différents à partir du même pointeur regardent le même objet ; fusionnez leurs champs.

### Listes chaînées

Un nœud de liste contient une valeur et un pointeur vers le nœud suivant. Le parcours est une boucle qui suit le pointeur jusqu'à ce qu'il soit nul.

```text
list_sum:
    test    rdi, rdi
    je    .L16
    mov    eax, 0
.L15:
    add    eax, DWORD PTR [rdi]
    mov    rdi, QWORD PTR [rdi+8]
    test    rdi, rdi
    jne    .L15
    ret
.L16:
    mov    eax, 0
    ret
```

La boucle ajoute `[rdi]` (un `value` de 4 octets) à la somme et remplace `rdi` par `[rdi+8]`, le pointeur `next` de 8 octets. Elle se répète tant que `rdi` n'est pas nul. La signature d'une liste chaînée est une boucle dans laquelle le registre utilisé comme base est **rechargé depuis la mémoire sur laquelle il pointe** : `mov rdi, [rdi+8]`. La structure est `{ int value; struct node *next; }`, avec 4 octets de remplissage après `value`. Le `test rdi, rdi / je` avant la boucle est la vérification de liste vide.

### Arbres

Un nœud d'arbre binaire a deux pointeurs vers ses enfants. Une recherche en suit un selon une comparaison.

```text
tree_has:
    test    rdi, rdi
    jne    .L22
    mov    eax, 0
    ret
.L20:
    mov    rdi, QWORD PTR [rdi+16]
.L21:
    test    rdi, rdi
    je    .L26
.L22:
    mov    eax, DWORD PTR [rdi]
    cmp    eax, esi
    je    .L24
    jle    .L20
    mov    rdi, QWORD PTR [rdi+8]
    jmp    .L21
.L26:
    mov    eax, 0
    ret
.L24:
    mov    eax, 1
    ret
```

Le pointeur de nœud est dans `rdi`. Le nœud contient `key` au décalage 0 et deux pointeurs aux décalages 8 et 16. Le code compare `[rdi]` à la clé recherchée `esi` : égal signifie trouvé ; `jle` (la clé du nœud est inférieure ou égale à la clé recherchée, donc la clé recherchée est plus grande) charge `[rdi+16]`, l'enfant droit ; sinon il charge `[rdi+8]`, l'enfant gauche. Deux champs de pointeur chargés alternativement, choisis par une comparaison avec la clé, forment un arbre binaire de recherche. L'enfant gauche est celui qu'on prend quand la clé recherchée est inférieure à la clé du nœud. Lire quel décalage est pris de chaque côté de la comparaison vous dit quel pointeur est `left` et lequel est `right`.

### Objets C++

Un objet C++ avec des fonctions virtuelles commence par un champ caché : le **pointeur de table virtuelle** (vptr), qui pointe vers une table d'adresses de fonctions, la **vtable**. Les champs de données de la classe suivent, ceux de la classe de base d'abord.

L'exemple `examples/15_cpp.cpp` a trois classes : `Shape`, avec les fonctions virtuelles `area` et `sides`, `Rect`, dérivée de `Shape`, et `Square`, dérivée de `Rect`. Les constructeurs sont mis en ligne dans la fonction qui crée les objets :

```text
_Z16make_and_measurev:
    push    rbx
    sub    rsp, 64
    mov    DWORD PTR [rsp+40], 1
    mov    QWORD PTR [rsp+32], OFFSET FLAT:_ZTV4Rect+16
    mov    DWORD PTR [rsp+44], 3
    mov    DWORD PTR [rsp+48], 4
    mov    DWORD PTR [rsp+8], 2
    mov    DWORD PTR [rsp+12], 5
    mov    DWORD PTR [rsp+16], 5
    mov    QWORD PTR [rsp], OFFSET FLAT:_ZTV6Square+16
    lea    rdi, [rsp+32]
    call    _Z10total_areaPK5Shape
    mov    ebx, eax
    mov    rdi, rsp
    call    _Z10total_areaPK5Shape
    add    eax, ebx
    add    rsp, 64
    pop    rbx
    ret
```

Regardez l'objet en `[rsp+32]`. Il commence par le rangement d'une adresse, `OFFSET _ZTV4Rect+16`, au décalage 0 : c'est le vptr, qui pointe 16 octets à l'intérieur de la vtable de `Rect`. Les rangements suivants sont `1` au décalage 8, `3` au décalage 12 et `4` au décalage 16. La disposition de `Rect` est donc :

| Décalage | Champ |
|---|---|
| 0 | vptr, caché |
| 8 | `int id`, issu de `Shape` |
| 12 | `int w` |
| 16 | `int h` |
| 20 | remplissage jusqu'à un multiple de 8 ; la taille est 24 |

La taille 24 est confirmée par le destructeur de suppression, qui la transmet à `operator delete` :

```text
_ZN4RectD0Ev:
    sub    rsp, 8
    mov    esi, 24
    call    _ZdlPvm
    add    rsp, 8
    ret
```

Le second objet, en `[rsp]`, est un `Square` : même disposition, avec le vptr pointant dans la vtable de `Square` et une largeur et une hauteur de 5.

La vtable elle-même est une donnée, dans `.rodata`. Avec les symboles et `c++filt` pour décoder (demangle) les noms, les deux tables sont :

```text
vtable for Rect (_ZTV4Rect)             vtable for Square (_ZTV6Square)
    0                                       0
    0                                       0
    Rect::~Rect()      (complete)           Square::~Square()  (complete)
    Rect::~Rect()      (deleting)           Square::~Square()  (deleting)
    Rect::area() const                      Rect::area() const
    Rect::sides() const                     Square::sides() const
```

Toute vtable commence par deux mots, le décalage vers le début de l'objet et un pointeur vers des informations de type (zéro ici, car l'exemple a été construit sans information de type à l'exécution, qu'un vrai binaire a généralement : le pointeur mène alors à une structure qui nomme la classe). Le vptr pointe juste après eux, de sorte que la première entrée adressée par le vptr est la première fonction virtuelle : l'emplacement 0 est le destructeur complet, l'emplacement 1 le destructeur de suppression, l'emplacement 2 `area`, l'emplacement 3 `sides`. **`Square` hérite de `area` de `Rect`**, donc l'emplacement 2 contient la même adresse dans les deux tables ; **`Square` redéfinit `sides`**, donc l'emplacement 3 diffère.

Un appel virtuel charge le vptr depuis l'objet, charge une adresse de fonction depuis la table et l'appelle avec le pointeur d'objet comme premier argument :

```text
_Z10total_areaPK5Shape:
    push    rbp
    push    rbx
    sub    rsp, 8
    mov    rbx, rdi
    mov    rax, QWORD PTR [rdi]
    call    [QWORD PTR [rax+16]]
    mov    ebp, eax
    mov    rax, QWORD PTR [rbx]
    mov    rdi, rbx
    call    [QWORD PTR [rax+24]]
    add    eax, ebp
    add    rsp, 8
    pop    rbx
    pop    rbp
    ret
```

`mov rax, [rdi]` charge le vptr, et `call [rax+16]` appelle la fonction de l'emplacement 2 (le décalage 16 correspond à 2 emplacements de 8 octets) : `area`. Le second appel, `call [rax+24]`, est l'emplacement 3 : `sides`. Le pointeur d'objet `this` reste dans `rdi` (il est sauvegardé dans `rbx` pendant le premier appel et remis en place). La fonction concrète qui s'exécute dépend de la vtable de l'objet. Un appel par un registre chargé depuis `[object]` plus une petite constante est un appel virtuel ; la constante divisée par 8 est le numéro d'emplacement.

Et une fonction membre non virtuelle est un appel ordinaire avec `this` comme premier argument :

```text
_ZNK4Rect4areaEv:
    mov    eax, DWORD PTR [rdi+12]
    imul    eax, DWORD PTR [rdi+16]
    ret
```

`Rect::area` lit `w` au décalage 12 et `h` au décalage 16 depuis `this` dans `rdi` et les multiplie. Rien dans le code n'indique que c'est une fonction membre, hormis le nom décoré (mangled) et les décalages à partir du pointeur.

| Indice | Signification |
|---|---|
| Un rangement d'une adresse de la section de code dans `[obj]` à la construction | Le vptr ; l'adresse est une vtable plus 16 |
| `mov rax, [rdi]` puis `call [rax+N]` | Un appel virtuel à l'emplacement `N / 8` |
| Une table d'adresses de fonctions dans `.rodata`, précédée de deux mots | Une vtable |
| Deux tables qui partagent la plupart des entrées | Une classe de base et une classe dérivée ; les différences sont les redéfinitions |
| Le premier argument `rdi` utilisé comme base dans chaque fonction d'un groupe | Un ensemble de fonctions membres d'une classe |

### Construire des types dans Ghidra

Dès que vous connaissez une disposition, enregistrez-la pour que le décompilateur puisse l'utiliser.

1. Ouvrez le Data Type Manager et créez une **structure**. Ajoutez les champs avec leurs décalages, tailles et noms. Utilisez des champs de remplissage « undefined » pour atteindre les bons décalages.
2. Dans le décompilateur, sélectionnez une variable qui contient un pointeur vers l'objet et changez son type (touche par défaut `Ctrl+L`) en pointeur vers votre structure. Le décompilateur réécrit `*(int *)(p + 4)` en `p->id`.
3. Pour une classe avec des fonctions virtuelles, créez une structure pour la vtable dont les champs sont des pointeurs de fonction, puis faites du premier champ de l'objet un pointeur vers elle. Le décompilateur affiche alors les appels virtuels sous la forme `obj->vtable->area(obj)`.
4. Renommez les fonctions au fur et à mesure que vous comprenez ce qu'elles font et fixez leur premier paramètre au pointeur de classe.

Les types se propagent : dès qu'une fonction utilise la structure, toutes les fonctions qui reçoivent le même pointeur deviennent plus lisibles.

## Essayez

1. Exécutez `make` et ouvrez `build/15_data.s`. Pour `record_name`, déterminez ce que signifient le déplacement 24 et l'absence de facteur.
2. Avec `readelf -S -W build/15_data.o` et `nm build/15_data.o`, trouvez quelle section contient `table` et `counter`, et quelle est la taille de `table`.
3. Ajoutez un champ `char pad` après `flags` dans `struct record` et prédisez quels décalages changent avant de reconstruire. Puis reconstruisez et vérifiez.
4. Écrivez une fonction C qui renvoie la clé la plus à droite d'un `struct tree` et lisez la boucle dans la sortie compilée.
5. Exécutez `nm -C build/15_cpp.o` et `objdump -d -M intel -C build/15_cpp.o`. Trouvez `Rect::area() const` et les deux tables. Ajoutez une nouvelle fonction virtuelle à `Shape`, reconstruisez et regardez quels numéros d'emplacement changent.

## Pièges courants

- **Lire le remplissage comme un champ.** Un trou entre des décalages peut être de l'alignement ou un champ que cette fonction n'utilise pas. Attendez la confirmation d'une autre fonction.
- **Confondre le facteur avec la taille de la structure.** Un facteur de 8 peut être un tableau de structures de 8 octets ou un tableau de `long`. Le déplacement et la largeur de l'accès font la différence.
- **Manquer le vptr caché.** Dans une classe C++ avec des fonctions virtuelles, le décalage 0 est le vptr et le premier membre est au décalage 8.
- **Oublier que `this` est le premier argument.** Dans une fonction membre, `rdi` est le pointeur d'objet et les arguments visibles commencent en `rsi`.
- **Traiter tout suivi de pointeur comme une liste.** Une boucle qui recharge son registre de base depuis l'objet est une liste, mais une boucle qui recharge depuis `[rdi+8]` ou `[rdi+16]` selon une comparaison est un arbre.

## Résumé

- Le facteur d'un indice et la largeur d'un chargement donnent le type d'un élément de tableau ; une constante ajoutée à un pointeur est un champ.
- Une structure est un ensemble de décalages et de largeurs collectés sur tous les accès, avec du remplissage entre eux ; sa taille est visible dans les allocations, les pas et l'espace de pile.
- Une liste chaînée est une boucle qui recharge son registre de base depuis le nœud sur lequel il pointe ; un arbre est la même chose avec deux pointeurs choisis par une comparaison.
- Un objet C++ avec des fonctions virtuelles commence par un vptr ; un appel virtuel est `mov rax, [obj]` puis `call [rax+8*slot]`.
- Noter la disposition retrouvée sous forme de type accélère le reste de l'analyse.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 16.
- Aide-mémoire : [Idiomes du compilateur](../../cheatsheets/compiler-idioms.fr.md).
- Exercices qui utilisent ce module : [Exercice 14](../../exercises/14-shape-shifter/README.fr.md), [Exercice 15](../../exercises/15-linked-inventory/README.fr.md), [Exercice 20](../../exercises/20-capstone-tiny-vm/README.fr.md).
