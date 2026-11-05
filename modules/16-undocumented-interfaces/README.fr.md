<!-- source: README.md @ sha256:d99c551debae -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 16. Interfaces non documentées

Durée : environ 2 heures. Prérequis : [15. Récupérer les données](../15-recovering-data/README.fr.md)

## Pourquoi c'est important

Une bonne part du code que vous rencontrez arrive sous forme de bibliothèque sans en-tête ni documentation : un binaire d'éditeur, un ancien composant dont le source est perdu, une interface de greffon. Pour l'appeler, le remplacer ou vérifier ce qu'il fait, il vous faut son interface : les noms, les arguments, la valeur de retour et les règles d'emploi. Le binaire contient tout cela. Ce module enseigne une méthode pour retrouver une interface à partir du code compilé, et pour confirmer le résultat avec un programme de test avant de vous y fier.

## Concepts

### Ce que l'on voit de l'extérieur

Une bibliothèque partagée énumère les fonctions qu'elle offre aux autres dans sa table de symboles dynamiques. Cette liste est la liste des **exportations**, et c'est la première chose à lire. L'image du laboratoire construit la bibliothèque d'entraînement `libblackbox.so` à partir de `examples/16_blackbox.c` ; la leçon ne travaille que sur le binaire.

```bash
make
nm -D build/libblackbox.so
```

```text
                 w _ITM_deregisterTMCloneTable
                 w _ITM_registerTMCloneTable
                 w __cxa_finalize@GLIBC_2.2.5
                 w __gmon_start__
0000000000001126 T bb_checksum
0000000000001119 T bb_clamp
000000000000121f T bb_close
00000000000011dc T bb_lerp
00000000000011a5 T bb_minmax
0000000000001213 T bb_next
00000000000011e9 T bb_open
0000000000001145 T bb_split
                 U calloc@GLIBC_2.2.5
                 U free@GLIBC_2.2.5
```

`T` marque une fonction que la bibliothèque définit et `U` une dont elle a besoin ailleurs. Les noms sont déjà un indice : `bb_open`, `bb_next` et `bb_close` ressemblent à un ensemble, et `calloc` et `free` figurent dans la liste des symboles non définis. Les noms peuvent tromper ; traitez chacun comme une hypothèse, jamais comme un fait. Les importations sont aussi un bon indice : une bibliothèque qui importe `calloc` alloue de la mémoire quelque part.

### La signature que vous cherchez

Pour une bibliothèque C, la **signature** d'une fonction est son type de retour, son nom et les types de ses paramètres. Le processeur ne garde pas les types ; vous les reconstruisez donc d'après la façon dont les instructions traitent chaque registre. Sur x86-64 System V, les premiers arguments entiers ou pointeurs arrivent dans `rdi`, `rsi`, `rdx`, `rcx`, `r8` et `r9`, les arguments flottants dans `xmm0` à `xmm7`, et le résultat sort dans `rax` ou `xmm0`. La méthode pose cinq questions à chaque fonction.

| Question | Indices à chercher |
|---|---|
| Combien d'arguments ? | Registres d'arguments lus avant d'être écrits. Le plus élevé qui est lu donne le nombre minimal d'arguments |
| Quelle largeur chacun ? | `edi` fait 32 bits, `rdi` 64 bits, `dil`, `sil` ou `r8b` 8 bits. La largeur de l'opérande suit le type C |
| Est-ce un pointeur ? | Le registre est utilisé entre crochets, `[rdi]`, ou ajouté à un autre registre comme adresse |
| Vers quoi pointe-t-il ? | La largeur de l'accès : `BYTE PTR`, `DWORD PTR`, `QWORD PTR` ; le facteur d'un indice ; `movzx` ou `movsx` après un chargement |
| Signé ou non signé ? | `jl`, `jg`, `cmovg`, `movsx`, `sar` pour le signé ; `jb`, `ja`, `movzx`, `shr` pour le non signé |
| Que renvoie-t-elle ? | La dernière écriture dans `eax`, `rax` ou `xmm0` avant `ret`, et sa largeur. Si aucun chemin n'écrit le registre, la fonction ne renvoie rien |

