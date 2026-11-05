<!-- source: README.md @ sha256:2c38d8bd75e3 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 21. Machines virtuelles et bytecode

Durée : environ 2,5 heures. Prérequis : [20. Lire du code obscurci](../20-reading-obfuscated-code/README.fr.md)

## Pourquoi c'est important

Certains programmes ne contiennent pas leur logique sous forme de code machine. Ils contiennent un petit processeur écrit en logiciel, un interpréteur, et un bloc de données qu'il exécute, le bytecode. Les langages utilisent cette conception pour fonctionner sur n'importe quel matériel, les applications l'utilisent pour des scripts et des macros, et les programmes protégés ou obscurcis l'utilisent pour cacher ce qu'ils font : la vraie logique est un flux d'octets pour un jeu d'instructions qui n'existe nulle part ailleurs, et un désassembleur ne montre que l'interpréteur. La méthode pour lire un tel programme est toujours la même. Vous traitez l'interpréteur comme la documentation d'un processeur inconnu, vous retrouvez son jeu d'instructions à partir du code de ses gestionnaires et vous écrivez un désassembleur pour lui. Ce module enseigne cette méthode sur deux petites machines inoffensives écrites pour la leçon. Le module porte sur la compréhension des programmes ; les machines qu'il contient ne font rien d'autre que calculer et afficher des nombres.

## Concepts

### Ce qu'est une machine virtuelle

Une machine virtuelle désigne ici un programme qui se comporte comme un processeur : il a un état, il lit des instructions en mémoire et il les exécute une à une. L'état est petit :

| Élément | Signification |
|---|---|
| Compteur ordinal | Le décalage de la prochaine instruction de bytecode. C'est une variable ordinaire, pas le registre `rip`. |
| Stockage de données | Une pile, ou un ensemble de registres virtuels, ou les deux. |
| Mémoire | Des variables, ou un tableau d'octets que le bytecode peut lire et écrire. |
| Indicateurs et état | Un indicateur d'arrêt, un indicateur d'erreur, parfois des résultats de comparaison. |
| Le programme | Le bytecode : un tableau d'octets lu par l'interpréteur. Ce sont des données, pas du code. |

L'interpréteur est une boucle qui fait les trois mêmes choses pour chaque instruction : elle récupère le code opération (le nombre qui sélectionne l'opération) au compteur ordinal, le décode avec ses opérandes et l'exécute en lançant le code d'un gestionnaire. La boucle est la même pour tous les programmes. Seul le bytecode change.

La conséquence pour la rétro-ingénierie est que l'information utile se déplace. Dans un programme ordinaire, le désassemblage est la logique. Dans un programme avec une machine virtuelle, le désassemblage est le même pour tous les programmes qui utilisent la machine, et la logique est dans le bytecode. Vous lirez l'interpréteur une fois, puis vous lirez le bytecode avec un outil que vous construisez.

### Deux conceptions

Les deux machines d'exemple représentent les deux conceptions courantes.

| | Machine à pile | Machine à registres |
|---|---|---|
| Opérandes | Implicites : les opérations prennent leurs entrées au sommet d'une pile et y déposent le résultat | Explicites : les instructions nomment les registres |
| Taille des instructions | Courte ; quelques instructions portent un octet d'opérande | Généralement fixe, avec plusieurs champs |
| Code typique | `push 3`, `push 4`, `add` | `add r1, r2, r3` |
| Dans l'interpréteur | Un tableau de pile et un indice de pile | Un tableau de registres ; le champ de l'instruction est un indice dans ce tableau |
| Facile à reconnaître grâce à | Un indice qui monte et descend dans presque tous les gestionnaires | Des gestionnaires qui lisent deux ou trois petits champs et indexent un seul tableau |

La première machine, `examples/21_stackvm.c`, est une machine à pile de 11 instructions, d'un octet de code opération chacune, certaines avec un octet d'opérande. Elle lit son programme dans un fichier. La seconde, `examples/21_regvm.c`, est une machine à registres de quatre registres et d'instructions fixes de 4 octets. Les deux sont décrites dans les commentaires en tête de leurs sources ; dans la leçon, vous retrouverez les jeux d'instructions à partir des programmes compilés, comme pour un échantillon inconnu.

