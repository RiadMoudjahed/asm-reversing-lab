<!-- source: HINTS.md @ sha256:e5914c768586 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 18

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Commencez par fabriquer de petits fichiers d'entrée et observer la sortie, afin de savoir ce que fait chaque type d'enregistrement. Cherchez ensuite dans le désassemblage l'aiguillage selon le type d'enregistrement : une comparaison avec un petit nombre suivie d'un saut indirect par une table. Chaque entrée de la table est le code d'un type d'enregistrement. Lisez le code de chaque type comme un puits : quelle taille ou quel indice vient de l'entrée, et quelle vérification se trouve entre l'entrée et l'usage ?

</details>

<details>
<summary>Niveau 2 : direction</summary>

Pour chaque gestionnaire, notez la taille de la destination, la plus grande taille que la vérification laisse passer, et les largeurs et signes des valeurs en jeu. Dans le gestionnaire des nombres, regardez la largeur du registre dans la comparaison et quel saut la suit : signé ou non signé. Dans le gestionnaire des blocs de données, suivez le nombre à travers l'allocation puis à travers la copie, et comparez à chaque fois la largeur du registre utilisé. Regardez comment le parcours des enregistrements calcule le suivant et à quoi il compare cette position. Dans les gestionnaires qui libèrent de la mémoire, suivez le pointeur jusqu'à la fin de la fonction.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Le gestionnaire des noms compare la longueur à une constante, puis écrit un octet de plus que la longueur : quelle est la constante, et que fait l'octet supplémentaire à la plus grande longueur acceptée ? Pour les nombres, un octet comparé comme signé est négatif à partir d'une certaine valeur ; utilisez cette valeur. La table des étiquettes a huit entrées, et la constante de la comparaison vous donne le plus grand indice qui passe ; comparez les deux. Pour les blocs de données, trouvez l'instruction qui tronque la taille d'allocation à 8 bits et calculez le plus petit nombre dont la taille en octets ne tient pas sur 8 bits. Pour la libération de mémoire, trouvez dans la table de sauts le numéro de type dont le code appelle la fonction de libération puis laisse le pointeur tranquille ; l'entrée du type 4 libère aussi de la mémoire, mais on vous demande le type d'enregistrement qui en fait toute sa mission.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Écrivez d'abord la boucle : combien d'octets restent, lire le type et la longueur, vérifier que la charge utile tient, puis avancer. Faites ces vérifications avant de regarder le type. Mettez chaque type d'enregistrement dans un bloc à part et faites revenir chacun à la boucle. N'utilisez que les registres qu'une fonction peut modifier, il n'y aura ainsi rien à sauvegarder.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Gardez une position et la longueur du tampon, et calculez toujours les octets restants par une soustraction avant de comparer : elle ne peut pas déborder, alors que position plus longueur le peut. Pour les nombres, chargez chaque valeur de 32 bits avec un déplacement à extension de signe et ajoutez-la à la somme de 64 bits en mémoire. Pour les étiquettes, vérifiez l'octet avant de l'utiliser comme indice. Pour le bloc de données, vérifiez la longueur par rapport au nombre avant de lire quoi que ce soit après le nombre. Écrivez les résultats dans la structure au fur et à mesure ; les tests ne la regardent pas quand vous renvoyez une erreur.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Pour le nom, écrivez d'abord 16 octets nuls (une seule écriture de 16 octets suffit) puis copiez les octets, de sorte qu'un nom plus court remplace un plus long. Pour les nombres, testez les bits bas de la longueur pour le multiple de 4, puis comparez-la à 32. Pour le bloc de données, calculez 1 plus 4 fois le nombre dans un registre de 32 bits, qui ne peut pas déborder pour un nombre tenant dans un octet, et comparez-le à la longueur. Un bloc remplace un bloc antérieur : mettez donc la somme à 0 avant d'additionner. L'enregistrement de type 0 suit la même règle de troncature que les autres. Si le vérificateur se termine par un signal, une lecture est sortie du tampon : regardez d'abord la vérification de l'en-tête et celle de la charge utile.

</details>
