<!-- source: README.md @ sha256:4d47465582de -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 10. Fondamentaux des systèmes d'exploitation pour le rétro-ingénieur

Durée : environ 2 heures. Prérequis : [09. Ce qu'est la rétro-ingénierie](../09-what-is-re/README.fr.md)

## Pourquoi c'est important

Un programme ne s'exécute jamais seul. Le système d'exploitation lui donne de la mémoire, décide de ce qu'il peut toucher, charge son code et effectue chaque action qui sort du processus : ouvrir un fichier, envoyer un paquet, créer un fil d'exécution. Quand vous lisez un désassemblage, beaucoup d'appels ne relèvent pas de la logique propre du programme mais de demandes au système. Connaître les couches permet de distinguer les deux, de trouver vite le code intéressant et de comprendre pourquoi un débogueur affiche les adresses qu'il affiche.

## Concepts

### Processus et fils d'exécution

Un **processus** est un programme en cours d'exécution, avec son propre espace d'adressage virtuel, ses propres fichiers ouverts et sa propre identité. Un **fil d'exécution** (thread) est un flux d'exécution à l'intérieur d'un processus. Tous les fils d'un processus partagent sa mémoire et ses fichiers ouverts, mais chacun a ses propres registres et sa propre pile. C'est pourquoi la pile étudiée au module 08 appartient à un fil, et non à un processus.

### Mémoire virtuelle

Chaque processus voit un espace d'adressage privé. Une adresse utilisée par votre code est une **adresse virtuelle**. Le processeur et le noyau la traduisent en adresse physique au moyen de tables de pages. Deux processus peuvent utiliser la même adresse virtuelle pour des données différentes, et un processus ne peut pas désigner la mémoire d'un autre.

La mémoire est gérée en **pages**, normalement de 4096 octets. Chaque page a des permissions : lecture, écriture, exécution. Le processeur les fait respecter : écrire dans une page en lecture seule ou exécuter du code depuis une page non exécutable déclenche une faute, et le noyau termine normalement le processus par un signal (`SIGSEGV` sous Linux).

Les systèmes modernes suivent une règle souvent écrite **W xor X** : une page est inscriptible ou exécutable, pas les deux, sauf demande d'un programme. Le code compilé se trouve dans des pages exécutables en lecture seule ; les données se trouvent dans des pages inscriptibles non exécutables. C'est utile au rétro-ingénieur : les permissions d'une région indiquent ce qu'elle contient.

La **randomisation de la disposition de l'espace d'adressage** (address space layout randomization, ASLR) place la pile, le tas et les bibliothèques à des adresses différentes à chaque exécution. Les adresses de vos notes d'une exécution ne correspondront pas à la suivante. Travaillez avec des décalages depuis le début d'un module et utilisez des noms plutôt que des adresses absolues.

### Ce que contient un processus

| Région | Contenu | Permissions habituelles |
|---|---|---|
| Code du programme | Les instructions de l'exécutable | lecture, exécution |
| Constantes | Chaînes et tables qui ne changent jamais | lecture |
| Données | Globales initialisées et initialisées à zéro | lecture, écriture |
| Tas | Mémoire demandée à l'exécution (`malloc`) | lecture, écriture |
| Bibliothèques | Le code et les données de chaque bibliothèque partagée | comme ci-dessus, par bibliothèque |
| Pile | Une par fil : cadres, variables locales, adresses de retour | lecture, écriture |

### Comment un programme est chargé

1. Un processus parent demande au noyau d'exécuter un fichier (sous Linux l'appel système est `execve`).
2. Le noyau lit les en-têtes du fichier, projette le code et les données du fichier dans un nouvel espace d'adressage et crée la pile avec les arguments et l'environnement.
3. Si le programme utilise des bibliothèques partagées, le noyau projette aussi un **chargeur dynamique** et le démarre en premier. Le chargeur projette chaque bibliothèque requise, résout les références du programme aux fonctions de bibliothèque, puis saute au point d'entrée du programme.
4. Le point d'entrée exécute du code de démarrage, qui appelle `main`.

