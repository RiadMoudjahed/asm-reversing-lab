<!-- source: README.md @ sha256:ea5aa64cd376 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 12. Formats exécutables : ELF et PE

Durée : environ 3 heures. Prérequis : [11. La boîte à outils du rétro-ingénieur](../11-toolkit/README.fr.md)

## Pourquoi c'est important

Tout exécutable est un fichier qui dit au chargeur ce qu'il faut placer en mémoire, où, avec quelles permissions, quelles bibliothèques faire venir et par où démarrer. Si vous savez lire ces en-têtes, vous identifiez un binaire d'un coup d'œil, trouvez son code et ses données sans désassembleur, comprenez importations et exportations, et remarquez un fichier construit ou modifié de façon inhabituelle. Linux utilise ELF et Windows utilise PE ; ce module couvre les deux et se termine par l'écriture à la main d'un exécutable complet.

## Concepts

### Deux vues d'un même fichier

Les deux formats décrivent le même fichier de deux manières.

- La **vue de chargement** dit comment projeter le fichier en mémoire : quelles plages d'octets, à quelles adresses, avec quelles permissions. Le chargeur n'utilise que cette vue.
- La **vue d'édition de liens** découpe le fichier en parties nommées (sections) pour l'éditeur de liens, le débogueur et le rétro-ingénieur.

Distinguez deux sortes d'adresses. Un **décalage dans le fichier** compte les octets depuis le début du fichier. Une **adresse virtuelle** est l'endroit où ces octets se trouveront en mémoire. Elles diffèrent parce que le chargeur aligne les parties sur des frontières de page et parce que certaines parties, comme les données non initialisées, occupent de la mémoire mais pas de place dans le fichier. Convertir de l'une à l'autre est une tâche quotidienne.

### ELF

Un fichier ELF commence par l'**en-tête ELF**, suivi de la table des en-têtes de programme, des données des sections et de la table des en-têtes de section.

#### L'en-tête ELF

Sortie réelle pour le programme d'exemple construit à partir de `examples/12_sample.c`, exécuté depuis `modules/12-executable-formats/examples` :

```bash
make
readelf -h build/12_sample
```

```text
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x401050
  Start of program headers:          64 (bytes into file)
  Start of section headers:          14008 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         13
  Size of section headers:           64 (bytes)
  Number of section headers:         30
```

| Décalage | Taille | Champ | Signification |
|---|---|---|---|
| 0x00 | 4 | nombre magique | `7f 45 4c 46`, les octets `.ELF` |
| 0x04 | 1 | classe | 1 pour 32 bits, 2 pour 64 bits |
| 0x05 | 1 | données | 1 pour petit-boutiste, 2 pour gros-boutiste |
| 0x10 | 2 | type | 2 exécutable, 3 objet partagé (aussi les exécutables indépendants de la position) |
| 0x12 | 2 | machine | 0x3e pour x86-64, 3 pour x86 |
| 0x18 | 8 | entrée | adresse virtuelle de la première instruction |
| 0x20 | 8 | décalage des en-têtes de programme | décalage dans le fichier de la table des segments |
| 0x28 | 8 | décalage des en-têtes de section | décalage dans le fichier de la table des sections |
| 0x36 | 2 | taille d'une entrée d'en-tête de programme | 56 dans les fichiers 64 bits |
| 0x38 | 2 | nombre d'en-têtes de programme | nombre de segments |
| 0x3a | 2 | taille d'une entrée d'en-tête de section | 64 dans les fichiers 64 bits |
| 0x3c | 2 | nombre d'en-têtes de section | nombre de sections |

Les décalages sont ceux du format 64 bits. L'en-tête 32 bits est plus petit parce que les adresses et les décalages font 4 octets.

#### En-têtes de programme : la vue de chargement

```bash
readelf -l -W build/12_sample
```

```text
Program Headers:
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  PHDR           0x000040 0x0000000000400040 0x0000000000400040 0x0002d8 0x0002d8 R   0x8
  INTERP         0x000318 0x0000000000400318 0x0000000000400318 0x00001c 0x00001c R   0x1
      [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]
  LOAD           0x000000 0x0000000000400000 0x0000000000400000 0x000548 0x000548 R   0x1000
  LOAD           0x001000 0x0000000000401000 0x0000000000401000 0x000179 0x000179 R E 0x1000
  LOAD           0x002000 0x0000000000402000 0x0000000000402000 0x0000c4 0x0000c4 R   0x1000
  LOAD           0x002df8 0x0000000000403df8 0x0000000000403df8 0x00022c 0x000230 RW  0x1000
  DYNAMIC        0x002e08 0x0000000000403e08 0x0000000000403e08 0x0001d0 0x0001d0 RW  0x8
  GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RW  0x10
  GNU_RELRO      0x002df8 0x0000000000403df8 0x0000000000403df8 0x000208 0x000208 R   0x1
```

