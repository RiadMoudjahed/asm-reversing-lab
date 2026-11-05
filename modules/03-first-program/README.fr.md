<!-- source: README.md @ sha256:2dd2136139ab -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 03. Votre premier programme

Durée : environ 2 heures. Prérequis : [01. Représentation des données](../01-data-representation/README.fr.md), [02. Le processeur et ses registres](../02-cpu-and-registers/README.fr.md)

## Pourquoi c'est important

Un appel système est le seul moyen pour un programme sans bibliothèques de faire quoi que ce soit de visible : afficher, lire un fichier, se terminer. Une fois que vous savez émettre des appels système en assembleur, vous pouvez écrire de vrais programmes et lire ces mêmes appels dans n'importe quel binaire que vous analysez. Bien des enquêtes de rétro-ingénierie commencent par la question : quels appels système ce programme effectue-t-il ?

## Notions

### Anatomie d'une instruction source

Chaque ligne de code source NASM comporte les mêmes parties facultatives :

```text
label:    mnemonic   operands        ; comment
```

- Une **étiquette** (label) nomme une adresse. `_start:` et `message:` sont des étiquettes.
- Un **mnémonique** nomme une instruction, comme `mov` ou `syscall`.
- Les **opérandes** sont les valeurs sur lesquelles travaille l'instruction, séparées par des virgules. La destination vient en premier : `mov eax, 1` met 1 dans `eax`.
- Un **commentaire** commence par `;` et court jusqu'à la fin de la ligne. L'assembleur l'ignore.

Les directives comme `section`, `global`, `db` et `equ` ont l'air semblables, mais ce sont des ordres adressés à l'assembleur, pas au processeur. Elles ne produisent pas de code machine à elles seules.

### Sections

Un programme est stocké dans des sections qui ont des rôles et des droits différents :

| Section | Contient | Droits | Occupe de la place dans le fichier |
|---|---|---|---|
| `.text` | Code | lecture, exécution | oui |
| `.rodata` | Constantes | lecture | oui |
| `.data` | Variables initialisées | lecture, écriture | oui |
| `.bss` | Variables initialement à zéro | lecture, écriture | non |

`.bss` n'occupe pas de place dans le fichier car elle ne contient qu'une taille ; c'est le chargeur (loader) qui fournit la mémoire mise à zéro. Vous pouvez voir ces propriétés dans la table des sections du programme d'exemple :

```text
$ readelf -S -W build/03_data_bss
  [ 1] .text    PROGBITS  0000000000401000 001000 000039 00  AX  0   0 16
  [ 2] .rodata  PROGBITS  0000000000402000 002000 000001 00   A  0   0  4
  [ 3] .data    PROGBITS  0000000000403004 002004 000001 00  WA  0   0  4
  [ 4] .bss     NOBITS    0000000000403008 002005 000008 00  WA  0   0  4
```

La colonne `Flg` affiche `A` (allouée), `X` (exécutable) et `W` (inscriptible). `PROGBITS` signifie que la section a des octets dans le fichier ; `NOBITS` signifie qu'elle n'en a pas.

### Définir des données

| Directive | Signification |
|---|---|
| `db`, `dw`, `dd`, `dq` | Placent des octets, des mots (2 octets), des doubles mots (4) ou des quadruples mots (8) dans le fichier |
| `resb`, `resw`, `resd`, `resq` | Réservent de la place sans placer d'octets (utilisées dans `.bss`) |
| `name equ expression` | Définit une constante ; aucune mémoire n'est utilisée |
| `$` | L'adresse de la position courante |

Le motif `message_len equ $ - message`, écrit juste après une chaîne, fait calculer la longueur de la chaîne par l'assembleur. Utilisez-le plutôt que de compter à la main. Les chaînes en assembleur sont de simples octets. Elles n'ont pas de terminateur à moins que vous n'en ajoutiez un : le `10` (un saut de ligne) n'est qu'un octet de plus que vous placez vous-même.

### Le point d'entrée

Le programme démarre à l'étiquette `_start`. La directive `global _start` rend l'étiquette visible de l'éditeur de liens, qui enregistre son adresse comme point d'entrée.

### Assembler et lier

Deux outils transforment le source en programme :

