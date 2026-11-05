<!-- source: quiz.md @ sha256:fef7d1c398b4 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 21. Machines virtuelles et bytecode

Répondez d'abord, puis ouvrez la réponse.

**1. Dans un programme qui contient une machine virtuelle, où se trouve la logique que le programme exécute ?**

<details>
<summary>Réponse</summary>

Dans le bytecode, qui est une donnée lue par l'interpréteur. Le désassemblage montre l'interpréteur, qui est le même pour tous les programmes utilisant la machine. Pour comprendre le programme, vous retrouvez le jeu d'instructions à partir de l'interpréteur, puis vous lisez le bytecode.

</details>

**2. Quelles sont les trois étapes que la boucle de l'interpréteur répète pour chaque instruction ?**

<details>
<summary>Réponse</summary>

Elle récupère le code opération au compteur ordinal virtuel, le décode avec ses opérandes et l'exécute en lançant le gestionnaire de ce code. Le compteur ordinal virtuel est une variable ordinaire, pas le registre `rip`.

</details>

**3. En quoi une machine à pile diffère-t-elle d'une machine à registres dans le code de son interpréteur ?**

<details>
<summary>Réponse</summary>

Dans une machine à pile, les opérandes sont implicites : les gestionnaires travaillent au sommet d'une pile et modifient un indice de pile, de sorte qu'un indice qui monte et descend apparaît dans presque tous les gestionnaires. Dans une machine à registres, l'instruction nomme ses registres : les gestionnaires lisent de petits champs de l'instruction et les utilisent comme indices dans un tableau de registres.

</details>

**4. À quoi ressemble une répartition par `switch` dans le désassemblage, et à quoi ressemble une table de gestionnaires ?**

<details>
<summary>Réponse</summary>

Un `switch` se présente comme un contrôle de borne sur le code opération (`cmp` et `ja` vers le cas par défaut) suivi d'un `jmp` indirect à travers une table d'adresses de code. Une table de gestionnaires se présente comme le chargement d'un pointeur depuis un tableau indexé par le code opération, un test de pointeur nul et un `call` indirect.

</details>

**5. Vous trouvez une table de 11 adresses de code et trois d'entre elles sont égales. Que cela vous apprend-il ?**

<details>
<summary>Réponse</summary>

Trois codes opération partagent un gestionnaire. Ce gestionnaire doit alors tester le code opération pour choisir entre les opérations, comme le fait le gestionnaire de `add`, `sub` et `mul` dans la machine à pile de la leçon. Cela signifie aussi qu'il faut lire la table avant les gestionnaires, pour ne pas décrire trois fois le même code.

</details>

**6. Votre désassembleur d'un bytecode personnalisé affiche des instructions sensées au début et n'importe quoi à partir d'un certain point. Quelle est la cause la plus probable, et comment la vérifiez-vous ?**

<details>
<summary>Réponse</summary>

Une instruction avant ce point a la mauvaise taille dans votre table, de sorte que toutes les instructions suivantes sont lues à partir d'une mauvaise position. Vérifiez en traçant la machine pendant son exécution, ce qui donne les instructions réellement exécutées avec les bonnes tailles, ou en testant le désassembleur sur un programme dont vous connaissez le comportement.

</details>

**7. Pourquoi un désassembleur doit-il afficher un octet inconnu comme une donnée et continuer, au lieu de s'arrêter ?**

<details>
<summary>Réponse</summary>

S'arrêter cache le reste du programme. Afficher l'octet comme une donnée et avancer d'un octet permet de tout voir et de remarquer où la table du jeu d'instructions est incomplète ou fausse.

</details>

**8. Dans la machine à registres de la leçon, les numéros de registre sont masqués avec 3. Qu'est-ce que cela signifie pour la spécification ?**

<details>
<summary>Réponse</summary>

Un numéro de registre supérieur à 3 ne provoque pas d'erreur ; il boucle et nomme l'un des quatre registres. C'est un comportement de la machine et il appartient à la spécification, avec les autres masques et bornes.

</details>
