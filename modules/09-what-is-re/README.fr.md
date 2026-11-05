<!-- source: README.md @ sha256:26989c484734 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 09. Ce qu'est la rétro-ingénierie

Durée : environ 1 heure. Prérequis : [08. La pile, les procédures et les conventions d'appel](../08-stack-and-calls/README.fr.md)

## Pourquoi c'est important

Les modules 00 à 08 vous ont appris à écrire de l'assembleur. La rétro-ingénierie (reverse engineering) est la même compétence utilisée en sens inverse : vous partez d'un programme terminé et vous déterminez ce qu'il fait et comment. Avant les outils et les formats des modules suivants, il vous faut une définition claire, un ensemble d'usages légitimes, les limites à respecter et une manière de travailler reproductible qui vous évite de vous perdre dans un binaire.

## Concepts

### Définition

**La rétro-ingénierie** consiste à retrouver la conception, le comportement ou les données d'un produit à partir du produit fini lui-même, sans les documents de conception d'origine. Pour un logiciel, le produit est généralement un fichier exécutable, une bibliothèque ou une image de micrologiciel, et le résultat est une connaissance : la description d'un format de fichier, la logique d'une fonction, la liste des actions qu'un programme effectue.

Ce n'est pas la même chose que réécrire le programme. Le but est de comprendre. Réécrire est parfois une étape ultérieure, comme dans l'exercice 08 et l'exercice 09, où vous réimplémentez des fonctions que vous avez d'abord comprises.

### Usages légitimes

| Usage | Exemple |
|---|---|
| Interopérabilité | Documenter un format de fichier non documenté pour qu'un autre outil puisse le lire |
| Audit de sécurité | Rechercher des erreurs de sûreté mémoire dans un composant à code fermé |
| Analyse de logiciels malveillants | Découvrir ce que fait un échantillon hostile pour que les défenseurs puissent le détecter et le supprimer |
| Retrouver un savoir perdu | Comprendre un outil dont le code source a disparu |
| Débogage | Comprendre pourquoi un plantage se produit dans un code que vous ne pouvez pas recompiler avec des symboles |
| Apprentissage | Voir ce que fait un compilateur de votre propre code |

### Limites

Ce cours enseigne l'analyse et la compréhension. Il n'enseigne pas, et ses exercices ne demandent jamais :

- de développer des exploits, du shellcode ou des logiciels malveillants ;
- de contourner une protection contre la copie, une licence ou la gestion des droits numériques ;
- toute activité visant des systèmes ou des logiciels que vous n'avez pas le droit d'examiner.

Avant d'analyser un programme qui n'est pas le vôtre, vérifiez trois choses : à qui il appartient, ce que la licence ou le contrat dit de l'analyse, et ce que dit la loi de votre pays. L'interopérabilité et la recherche en sécurité bénéficient de protections juridiques précises dans certains endroits et d'aucune dans d'autres. Les règles diffèrent selon les pays et évoluent avec le temps ; ce cours ne donne donc pas de conseil juridique. Quand la situation n'est pas claire, demandez d'abord à une personne qualifiée. Chaque binaire de ce dépôt est construit à partir d'un source inclus dans le dépôt : travailler sur les exercices est donc toujours permis. Voir [ETHICS.md](../../ETHICS.fr.md).

### Deux niveaux de rétro-ingénierie

La rétro-ingénierie **au niveau système** observe le comportement d'un programme de l'extérieur : quels fichiers il ouvre, quelles connexions réseau il établit, quels appels système il émet, ce qu'il écrit sur le disque. Le programme est traité comme une boîte noire et observé.

La rétro-ingénierie **au niveau du code** lit les instructions du programme pour savoir exactement comment il calcule. Elle est plus lente et plus précise. Les enquêtes réelles alternent entre les deux : la vue extérieure indique où regarder, la vue du code explique pourquoi.

### Analyse statique et dynamique

| | Analyse statique | Analyse dynamique |
|---|---|---|
| Ce que vous faites | Lire le fichier sans l'exécuter | Exécuter le programme et l'observer |
| Outils | `file`, `strings`, `readelf`, `objdump`, un désassembleur, un décompilateur | Un débogueur, `strace`, `ltrace` |
| Points forts | Sûre ; voit tout le code, y compris les chemins qui ne s'exécutent jamais | Montre les valeurs réelles ; décompacte des données que le programme décode lui-même |
| Points faibles | Ne voit pas les valeurs calculées à l'exécution ; le code obscurci est difficile à lire | Ne voit que les chemins réellement exécutés ; exécuter du code inconnu exige un environnement sûr |

Aucune n'est meilleure. Un bon déroulement utilise l'analyse statique pour formuler une question et l'analyse dynamique pour y répondre, puis revient à l'analyse statique avec les nouveaux faits.

### Un déroulement reproductible

1. **Triage.** Identifiez le fichier sans l'exécuter : type, architecture, taille, empreinte, chaînes, fonctions importées, sections. Cela prend quelques minutes et répond souvent à la question ou montre où chercher.
2. **Observer.** Exécutez le programme dans un endroit sûr et notez ce qu'il fait de l'extérieur : sortie, appels système, fichiers.
3. **Formuler une question.** Écrivez une chose précise que vous devez savoir, par exemple « où l'entrée est-elle vérifiée ? » ou « quelle est la disposition de cet enregistrement ? ».
4. **Trouver le code.** Prenez les chaînes, les importations et les appels système comme points de départ et suivez les références croisées jusqu'au code qui compte.
5. **Lire et nommer.** Lisez les instructions, déterminez ce que fait chaque fonction et donnez-lui un nom significatif. Les noms sont le principal résultat du travail.
6. **Tester l'idée.** Vérifiez votre compréhension avec un débogueur, en parcourant le code avec des entrées choisies, ou en réimplémentant la fonction et en comparant les résultats.
7. **Rendre compte.** Notez ce que vous avez trouvé, comment vous le savez et ce qui reste inconnu. Séparez les faits des suppositions.

Prenez des notes au fur et à mesure. La mémoire flanche vite dans un gros binaire, et une note comme « la fonction à 0x401136 copie l'en-tête dans un tampon de 32 octets » fait gagner une heure le lendemain.

### Un endroit sûr pour exécuter des programmes

N'exécutez des programmes inconnus que dans un environnement que vous pouvez jeter. Pour ce cours, le conteneur du laboratoire suffit, car tous les binaires proviennent de ce dépôt. Pour de vrais échantillons, utilisez une machine virtuelle sans réseau et avec un instantané auquel revenir. N'exécutez jamais un échantillon inconnu sur la machine qui contient vos fichiers.

## Essayez

Faites le triage du plus petit programme du cours sans l'exécuter. Les commandes s'exécutent depuis `modules/03-first-program/examples`, une fois que `make` a construit les exemples.

```bash
file build/03_hello64
strings -n 4 build/03_hello64 | head -6
readelf -h build/03_hello64 | grep -E "Type|Entry"
```

Sortie réelle :

```text
build/03_hello64: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, with debug_info, not stripped
Hello, assembly!
03_hello64.asm
/lab/modules/03-first-program/examples/
NASM 2.16.01
03_hello64.asm
YYuY0Y/
  Type:                              EXEC (Executable file)
  Entry point address:               0x401000
```

Sans rien exécuter, vous savez que c'est un programme Linux 64 bits pour x86-64 sans bibliothèques, qu'il a été écrit en assembleur avec NASM (la chaîne du producteur et le nom du fichier source sont conservés dans le fichier, avec le répertoire de compilation), que ses symboles n'ont pas été supprimés et qu'il affiche un message d'accueil. C'est le triage.

Observez-le maintenant de l'extérieur :

```bash
strace -q build/03_hello64
```

```text
execve("build/03_hello64", ["build/03_hello64"], 0x7ffcb16ebed8 /* 15 vars */) = 0
write(1, "Hello, assembly!\n", 17Hello, assembly!
)      = 17
exit(0)                                 = ?
+++ exited with 0 +++
```

Les appels système confirment ce que suggéraient les chaînes : un `write` de 17 octets vers la sortie standard, puis `exit`. La sortie du programme lui-même se mêle à la trace parce que les deux vont vers le terminal. C'est de la rétro-ingénierie au niveau système ; lire les instructions serait du niveau du code.

Écrivez en trois lignes ce que vous savez de ce fichier et comment vous le savez. C'est l'habitude à prendre.

## Pièges courants

- **Commencer par le désassembleur.** Sans triage, vous lisez du code sans savoir à quoi sert le programme. Consacrez les premières minutes à la vue extérieure.
- **Tout lire.** Un binaire peut contenir des milliers de fonctions. Suivez la question, pas l'ordre du fichier.
- **Se fier aux noms.** Les noms de symboles et les chaînes peuvent être faux ou volontairement trompeurs. Traitez-les comme des indices à confirmer.
- **Exécuter du code inconnu sur votre propre machine.** Utilisez un environnement jetable.
- **Mélanger faits et suppositions dans les notes.** Marquez ce que vous avez vérifié et ce que vous avez supposé.

## Résumé

- La rétro-ingénierie retrouve le fonctionnement d'un programme à partir du programme lui-même ; le but est de comprendre.
- Les usages légitimes comprennent l'interopérabilité, l'audit de sécurité, l'analyse de logiciels malveillants, la récupération d'un savoir perdu et l'apprentissage. Vérifiez la propriété, la licence et la loi avant d'analyser un logiciel qui n'est pas le vôtre.
- L'analyse statique lit le fichier ; l'analyse dynamique l'exécute. Un bon travail alterne entre les deux.
- Un déroulement de triage, observation, question, recherche du code, nommage, test et compte rendu garde le travail concentré.
- N'exécutez des programmes inconnus que dans un environnement jetable.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 10, les fondamentaux des systèmes d'exploitation pour le rétro-ingénieur.
