<!-- source: HINTS.md @ sha256:522129361d31 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 15

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Trouvez l'appel qui alloue de la mémoire et regardez ses arguments : ils donnent la taille d'un nœud. Lisez ensuite les écritures qui suivent l'appel. Chacune écrit un champ du nouveau nœud, à un décalage fixe depuis le pointeur renvoyé, et la taille de son opérande indique la largeur du champ.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Le parcours est une boucle. Chaque tour charge la clé du nœud courant et la compare à la clé que l'on insère. Une comparaison termine la boucle et les deux autres choisissent quel pointeur charger ensuite, tous deux à des décalages fixes depuis le nœud courant. Le code de recherche et le code d'insertion utilisent les mêmes deux décalages de la même façon ; lire la boucle de recherche, plus courte, vous dit donc quel pointeur correspond à quel cas. Une instruction qui additionne sur un opérande mémoire dans la boucle, avant que la comparaison ne décide quoi que ce soit, est un compteur que tout parcours incrémente.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Deux opérandes mémoire utilisent `add` ou `inc` sur un champ : l'un dans la boucle de parcours et l'autre après le saut d'égalité. Le premier est le compteur de passage, le second le compteur de doublons. Regardez la taille de l'opérande du second : la largeur d'un champ indique ce qui se passe lorsqu'on l'incrémente assez souvent. Exécutez les 65540 insertions et comparez le nombre que vous voyez avec 65540. La taille d'un nœud est l'argument de l'allocation, et non le décalage du dernier champ plus sa largeur.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Écrivez d'abord `inv_find`. Elle n'a besoin ni d'appels ni de pile : charger la clé du nœud, comparer, puis passer au pointeur suivant ou s'arrêter. Écrivez ensuite `inv_insert` comme la même boucle avec les étapes supplémentaires trouvées dans la Partie 1.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Dans `inv_insert`, gardez l'adresse du lien que vous allez suivre, et non le nœud. Le lien commence comme l'adresse d'une variable qui contient la racine. Lorsque le lien contient 0, appelez `inv_alloc` et rangez le résultat dans le lien. Ce seul cas couvre une structure vide et une nouvelle feuille à n'importe quelle profondeur. La variable de la racine doit résider en mémoire ; réservez donc un emplacement de pile pour elle et renvoyez sa valeur finale.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Ordonnez le travail dans la boucle comme le fait le programme : ajoutez d'abord au compteur de passage, puis comparez, puis soit mettez à jour le compteur de doublons et arrêtez-vous, soit choisissez le lien suivant. Utilisez pour chaque compteur la taille d'opérande que vous avez trouvée ; un incrément de 32 bits sur un champ de 16 bits modifie aussi les octets qui le suivent, ce que les tests remarquent. Comparez les clés comme des valeurs signées. Gardez la pile alignée pour l'appel : après les push des registres que vous utilisez, vérifiez que `rsp` est un multiple de 16 avant `call inv_alloc`. Si le vérificateur affiche `find`, votre recherche diffère de celle de la bibliothèque ; `alloc` signifie que le nombre d'appels à `inv_alloc` est faux ; `overrun` signifie que vous avez écrit en dehors des champs ; `align` signifie que la pile était mal alignée à l'appel ; `regs` signifie qu'un registre à sauvegarder par l'appelé a été modifié.

</details>