Traitez le résultat comme une hypothèse tant qu'un test ne l'a pas confirmé. Une fonction void peut laisser une valeur sans rapport dans `rax`, et un argument que la fonction ignore ne laisse aucune trace.

### Exemple 1 : une fonction à valeurs simples

```bash
objdump -d -M intel --no-show-raw-insn build/libblackbox.so | sed -n '/ 1119:/,/ 1125:/p'
```

```text
    1119:    mov    eax,esi
    111b:    cmp    edi,edx
    111d:    cmovle edx,edi
    1120:    cmp    edi,esi
    1122:    cmovge eax,edx
    1125:    ret
```

La fonction lit `edi`, `esi` et `edx` et écrit `eax` : trois arguments de 32 bits et un résultat de 32 bits. Toutes les comparaisons sont signées (`cmovle`, `cmovge`), donc les valeurs sont des `int`. Suivez la logique. Le résultat commence comme `esi`. Le premier `cmp` compare `edi` avec `edx`, et lorsque `edi` est inférieur ou égal à `edx`, il remplace `edx` par `edi` ; `edx` devient donc le plus petit de `edi` et `edx`. Si `edi` est supérieur ou égal à `esi`, le résultat devient cette valeur bornée ; sinon il reste `esi`. La conclusion : `esi` est une borne inférieure, `edx` une borne supérieure, et `edi` la valeur que l'on borne.

```c
int bb_clamp(int v, int lo, int hi);
```

Les noms `v`, `lo` et `hi` sont les vôtres. Choisir des noms qui décrivent le rôle fait partie du travail.

### Exemple 2 : un tampon et une longueur

```text
0000000000001126 <bb_checksum>:
    1126:    mov    eax,edx
    1128:    test   rsi,rsi
    112b:    je     1144
    112d:    mov    rdx,rdi
    1130:    add    rdi,rsi
    1133:    rol    eax,0x5
    1136:    movzx  ecx,BYTE PTR [rdx]
    1139:    xor    eax,ecx
    113b:    add    rdx,0x1
    113f:    cmp    rdx,rdi
    1142:    jne    1133
    1144:    ret
```

- `rdi` est utilisé comme adresse et on lui ajoute `rsi` pour obtenir une adresse de fin : `rdi` est un pointeur et `rsi` un nombre. Le nombre est testé comme une valeur de 64 bits (`test rsi,rsi`), c'est donc un `size_t`, pas un `int`.
- Chaque tour charge un octet avec `movzx ecx,BYTE PTR [rdx]` et l'étend avec des zéros. L'extension par des zéros indique que le type de l'élément est `unsigned char`. Le code ne fait que lire le tampon, le pointeur est donc `const`.
- `eax` commence comme `edx` : le troisième argument est une valeur initiale de 32 bits. Chaque tour fait tourner `eax` de 5 bits vers la gauche et y mêle l'octet par un xor. Le résultat est dans `eax`.

```c-fragment
unsigned bb_checksum(const unsigned char *p, size_t n, unsigned seed);
```

Le comportement est une hypothèse que vous pouvez énoncer exactement : partir de `seed`, puis pour chaque octet faire tourner de 5 bits vers la gauche et mêler l'octet par un xor. Vous la testerez plus tard avec un programme indépendant.

### Exemple 3 : une valeur de retour qui est un code d'erreur

```text
0000000000001145 <bb_split>:
    1145:    mov    r9d,esi
    1148:    test   rcx,rcx
    114b:    je     1195
    114d:    movzx  r8d,BYTE PTR [rdi]
    1151:    cmp    r8b,sil
    1154:    je     118b
    1156:    test   r8b,r8b
    1159:    je     118b
    115b:    cmp    rcx,0x1
    115f:    jbe    119d
    1161:    mov    eax,0x1
    1166:    mov    BYTE PTR [rdx+rax*1-0x1],r8b
    116b:    movzx  r8d,BYTE PTR [rdi+rax*1]
    1170:    cmp    r8b,r9b
    1173:    je     1190
    1175:    test   r8b,r8b
    1178:    je     1190
    117a:    add    rax,0x1
    117e:    cmp    rcx,rax
    1181:    jne    1166
    1183:    mov    rax,0xffffffffffffffff
    118a:    ret
    118b:    mov    eax,0x0
    1190:    mov    BYTE PTR [rdx+rax*1],0x0
    1194:    ret
    1195:    mov    rax,0xffffffffffffffff
    119c:    ret
    119d:    mov    rax,0xffffffffffffffff
    11a4:    ret
```