### Les façons de répartir

La répartition (dispatch) est l'étape qui va du code opération à son gestionnaire. C'est la partie de l'interpréteur la plus facile à reconnaître.

| Style | Dans la source | Dans le désassemblage |
|---|---|---|
| Un `switch` | `switch (op) { case 0: ... }` | Un contrôle de borne (`cmp`, puis `ja` vers le cas par défaut), puis un `jmp` indirect à travers une table d'adresses de code |
| Une table de gestionnaires | `table[op](state, instruction)` | Le chargement d'un pointeur depuis un tableau indexé par le code opération, un test de pointeur nul, puis un `call` indirect |
| Répartition enfilée | Chaque gestionnaire se termine par un saut vers le suivant | Beaucoup de sauts indirects, un à la fin de chaque gestionnaire, et aucune boucle centrale |
| Une chaîne de comparaisons | `if (op == 1) ... else if (op == 2) ...` | Une série de `cmp` et de sauts conditionnels ; lente, présente dans des interpréteurs simples ou volontairement dépouillés |

Les deux premiers sont le sujet des exemples. Dans les deux cas, l'objet clé est la table : un tableau de pointeurs dans la section de données en lecture seule. Lire la table vous donne, pour chaque code opération, l'adresse de son gestionnaire. Un code opération sans gestionnaire a une entrée nulle (la table de gestionnaires) ou va au cas d'erreur (le `switch`). Plusieurs codes opération peuvent partager un gestionnaire.

### Reconnaître une machine virtuelle

Cherchez ces signes ensemble ; aucun ne suffit seul.

- Une grande fonction avec un point central : une boucle contenant un saut ou un appel indirect, et beaucoup de petits blocs qui reviennent au début de la boucle.
- Le chargement d'un octet (ou d'un mot) depuis un tampon à une position variable, suivi d'un contrôle de borne contre un petit nombre.
- Une table d'adresses de code dans la section de données en lecture seule (une liste de valeurs qui pointent toutes dans la section de texte).
- Un tampon d'octets dans la section de données, ou un fichier lu au démarrage, sans chaînes lisibles et avec une structure qui se répète.
- Des gestionnaires courts qui modifient tous les mêmes quelques variables : l'indice de pile, le tableau de registres, le compteur ordinal.
- Un compteur ordinal modifié dans quelques gestionnaires seulement (les sauts) et incrémenté d'une valeur fixe ailleurs.

### Retrouver le jeu d'instructions

La méthode est une suite d'étapes que vous pouvez appliquer à n'importe quel interpréteur.

1. **Trouver l'état.** Identifiez les variables que chaque gestionnaire lit ou écrit. L'une avance à chaque instruction (le compteur ordinal). L'une est un indice qui monte et descend (un pointeur de pile) ou un tableau indexé par un petit champ (les registres). Les autres sont la mémoire et l'état.
2. **Trouver la répartition.** Trouvez le chargement du code opération et le transfert indirect qui le suit. Notez le registre qui contient le code opération et le contrôle de borne, qui vous indique le nombre de codes opération.
3. **Lire la table.** Extrayez la table de la section de données et listez chaque code opération avec l'adresse de son gestionnaire. Notez les codes qui partagent un gestionnaire ou qui manquent.
4. **Lire chaque gestionnaire.** Pour chaque gestionnaire, notez quels opérandes il consomme (de combien il avance le compteur ordinal), ce qu'il lit, ce qu'il écrit et quelle est l'opération. Les gestionnaires qui partagent du code peuvent différer par un test sur le code opération.
5. **Écrire la spécification.** Une ligne par code opération : numéro, nom de votre choix, opérandes, effet. Les noms sont les vôtres ; le comportement est celui de l'interpréteur.
6. **Écrire un désassembleur** à partir de la table (ci-dessous), le vérifier sur du bytecode dont vous connaissez le comportement, puis l'utiliser sur le bytecode qui vous intéresse.

#### La machine à pile dans le désassemblage

La version sans symboles n'a pas de noms, et l'interpréteur est fondu dans `main`. Voici la répartition :

```bash
make
objdump -d -M intel --no-show-raw-insn build/21_stackvm_stripped | sed -n '/401286:/,/4012aa:/p'
```