Le listing est abrégé aux entrées utiles ici. Chaque entrée `LOAD` demande au chargeur de projeter `FileSiz` octets à partir du décalage `Offset` du fichier à l'adresse `VirtAddr`, avec les permissions de `Flg`, et d'étendre la projection avec des zéros jusqu'à `MemSiz`. Il y en a quatre : une partie en lecture seule contenant les en-têtes, une partie lisible et exécutable (le code), une partie en lecture seule (constantes) et une partie en lecture-écriture (données). Dans la dernière, `MemSiz` dépasse `FileSiz` de 4 octets : c'est le `.bss`, de la mémoire à zéro au démarrage qui n'a pas besoin de place dans le fichier.

| Type | Signification |
|---|---|
| `LOAD` | Une plage à projeter en mémoire |
| `INTERP` | Le chemin du chargeur dynamique à démarrer en premier |
| `DYNAMIC` | La table que lit le chargeur dynamique : bibliothèques requises, tables de symboles, relocalisations |
| `GNU_STACK` | Permissions de la pile ; ici lecture et écriture sans exécution |
| `GNU_RELRO` | Une plage à passer en lecture seule une fois les relocalisations appliquées |

#### Sections : la vue d'édition de liens

```bash
readelf -S -W build/12_sample
```

```text
  [Nr] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
  [12] .init             PROGBITS        0000000000401000 001000 00001b 00  AX  0   0  4
  [13] .plt              PROGBITS        0000000000401020 001020 000030 10  AX  0   0 16
  [14] .text             PROGBITS        0000000000401050 001050 00011c 00  AX  0   0 16
  [15] .fini             PROGBITS        000000000040116c 00116c 00000d 00  AX  0   0  4
  [16] .rodata           PROGBITS        0000000000402000 002000 00000e 00   A  0   0  4
  [21] .dynamic          DYNAMIC         0000000000403e08 002e08 0001d0 10  WA  7   0  8
  [22] .got              PROGBITS        0000000000403fd8 002fd8 000010 08  WA  0   0  8
  [23] .got.plt          PROGBITS        0000000000403fe8 002fe8 000028 08  WA  0   0  8
  [24] .data             PROGBITS        0000000000404010 003010 000014 00  WA  0   0  8
  [25] .bss              NOBITS          0000000000404024 003024 000004 00  WA  0   0  1
```

Ce listing est aussi abrégé. Les sections s'inscrivent dans les segments : `.init`, `.plt`, `.text` et `.fini` sont dans le `LOAD` exécutable ; `.rodata` dans celui en lecture seule ; les sections inscriptibles forment le dernier. Les sections comptent pour les outils ; le chargeur les ignore, c'est pourquoi un fichier peut s'exécuter avec la table des sections supprimée ou endommagée.

| Section | Contenu |
|---|---|
| `.text` | Le code du programme |
| `.rodata` | Constantes et littéraux de chaîne |
| `.data` | Données inscriptibles initialisées |
| `.bss` | Données initialisées à zéro, sans place dans le fichier |
| `.plt`, `.got`, `.got.plt` | Code de liaison pour appeler les fonctions des bibliothèques partagées |
| `.dynsym`, `.dynstr` | Symboles importés et exportés via le chargeur dynamique |
| `.symtab`, `.strtab` | La table complète des symboles, présente sauf si le fichier est dépouillé (stripped) |
| `.dynamic` | La table dynamique |

#### Importations : PLT, GOT et relocalisations

Le programme appelle `puts` et `printf` de la bibliothèque C, dont les adresses sont inconnues jusqu'au chargement. L'éditeur de liens produit donc, pour chacune, un relais de la **table de liaison des procédures** (procedure linkage table, PLT) et un emplacement de la **table des décalages globaux** (global offset table, GOT) par lequel le relais saute.

```bash
readelf -r build/12_sample
```

```text
Relocation section '.rela.dyn' at offset 0x4e8 contains 2 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000403fd8  000100000006 R_X86_64_GLOB_DAT 0000000000000000 __libc_start_main@GLIBC_2.34 + 0
000000403fe0  000300000006 R_X86_64_GLOB_DAT 0000000000000000 __gmon_start__ + 0

Relocation section '.rela.plt' at offset 0x518 contains 2 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
000000404000  000200000007 R_X86_64_JUMP_SLO 0000000000000000 puts@GLIBC_2.2.5 + 0
000000404008  000400000007 R_X86_64_JUMP_SLO 0000000000000000 __printf_chk@GLIBC_2.3.4 + 0
```