- Quatre arguments : `rdi`, `sil` (l'octet bas de `esi`), `rdx` et `rcx` sont lus. `sil` est comparé à des octets (`cmp r8b,sil`), le deuxième argument est donc un `char`.
- `rdi` est lu octet par octet jusqu'à ce qu'un octet soit égal à `sil` ou nul : une chaîne source et un séparateur.
- `rdx` est écrit octet par octet (`mov BYTE PTR [rdx+rax*1-0x1],r8b`) et se termine par un octet nul (`mov BYTE PTR [rdx+rax*1],0x0`) : un tampon de destination qui reçoit une chaîne.
- `rcx` est comparé à la position et borne la boucle : la capacité de la destination, de type `size_t`.
- Le résultat est fixé par `mov rax,0xffffffffffffffff` sur trois chemins. C'est un `-1` de 64 bits, le type de retour est donc `long` (ou `ssize_t`), et les trois chemins sont les cas d'erreur : capacité nulle, capacité de un avec un champ non vide, et champ source qui ne tient pas. En cas de succès, le résultat est le nombre de caractères copiés.

```c-fragment
long bb_split(const char *s, char sep, char *out, size_t cap);
```

Deux détails ne viennent que d'une lecture attentive : la capacité inclut l'octet nul final, et en cas d'échec la fonction peut avoir déjà écrit une partie de la sortie. Un test vérifiera les deux.

### Exemple 4 : paramètres de sortie

```text
00000000000011a5 <bb_minmax>:
    11a5:    mov    r9d,DWORD PTR [rdi]
    11a8:    cmp    rsi,0x1
    11ac:    jbe    11d7
    11ae:    lea    r8,[rdi+0x4]
    11b2:    lea    rdi,[rdi+rsi*4]
    11b6:    mov    esi,r9d
    11b9:    mov    eax,DWORD PTR [r8]
    11bc:    cmp    r9d,eax
    11bf:    cmovg  r9d,eax
    11c3:    cmp    esi,eax
    11c5:    cmovl  esi,eax
    11c8:    add    r8,0x4
    11cc:    cmp    r8,rdi
    11cf:    jne    11b9
    11d1:    mov    DWORD PTR [rdx],r9d
    11d4:    mov    DWORD PTR [rcx],esi
    11d6:    ret
    11d7:    mov    esi,r9d
    11da:    jmp    11d1
```

- `rdi` pointe vers des éléments de 4 octets, et `rsi` est le nombre d'éléments (l'adresse de fin est `rdi+rsi*4`) : `const int *a, size_t n`.
- `rdx` et `rcx` ne sont pas lus ; les dernières instructions écrivent dans `[rdx]` et `[rcx]`. Un pointeur à travers lequel la fonction ne fait qu'écrire est un **paramètre de sortie** : l'appelant passe l'adresse d'une variable et y trouve la valeur.
- Aucun chemin n'écrit `eax` comme résultat, la fonction ne renvoie donc rien : `void`. (`eax` sert de registre de travail dans la boucle.)
- La première instruction lit `[rdi]` avant que le nombre soit vérifié. La fonction exige donc `n` d'au moins 1, une règle d'emploi qu'aucun type ne montre.

```c-fragment
void bb_minmax(const int *a, size_t n, int *lo, int *hi);
```

### Exemple 5 : valeurs flottantes

```text
00000000000011dc <bb_lerp>:
    11dc:    subsd  xmm1,xmm0
    11e0:    mulsd  xmm1,xmm2
    11e4:    addsd  xmm0,xmm1
    11e8:    ret
```

Le suffixe `sd` signifie scalaire double. La fonction lit `xmm0`, `xmm1` et `xmm2` et renvoie dans `xmm0`. Avec `a` dans `xmm0`, `b` dans `xmm1` et `t` dans `xmm2`, le code calcule `a + (b - a) * t`. Les registres entiers n'interviennent pas ; la signature n'a donc aucun argument entier. Un suffixe `ss` signifierait `float`.