```text
  401286:  lea    r12,[r8+0x1]
  40128a:  movzx  ecx,BYTE PTR [rsp+r8*1+0xa0]
  401293:  add    rbp,0x1
  401297:  cmp    cl,0xa
  40129a:  ja     4013e9 <__fprintf_chk@plt+0x369>
  4012a0:  movzx  eax,cl
  4012a3:  jmp    QWORD PTR [rax*8+0x402078]
```

`r8` est le compteur ordinal : le code opération est chargé depuis le tampon à `[rsp+r8+0xa0]`, et le compteur ordinal plus un est conservé dans `r12`. `rbp` est incrémenté à chaque instruction, c'est donc un compteur de pas. Le code opération est comparé à 0xa, et tout ce qui est au-dessus va au cas d'erreur ; il y a donc 11 codes opération, de 0 à 10. La table est à 0x402078. Son contenu donne le gestionnaire de chaque code opération :

```bash
objdump -s -j .rodata build/21_stackvm_stripped
```

```text
 402070 00000000 00000000 37124000 00000000  ........7.@.....
 402080 55124000 00000000 aa124000 00000000  U.@.......@.....
 402090 aa124000 00000000 aa124000 00000000  ..@.......@.....
 4020a0 e8124000 00000000 09134000 00000000  ..@.......@.....
 4020b0 44134000 00000000 7e134000 00000000  D.@.....~.@.....
 4020c0 95134000 00000000 c0134000 00000000  ..@.......@.....
```

Lisez les valeurs de huit octets à partir de 0x402078, en petit-boutiste. Le code 0 va à 0x401237, le code 1 à 0x401255, les codes 2, 3 et 4 vont tous à 0x4012aa, le code 5 à 0x4012e8, et ainsi de suite. Trois codes partagent un gestionnaire : ce gestionnaire doit tester le code opération pour choisir entre trois opérations, et le code à 0x4012aa commence par `cmp rbx,0x1` et `jbe`, un contrôle que la pile contient deux valeurs, suivi de deux dépilements. C'est la forme des opérations binaires. En lisant les autres gestionnaires de la même façon, on obtient la spécification des commentaires de la source ; le gestionnaire du code 1 lit un octet de plus et incrémente le compteur ordinal de 2, les gestionnaires de 8 et 9 affectent le compteur ordinal depuis l'opérande, et le gestionnaire de 10 appelle `printf` avec la chaîne de format `%d`.

Suivre le compteur ordinal est la façon de décider de la taille d'une instruction. Un gestionnaire qui lit l'octet suivant le code opération consomme un opérande. Si vous vous trompez sur la taille d'une instruction, toutes les instructions suivantes du désassemblage sont fausses, ce qui est le signe à chercher : un désassemblage qui se met à produire n'importe quoi au milieu d'un programme a une taille erronée avant ce point.

#### La machine à registres dans le désassemblage

La machine à registres utilise une table de gestionnaires. L'appel ressemble à ceci :

```text
  4012b5:  lea    rsi,[rsp+r8*4+0x20]
  4012ba:  movzx  ecx,BYTE PTR [rsi]
  4012bd:  movsxd rax,ecx
  4012c0:  mov    rax,QWORD PTR [rax*8+0x4020a0]
  4012c8:  test   rax,rax
  4012cb:  je     401359 <__fprintf_chk@plt+0x2d9>
  4012d1:  mov    rdi,rsp
  4012d4:  call   rax
```

L'adresse de l'instruction est calculée comme la base plus quatre fois le compteur ordinal, ce qui montre que les instructions font 4 octets. Le code opération est le premier octet de l'instruction, la table est à 0x4020a0, une entrée nulle va au cas d'erreur, et le gestionnaire est appelé avec `rdi` pointant vers l'état de la machine et `rsi` vers l'instruction. La table compte 256 entrées, parce que le code opération est un octet entier, et seules sept ne sont pas nulles :

```text
0x10 -> 0x401176
0x11 -> 0x40118e
0x12 -> 0x4011ad
0x13 -> 0x4011cc
0x14 -> 0x4011ec
0x15 -> 0x40120a
0xff -> 0x401202
```

Les gestionnaires sont courts et réguliers. Voici le gestionnaire du code opération 0x11 :