```bash
nasm -f elf64 prog.asm -o prog.o      # assemble: source to object file
ld prog.o -o prog                     # link: object file to executable
```

Pour du code 32 bits, modifiez les deux commandes :

```bash
nasm -f elf32 prog.asm -o prog.o
ld -m elf_i386 prog.o -o prog
```

### Appels système

Un appel système demande au noyau du système d'exploitation de faire quelque chose en votre nom. La convention sous Linux dépend du mode :

| | 64 bits | 32 bits |
|---|---|---|
| Instruction | `syscall` | `int 0x80` |
| Numéro d'appel | `rax` | `eax` |
| Arguments 1 à 6 | `rdi`, `rsi`, `rdx`, `r10`, `r8`, `r9` | `ebx`, `ecx`, `edx`, `esi`, `edi`, `ebp` |
| Résultat | `rax` | `eax` |

Les numéros d'appel diffèrent aussi d'un mode à l'autre. `write` vaut 1 en code 64 bits et 4 en code 32 bits. `exit` vaut 60 et 1. Le tableau complet se trouve dans l'[aide-mémoire des appels système Linux](../../cheatsheets/syscalls-linux.fr.md).

`write(fd, buffer, length)` envoie `length` octets à partir de `buffer` vers le descripteur de fichier `fd`. Le descripteur 1 est la sortie standard et le 2 la sortie d'erreur standard. L'appel renvoie le nombre d'octets écrits.

### Le programme complet

Voici [03_hello64.asm](examples/03_hello64.asm). Chaque ligne est commentée parce que c'est votre premier programme.

```nasm
bits 64
default rel

SYS_WRITE   equ 1               ; system call numbers (64-bit table)
SYS_EXIT    equ 60
STDOUT      equ 1               ; file descriptor of standard output

section .rodata
message:    db "Hello, assembly!", 10       ; 10 is the newline byte
message_len equ $ - message                 ; bytes from 'message' to here

section .text
global _start                   ; make the entry point visible to the linker

_start:
    mov     eax, SYS_WRITE      ; rax = call number
    mov     edi, STDOUT         ; rdi = argument 1: file descriptor
    lea     rsi, [message]      ; rsi = argument 2: address of the text
    mov     edx, message_len    ; rdx = argument 3: number of bytes
    syscall                     ; enter the kernel: write(1, message, len)

    mov     eax, SYS_EXIT       ; rax = call number
    xor     edi, edi            ; rdi = argument 1: exit status 0
    syscall                     ; exit(0)
```

`lea rsi, [message]` charge l'adresse de `message` ; elle ne lit pas le texte. Le module 04 traite `lea` et l'adressage en détail. `xor edi, edi` est la manière habituelle de mettre un registre à zéro.

[03_hello32.asm](examples/03_hello32.asm) fait la même chose en code 32 bits avec `int 0x80`. [03_data_bss.asm](examples/03_data_bss.asm) montre les quatre sections dans un seul programme.

### Le point de vue de la rétro-ingénierie

L'assembleur conserve votre structure, mais pas vos noms. Ouvrez le programme que vous venez de construire :

```text
$ objdump -d -M intel build/03_hello64
0000000000401000 <_start>:
  401000:  mov  eax,0x1
  401005:  mov  edi,0x1
  40100a:  lea  rsi,[rip+0xfef]        # 402000 <message>
  401011:  mov  edx,0x11
  401016:  syscall
  401018:  mov  eax,0x3c
  40101d:  xor  edi,edi
  40101f:  syscall
```

Ce qui a survécu : les instructions et les étiquettes `_start` et `message` (la table des symboles les conserve). Ce qui a disparu : les constantes `SYS_WRITE`, `STDOUT` et `message_len`, devenues les nombres 1, 1 et 0x11 (17), ainsi que tous les commentaires. C'est ce que vous verrez dans des programmes sans source : des nombres là où l'auteur avait des noms. Reconnaître que `mov eax, 1` avant `syscall` signifie `write` est la première compétence de rétro-ingénierie de ce cours.

## À vous de jouer

Construisez les exemples et exécutez-les :

```bash
cd modules/03-first-program/examples
make run
```

Sortie attendue :

```text
== 03_hello64
Hello, assembly!
== 03_data_bss
A
== 03_hello32
Hello, assembly!
```