```c
double bb_lerp(double a, double b, double t);
```

### Exemple 6 : un descripteur avec une durée de vie

Trois fonctions travaillent ensemble.

```text
00000000000011e9 <bb_open>:
    11e9:    push   rbp
    11ea:    push   rbx
    11eb:    sub    rsp,0x8
    11ef:    mov    ebp,edi
    11f1:    mov    ebx,esi
    11f3:    mov    esi,0xc
    11f8:    mov    edi,0x1
    11fd:    call   1040 <calloc@plt>
    1202:    test   rax,rax
    1205:    je     120c
    1207:    mov    DWORD PTR [rax],ebp
    1209:    mov    DWORD PTR [rax+0x4],ebx
    120c:    add    rsp,0x8
    1210:    pop    rbx
    1211:    pop    rbp
    1212:    ret

0000000000001213 <bb_next>:
    1213:    add    DWORD PTR [rdi+0x8],0x1
    1217:    mov    eax,DWORD PTR [rdi+0x4]
    121a:    add    eax,DWORD PTR [rdi]
    121c:    mov    DWORD PTR [rdi],eax
    121e:    ret

000000000000121f <bb_close>:
    121f:    sub    rsp,0x8
    1223:    call   1030 <free@plt>
    1228:    add    rsp,0x8
    122c:    ret
```

- `bb_open` prend deux valeurs de 32 bits, appelle `calloc(1, 12)` et range les deux valeurs dans le nouveau bloc aux décalages 0 et 4. Elle renvoie le bloc, ou null quand l'allocation échoue (la branche `test rax,rax` saute les écritures). Le bloc fait 12 octets.
- `bb_next` prend le bloc, incrémente le champ de 32 bits au décalage 8, ajoute le champ du décalage 4 au champ du décalage 0 et renvoie la nouvelle valeur du champ du décalage 0.
- `bb_close` passe son argument à `free`.

Le bloc est un **descripteur opaque** : l'appelant reçoit un pointeur, le repasse à d'autres fonctions et n'a jamais besoin de connaître la disposition. De l'extérieur, son type est `void *`. De l'intérieur, vous avez retrouvé la disposition : une valeur courante en 0, un pas en 4 et un compteur d'appels en 8. Les règles d'emploi viennent de l'ensemble : chaque descripteur issu de `bb_open` doit aller à `bb_close` exactement une fois, un descripteur ne doit pas servir après sa fermeture, et un résultat nul de `bb_open` doit être vérifié.

```c
void *bb_open(int start, int step);
int bb_next(void *c);
void bb_close(void *c);
```

### Les sites d'appel sont des indices

Quand un programme utilise la bibliothèque, le code autour de chaque appel montre comment le résultat est utilisé. Le fichier `examples/16_caller.c` appelle `bb_split` et signale un échec. Compilé avec `-O1` :

```text
copy_key:
    sub    rsp, 8
    mov    rdx, rsi
    mov    ecx, 32
    mov    esi, 61
    call    bb_split@PLT
    test    rax, rax
    mov    rdx, -1
    cmovs    rax, rdx
    add    rsp, 8
    ret
```

L'appelant passe son deuxième paramètre (`rsi`) comme troisième argument, la constante 32 comme capacité et le caractère 61, qui est `=`, comme séparateur. Après l'appel, `test rax,rax` et `cmovs` regardent le signe des 64 bits de `rax` : le résultat est une valeur signée de 64 bits, et une valeur négative signifie un échec. Si vous avez un programme qui utilise une bibliothèque, cherchez d'abord les appels. Les constantes, les tests après chaque appel et l'ordre des appels disent ce que l'auteur attendait.

### Confirmer avec un programme de test

Une hypothèse devient une connaissance quand un programme écrit à partir d'elle se comporte comme prévu. Écrivez les prototypes retrouvés, appelez les fonctions avec des valeurs normales, puis avec les limites, et comparez avec ce que vous aviez prédit avant l'exécution. Le fichier `examples/16_probe.c` le fait pour la bibliothèque d'entraînement :

```bash
make run
```

