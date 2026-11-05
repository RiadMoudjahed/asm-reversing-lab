<!-- source: README.md @ sha256:002e7d34793e -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 20. Lire du code obscurci

Durée : environ 2,5 heures. Prérequis : [19. Analyser des programmes suspects](../19-analyzing-suspicious-programs/README.fr.md)

## Pourquoi c'est important

Certains programmes sont écrits pour être difficiles à lire. Leurs chaînes sont encodées pour qu'une recherche ne trouve rien, leur logique est découpée en morceaux et mélangée, leur code est bourré d'instructions qui ne font rien, et certains remarquent quand on les observe. Les défenseurs rencontrent cela dans les logiciels malveillants, et tout le monde le rencontre dans les logiciels protégés. L'obscurcissement ne change jamais ce que fait un programme ; il change seulement la difficulté de le voir. Le processeur doit toujours exécuter le comportement réel, ce qui signifie que le comportement peut toujours être retrouvé. Ce module enseigne les techniques courantes, comment reconnaître chacune dans un désassemblage et comment choisir le moyen le moins coûteux de voir à travers elles. Tous les exemples sont des programmes inoffensifs écrits pour la leçon. Le module porte sur la lecture du code pour le comprendre ; il ne traite pas du retrait des protections de logiciels commerciaux.

## Concepts

### Ce que fait l'obscurcissement et que faire face à lui

On obscurcit des programmes pour protéger une idée, pour échapper aux scanners qui cherchent des chaînes et des motifs connus, et pour faire perdre du temps à celui qui les analyse. Une technique qui rend la lecture plus difficile n'est pas une barrière à la compréhension, car tout programme obscurci doit tout de même faire son travail. Votre tâche est de trouver le chemin le moins coûteux vers le comportement. Les trois chemins utilisés dans ce module sont :

| Chemin | Idée | Bon pour |
|---|---|---|
| Lire le code | Reconnaître la technique et la défaire à la main ou avec un petit script | Chaînes encodées, code inutile, une petite fonction aplatie |
| Émulation | N'exécuter qu'une petite routine dans un émulateur, sans accès au système | Routines de décodage, dérivation de hachages et de clés, tout calcul autonome |
| Flux de données | Partir du résultat qui vous intéresse et ne suivre que ce qui y contribue | Code bourré de déchets, grandes fonctions, tout ce où la plupart des instructions ne sont que du bruit |

Exécuter le programme entier est le dernier choix, pas le premier, et ne se fait que dans le laboratoire isolé du module 19.

### Chaînes encodées

Les chaînes sont la preuve la plus utile lors du triage, elles sont donc la première chose à cacher. Le programme garde le texte sous forme encodée et le décode quand il en a besoin. Les signes :

- `strings` ne trouve rien là où le programme utilise manifestement un chemin, un hôte ou un message. Comparez ce que vous savez que fait le programme avec ce que montre la liste de chaînes.
- Une table d'octets dans la section de données dont les valeurs semblent aléatoires, à côté d'une petite fonction qui la parcourt.
- Une fonction appelée avec une destination, une source, une longueur et une petite constante avant chaque utilisation d'une chaîne.
- Une boucle avec un `xor` d'un octet de la mémoire avec un registre ou une constante.

L'exemple `examples/20_strings.c` cache trois chaînes de trois façons. Le programme les affiche :

```bash
make
build/20_strings
strings -a build/20_strings_stripped | grep -iE "http|temp|status|example"
```

```text
https://config.example.test/a
C:\Temp\out.dat
status=ok
```

La deuxième commande, lancée sur le programme dépouillé de ses symboles, n'affiche rien : aucune des trois chaînes n'existe dans le fichier.

**Schéma 1 : une clé fixe.** Le décodeur est la plus courte boucle possible :

