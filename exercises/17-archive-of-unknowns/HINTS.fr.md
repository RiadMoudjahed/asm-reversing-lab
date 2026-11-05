<!-- source: HINTS.md @ sha256:2eebb8b3da83 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 17

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Commencez par des échantillons : empaquetez des fichiers dont le contenu est facile à reconnaître, comme une seule lettre répétée, et regardez le vidage hexadécimal. Trouvez la valeur magique, les nombres qui ressemblent à des comptes ou à des décalages, et les noms. Empaquetez ensuite un fichier de plus et regardez ce qui bouge. Dans le programme, trouvez le code qui range les premiers octets de la sortie ; il range des constantes et des valeurs calculées à partir du nombre de fichiers.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Comparez des archives faites avec des nombres de fichiers différents et des contenus différents. Un fichier d'octets identiques n'apparaît pas comme des octets identiques dans l'archive, le programme modifie donc les données ; et une courte boucle avec une rotation calcule une valeur rangée pour chaque fichier. Le pas entre les entrées de la table est un décalage de bits ou une multiplication dans le code. Pour les limites, cherchez des comparaisons avec des constantes avant la lecture d'un fichier : elles protègent les messages d'erreur que vous voyez avec `strings`.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Le décalage de chaque bloc de données est arrondi par le haut avec une addition et un masque ; le masque indique l'alignement. La clé est calculée à partir du nombre de fichiers par une multiplication et une addition, toutes deux avec des constantes dans le code ; évaluez-la pour 3 fichiers. L'instruction de rotation est dans la boucle qui parcourt les octets de données. Vérifiez chaque nombre en empaquetant un fichier avec l'outil et en lisant le résultat dans le vidage. La limite du nombre de fichiers est la constante de la comparaison qui précède le message sur le trop grand nombre de fichiers.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Écrivez d'abord `ark_count` et faites-la appeler par les deux autres, afin que l'en-tête soit validé en un seul endroit. Utilisez des registres de 64 bits pour chaque décalage, chaque taille et chaque somme, et étendez par des zéros ce que vous chargez depuis le fichier.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Calculez où se trouve l'entrée de la table pour un indice : le décalage de la table plus 16 fois l'indice. Une comparaison non signée de l'indice avec le nombre rejette les indices négatifs en même temps que les grands. Pour les noms, cherchez l'octet nul avec une limite à la fin de l'archive et pas au-delà. Pour l'extraction, décodez chaque octet avec la clé et la position, mettez à jour la somme de contrôle avec l'octet décodé et comparez à la fin ; faites les vérifications de bornes et de capacité avant de toucher aux données.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Comme le décalage de la table, celui des données et la taille sont des valeurs de 32 bits dans le fichier, additionnez-les dans des registres de 64 bits : `mov ecx, [..]` efface déjà la moitié haute. Une plage de données se termine à décalage plus taille, et cette somme ne doit pas dépasser `n`. L'octet de clé de la position `i` est l'octet de poids faible de la somme de la clé et de `i`, que donne un `lea` vers un registre de 32 bits suivi d'un `xor` d'octet. La somme de contrôle part de la constante que vous avez trouvée, est tournée puis additionnée pour chaque octet décodé, et est enfin combinée avec la taille. Si le vérificateur se termine par un signal, vous avez lu en dehors de l'archive ; vérifiez d'abord la recherche des noms et les sommes.

</details>