Les champs du fichier qui pilotent chaque étape sont le sujet du module 12. Retenez pour l'instant que la première instruction exécutée n'est pas `main`, et que l'adresse d'une fonction de bibliothèque n'est connue qu'après le passage du chargeur.

### Mode utilisateur et mode noyau

Le processeur fonctionne à au moins deux niveaux de privilège. Les programmes s'exécutent en **mode utilisateur** et ne peuvent toucher ni le matériel ni la mémoire d'autres processus. Le noyau s'exécute en **mode noyau** avec un accès total. Un programme demande au noyau d'agir en son nom par un **appel système** : une instruction spéciale (`syscall` dans le code Linux 64 bits, `int 0x80` dans l'interface 32 bits du module 03) qui passe en mode noyau à un point d'entrée fixe, exécute le service demandé et revient.

### Appels système et fonctions de bibliothèque

Les programmes émettent rarement eux-mêmes des appels système. Ils appellent des fonctions d'une bibliothèque, et la bibliothèque émet les appels système.

```text
your code  ->  puts()      ->  write()  ->  syscall instruction  ->  kernel
              C library        wrapper
```

Sous Linux, la bibliothèque C (libc) fournit de fines enveloppes autour des appels système (`write`, `open`, `read`) et des services plus larges construits dessus (`printf`, `malloc`, `fopen`). Le rétro-ingénieur dispose ainsi de deux points d'observation :

- `ltrace` montre les appels vers les bibliothèques partagées.
- `strace` montre les appels système qui entrent réellement dans le noyau.

Les deux vues du même programme diffèrent : un `puts` devient un `write`, tandis qu'un `printf` avec tampon peut ne produire aucun appel système tant que le tampon n'est pas vidé.

### Couches sous Windows

Windows utilise une pile de couches plus profonde.

| Couche | Exemples | Rôle |
|---|---|---|
| Programme | L'exécutable | Votre code |
| API Windows | `kernel32.dll`, `user32.dll`, `advapi32.dll` | L'interface documentée : `CreateFileW`, `MessageBoxW` |
| API native | `ntdll.dll` | Fonctions de plus bas niveau nommées `Nt...` et `Zw...` ; chacune est une minuscule fonction relais qui charge un numéro de service et exécute `syscall` |
| Noyau | `ntoskrnl.exe` | Les services eux-mêmes |

Un appel d'API Windows traverse en général une ou plusieurs couches avant d'atteindre le noyau. Les numéros d'appel système changent d'une version de Windows à l'autre ; c'est pourquoi les programmes appellent `ntdll` au lieu d'émettre eux-mêmes l'instruction.

### Descripteurs, handles et objets

Le noyau possède les ressources : fichiers, processus, fils, sockets. Un programme y fait référence par un petit nombre qu'il ne peut pas interpréter seul.

- Sous Linux, le nombre est un **descripteur de fichier** (0 est l'entrée standard, 1 la sortie standard, 2 l'erreur standard). Fichiers, tubes et sockets l'utilisent tous.
- Sous Windows, le nombre est un **handle** : un indice dans une table par processus d'objets du noyau. Les handles désignent des fichiers, des processus, des fils, des événements, des clés de registre et plus encore, et chaque type d'objet a ses propres fonctions.

Quand vous voyez un petit entier passé à un appel, demandez-vous quel objet il désigne et d'où le programme l'a obtenu. Remonter le nombre jusqu'à sa création montre souvent ce que fait une fonction.

## Essayez

Construisez les exemples, puis exécutez-les depuis `modules/10-os-for-reversers/examples`.

```bash
make
build/10_regions
```

Le programme indique quelle région contient une fonction, une constante, une globale, un bloc de tas et une variable locale. Sortie réelle (les adresses ne sont pas affichées car elles changent à chaque exécution) :

```text
function main          r-xp  /lab/modules/10-os-for-reversers/examples/build/10_regions
function printf        r-xp  /usr/lib/x86_64-linux-gnu/libc.so.6
constant string        r--p  /lab/modules/10-os-for-reversers/examples/build/10_regions
global variable        rw-p  /lab/modules/10-os-for-reversers/examples/build/10_regions
heap block             rw-p  [heap]
local variable         rw-p  [stack]
```

Le code est exécutable et en lecture seule, les constantes sont en lecture seule, les données, le tas et la pile sont inscriptibles et non exécutables, et `printf` se trouve dans un autre fichier : la bibliothèque C partagée. La quatrième lettre `p` signifie privé.

Observez maintenant le processeur faire respecter les permissions :

```bash
build/10_perms
```

```text
page is read-write: wrote "writable"
page is read-only: the write caused a fault (SIGSEGV)
the contents are still "writable"
```

Comparez enfin les deux couches :

```bash
strace -e trace=write build/10_layers
ltrace -e puts+fflush+syscall build/10_layers
```

```text
write(1, "through puts()\n", 15through puts()
)        = 15
write(1, "through syscall()\n", 18through syscall()
)     = 18
```

```text
10_layers->puts("through puts()")                = 15
10_layers->fflush(0x7d3f5d4745c0through puts()
)                = 0
10_layers->syscall(1, 1, 0x402004, 18through syscall()
)           = 18
+++ exited (status 0) +++
```

Le programme a effectué deux écritures par des chemins différents. `strace` les montre toutes deux comme le même appel système `write`. `ltrace` montre que la première est passée par `puts` et la seconde par la fonction `syscall` de la bibliothèque. La sortie du programme lui-même apparaît dans les traces parce que les deux vont vers le terminal. Le pointeur affiché pour `fflush` varie d'une exécution à l'autre.

Listez enfin les bibliothèques dont le programme a besoin et constatez qu'elles sont projetées en mémoire et ne font pas partie du fichier :

```bash
ldd build/10_layers
```

```text
    linux-vdso.so.1 (addr)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (addr)
    /lib64/ld-linux-x86-64.so.2 (addr)
```

Les adresses sont remplacées ici par `(addr)` parce qu'elles diffèrent à chaque exécution. La dernière ligne est le chargeur dynamique, et la première est une petite région fournie par le noyau.

## Pièges courants

- **Coder des adresses en dur.** L'ASLR déplace bibliothèques, pile et tas à chaque exécution. Notez des décalages et des noms.
- **Confondre la fonction de bibliothèque et l'appel système.** `printf` n'est pas un appel système. Regardez quelle couche vous lisez avant de conclure.
- **Supposer qu'un appel égale un appel système.** La mise en tampon, les nouvelles tentatives et les enveloppes changent le nombre.
- **Ignorer les permissions.** Une région à la fois inscriptible et exécutable est inhabituelle et mérite une explication.
- **Traiter un handle ou un descripteur comme parlant par lui-même.** Le nombre n'est qu'un indice ; c'est l'objet derrière qui compte.

## Résumé

- Un processus a un espace d'adressage virtuel privé fait de pages avec des permissions de lecture, d'écriture et d'exécution ; les fils le partagent et chacun a sa propre pile.
- Un programme est chargé par le noyau, généralement avec l'aide d'un chargeur dynamique, donc la première instruction exécutée n'est pas `main`.
- Le code en mode utilisateur n'atteint le noyau que par des appels système ; les bibliothèques les enveloppent.
- `ltrace` montre les appels de bibliothèque et `strace` les appels système ; sous Windows il y a plus de couches : API Windows, API native, noyau.
- Les ressources sont désignées par des descripteurs de fichier (Linux) et des handles (Windows).

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 11, la boîte à outils du rétro-ingénieur.
- Exercices qui utilisent ce module : [Exercice 11](../../exercises/11-portable-puzzle/README.fr.md).