```text
  40118e:  movzx  eax,BYTE PTR [rsi+0x1]
  401192:  and    eax,0x3
  401195:  movzx  ecx,BYTE PTR [rsi+0x2]
  401199:  and    ecx,0x3
  40119c:  movzx  edx,BYTE PTR [rsi+0x3]
  4011a0:  and    edx,0x3
  4011a3:  mov    edx,DWORD PTR [rdi+rdx*4]
  4011a6:  add    edx,DWORD PTR [rdi+rcx*4]
  4011a9:  mov    DWORD PTR [rdi+rax*4],edx
  4011ac:  ret
```

Les trois octets qui suivent le code opération sont masqués avec 3 et utilisés comme indices dans un tableau de valeurs de 32 bits au début de l'état : un banc de registres de quatre registres. Le gestionnaire additionne les registres nommés par le deuxième et le troisième champ et range le résultat dans le registre nommé par le premier. C'est `add a, b, c`. Le masque signifie qu'un numéro de registre supérieur à 3 boucle au lieu d'échouer, un détail qui appartient à la spécification. Le gestionnaire à 0x40118e ne diffère de ceux à 0x4011ad et 0x4011cc que par l'opération (`sub`, `imul`). Le gestionnaire à 0x4011ec compare un registre à zéro et, s'il n'est pas nul, écrit un octet de l'instruction dans un champ de l'état au décalage 0x10 : ce champ est le compteur ordinal, c'est donc un saut conditionnel. Le gestionnaire à 0x401202 positionne un champ au décalage 0x18, l'indicateur d'arrêt.

### Lire le bytecode

Avec la spécification, vous pouvez lire un programme. L'assembleur `examples/21_asm.py` construit les programmes d'exemple à partir de texte, et le désassembleur `examples/21_disasm.py` les réaffiche. Le programme de la machine à pile dans `programs/fact.asm` calcule un produit dans une boucle et l'affiche :

```bash
python3 21_asm.py stack programs/fact.asm build/fact.bin
xxd build/fact.bin
build/21_stackvm build/fact.bin
```

```text
build/fact.bin: 32 bytes, 18 instructions
00000000: 0105 0700 0101 0701 0600 091c 0601 0600  ................
00000010: 0407 0106 0001 0103 0700 0808 0601 0a00  ................
120
halt after 64 steps
```

Les octets ne montrent rien par eux-mêmes. Le désassembleur les transforme en un listing :

```bash
python3 21_disasm.py stack build/fact.bin
```

```text
0000  01 05           push 5
0002  07 00           store 0
0004  01 01           push 1
0006  07 01           store 1
0008  06 00           load 0
000a  09 1c           jz @28
000c  06 01           load 1
000e  06 00           load 0
0010  04              mul
0011  07 01           store 1
0013  06 00           load 0
0015  01 01           push 1
0017  03              sub
0018  07 00           store 0
001a  08 08           jmp @8
001c  06 01           load 1
001e  0a              print
001f  00              halt
```

Le listing a la structure d'une boucle : le saut à 0x1a revient au décalage 8, le saut conditionnel à 0x0a sort de la boucle vers le décalage 0x1c quand la variable 0 est nulle, et le corps multiplie la variable 1 par la variable 0 et décrémente la variable 0. Le programme affiche la factorielle de 5. Remarquez que les opérandes de saut sont des décalages en octets, pas des numéros d'instruction, et qu'un désassembleur qui les affiche sous la forme `@28` est plus utile s'il marque aussi la ligne cible avec une étiquette.

### Écrire un désassembleur

Un désassembleur pour un jeu d'instructions personnalisé a trois parties : une table qui décrit le jeu d'instructions, une boucle qui parcourt les octets et un formateur. Le code de `21_disasm.py` a cette forme :

```c-fragment
for each position in the bytecode:
    look up the opcode in the table
    if it is not in the table:
        print the byte as data and move on by one
    read as many operand bytes as the table says
    print the address, the bytes, the name and the formatted operands
    move on by 1 + the number of operand bytes
```

Gardez le jeu d'instructions sous forme de données, une table par machine, et gardez la boucle générique. Quand vous retrouvez une autre machine, vous remplissez une nouvelle table et vous ne touchez pas à la boucle. Quatre points font la différence entre un désassembleur qui fonctionne et un qui en a seulement l'air :

