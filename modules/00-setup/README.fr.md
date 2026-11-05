<!-- source: README.md @ sha256:b48e9369096e -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 00. Mise en place du laboratoire

Durée : environ 45 minutes. Prérequis : uniquement les bases du C et un ordinateur Linux, macOS ou Windows capable d'exécuter Docker.

## Pourquoi c'est important

Tous les exemples et exercices du cours s'exécutent dans un même environnement préparé, afin que vos résultats correspondent à ceux des leçons. Dix minutes de mise en place ici vous évitent des heures de « ça marche chez moi » plus tard.

À la fin de ce module, vous aurez un laboratoire fonctionnel, une chaîne d'outils vérifiée, Ghidra sur votre machine hôte et une idée claire de la façon dont les exercices sont vérifiés.

## Notions

### Deux façons d'exécuter le laboratoire

**Docker (recommandé).** L'image du laboratoire contient chaque outil dans une version fixe. Le dossier de votre dépôt y est monté sur `/lab`, si bien que les fichiers que vous modifiez sur votre machine apparaissent immédiatement dans le laboratoire.

```bash
make shell
```

La première exécution construit l'image, qui occupe environ 6,6 Go sur le disque et télécharge de nombreux paquets : prévoyez du temps et une bonne connexion. Les exécutions suivantes démarrent en quelques secondes. Dans le laboratoire, l'invite de commande affiche l'utilisateur `lab` et les commandes du cours (`make check`, `nasm`, `gdb`) sont disponibles.

**Installation native (Ubuntu ou Debian).** Si vous préférez ne pas utiliser Docker :

```bash
sudo scripts/install-native.sh
```

Faites ensuite précéder les commandes du cours de `NATIVE=1`, par exemple `make NATIVE=1 check EX=01`. Le script installe le même assembleur, les mêmes compilateurs, le même débogueur et les mêmes bibliothèques Python que l'image. Il n'installe ni Rizin ni Ghidra.

### Ce que contient le laboratoire

| Outil | À quoi il sert |
|---|---|
| `nasm` | L'assembleur : transforme du code source assembleur en fichier objet |
| `ld` | L'éditeur de liens : transforme des fichiers objets en exécutable |
| `gcc`, `g++` | Compilateurs C et C++, également utilisés pour lier l'assembleur à des harnais de test en C |
| `gdb` avec GEF | Le débogueur, avec un affichage lisible des registres et de la mémoire |
| `objdump`, `readelf` | Désassemblent le code et inspectent la structure des exécutables |
| `rizin` | Un désassembleur et cadriciel d'analyse en ligne de commande |
| `strace`, `ltrace` | Montrent les appels système et les appels de bibliothèque d'un programme |
| `yara` | Recherche de motifs, utilisée dans le module d'analyse de logiciels malveillants |
| `mingw-w64`, `wine` | Compilent et exécutent des programmes Windows sous Linux |
| Python 3 | Exécute les vérificateurs des exercices ; bibliothèques pour lire les fichiers ELF et PE |

### Ghidra sur votre machine hôte

Ghidra est une suite graphique de rétro-ingénierie. Elle est trop volumineuse et trop graphique pour l'image du laboratoire : vous l'installez donc sur votre propre machine et vous ouvrez les fichiers des défis depuis le dossier de votre dépôt.

1. Téléchargez Ghidra depuis la page officielle des versions du projet.
2. Installez la version du JDK exigée par le README de cette version.
3. Décompressez Ghidra, lancez-le avec son script de démarrage et créez un projet.
4. Importez les binaires depuis `exercises/NN-name/bin/` dans le dossier de votre dépôt.

Vous commencerez à utiliser Ghidra au module 11. L'installer maintenant vous permet de suivre ensuite sans interruption.

### Comment fonctionnent les exercices et les vérificateurs

Chaque exercice se trouve dans `exercises/NN-name/` et comporte deux parties :

- **Partie 1, Reverse.** Vous analysez le programme de `bin/` et répondez à des questions. Les réponses produisent un drapeau (flag) de la forme `ARL{...}`.
- **Partie 2, Assemble.** Vous modifiez `starter/solution.asm` pour qu'il respecte une spécification.