```text
clamp(5, 0, 3) = 3
clamp(-5, 0, 3) = 0
clamp(2, 0, 3) = 2
checksum("abc", 3, 0) = 0x00018823
checksum("abc", 3, 1) = 0x00010823
checksum("abc", 2, 0) = 0x00000c42
split("key=value", '=', out, 8) = 3, out = "key"
split("abcdefghij", ',', out, 8) = -1
split("abcdefg", ',', out, 8) = 7
minmax = -3 12
lerp(10.0, 20.0, 0.25) = 12.50
next = 105
next = 110
next = 115
counter bytes: 73 00 00 00 05 00 00 00 03 00 00 00 
```

Lisez les résultats comme des tests des hypothèses. `clamp` borne des deux côtés. L'appel de `split` avec une capacité de 8 échoue pour un champ de 10 caractères et réussit pour un champ de 7, donc un champ de 7 caractères plus le terminateur tient exactement dans 8 octets ; cela confirme que la capacité inclut l'octet nul. La dernière ligne affiche le descripteur : la valeur est 0x73 (115), le pas est 5 et le compteur est 3, la disposition retrouvée à partir de `bb_open` et `bb_next`.

Pour la somme de contrôle, l'hypothèse est assez précise pour être testée avec une implémentation indépendante dans un autre langage. Le script `examples/16_checksum.py` suit la règle « tourner de 5 bits vers la gauche, puis mêler l'octet par un xor » :

```bash
python3 16_checksum.py abc 3 0
python3 16_checksum.py abc 3 1
python3 16_checksum.py abc 2 0
```

```text
0x00018823
0x00010823
0x00000c42
```

Les trois correspondent à la bibliothèque. L'accord sur une entrée est un indice faible ; l'accord sur beaucoup d'entrées, y compris les limites (un tampon vide, une graine nulle, de longs tampons), est fort.

Un piège du programme de test : le C ne définit pas l'ordre d'évaluation des arguments d'un appel. Une ligne comme `printf("%d %d", bb_next(c), bb_next(c))` peut afficher les valeurs dans n'importe quel ordre. Le programme de test appelle la fonction dans une boucle, un appel par instruction.

### Écrire l'en-tête

Le produit du travail est un en-tête qu'un autre programmeur peut utiliser. Il contient les prototypes, un court commentaire pour chaque règle d'emploi, et rien d'autre. Il indique aussi quelles affirmations sont confirmées et lesquelles ne le sont pas.

```c
#include <stddef.h>

/* Limits v to the range lo..hi. Requires lo <= hi. */
int bb_clamp(int v, int lo, int hi);

/* Rotate-and-xor checksum of n bytes, starting from seed. */
unsigned bb_checksum(const unsigned char *p, size_t n, unsigned seed);

/* Copies the text of s up to the first sep into out and ends it with a zero byte.
   cap is the size of out including the zero byte. Returns the number of characters
   copied, or -1 when the field does not fit. */
long bb_split(const char *s, char sep, char *out, size_t cap);

/* Smallest and largest of n values; n must be at least 1. */
void bb_minmax(const int *a, size_t n, int *lo, int *hi);

/* a + (b - a) * t */
double bb_lerp(double a, double b, double t);

/* Counter handle. bb_open returns null when memory runs out; every handle must be passed
   to bb_close exactly once and not used afterwards. bb_next adds step to the value and
   returns it. */
void *bb_open(int start, int step);
int bb_next(void *c);
void bb_close(void *c);
```

### Où la méthode demande plus de soin

- **Arguments jamais lus sur certains chemins.** Un paramètre utilisé seulement dans une branche rarement prise peut sembler inutilisé. Lisez tous les chemins.
- **Structures passées ou renvoyées par valeur.** Une structure de plus de 16 octets est renvoyée par un pointeur caché qui arrive dans `rdi`, de sorte que chaque argument visible est décalé d'un cran. Les petites structures voyagent dans des registres répartis par type.
- **Fonctions variadiques.** `al` contient le nombre de registres vectoriels utilisés. Une fonction qui lit `al` tôt prend une liste variable d'arguments.
- **Rappels.** Un argument que la fonction appelle (`call rdx`, `call rax`) est un pointeur de fonction. Sa signature se déduit de la façon dont la fonction prépare les registres avant cet appel.
- **Sortie qui dépend d'un état caché.** Une fonction qui lit une variable globale donne des réponses différentes avec les mêmes arguments. Exécutez le programme de test deux fois dans des états différents.