- **Octets inconnus.** Ne vous arrêtez jamais sur un octet qui n'est pas un code opération. Affichez-le comme une donnée et continuez, pour voir tout le programme et remarquer où votre table est incomplète.
- **Taille des instructions.** La taille fait partie de la table. Une taille fausse décale tout ce qui suit ; vérifiez donc la sortie sur un programme dont vous connaissez le comportement, par exemple un programme qui affiche un résultat que vous pouvez calculer à la main.
- **Sens des opérandes.** Un numéro de registre, une valeur immédiate, un décalage cible et un numéro de variable sont des sortes d'opérande différentes. Affichez chaque sorte à sa façon (`r2`, `5`, `@28`).
- **Cibles.** Pour un saut, la sortie utile est la ligne vers laquelle il mène. Rassemblez les cibles dans une première passe et affichez une étiquette à chacune dans une seconde.

Le même désassembleur gère la machine à registres avec une seconde table. Sa sortie pour `programs/sum.asm` :

```bash
python3 21_asm.py reg programs/sum.asm build/sum.bin
python3 21_disasm.py reg build/sum.bin
build/21_regvm build/sum.bin
```

```text
build/sum.bin: 32 bytes, 8 instructions
0000  10 00 0a 00     movi r0, 10
0004  10 01 00 00     movi r1, 0
0008  10 02 01 00     movi r2, 1
000c  11 01 01 00     add r1, r1, r0
0010  12 00 00 02     sub r0, r0, r2
0014  14 00 03 00     jnz r0, @3
0018  15 01 00 00     out r1
001c  ff 00 00 00     halt
55
halt after 35 steps
```

Ici la cible du saut est un numéro d'instruction, `@3`, et non un décalage en octets : le désassembleur d'une machine à instructions de taille fixe peut utiliser l'un ou l'autre, et la spécification doit dire lequel la machine utilise.

### Tracer la machine

Une lecture statique vous donne le jeu d'instructions. Pour le vérifier, et pour suivre un programme que vous ne pouvez pas encore désassembler, tracez la machine pendant son exécution : arrêtez-vous à la répartition et affichez le code opération à chaque fois. Le script `examples/21_trace.gdb` le fait pour la machine à pile. L'adresse du saut de répartition provient du désassemblage de cette version exacte ; c'est donc le seul endroit où vous adaptez le script à un autre binaire :

```bash
gdb -nx -q -batch -x 21_trace.gdb build/21_stackvm_stripped 2>&1 | grep "^offset" | head -9
```

```text
offset   0  opcode 01  stack depth 0
offset   2  opcode 07  stack depth 1
offset   4  opcode 01  stack depth 0
offset   6  opcode 07  stack depth 1
offset   8  opcode 06  stack depth 0
offset  10  opcode 09  stack depth 1
offset  12  opcode 06  stack depth 0
offset  14  opcode 06  stack depth 1
offset  16  opcode 04  stack depth 2
```

La trace est la liste des instructions exécutées, dans l'ordre, et elle correspond au listing : push 5, store 0, push 1, store 1, load 0, un saut conditionnel, et ainsi de suite. L'exécution complète fait 64 lignes, le même nombre que les pas indiqués par le programme. Une trace est aussi le moyen de contourner le problème d'un bytecode que vous ne savez pas encore analyser : elle ne montre que les instructions qui s'exécutent vraiment, avec les bonnes tailles, même si vous ne connaissez pas encore tout le jeu d'instructions.

### Un ordre de travail

1. **Triage.** Cherchez les signes d'une machine virtuelle : la grande fonction avec un point central, la table d'adresses de code, le tampon sans chaînes.
2. **Trouver l'état et la répartition.** Nommez le compteur ordinal, l'indice de pile ou le tableau de registres, et le chargement du code opération.
3. **Extraire la table.** Listez chaque code opération et son gestionnaire. Notez les entrées partagées et manquantes.
4. **Lire les gestionnaires.** Écrivez une ligne de la spécification pour chacun, avec les opérandes qu'il consomme.
5. **Désassembler.** Remplissez la table de votre désassembleur et vérifiez-la avec une trace.
6. **Lire le bytecode.** Trouvez les boucles, les appels et les instructions qui produisent une sortie, et décrivez le comportement en termes de ce que fait le programme.