```text
401126:  test   rdx,rdx
401129:  je     401144
40112b:  mov    eax,0x0
401130:  mov    r8d,ecx                      ; the key, from the fourth argument
401133:  xor    r8b,BYTE PTR [rsi+rax*1]     ; key xor the encoded byte
401137:  mov    BYTE PTR [rdi+rax*1],r8b     ; store the decoded byte
40113b:  add    rax,0x1
40113f:  cmp    rdx,rax                      ; length in rdx
401142:  jne    401130
401144:  mov    BYTE PTR [rdi+rdx*1],0x0     ; terminator
401148:  ret
```

Les arguments sont la destination (`rdi`), les octets encodés (`rsi`), la longueur (`rdx`) et la clé (`ecx`). L'appelant passe la clé comme constante (`mov ecx,0x5a`), la clé est donc visible dans le code. Une clé d'un octet peut aussi se trouver sans le code : les premiers octets d'une URL ou d'un chemin sont connus (`http`, `C:\`), et un xor du premier octet encodé avec la lettre connue donne la clé. Le script `examples/20_decode.py` va plus loin et essaie les 256 clés, en gardant les résultats qui ressemblent à une URL ou à un chemin :

```bash
python3 20_decode.py
```

```text
blob A, every key that gives plausible text:
  key 0x5a: https://config.example.test/a
blob B with the rolling rule, seed 0x21:
  C:\Temp\out.dat
```

**Schéma 2 : une clé tournante.** La clé change à chaque octet, donc un seul xor de tout le blob ne fonctionne pas :

```text
401149:  test   rdx,rdx
40114c:  je     40116b
40114e:  mov    eax,0x0
401153:  lea    ecx,[rcx+rcx*4+0x3]          ; k = k * 5 + 3
401157:  mov    r8d,ecx
40115a:  xor    r8b,BYTE PTR [rsi+rax*1]     ; xor with the low byte of k
40115e:  mov    BYTE PTR [rdi+rax*1],r8b
401162:  add    rax,0x1
401166:  cmp    rdx,rax
401169:  jne    401153
40116b:  mov    BYTE PTR [rdi+rdx*1],0x0
40116f:  ret
```

La signature est une mise à jour du registre de clé dans la boucle, ici `lea ecx,[rcx+rcx*4+3]`, qui calcule `k * 5 + 3`. Pour décoder, il vous faut la règle et la graine (la valeur de départ passée par l'appelant, 0x21). Quand la règle est simple, quelques lignes de script la reproduisent, comme dans `20_decode.py`. Quand elle ne l'est pas, émulez la fonction.

**Schéma 3 : une chaîne de pile.** Le texte n'existe jamais d'un seul tenant dans le fichier. Le code le construit octet par octet à partir de constantes des instructions :

```text
401170:  mov    BYTE PTR [rdi],0x73          ; s
401173:  mov    BYTE PTR [rdi+0x1],0x74      ; t
401177:  mov    BYTE PTR [rdi+0x2],0x61      ; a
40117b:  mov    BYTE PTR [rdi+0x3],0x74      ; t
40117f:  mov    BYTE PTR [rdi+0x4],0x75      ; u
401183:  mov    BYTE PTR [rdi+0x5],0x73      ; s
401187:  mov    BYTE PTR [rdi+0x6],0x3d      ; =
40118b:  mov    BYTE PTR [rdi+0x7],0x6f      ; o
40118f:  mov    BYTE PTR [rdi+0x8],0x6b      ; k
401193:  mov    BYTE PTR [rdi+0x9],0x0
401197:  ret
```

Une suite d'écritures de valeurs immédiates à des décalages consécutifs est une chaîne de pile, et les immédiates sont de l'ASCII imprimable. Un listing de désassemblage montre les morceaux, mais un décompilateur ou un court script qui lit les immédiates dans l'ordre donne la chaîne entière. Les compilateurs peuvent fusionner les écritures en écritures plus larges (`mov DWORD PTR`, `movabs`) ; cherchez donc le même motif avec des constantes de 4 et 8 octets dont les octets sont de l'ASCII.

### Émuler une petite routine

Décoder à la main suffit pour les schémas simples. Pour tout le reste, exécutez la routine. Un émulateur exécute les instructions d'une fonction dans un modèle du processeur et vous donne le résultat sans exécuter le programme, sans appels système et sans aucun effet en dehors de sa propre mémoire. Le script `examples/20_emulate.py` utilise l'émulateur Unicorn : il projette les segments chargeables du fichier, prépare une pile, une adresse de retour et les arguments, exécute depuis l'adresse de la fonction jusqu'à son retour et lit le tampon de sortie.

```bash
python3 20_emulate.py build/20_strings_stripped 401126 402020 29 5a
python3 20_emulate.py build/20_strings_stripped 401149 402010 15 21
```

```text
179 instructions emulated
https://config.example.test/a
110 instructions emulated
C:\Temp\out.dat
```

Les arguments sont le fichier, l'adresse du décodeur, l'adresse des données encodées, leur longueur et la clé. Les quatre nombres viennent de l'appelant : le `main` dépouillé les charge dans `ecx`, `edx` et `esi` juste avant chaque `call` (`mov ecx,0x5a`, `mov edx,0x1d`, `mov esi,0x402020`, `call 401126`). C'est la méthode : trouver le décodeur à sa forme, trouver un appel vers lui, lire ses arguments dans les instructions avant l'appel et émuler. La même approche lit des tables entières de chaînes encodées : parcourez les sites d'appel et émulez chacun.

L'émulation a des limites. Une routine qui fait des appels système ou appelle des fonctions de bibliothèque a besoin de substituts pour eux, ou doit être coupée à cet endroit. Une routine qui lit de la mémoire que l'émulation n'a pas (une globale que le programme a préparée plus tôt) a besoin que cette mémoire soit d'abord remplie.

### Code inutile et prédicats opaques

Le **code inutile** est du code sans effet sur le résultat : des calculs dont les valeurs ne servent jamais, des appels à des fonctions qui ne font rien, des sauts vers l'instruction suivante. Il allonge les fonctions et cache la vraie structure. Un **prédicat opaque** (opaque predicate) est une condition qui a toujours la même valeur, mais dont la valeur est difficile à voir d'un coup d'œil. La branche est alors toujours prise ou jamais prise, un côté est donc du code mort ; le but est d'agrandir le graphe de flux de contrôle et d'ajouter du travail.

La fonction `score_flat` de `examples/20_flat.c` a les deux. Son code inutile est la mise à jour de `g_junk` (rien ne la lit sauf la branche morte), et son prédicat opaque est `(n * (n + 1)) % 2 == 1`. Le produit d'un nombre et de son successeur est toujours pair, le prédicat est donc faux pour tout `n` :

```text
4011d2:  lea    eax,[rdx+0x1]                ; n + 1
4011d5:  imul   eax,edx                      ; n * (n + 1)
4011d8:  test   al,0x1                       ; low bit: the product is odd?
4011da:  je     4011e2                       ; the product is always even, so this jump is always taken
4011dc:  mov    ecx,DWORD PTR [rip+0x2e3a]   ; dead: r = g_junk
4011e2:  ...
```

Pour trouver des prédicats opaques, demandez pour chaque branchement conditionnel si la condition peut dépendre de quelque chose qui change. Les identités arithmétiques (un produit de nombres consécutifs est pair ; `x * x` ne vaut jamais 2 ; `(x | 1)` n'est jamais 0) sont la source habituelle. Testez votre soupçon en émulant sur plusieurs entrées ou en écrivant l'identité. Quand une branche va toujours dans le même sens, traitez le côté mort comme supprimé.

### Aplatissement du flux de contrôle

Une fonction normale est un graphe de blocs qui suivent sa logique. L'**aplatissement du flux de contrôle** (control-flow flattening) remplace le graphe par une boucle. Une variable contient le numéro du bloc à exécuter ensuite. Un répartiteur (dispatcher) en tête de boucle lit la variable et saute au bloc correspondant. Chaque bloc se termine en rangeant le numéro de son successeur et en revenant au répartiteur. La structure du programme quitte la forme du code et passe dans les valeurs de la variable.

Comparez la version simple de la fonction avec la version aplatie (le compilateur a produit les deux à partir du source de `20_flat.c`) :

```text
0000000000401126 <score_plain>:
  401126:  mov    eax,edi
  401128:  mov    edx,0xaaaaaaab
  40112d:  imul   rax,rdx
  401131:  shr    rax,0x21                     ; n / 3 (module 14)
  401135:  lea    eax,[rax+rax*2]
  401138:  cmp    edi,eax                      ; n == (n / 3) * 3 ?
  40113a:  je     40115c
  ...
  40115b:  ret
```

```text
0000000000401160 <score_flat>:
  401162:  mov    DWORD PTR [rip+0x2eb4],0x0   ; g_state = 0
  401171:  mov    eax,DWORD PTR [rip+0x2ea9]   ; the dispatcher: load the state
  401177:  cmp    eax,0x4
  40117a:  ja     401230                       ; above 4: leave the loop
  401182:  jmp    QWORD PTR [rax*8+0x402008]   ; jump table indexed by the state
  ...                                          ; the blocks
  4011c6:  mov    DWORD PTR [rip+0x2e50],0x5   ; ... each one stores the next state
  4011d0:  jmp    401171                       ; ... and goes back to the dispatcher
```

Les signes de l'aplatissement : un bloc central avec une table de sauts ou une chaîne de comparaisons sur une seule variable ; de nombreux blocs qui se terminent tous par un saut de retour vers le centre ; des affectations de constantes à cette variable ; et une fonction dont le graphe ressemble à une étoile. Deux idées aident.

1. Les affectations de chaque bloc donnent ses successeurs. Un bloc qui range une constante a un successeur. Un bloc qui range l'une de deux valeurs selon une comparaison est une bifurcation, et la comparaison est la condition d'origine.
2. On peut retrouver le graphe par traçage. Exécutez la fonction pour une poignée d'entrées et notez chaque valeur rangée dans la variable d'état ; les suites sont des chemins dans le graphe d'origine. Le script `examples/20_trace_states.py` le fait dans l'émulateur, en interceptant les écritures à l'adresse de la variable d'état (dans un échantillon réel c'est une variable locale ou un registre, et l'idée est la même) :

```bash
python3 20_trace_states.py build/20_flat 401160 404020
```

```text
n= 0: states 0 -> 1 -> 5  result 0
n= 1: states 0 -> 2 -> 4 -> 5  result 84
n= 2: states 0 -> 2 -> 4 -> 5  result 87
n= 3: states 0 -> 1 -> 5  result 6
n= 4: states 0 -> 2 -> 4 -> 5  result 81
n= 5: states 0 -> 2 -> 3 -> 5  result 12
...
n=14: states 0 -> 2 -> 4 -> 5  result 91
edges of the recovered graph:
  0 -> 1, 2
  1 -> 5
  2 -> 3, 4
  3 -> 5
  4 -> 5
```

Le graphe retrouvé est un arbre à la forme du `if / else if / else` d'origine : l'état 0 teste la divisibilité par 3 (les entrées 0, 3, 6, 9, 12 prennent l'état 1, dont le bloc double l'entrée), l'état 2 teste la divisibilité par 5 (5 et 10 prennent l'état 3, qui ajoute 7) et les autres prennent l'état 4 (un xor avec 0x55). C'est exactement `score_plain`. Remarquez que le prédicat opaque de l'état 2 n'a jamais changé le chemin.

### Reconnaître les vérifications de débogueur et d'analyse

Certains programmes cherchent des signes qu'ils sont analysés et changent de comportement quand ils en trouvent, par exemple en se terminant, en prenant un chemin inoffensif ou en décodant autre chose. Vous devez reconnaître ces vérifications pendant le triage, car un programme qui se comporte autrement sous observation peut rendre une analyse dynamique trompeuse. Les vérifications habituelles :

| Vérification | À quoi elle ressemble |
|---|---|
| Un appel système qui échoue si un traceur est attaché (Linux : `ptrace` avec la requête qui demande à être tracé ; Windows : `IsDebuggerPresent`, `CheckRemoteDebuggerPresent`, une lecture de l'indicateur de débogage dans le bloc du processus) | Une importation ou un appel à ces fonctions, suivi d'un test du résultat et d'un branchement |
| Lecture de l'état du processus pour y chercher un traceur (Linux : le champ `TracerPid` de `/proc/self/status`) | La chaîne `/proc/self/status` et une comparaison du champ avec 0 |
| Mesure du temps | Deux lectures d'une horloge (`rdtsc`, un compteur de ticks) autour d'un bloc, puis une comparaison avec une limite |
| Astuces de points d'arrêt et d'exceptions | Du code qui lève une exception ou contient une instruction d'interruption exprès et attend qu'un gestionnaire s'exécute |
| Recherche d'outils d'analyse ou de machines virtuelles | Recherches dans la liste des processus, les noms de fenêtres, les clés du registre, les noms de périphériques et les identifiants matériels |

L'exemple `examples/20_dbg.c` fait deux vérifications et se contente de signaler ce qu'il a trouvé. Une exécution simple, une exécution sous un traceur d'appels système et une exécution sous le débogueur donnent des réponses différentes :

```bash
build/20_dbg
strace -f -o /dev/null build/20_dbg
gdb -nx -q -batch -ex run build/20_dbg
```

```text
status check: no debugger
ptrace check: no debugger
status check: debugger detected
ptrace check: debugger detected
status check: debugger detected
ptrace check: debugger detected
```

(La première paire est l'exécution simple, la deuxième l'exécution sous `strace`, la troisième l'exécution sous `gdb`.) Le triage statique trouve les deux vérifications sans rien exécuter : le programme importe `ptrace` et `fopen`, et contient les chaînes `/proc/self/status` et `TracerPid:`. Quand vous trouvez une vérification, notez où elle est, ce qu'elle teste et ce que fait le programme de différent quand elle se déclenche. L'analyse statique et l'émulation ne sont pas affectées par ces vérifications, car le programme ne s'exécute pas au sens normal, une raison de plus de les préférer. Quand une observation doit se faire pendant l'exécution, indiquez dans le rapport qu'une vérification est présente et que l'observation peut ne pas représenter une exécution normale.

### Automatiser l'analyse par script

Quand le même travail se répète (nombreuses chaînes encodées, nombreux sites d'appel), écrivez un script. Dans Ghidra, un script peut lire des octets à une adresse, les décoder et mettre le résultat dans un commentaire pour qu'il apparaisse à côté du code. L'esquisse ci-dessous montre le motif pour le schéma à clé fixe. Elle utilise l'interface de script de Ghidra : elle s'exécute donc dans Ghidra sur l'hôte, pas dans l'image du laboratoire, et les vérifications du laboratoire ne l'exécutent pas ; vérifiez les noms avec l'aide du gestionnaire de scripts de votre version de Ghidra.

```text
# Decode an xor-encoded string and write it as a comment (Ghidra Script Manager, Python)
from ghidra.program.model.listing import CodeUnit
addr = toAddr(0x402020)                       # the address of the encoded bytes
data = getBytes(addr, 29)                     # 29 bytes
text = "".join(chr((b & 0xff) ^ 0x5a) for b in data)
currentProgram.getListing().setComment(addr, CodeUnit.EOL_COMMENT, text)
```

Pour beaucoup de chaînes, rassemblez les sites d'appel du décodeur (les références croisées vers son adresse), lisez les arguments dans les instructions avant chaque appel et appliquez le même décodage à chacun.

### Un ordre de travail

1. **Triage.** Faites les vérifications du module 19 : chaînes, entropie, importations. Marquez ce qui manque (aucune chaîne là où vous en attendez) et ce qui est bizarre (tables d'aspect aléatoire, liste d'importations minuscule).
2. **Trouver les décodeurs.** Cherchez de petites boucles avec un `xor`, des fonctions appelées de nombreuses fois avec une constante et une longueur, et des suites d'écritures d'octets.
3. **Décoder.** À la main ou par script pour les schémas simples, par émulation pour les autres. Mettez les résultats dans des commentaires ou dans un tableau.
4. **Trouver la structure.** Cherchez un bloc central avec de nombreux retours. Tracez la variable d'état et reconstruisez le graphe.
5. **Découper par flux de données.** Partez de ce que la fonction renvoie ou des arguments d'un appel intéressant et ne suivez que les instructions qui y contribuent. Tout le reste est du bruit.
6. **Noter les vérifications.** Listez chaque vérification d'analyse trouvée, avec son emplacement et son effet.

## Essayez

Lancez les commandes du laboratoire depuis `modules/20-reading-obfuscated-code/examples`.

1. Construisez avec `make`. Cherchez les trois chaînes dans le programme dépouillé avec `strings`, puis affichez-les en lançant le programme. Lequel des trois schémas trouveriez-vous en cherchant `xor` dans le désassemblage ?
2. Désassemblez `build/20_strings_stripped`. Trouvez les deux décodeurs et le constructeur de la chaîne de pile. Dans `main`, lisez les arguments de chaque appel.
3. Utilisez `20_emulate.py` pour décoder la deuxième chaîne. Changez la graine d'une unité et regardez ce qui arrive au résultat.
4. Écrivez un décodeur pour un troisième schéma de votre cru (par exemple, ajoutez la position de l'octet à la clé), encodez une chaîne avec lui et vérifiez que `20_emulate.py` la décode quand vous compilez le décodeur.
5. Désassemblez `score_flat` et dessinez ses blocs et les arêtes entre eux d'après les écritures dans la variable d'état. Comparez le dessin avec la sortie de `20_trace_states.py`.
6. Lancez `20_dbg` normalement, sous `strace` et sous `gdb`. Trouvez les deux vérifications avec `strings` et `objdump` sans exécuter le programme.

## Pièges courants

- **Chercher la chaîne au lieu du décodeur.** Une chaîne encodée ne se trouve pas par son texte. Trouvez le code qui la produit.
- **Supposer que la clé est une constante.** Une clé qui change dans la boucle (tournante) ou dépend de la position demande la règle, pas une valeur unique.
- **Émuler sans vérifier le résultat.** Vérifiez que la sortie ressemble à ce qu'elle doit être : du texte imprimable, un chemin plausible. Un mauvais argument donne du bruit, qui ressemble à un décodeur raté.
- **Suivre le répartiteur au lieu des données.** Dans une fonction aplatie, l'ordre des blocs en mémoire ne veut rien dire. Suivez la variable d'état.
- **Prendre une branche morte pour réelle.** Une branche dont la condition est une identité n'est pas un comportement. Vérifiez-la avant de la documenter.
- **Oublier que le programme peut vous observer.** Un résultat obtenu sous un débogueur peut différer d'une exécution normale. Dites-le dans le rapport.

## Résumé

- L'obscurcissement change la difficulté de lire le code, pas ce qu'il fait, donc le comportement peut toujours être retrouvé.
- Les chaînes encodées se manifestent par une chaîne absente, une table d'octets d'aspect aléatoire et une petite boucle de décodage ; les schémas sont une clé fixe, une clé tournante et une chaîne de pile.
- Émuler une petite routine donne son résultat sans exécuter le programme ; trouvez la routine, lisez ses arguments aux sites d'appel et exécutez-la.
- Le code inutile et les prédicats opaques ajoutent du bruit ; une condition qui est une identité arithmétique est toujours la même et son côté mort peut être supprimé.
- L'aplatissement du flux de contrôle remplace le graphe par un répartiteur et une variable d'état ; les affectations ou une trace de la variable redonnent le graphe d'origine.
- Les vérifications de débogueur et d'analyse se reconnaissent lors du triage à leurs importations, chaînes et motifs de comparaison et de branchement, et sont notées dans le rapport.
- Un ordre de travail, du triage au décodage, à la structure et au flux de données, garde le travail peu coûteux.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 21, les machines virtuelles et le bytecode.
- Exercices qui utilisent ce module: [Exercice 19](../../exercises/19-triage-desk/README.fr.md).