## Essayez

Lancez les commandes du laboratoire depuis `modules/16-undocumented-interfaces/examples`.

1. Construisez avec `make`. Listez les exportations avec `nm -D build/libblackbox.so`. Avant de lire du code, notez ce que vous attendez de chaque nom.
2. Désassemblez `bb_clamp` et `bb_lerp`. Écrivez chaque signature à l'aide du tableau de cette leçon, puis comparez-la avec la leçon.
3. Exécutez la bibliothèque sous GDB et observez un appel de `bb_split` pendant qu'il se produit (commande et sortie sous la liste).
4. Modifiez le programme de test pour appeler `bb_minmax` avec `n` égal à 0. Prédisez ce qui se passe, puis exécutez-le.
5. Écrivez l'en-tête d'une bibliothèque de votre choix : compilez un petit fichier C de trois fonctions en bibliothèque partagée, oubliez le source et retrouvez les prototypes à partir du seul binaire.

Pour l'étape 3 :

```bash
gdb -nx -q -batch -ex "break bb_split" -ex "run" -ex "info registers rdi rsi rdx rcx" \
    -ex "x/s \$rdi" -ex "print/c \$rsi" -ex "finish" -ex "print \$rax" -ex "x/s \$rdx" build/16_probe
```

```text
Breakpoint 1, 0x00007ed57bbc4145 in bb_split () from /lab/modules/16-undocumented-interfaces/examples/build/libblackbox.so
rdi            0x598f749bb0a8      98472671555752
rsi            0x3d                61
rdx            0x7ffe9bb3f5a0      140731510683040
rcx            0x8                 8
0x598f749bb0a8:    "key=value"
$1 = 61 '='
0x0000598f749ba3cd in main ()
$2 = 3
0x7ffe9bb3f5a0:    "key"
```

Les quatre registres contiennent la chaîne source, le séparateur `=` (61), le tampon de destination et la capacité 8. Après `finish`, `rax` vaut 3 et la destination contient `key`. Les adresses varient d'une exécution à l'autre.

## Pièges courants

- **Se fier aux noms.** Une fonction nommée `bb_next` pourrait faire n'importe quoi. Lisez le code et testez-le.
- **Lire le type de retour d'après `eax` seul.** Si `rax` est fixé par une constante de 64 bits comme -1, le type de retour fait 64 bits. Une écriture de 32 bits donne un résultat de 32 bits.
- **Manquer un paramètre simplement transmis.** Un argument transmis à une autre fonction est lu une fois et semble inutilisé. Suivez chaque registre d'argument jusqu'à son dernier emploi.
- **Appeler avec la mauvaise largeur.** Un prototype avec `int` là où la bibliothèque lit un registre de 64 bits laisse indéfinie la moitié haute du registre. L'appel peut fonctionner par chance.
- **Oublier les règles d'emploi.** Une signature correcte ne suffit pas : un nombre qui ne doit pas être nul, une taille de tampon qui inclut le terminateur et un descripteur qui doit être fermé une fois font partie de l'interface.
- **Conclure à partir d'un seul test.** Une entrée qui correspond ne confirme rien. Testez les limites et les valeurs inhabituelles.

## Résumé

- La liste des exportations est la première vue d'une bibliothèque non documentée, et les noms sont des hypothèses.
- Une signature se reconstruit d'après les registres d'arguments lus, la largeur de chaque emploi, la façon dont les pointeurs sont déréférencés et ce qui est écrit en dernier dans le registre de résultat.
- Un pointeur à travers lequel on ne fait qu'écrire est un paramètre de sortie ; un pointeur renvoyé par une fonction et passé à d'autres est un descripteur avec une durée de vie.
- Les sites d'appel, les constantes et les tests après un appel montrent comment l'interface doit être utilisée.
- Un programme de test confirme chaque hypothèse, limites comprises, et le résultat est un en-tête avec les prototypes et les règles d'emploi.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 17, le déchiffrage des formats de fichier.
- Exercices qui utilisent ce module : [Exercice 16](../../exercises/16-silent-library/README.fr.md).