## À essayer

Exécutez les commandes du laboratoire depuis `modules/21-virtual-machines-bytecode/examples`.

1. Compilez avec `make` et exécutez les deux programmes. Désassemblez `build/fact.bin` et `build/sum.bin` et décrivez en une phrase ce que calcule chaque programme.
2. Désassemblez `build/21_stackvm_stripped`. Trouvez le chargement du code opération, le contrôle de borne et le saut indirect. Combien de codes opération la machine a-t-elle ? Lesquels partagent un gestionnaire ?
3. Extrayez la table de la machine à registres de `build/21_regvm_stripped` et notez l'adresse du gestionnaire de chaque code opération. Lisez le gestionnaire du code 0x14 et décrivez son effet en une ligne.
4. Exécutez `21_trace.gdb` et comparez les premières lignes avec le désassemblage. Modifiez le programme de `programs/fact.asm` (par exemple, partez de 6), reconstruisez-le et comparez la longueur de la trace au nombre de pas indiqué par le programme.
5. Écrivez un programme de votre cru pour la machine à registres qui affiche 2 puissance 10 par multiplications répétées dans une boucle, en n'utilisant que les instructions de la table. Assemblez-le avec `21_asm.py` et exécutez-le.
6. Étendez la machine à pile dans votre propre copie de `21_stackvm.c` avec un nouveau code opération, par exemple `swap`. Ajoutez-le à la table de `21_disasm.py`. Quelles parties du désassembleur avez-vous modifiées ?
7. Marquez les cibles de saut : modifiez `21_disasm.py` pour qu'il affiche une étiquette sur chaque ligne vers laquelle un saut mène, et l'étiquette au lieu du décalage dans le saut.

## Pièges courants

- **Lire l'interpréteur comme s'il était le programme.** L'interpréteur est le même pour tous les programmes ; la logique est dans le bytecode. Si vous passez votre temps sur les gestionnaires après avoir la spécification, vous ne lisez pas le programme.
- **Une taille d'instruction fausse.** Une seule taille fausse rend tout ce qui suit absurde. Comparez avec une trace et cherchez le point où la sortie devient absurde.
- **Manquer les gestionnaires partagés.** Plusieurs codes opération peuvent partager un gestionnaire et choisir l'opération par un test à l'intérieur. Lisez la table avant les gestionnaires et cherchez les adresses répétées.
- **Ignorer les masques et les bornes.** Le `and 3` sur un numéro de registre, le contrôle de borne sur le code opération et le contrôle de la profondeur de la pile font partie du comportement. Mettez-les dans la spécification et testez ce qui se passe aux limites.
- **Confondre les deux compteurs ordinaux.** Le registre `rip` exécute l'interpréteur ; le compteur ordinal virtuel indexe le bytecode. Les cibles des sauts dans le bytecode sont des valeurs du second.
- **Se fier à une adresse de table d'une autre version.** Les adresses d'un script de trace ou d'une note appartiennent à un seul binaire. Recherchez-les de nouveau pour chaque version.

## Résumé

- Une machine virtuelle est une boucle d'interpréteur avec un état et un bloc de bytecode ; le désassemblage montre l'interpréteur, et la logique est dans le bytecode.
- Les machines sont à pile (opérandes implicites) ou à registres (opérandes nommés) ; la répartition se fait par un `switch`, une table de gestionnaires ou des sauts enfilés.
- On en reconnaît une à une boucle avec un point central, une table d'adresses de code, un chargement d'octet avec contrôle de borne et des gestionnaires courts qui modifient les mêmes quelques variables.
- Retrouver le jeu d'instructions, c'est trouver l'état et la répartition, extraire la table et lire chaque gestionnaire pour en faire une ligne de spécification.
- Un désassembleur est une table plus une boucle générique ; il affiche les octets inconnus comme des données et se vérifie avec une trace de la machine en cours d'exécution.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 22, le code managé.
- Exercices qui utilisent ce module: [Exercice 20](../../exercises/20-capstone-tiny-vm/README.fr.md).