Pour vérifier votre travail :

```bash
make check EX=01 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte de chaque partie séparément. Il n'affiche jamais le drapeau attendu : le dépôt n'en conserve qu'une empreinte salée. Si une partie échoue, lisez `HINTS.md` dans le dossier de l'exercice. Les indices sont écrits sur trois niveaux ; ouvrez un niveau à la fois.

Le dossier `solutions/` contient les corrigés complets. Gardez-le fermé jusqu'à ce que vous ayez terminé ou sérieusement essayé.

### Plan du dépôt

```text
modules/     the lessons, each with runnable examples
exercises/   the exercises, each with a binary, a starter file and tests
solutions/   complete solutions (spoilers)
cheatsheets/ one-page references to keep open while you work
glossary/    definitions of the terms used in the course
```

## À vous de jouer

Démarrez le laboratoire et lancez le test de fumée. Il assemble un programme de quatre instructions, le lie, l'exécute, puis vérifie la chaîne d'outils.

```bash
make shell
cd modules/00-setup/examples
make run
```

Sortie attendue :

```text
mkdir -p build
nasm -f elf64 00_exit42.asm -o build/00_exit42.o
ld build/00_exit42.o -o build/00_exit42
./build/00_exit42; echo "exit status: $?"
exit status: 42
bash check_tools.sh
nasm       ok
ld         ok
gcc        ok
g++        ok
make       ok
gdb        ok
objdump    ok
readelf    ok
strace     ok
ltrace     ok
rizin      ok
yara       ok
python3    ok
xxd        ok
file       ok
gcc -m32   ok
```

Le programme de `00_exit42.asm` ne fait qu'une chose : il se termine avec le code de sortie 42. Vous n'avez pas encore besoin de le comprendre ; le module 03 explique chaque ligne. L'essentiel est que les trois étapes (assembler, lier, exécuter) ont fonctionné, ce qui prouve que la chaîne d'outils est en état de marche.

Ouvrez maintenant GDB une fois pour confirmer que GEF s'est chargé :

```bash
gdb -q build/00_exit42
```

La première ligne de la sortie doit mentionner `GEF for linux ready`. Tapez `quit` pour quitter.

## Pièges courants

- **`permission denied` en lançant Docker.** Sous Linux, votre utilisateur doit appartenir au groupe `docker`, ou vous devez exécuter Docker avec des droits d'administrateur. Consultez la documentation de Docker pour votre système.
- **`make shell` échoue sous Windows.** Lancez-le depuis un terminal où `make` et `docker` fonctionnent tous deux, comme un shell WSL 2.
- **Un outil affiche `MISSING` après une installation native.** Relancez `scripts/install-native.sh` et lisez sa sortie. L'absence de `gcc -m32` signifie que les paquets multilib ne se sont pas installés.
- **Ghidra ne trouve pas votre fichier.** Ghidra s'exécute sur l'hôte : ouvrez donc le chemin de votre hôte (par exemple `~/asm-reversing-lab/exercises/01-signal-flare/bin/`), pas `/lab/...`.
- **Modifier des fichiers à la fois dans le laboratoire et sur l'hôte.** Choisissez un éditeur et un emplacement. Le dossier du dépôt est partagé, les deux voient donc les mêmes fichiers, mais deux éditeurs peuvent s'écraser mutuellement.

## Résumé

- Exécutez le cours dans le laboratoire Docker (`make shell`) ou sur une installation native d'Ubuntu ou de Debian.
- `make run` dans le dossier `examples` de ce module prouve que l'assembleur, l'éditeur de liens et les outils fonctionnent.
- Ghidra s'installe sur votre hôte et ouvre les fichiers du dossier de votre dépôt.
- Chaque exercice a une partie Reverse (un drapeau) et une partie Assemble (du code que les tests vérifient).
- `solutions/` contient des divulgâcheurs ; utilisez d'abord `HINTS.md`.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 01, représentation des données.
- Exercices qui utilisent ce module : [Exercice 01](../../exercises/01-signal-flare/README.fr.md).
