<!-- source: quiz.md @ sha256:8c505eab92e0 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 03. Votre premier programme

**1. Quelle partie de `mov eax, 1` est la destination ?**

<details>
<summary>Réponse</summary>

`eax`. Dans la syntaxe Intel de NASM, la destination vient en premier et la source en second.

</details>

**2. Pourquoi `.bss` n'occupe-t-elle pas de place dans le fichier exécutable ?**

<details>
<summary>Réponse</summary>

Parce qu'elle ne contient qu'une taille. Le chargeur fournit une mémoire mise à zéro de cette taille au démarrage du programme, donc aucun octet n'a besoin d'être stocké.

</details>

**3. Que calcule `message_len equ $ - message` lorsqu'il est placé juste après une chaîne ?**

<details>
<summary>Réponse</summary>

Le nombre d'octets entre l'étiquette `message` et la position courante, c'est-à-dire la longueur de la chaîne, y compris des octets comme le saut de ligne.

</details>

**4. Renseignez les registres 64 bits pour `write(1, text, 17)` : numéro d'appel, descripteur, adresse, longueur.**

<details>
<summary>Réponse</summary>

`rax` = 1, `rdi` = 1, `rsi` = adresse du texte, `rdx` = 17.

</details>

**5. Quelles sont les différences entre la manière 64 bits et la manière 32 bits d'appeler `write` ?**

<details>
<summary>Réponse</summary>

64 bits : instruction `syscall`, numéro d'appel 1 dans `rax`, arguments dans `rdi`, `rsi`, `rdx`. 32 bits : `int 0x80`, numéro d'appel 4 dans `eax`, arguments dans `ebx`, `ecx`, `edx`.

</details>

**6. Après une instruction `syscall`, quels deux registres le processeur écrase-t-il en plus de `rax` ?**

<details>
<summary>Réponse</summary>

`rcx` (il reçoit l'adresse de l'instruction suivante) et `r11` (il reçoit les indicateurs sauvegardés).

</details>

**7. Vous désassemblez un programme sans source et voyez `mov eax, 0x3c` suivi de `syscall`. Que fait-il ?**

<details>
<summary>Réponse</summary>

Il appelle `exit` (numéro d'appel 60 en 64 bits = 0x3c). La valeur de `edi` avant lui est le code de sortie.

</details>

**8. Le programme s'exécute puis plante avec une erreur de segmentation après l'affichage. Quelle est la cause la plus probable ?**

<details>
<summary>Réponse</summary>

Il manque l'appel `exit` à la fin du programme : le processeur continue d'exécuter les octets qui suivent la dernière instruction.

</details>
