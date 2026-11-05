<!-- source: README.md @ sha256:cdab37d79f42 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 11. La boîte à outils du rétro-ingénieur

Durée : environ 3 heures. Prérequis : [10. Fondamentaux des systèmes d'exploitation pour le rétro-ingénieur](../10-os-for-reversers/README.fr.md)

## Pourquoi c'est important

Le déroulement du module 09 ne fonctionne que si vous pouvez exécuter chaque étape rapidement. Ce module présente les outils dans l'ordre où vous les utilisez : identifier le fichier, lire sa structure, le désassembler, l'explorer de façon interactive, l'exécuter sous un débogueur et l'observer de l'extérieur. Il utilise un petit programme, construit à partir de `examples/11_target.c`, afin que vous puissiez comparer ce que chaque outil montre du même code.

## Concepts

### Le programme d'entraînement

`11_target` prend un mot en argument, affiche un nombre calculé à partir de lui, et affiche `access granted` pour un mot précis et `denied` pour tout autre. Construisez-le et exécutez-le :

```bash
make
build/11_target hello
build/11_target open-sesame
```

```text
score 99162322
denied
score -1484336485
access granted
```

Les commandes de ce module s'exécutent depuis `modules/11-toolkit/examples`. Le programme est volontairement simple : il contient une boucle (`score`), une comparaison (`check`) et un appel de bibliothèque (`strcmp`).

### Étape 1 : identifier le fichier

`file` lit les en-têtes et dit ce qu'est le fichier :

```bash
file build/11_target
```

```text
build/11_target: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=3690f8d5a33ef0140c7ee95bff861bc16fb6e86b, for GNU/Linux 3.2.0, not stripped
```

`strings` affiche les suites de caractères imprimables. `-t x` ajoute le décalage dans le fichier en hexadécimal, et `-n` fixe la longueur minimale :

```bash
strings -t x -n 6 build/11_target | grep sesame
```

```text
   2038 open-sesame
```

`xxd` montre les octets bruts. Voici le début de l'en-tête ELF :

```bash
xxd -l 32 build/11_target
```

```text
00000000: 7f45 4c46 0201 0100 0000 0000 0000 0000  .ELF............
00000010: 0200 3e00 0100 0000 7010 4000 0000 0000  ..>.....p.@.....
```

Les quatre premiers octets `7f 45 4c 46` sont le nombre magique (`.ELF`). L'octet 4 vaut `02` pour 64 bits, l'octet 5 vaut `01` pour petit-boutiste. Au décalage 0x10, le type est `02 00` (exécutable) et la machine est `3e 00` (x86-64). Au décalage 0x18, le point d'entrée de 8 octets est `70 10 40 00 00 00 00 00`, soit l'adresse 0x401070. Le module 12 explique chaque champ.

### Étape 2 : lire la structure

`readelf` affiche la structure du fichier sans interpréter le code :

```bash
readelf -h build/11_target | grep -E "Class|Type|Machine|Entry"
readelf -d build/11_target | grep NEEDED
readelf -S -W build/11_target | grep -E " \.(text|rodata|data|bss|got)"
readelf -s build/11_target | grep -E "FUNC.*(score|check|main)"
```

```text
  Class:                             ELF64
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Entry point address:               0x401070
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
  [14] .text             PROGBITS        0000000000401070 001070 0001b5 00  AX  0   0 16
  [16] .rodata           PROGBITS        0000000000402000 002000 000044 00   A  0   0  8
  [22] .got              PROGBITS        0000000000403fd8 002fd8 000010 08  WA  0   0  8
  [23] .got.plt          PROGBITS        0000000000403fe8 002fe8 000038 08  WA  0   0  8
  [24] .data             PROGBITS        0000000000404020 003020 000010 00  WA  0   0  8
  [25] .bss              NOBITS          0000000000404040 003030 000010 00  WA  0   0 32
    28: 0000000000401156    46 FUNC    GLOBAL DEFAULT   14 score
    34: 00000000004011a1   132 FUNC    GLOBAL DEFAULT   14 main
    36: 0000000000401184    29 FUNC    GLOBAL DEFAULT   14 check
```

Les sections indiquent où se trouvent le code (`.text`, indicateurs `AX` : alloué et exécutable), les constantes (`.rodata`, `A`) et les données (`.data`, `.bss`, indicateurs `WA` : inscriptible). La table des symboles donne l'adresse et la taille de chaque fonction. Un binaire dépouillé (stripped) n'a pas de noms de fonctions ; vous les redécouvrez vous-même.

### Étape 3 : désassembler

`objdump -d` désassemble les sections exécutables. Il affiche la syntaxe AT&T par défaut et la syntaxe Intel avec `-M intel`. Ce cours utilise la syntaxe Intel, mais la sortie des compilateurs et beaucoup d'outils utilisent AT&T par défaut : il faut savoir lire les deux.

```bash
objdump -d -M intel --no-show-raw-insn build/11_target | awk '/<check>:/{p=1} p{print} /^$/{if(p)exit}'
objdump -d --no-show-raw-insn build/11_target | awk '/<check>:/{p=1} p{print} /^$/{if(p)exit}'
```

```text
0000000000401184 <check>:
  401184:    sub    rsp,0x8
  401188:    lea    rsi,[rip+0xea9]        # 402038 <secret>
  40118f:    call   401040 <strcmp@plt>
  401194:    test   eax,eax
  401196:    sete   al
  401199:    movzx  eax,al
  40119c:    add    rsp,0x8
  4011a0:    ret
```

```text
0000000000401184 <check>:
  401184:    sub    $0x8,%rsp
  401188:    lea    0xea9(%rip),%rsi        # 402038 <secret>
  40118f:    call   401040 <strcmp@plt>
  401194:    test   %eax,%eax
  401196:    sete   %al
  401199:    movzbl %al,%eax
  40119c:    add    $0x8,%rsp
  4011a0:    ret
```

Lecture de la fonction : elle charge l'adresse de `secret` dans `rsi` (le deuxième argument), appelle `strcmp` (le premier argument est déjà dans `rdi`), puis transforme « le résultat est nul » en un 0 ou un 1 dans `eax`. La fonction renvoie si les deux chaînes sont égales.

#### Lire la syntaxe AT&T

| | Intel | AT&T |
|---|---|---|
| Ordre des opérandes | destination, source | source, destination |
| Registres | `eax` | `%eax` |
| Constantes | `0x8` | `$0x8` |
| Mémoire | `[rsi + rcx*4 + 8]` | `8(%rsi,%rcx,4)` |
| Taille de l'opérande | Dans l'opérande : `dword ptr [rax]` | Dans le suffixe du mnémonique : `movl`, `movq`, `movzbl` |
| Exemple | `movzx eax, al` | `movzbl %al, %eax` |

Pour convertir, inversez les opérandes et retirez les décorations. Les lettres de suffixe sont `b` (octet), `w` (mot), `l` (double mot), `q` (quadruple mot). Dans `movzbl`, les deux lettres signifient « d'un octet vers un long ».

### Étape 4 : explorer de façon interactive avec Rizin

Rizin est un désassembleur et un cadre d'analyse en ligne de commande. `-A` analyse le fichier au chargement, `-q` quitte après les commandes, et `-c` les exécute.

```bash
rizin -q -e scr.color=0 -A -c "pdf @ sym.check" build/11_target
```

```text
            ; CALL XREF from main @ 0x4011cf
┌ sym.check(const char *s1);
│           ; arg const char *s1 @ rdi
│           0x00401184      sub   rsp, 0x08
│           0x00401188      lea   rsi, qword obj.secret                ; 0x402038 ; "open-sesame" ; const char *s2
│           0x0040118f      call  sym.imp.strcmp                       ; int strcmp(const char *s1, const char *s2)
│           0x00401194      test  eax, eax
│           0x00401196      setz  al
│           0x00401199      movzx eax, al
│           0x0040119c      add   rsp, 0x08
└           0x004011a0      ret
```

Rizin a fait plus qu'`objdump` : il a nommé l'argument, résolu la référence de données vers la chaîne et montré le prototype de `strcmp`. Les commandes que vous utiliserez le plus :

| Commande | Signification |
|---|---|
| `aaa` | Analyse tout (l'option `-A` le fait au chargement) |
| `afl` | Liste les fonctions trouvées |
| `pdf @ sym.check` | Affiche le désassemblage d'une fonction |
| `axt @ sym.check` | Liste les références croisées vers `sym.check` (qui l'appelle) |
| `iz` | Liste les chaînes des sections de données |
| `s 0x401184` | Se positionne à une adresse |
| `?` | Aide sur n'importe quelle commande ; ajoutez `?` à un préfixe comme `a?` |

```bash
rizin -q -e scr.color=0 -A -c "axt @ sym.check; iz~sesame" build/11_target
```

```text
main 0x4011cf [CALL] call sym.check
0x00002038 0x00402038  11   12 .rodata ascii open-sesame
```

La première ligne indique que `main` appelle `check` en 0x4011cf. La seconde indique que la chaîne se trouve au décalage de fichier 0x2038 et à l'adresse virtuelle 0x402038, ce qui correspond à `strings -t x`.

### Étape 5 : Ghidra

Ghidra est une suite graphique avec un désassembleur et un décompilateur. Vous l'avez installée sur votre machine au module 00. Voici le déroulement ; il est le même pour chaque binaire du cours.

1. Créez un projet, importez le binaire depuis `exercises/NN-name/bin/` et acceptez le format détecté.
2. Ouvrez le fichier dans le navigateur de code et laissez l'analyse par défaut s'exécuter.
3. Regardez l'arbre des symboles pour les fonctions et la fenêtre des chaînes définies (Defined Strings) pour les chaînes. Double-cliquez sur une chaîne, puis regardez ses références croisées pour trouver le code qui l'utilise.
4. Lisez le listing (désassemblage) et le décompilateur côte à côte. Le décompilateur est approximatif : comparez-le avec les instructions quand quelque chose semble faux.
5. Améliorez le programme au fur et à mesure. Renommez fonctions et variables (touche par défaut `L`), changez les types (touche par défaut `Ctrl+L` sur une variable) et ajoutez des commentaires (touche par défaut `;`). Affichez les références croisées d'un élément avec la touche par défaut `X`.

L'intérêt de renommer et de retyper est que le décompilateur redessine sa sortie avec vos noms et vos types, si bien que chaque étape facilite la lecture de la fonction suivante. Les raccourcis clavier peuvent varier selon les versions et les réglages de Ghidra ; les menus Edit et Navigation listent les raccourcis actuels.

### Étape 6 : exécuter sous un débogueur

GDB exécute le programme sous votre contrôle. L'image du laboratoire charge GEF, une extension qui affiche les registres, la pile et le code autour de `rip` à chaque arrêt. La forme par lots ci-dessous désactive les extensions (`-nx`) pour que la sortie soit du texte simple ; au laboratoire, vous utiliserez normalement la forme interactive.

```bash
gdb -nx -q -batch -ex "break check" -ex "run hello" -ex "info registers rdi" \
    -ex "x/s \$rdi" -ex "x/s 0x402038" -ex "finish" -ex "print \$rax" build/11_target
```

```text
Breakpoint 1, 0x0000000000401184 in check ()
rdi            0x7fffd18f3e21      140736709213729
0x7fffd18f3e21:    "hello"
0x402038 <secret>:    "open-sesame"
0x00000000004011d4 in main ()
$1 = 0
```

Au point d'arrêt, `rdi` pointe vers la chaîne d'argument, « hello ». Le second `x/s` affiche la chaîne à laquelle `check` la compare. Après `finish`, `check` est revenue dans `main` et `rax` vaut 0, le cas « denied ». L'adresse dans `rdi` est sur la pile et varie d'une exécution à l'autre.

| Commande | Signification |
|---|---|
| `break check` or `b *0x401184` | S'arrête à une fonction ou à une adresse précise |
| `run ARGS` | Démarre le programme avec des arguments |
| `continue` | Reprend |
| `stepi`, `nexti` | Exécute une instruction, en entrant dans les appels ou en les enjambant |
| `finish` | Exécute jusqu'au retour de la fonction courante |
| `info registers` | Affiche les registres ; ajoutez des noms pour n'afficher que ceux-là |
| `x/NFU ADDR` | Examine la mémoire : `N` unités de format `F` et de taille `U`, comme `x/4gx $rsp`, `x/s $rdi`, `x/5i $pc` |
| `watch EXPR` | S'arrête quand une valeur change |
| `set disassembly-flavor intel` | Affiche la syntaxe Intel (l'image du laboratoire la définit) |

Une seconde session, parcourant la boucle de `score` :

```bash
gdb -nx -q -batch -ex "break score" -ex "run ab" -ex "x/i \$pc" -ex "stepi 3" \
    -ex "x/i \$pc" -ex "info registers eax ecx" build/11_target
```

```text
Breakpoint 1, 0x0000000000401156 in score ()
=> 0x401156 <score>:    movzbl (%rdi),%eax
0x000000000040115d in score ()
=> 0x40115d <score+7>:    mov    $0x0,%ecx
eax            0x61                97
ecx            0x403e00            4210176
```

Le premier caractère de « ab » est chargé : `eax` vaut 0x61, c'est-à-dire « a ». La valeur de `ecx` est ce que contenait le registre avant que le code ne l'initialise, ce qui se produit en 0x40115d. GDB affiche ici la syntaxe AT&T parce que les extensions étaient désactivées ; la configuration de l'image du laboratoire la bascule en Intel.

### Étape 7 : observer de l'extérieur

`strace` et `ltrace`, présentés au module 10, sont les outils d'observation les plus rapides :

```bash
strace -e trace=write build/11_target hello
ltrace build/11_target hello
```

```text
write(1, "score 99162322\ndenied\n", 22score 99162322
denied
) = 22
+++ exited with 1 +++
```

```text
__printf_chk(2, 0x402014, 0x5e918d2, 0x5e918d2)  = 15
strcmp("hello", "open-sesame")                   = -7
puts("denied")                                   = 7
score 99162322
denied
+++ exited (status 1) +++
```

`ltrace` révèle directement l'argument de `strcmp`, ce qui est souvent le moyen le plus rapide de savoir à quoi un programme compare son entrée. Le premier appel est `printf` compilé en `__printf_chk` ; le deuxième argument est un pointeur vers la chaîne de format, et les deux autres sont le nombre affiché (0x5e918d2 vaut 99162322).

### Choisir un outil

| Question | Commencer par |
|---|---|
| De quel type de fichier s'agit-il ? | `file`, `xxd` |
| Quel texte contient-il ? | `strings -t x` |
| Quelles sections, quels symboles et quelles bibliothèques ? | `readelf` |
| Que font les instructions ? | `objdump`, Rizin, Ghidra |
| Qui appelle cette fonction ou utilise cette chaîne ? | Rizin `axt`, références croisées de Ghidra |
| Quelles sont les valeurs réelles à l'exécution ? | GDB |
| Que fait-il vu de l'extérieur ? | `strace`, `ltrace` |

## Essayez

1. Sans exécuter le programme avec le bon mot, trouvez le mot attendu avec des outils statiques uniquement. Utilisez `strings`, puis confirmez avec `rizin` en trouvant à quoi `check` compare.
2. Avec GDB, arrêtez-vous dans `score` avec l'argument `abc` et parcourez la boucle jusqu'à ce que `ecx` contienne le score final. Comparez-le avec le nombre affiché par le programme.
3. Reconvertissez sur papier le listing AT&T de `check` en syntaxe Intel, puis comparez-le avec le listing Intel.
4. Avec `ltrace`, trouvez quelle fonction `main` appelle pour afficher le score.

## Pièges courants

- **Lire de l'AT&T comme de l'Intel.** L'ordre des opérandes est inversé. Vérifiez la syntaxe avant de lire.
- **Prendre `strings` pour le tableau complet.** Les chaînes construites ou décodées à l'exécution ne sont pas dans le fichier.
- **Croire le décompilateur.** C'est une reconstruction. Comparez avec les instructions.
- **Oublier que l'analyse est une supposition.** L'analyse automatique peut manquer des fonctions ou étiqueter des données comme du code. Vérifiez les limites des fonctions quand un résultat paraît étrange.
- **S'arrêter au mauvais endroit.** Un point d'arrêt sur un nom de fonction s'arrête à sa première instruction ; les arguments sont déjà dans les registres, mais les variables locales ne sont pas encore initialisées.

## Résumé

- Identifiez un fichier avec `file`, `strings` et `xxd` ; lisez sa structure avec `readelf`.
- Désassemblez avec `objdump`, Rizin ou Ghidra ; connaissez les syntaxes Intel et AT&T.
- Rizin offre une analyse interactive en ligne de commande ; Ghidra ajoute un décompilateur et permet de renommer, retyper et commenter.
- GDB montre les valeurs réelles : posez des points d'arrêt, avancez pas à pas et examinez registres et mémoire.
- `strace` et `ltrace` montrent les appels système et de bibliothèque d'un programme sans lire le code.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 12, les formats exécutables.
- Aide-mémoire : [GDB](../../cheatsheets/gdb.fr.md), [Rizin](../../cheatsheets/rizin.fr.md), [Ghidra](../../cheatsheets/ghidra.fr.md).
