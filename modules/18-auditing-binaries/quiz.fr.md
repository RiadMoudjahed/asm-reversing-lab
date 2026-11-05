<!-- source: quiz.md @ sha256:9a5da85b8914 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 18. Auditer des binaires à la recherche de bogues

Répondez d'abord, puis ouvrez la réponse.

**1. Que sont une source et un puits, et comment vous en servez-vous pour commencer un audit ?**

<details>
<summary>Réponse</summary>

Une source est une valeur que le programme ne contrôle pas, comme un champ lu dans un fichier ou un paquet. Un puits est un endroit où une valeur décide de la quantité de mémoire touchée : une copie, un indice, une allocation, une libération. Vous trouvez les puits dans le désassemblage, remontez chaque opérande de taille ou d'indice jusqu'à sa source et lisez chaque comparaison en chemin.

</details>

**2. Une fonction compare `n` à `cap`, refuse l'entrée si `n > cap`, puis copie `n + 1` octets. Qu'est-ce qui ne va pas ?**

<details>
<summary>Réponse</summary>

Une erreur d'un : une chaîne d'exactement `cap` caractères passe la vérification, mais la copie demande `cap + 1` octets avec le terminateur. La vérification doit comparer la quantité qui est copiée, donc l'entrée doit être refusée quand `n >= cap`.

</details>

**3. Quelle combinaison d'instructions signale un bogue de signe avant une copie ?**

<details>
<summary>Réponse</summary>

Une comparaison signée qui ne protège que l'extrémité haute (`cmp`, puis `jg` ou `jle`), suivie d'une extension de signe (`movsxd`, `cdqe`) quand la valeur devient la taille de 64 bits de la copie. Une valeur négative passe le test et devient une énorme taille non signée. Une vérification correcte est une seule comparaison non signée, ou deux comparaisons qui couvrent les deux extrémités.

</details>

**4. Qu'est-ce qui, dans le désassemblage d'une allocation, suggère un débordement d'entier, et pourquoi est-il important que les entrées soient réutilisées plus loin ?**

<details>
<summary>Réponse</summary>

Une instruction arithmétique (comme un `imul` ou un `add` de 32 bits) sur des valeurs dérivées de l'entrée dont le résultat devient la taille passée à `malloc`. Si les mêmes entrées commandent ensuite une boucle ou un indice, la boucle suppose une taille que l'allocation débordée n'a pas, et écrit au-delà du bloc.

</details>

**5. Citez quatre fonctions de bibliothèque qui ne peuvent pas être utilisées sans risque avec une source dépendant de l'entrée et un tampon fixe, et une qui prend une taille mais est quand même souvent mal utilisée.**

<details>
<summary>Réponse</summary>

`strcpy`, `strcat`, `sprintf` et `gets` ne prennent pas de taille de destination (`scanf` avec `%s` sans largeur est identique). `strncpy` prend une taille mais n'écrit pas de terminateur quand la source est aussi longue que la limite, et `memcpy` prend une taille que l'appelant doit avoir vérifiée.

</details>

**6. À quoi ressemble une utilisation après libération dans le désassemblage ?**

<details>
<summary>Réponse</summary>

Un chemin qui passe par un `call free` sur un pointeur et continue jusqu'à une instruction qui déréférence le même pointeur (le registre ou l'emplacement de pile qui contenait l'argument de `free`). Un second `call free` sur la même valeur est une double libération.

</details>

**7. Pourquoi vaut-il mieux arrêter un débogueur avant la copie fautive que laisser le programme aller au-delà ?**

<details>
<summary>Réponse</summary>

Au point d'arrêt, vous pouvez lire la taille et la capacité, qui sont la preuve dont le rapport a besoin, et rien n'est écrit hors du tampon. Laisser la copie excessive se dérouler n'ajoute que des dégâts et du bruit, et n'est pas nécessaire pour un diagnostic.

</details>

**8. Que doit contenir un rapport de bogue, et comment vérifiez-vous qu'un correctif fonctionne ?**

<details>
<summary>Réponse</summary>

La classe de la faiblesse, l'emplacement exact, la cause, la preuve, l'impact (énoncé sans exploitation, y compris ce qui n'a pas été évalué), le correctif et un test. Un correctif se vérifie en répétant l'entrée déclenchante et en constatant que le rapport a disparu, et en lançant les valeurs limites : la plus grande acceptée, la plus petite refusée, zéro, négatif et la valeur qui déborderait.

</details>