Observez maintenant les registres changer dans le débogueur :

```bash
gdb -q build/03_hello64
```

À l'invite `gef>`, tapez ces commandes une par une :

```text
break _start
run
stepi 4
info registers rax rdi rdx rip
x/s $rsi
```

GEF affiche un grand panneau après chaque arrêt. Cherchez les lignes des registres. Après quatre commandes `stepi`, les quatre premières instructions ont été exécutées et vous voyez :

```text
rax            0x1                 0x1
rdi            0x1                 0x1
rdx            0x11                0x11
rip            0x401016            0x401016 <_start+22>
0x402000 <message>: "Hello, assembly!\n,"
```

Les registres contiennent les trois arguments et le numéro d'appel, et `rip` pointe sur l'instruction `syscall`. La chaîne affiche une `,` superflue après le saut de ligne : `x/s` affiche jusqu'à trouver un octet nul, et `message` n'en contient aucun, donc il lit un octet de ce qui suit. Exécutez maintenant `stepi` une fois de plus. Le texte apparaît à l'écran et les registres changent :

- `rax` vaut maintenant `0x11` : le résultat de `write`, les 17 octets écrits.
- `rcx` contient `0x401018`, l'adresse de l'instruction suivante, et `r11` contient `0x302`. L'instruction `syscall` écrase les deux. Ne conservez jamais une valeur dans `rcx` ou `r11` à travers un `syscall`.

Pour finir, regardez le même programme de l'extérieur :

```bash
strace ./build/03_hello64
```

```text
execve("./build/03_hello64", ["./build/03_hello64"], ...) = 0
write(1, "Hello, assembly!\n", 17Hello, assembly!
)      = 17
exit(0)                                 = ?
+++ exited with 0 +++
```

Exécuter `strace ./build/03_hello32` montre les mêmes appels et ajoute la ligne `[ Process PID=... runs in 32 bit mode. ]`.

## Pièges courants

- **Oublier `global _start`.** L'éditeur de liens avertit avec `cannot find entry symbol _start; defaulting to ...` et choisit le début de `.text`. Le programme peut quand même fonctionner, par hasard, et cesser de fonctionner dès que vous ajoutez du code avant `_start`. Traitez l'avertissement comme une erreur.
- **Mélanger les formats.** `nasm -f elf32` avec un `ld` ordinaire produit une erreur de liaison ou un programme faux. Utilisez `-f elf32` avec `ld -m elf_i386`, et `-f elf64` avec `ld` ordinaire.
- **Oublier `exit`.** Après votre dernière instruction, le processeur exécute les octets qui suivent et le programme se termine sur une erreur de segmentation.
- **Utiliser `int 0x80` en code 64 bits.** Cela fonctionne encore sous Linux, mais utilise la table d'appels 32 bits et des arguments 32 bits, si bien que les adresses au-delà de 4 Gio échouent. En code 64 bits, utilisez `syscall`.
- **Mal compter une chaîne.** Le saut de ligne ajouté avec `10` est un octet de plus. Utilisez `$ - label`.
- **Supposer que les chaînes se terminent par zéro.** Ce n'est pas le cas. Passez la longueur à `write` et placez vous-même un `0` lorsqu'une routine ultérieure l'attend.

## Résumé

- Une instruction source est `label: mnemonic operands ; comment` ; les directives s'adressent à l'assembleur et ne produisent pas de code à elles seules.
- Le code vit dans `.text`, les constantes dans `.rodata`, les variables dans `.data` et `.bss`.
- Linux 64 bits utilise `syscall` avec le numéro dans `rax` et les arguments dans `rdi`, `rsi`, `rdx` ; le 32 bits utilise `int 0x80` avec `eax`, `ebx`, `ecx`, `edx`.
- Les constantes et les commentaires disparaissent à l'assemblage ; les nombres restent.
- `$ - label` calcule une longueur ; `exit` doit terminer tout programme.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 04, déplacer des données et modes d'adressage.
- Exercices qui utilisent ce module : [Exercice 01](../../exercises/01-signal-flare/README.fr.md), [Exercice 02](../../exercises/02-two-worlds/README.fr.md), [Exercice 05](../../exercises/05-ticket-counter/README.fr.md).
