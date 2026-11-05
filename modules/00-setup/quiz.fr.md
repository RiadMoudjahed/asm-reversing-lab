<!-- source: quiz.md @ sha256:26a6df12e021 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 00. Mise en place du laboratoire

Répondez d'abord, puis ouvrez la réponse.

**1. Où apparaît le dossier de votre dépôt dans le laboratoire, et pourquoi est-ce important ?**

<details>
<summary>Réponse</summary>

Sur `/lab`. Le dossier est monté, pas copié : un fichier que vous modifiez sur votre machine est donc immédiatement visible dans le laboratoire, et inversement.

</details>

**2. Que fait la commande `ld` que `nasm` ne fait pas ?**

<details>
<summary>Réponse</summary>

`nasm` assemble le code source en fichier objet. `ld` est l'éditeur de liens : il transforme un ou plusieurs fichiers objets en exécutable.

</details>

**3. Après `./build/00_exit42`, comment lisez-vous le code de sortie du programme dans le shell ?**

<details>
<summary>Réponse</summary>

Avec `echo $?`, qui affiche le code de sortie de la dernière commande. Le test de fumée affiche 42.

</details>

**4. Pourquoi un drapeau n'est-il jamais stocké en clair dans les dossiers des exercices ?**

<details>
<summary>Réponse</summary>

Pour que la lecture du dépôt ne révèle pas la réponse. Le vérificateur stocke une empreinte salée du drapeau et compare votre réponse à celle-ci.

</details>

**5. Vous avez téléchargé Ghidra sur votre machine hôte. Quel chemin utilisez-vous pour importer `exercises/01-signal-flare/bin/signal_flare` : celui de l'hôte ou celui de `/lab` ?**

<details>
<summary>Réponse</summary>

Celui de l'hôte. Ghidra s'exécute sur l'hôte et ne voit pas le montage `/lab`, qui n'existe que dans le conteneur.

</details>

**6. Une vérification affiche `Part 1 (Reverse): skipped, no --flag given`. Qu'avez-vous oublié ?**

<details>
<summary>Réponse</summary>

De transmettre votre drapeau : `make check EX=01 ARGS='--flag ARL{...}'`. La partie 2 s'exécute quand même sans lui.

</details>
