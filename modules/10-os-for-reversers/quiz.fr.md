<!-- source: quiz.md @ sha256:ee242f6d4249 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 10. Fondamentaux des systèmes d'exploitation pour le rétro-ingénieur

Répondez d'abord, puis ouvrez la réponse.

**1. Que partagent les fils d'un même processus, et que possède chaque fil en propre ?**

<details>
<summary>Réponse</summary>

Ils partagent l'espace d'adressage et les fichiers ouverts. Chaque fil a ses propres registres et sa propre pile.

</details>

**2. Que se passe-t-il quand du code écrit dans une page en lecture seule ?**

<details>
<summary>Réponse</summary>

Le processeur déclenche une faute, et le noyau termine normalement le processus par un signal (`SIGSEGV` sous Linux), sauf si le programme gère ce signal.

</details>

**3. Pourquoi faut-il noter des décalages et des noms plutôt que des adresses absolues ?**

<details>
<summary>Réponse</summary>

La randomisation de la disposition de l'espace d'adressage place la pile, le tas et les bibliothèques à des adresses différentes à chaque exécution : une adresse absolue d'une exécution est fausse à la suivante.

</details>

**4. Pourquoi `main` n'est-il pas le premier code exécuté dans un programme ?**

<details>
<summary>Réponse</summary>

Le chargeur et le code de démarrage s'exécutent d'abord : ils projettent les bibliothèques, résolvent les références et préparent l'environnement d'exécution. Le code de démarrage appelle ensuite `main`.

</details>

**5. Un programme appelle `printf` trois fois et `strace` montre un seul `write`. Expliquez.**

<details>
<summary>Réponse</summary>

La bibliothèque C met la sortie en tampon. `printf` remplit un tampon, et la bibliothèque émet un `write` quand le tampon est vidé. `ltrace` montrerait trois appels à `printf`.

</details>

**6. Citez les couches qu'un appel d'API Windows peut traverser avant d'atteindre le noyau.**

<details>
<summary>Réponse</summary>

L'API Windows (`kernel32.dll` et semblables), l'API native de `ntdll.dll` (les fonctions `Nt...`), puis l'instruction `syscall` vers le noyau.

</details>

**7. Qu'est-ce qu'un handle sous Windows, et quel est son équivalent sous Linux ?**

<details>
<summary>Réponse</summary>

Un handle est un nombre qui indexe une table par processus d'objets du noyau. L'équivalent sous Linux est le descripteur de fichier. Dans les deux cas, le nombre ne signifie rien en lui-même ; c'est l'objet derrière qui compte.

</details>