Une **relocalisation** est une instruction au chargeur : « écris l'adresse de ce symbole à cet emplacement ». Les entrées `JUMP_SLOT` indiquent que les emplacements 0x404000 et 0x404008 recevront les adresses de `puts` et de `__printf_chk`. Dans `main`, le code appelle le relais, pas la bibliothèque :

```bash
objdump -d -M intel --no-show-raw-insn -j .plt build/12_sample
```

```text
0000000000401020 <puts@plt-0x10>:
  401020:    push   QWORD PTR [rip+0x2fca]        # 403ff0 <_GLOBAL_OFFSET_TABLE_+0x8>
  401026:    jmp    QWORD PTR [rip+0x2fcc]        # 403ff8 <_GLOBAL_OFFSET_TABLE_+0x10>
  40102c:    nop    DWORD PTR [rax+0x0]

0000000000401030 <puts@plt>:
  401030:    jmp    QWORD PTR [rip+0x2fca]        # 404000 <puts@GLIBC_2.2.5>
  401036:    push   0x0
  40103b:    jmp    401020 <_init+0x20>
```

Avant le premier appel, l'emplacement 0x404000 contient 0x401036, l'adresse du `push` situé juste après le `jmp`. Le premier appel de `puts` tombe donc sur `push 0` (l'indice de la relocalisation) et saute au relais partagé en 0x401020, qui entre dans le résolveur du chargeur dynamique. Le résolveur cherche `puts`, écrit son adresse dans l'emplacement et y saute. Chaque appel suivant saute directement à `puts`. C'est la **liaison paresseuse** (lazy binding). De nombreux systèmes construisent plutôt avec une liaison immédiate, qui résout tout au démarrage ; vous pouvez donc voir un motif différent sur d'autres fichiers. Les relais et la table sont les mêmes.

Pour le rétro-ingénieur, la conséquence est simple : un appel à `foo@plt` est un appel à la fonction de bibliothèque `foo`, et les tables `.rela.plt` et `.dynsym` listent chaque fonction de bibliothèque que le programme peut appeler.

### PE

Un fichier PE (Portable Executable) commence par un **en-tête DOS** qui débute par les lettres `MZ`. C'est un vestige du format DOS 16 bits, et le seul champ qui compte encore se trouve au décalage 0x3c : `e_lfanew`, le décalage dans le fichier des **en-têtes NT**. Entre l'en-tête DOS et les en-têtes NT, on trouve en général un petit programme DOS qui affiche « This program cannot be run in DOS mode ».

Les en-têtes NT commencent par la signature `PE\0\0`, suivie de l'**en-tête de fichier**, qui indique la machine et le nombre de sections, puis de l'**en-tête facultatif**, qui est obligatoire pour les exécutables malgré son nom. Une **table des sections** suit.

Le programme `examples/12_pe_dump.py` analyse ces structures avec la seule bibliothèque standard. Un programme Windows construit avec MinGW sur l'image du laboratoire :

```bash
python3 12_pe_dump.py build/12_win.exe
```

```text
e_lfanew (offset of the NT headers): 0x80
machine: 0x8664  sections: 10  optional header size: 240
optional header magic: 0x20b (0x20b means PE32+)
entry point RVA: 0x1410  image base: 0x140000000
section alignment: 0x1000  file alignment: 0x200
import directory: RVA 0x8000, size 0x52c

name      virtual size  RVA       raw size  raw offset  flags
.text     0x00001768    0x001000  0x001800  0x000400    0x60000060
.data     0x00000080    0x003000  0x000200  0x001c00    0xc0000040
.rdata    0x000008a0    0x004000  0x000a00  0x001e00    0x40000040
.pdata    0x00000210    0x005000  0x000400  0x002800    0x40000040
.xdata    0x000001dc    0x006000  0x000200  0x002c00    0x40000040
.bss      0x00000180    0x007000  0x000000  0x000000    0xc0000080
.idata    0x0000052c    0x008000  0x000600  0x002e00    0xc0000040
.CRT      0x00000060    0x009000  0x000200  0x003400    0xc0000040
.tls      0x00000010    0x00a000  0x000200  0x003600    0xc0000040
.reloc    0x00000078    0x00b000  0x000200  0x003800    0x42000040

entry point RVA 0x1410 is at file offset 0x810

imported libraries and functions:
  KERNEL32.dll: DeleteCriticalSection, EnterCriticalSection, GetLastError, InitializeCriticalSection, LeaveCriticalSection, SetUnhandledExceptionFilter ...
  msvcrt.dll: __C_specific_handler, __getmainargs, __initenv, __iob_func, __set_app_type, __setusermatherr ...
```

Champs clés :

| Champ | Signification |
|---|---|
| Machine | 0x8664 est x86-64 ; 0x14c est x86 |
| Nombre magique de l'en-tête facultatif | 0x10b est PE32 (32 bits) ; 0x20b est PE32+ (64 bits) |
| Base de l'image | L'adresse préférée de chargement du fichier (ici 0x140000000) |
| Point d'entrée | Une RVA : relative à la base de l'image |
| Alignement des sections | Alignement en mémoire (0x1000, une page) |
| Alignement dans le fichier | Alignement dans le fichier (0x200) |
| Répertoires de données | Une table de 16 paires `(RVA, size)` pointant vers les importations, exportations, relocalisations, ressources et plus |

#### RVA et décalages dans le fichier

Les adresses PE à l'intérieur du fichier sont des **adresses virtuelles relatives** (RVA) : des décalages depuis la base de l'image une fois chargée. Chaque section occupe une plage en mémoire et une autre dans le fichier. Pour convertir une RVA en décalage dans le fichier, trouvez la section qui la contient et appliquez :

```text
file offset = RVA - section RVA + section raw offset
```

Exemple détaillé tiré de la table ci-dessus : la RVA du point d'entrée 0x1410 se situe dans `.text` (RVA 0x1000, décalage brut 0x400), donc le décalage est 0x1410 - 0x1000 + 0x400 = 0x810. Cela correspond à la sortie du programme. La taille virtuelle de la section (0x1768) peut dépasser sa taille brute, et `.bss` a une taille virtuelle mais pas de données brutes, comme dans ELF.

#### Importations

Le répertoire d'importation (ici la section `.idata`) est un tableau de descripteurs, un par bibliothèque, terminé par un descripteur entièrement nul. Chaque descripteur pointe vers le nom de la bibliothèque et vers deux tableaux parallèles : la **table des noms d'importation** (import name table, INT), qui liste les fonctions voulues, et la **table des adresses d'importation** (import address table, IAT), que le chargeur remplit avec leurs adresses. Chaque entrée de l'INT pointe vers un `hint` et un nom. Les appels du programme passent par l'IAT, sous la forme `call qword ptr [rip + offset]`. C'est l'équivalent PE de la PLT et de la GOT, à la différence que le chargeur remplit toute l'IAT avant le démarrage du programme.

#### Autres tables

Le **répertoire d'exportation** liste les fonctions qu'offre une DLL. La **table de relocalisation** (`.reloc`) liste les endroits à ajuster quand l'image est chargée à une adresse autre que la base préférée. Les ressources, le stockage local au fil (TLS), les données d'exceptions (`.pdata` et `.xdata` en 64 bits) et le répertoire de débogage ont leurs propres entrées dans les répertoires de données.

### Comparaison

| | ELF | PE |
|---|---|---|
| Nombre magique | `7f 45 4c 46` | `MZ`, puis `PE\0\0` à `e_lfanew` |
| Point d'entrée | Adresse virtuelle | RVA depuis la base de l'image |
| Vue de chargement | En-têtes de programme (segments) | Table des sections, plus l'en-tête facultatif |
| Importations | `.dynsym`, `.rela.plt`, PLT et GOT | Répertoire d'importation, INT et IAT |
| Exportations | Entrées de `.dynsym` marquées comme définies | Répertoire d'exportation |
| Relocalisations | `.rela.dyn`, `.rela.plt` | `.reloc` |
| Chargeur | `ld.so`, nommé dans `INTERP` | Le chargeur Windows dans le noyau et `ntdll` |

### Écrire un exécutable à la main

Comme le chargeur n'a besoin que de quelques champs, un programme Linux valide peut être aussi petit qu'un en-tête ELF, un en-tête de programme et quelques instructions. Le fichier `examples/12_tiny_elf.asm` s'assemble en un binaire brut avec `nasm -f bin`, qui écrit exactement les octets que vous déclarez et rien d'autre.

```bash
make
build/12_tiny_elf; echo $?
wc -c < build/12_tiny_elf
readelf -h -l -W build/12_tiny_elf | grep -E "Class|Type|Machine|Entry|Number of|LOAD"
```

```text
42
132
  Class:                             ELF64
  Type:                              EXEC (Executable file)
  Machine:                           Advanced Micro Devices X86-64
  Entry point address:               0x400078
  Number of program headers:         1
  Number of section headers:         0
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  LOAD           0x000000 0x0000000000400000 0x0000000000400000 0x000084 0x000084 R E 0x1000
```

Le programme entier fait 132 octets : 64 pour l'en-tête ELF, 56 pour l'unique en-tête de programme et 12 pour les instructions. L'unique entrée `LOAD` projette tout le fichier à partir du décalage 0 à l'adresse 0x400000 avec les droits de lecture et d'exécution ; les octets d'en-tête et le code partagent donc une projection, et le point d'entrée 0x400078 se trouve 120 octets plus loin, juste après les deux en-têtes. Il n'y a ni sections ni symboles ; le programme s'exécute parce que le chargeur n'a besoin ni des unes ni des autres.

Lisez le fichier en entier : chaque `db`, `dw`, `dd` et `dq` est un champ des tables ci-dessus, avec un commentaire. Notez que `p_offset` est nul et que `p_vaddr` égale l'adresse de chargement ; le chargeur exige que le décalage et l'adresse virtuelle soient congrus modulo la taille de page.

Un fichier PE peut s'écrire de la même façon, avec un en-tête DOS, des en-têtes NT, une table des sections et une section. Il est plus long parce que le format PE a plus de champs obligatoires ; l'exercice 11 vous demande d'en lire un de près, pas d'en écrire un.

## Essayez

1. Exécutez `readelf -h build/12_tiny_elf` et associez chaque ligne à un `dw`, `dd` ou `dq` de `12_tiny_elf.asm`.
2. Changez le code de sortie dans l'assembleur en 7, reconstruisez et confirmez avec `echo $?`.
3. Avec `readelf -S` sur `12_sample`, convertissez l'adresse de `counter` (0x404020, issue de `objdump -d`) en décalage dans le fichier à l'aide de l'adresse et du décalage de la section. Vérifiez ensuite avec `xxd -s OFFSET -l 4 build/12_sample` ; la valeur est 5.
4. Exécutez `python3 12_pe_dump.py` sur le PE et convertissez à la main la RVA 0x8000 du répertoire d'importation en décalage dans le fichier. Comparez avec la table des sections.
5. Trouvez la chaîne `hello` dans `12_sample` avec `strings -t x` et calculez l'adresse virtuelle à partir de la section `.rodata`.

## Pièges courants

- **Mélanger décalages dans le fichier et adresses virtuelles.** Les outils affichent les deux. Vérifiez l'en-tête de colonne avant d'utiliser un nombre.
- **Oublier que les RVA sont relatives à la base de l'image.** Dans un PE, ajoutez la base pour obtenir l'adresse que montre le débogueur.
- **Se fier à la table des sections.** Le chargeur l'ignore. Un fichier dont la table des sections est absente ou truquée peut quand même s'exécuter, et le nom d'une section ne dit rien de ce que sont les octets.
- **Lire les en-têtes avec la mauvaise largeur de mot.** Un ELF ou PE 32 bits utilise d'autres largeurs de champ. Vérifiez d'abord la classe ou le nombre magique.
- **Supposer que les appels à `foo@plt` exécutent `foo` dès la première fois.** Le premier appel passe par le résolveur quand la liaison paresseuse est activée.

## Résumé

- Un chargeur projette un fichier avec une vue (segments ELF, sections PE avec l'en-tête facultatif), tandis que les outils en utilisent une autre (sections).
- ELF a un en-tête ELF, des en-têtes de programme et des en-têtes de section ; PE a un en-tête DOS, des en-têtes NT et une table des sections.
- Les importations atteignent les fonctions de bibliothèque par PLT et GOT dans ELF, et par le répertoire d'importation et l'IAT dans PE.
- Convertir une adresse en décalage dans le fichier revient à retrancher l'adresse de la section et à ajouter son décalage dans le fichier.
- Un exécutable valide peut être très petit, parce que le chargeur n'a besoin que des champs de chargement.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 13, la reconnaissance des structures de code.
- Aide-mémoire : [ELF](../../cheatsheets/elf.fr.md), [PE](../../cheatsheets/pe.fr.md).
- Exercices qui utilisent ce module : [Exercice 10](../../exercises/10-header-surgeon/README.fr.md), [Exercice 11](../../exercises/11-portable-puzzle/README.fr.md).
