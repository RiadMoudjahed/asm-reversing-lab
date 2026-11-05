<!-- source: HINTS.md @ sha256:f4a45beb89f9 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 05

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Exécutez le programme avec quelques entrées et lisez à chaque fois le code de sortie avec `echo $?`. Regardez ensuite la routine qui vérifie chaque ligne ; les limites sont des constantes dans des instructions de comparaison.

</details>

<details>
<summary>Niveau 2 : direction</summary>

`sub al, 0x30` suivi de `cmp al, 9` et de `ja` est le test classique d'un chiffre ASCII : tout ce qui n'est pas de `0` à `9` se retrouve au-dessus de 9 en tant que nombre non signé. Une comparaison du nombre de chiffres avec une constante est la limite de longueur. Le code de sortie est la valeur de `ebx` lorsque `eax` vaut 1 et que `int 0x80` s'exécute.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Le contrôle de longueur compare `ecx` à une constante, et tout ce qui la dépasse saute vers le chemin d'erreur. Chacun des deux chemins de fin, l'erreur et la somme trop grande, charge `ebx` avec sa propre valeur juste avant de quitter. Lisez la constante dans le contrôle de longueur, et lisez les deux codes à la fin de chaque chemin. Convertissez l'hexadécimal en décimal avant de construire le drapeau.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Un nombre de 12 chiffres ne tient pas dans un registre de 32 bits : travaillez donc sur le texte lui-même, en additionnant chiffre par chiffre depuis la droite et en propageant une retenue.

</details>

<details>
<summary>Niveau 2 : direction</summary>

À chaque position, soustrayez `0x30` du caractère pour obtenir le chiffre, additionnez les deux chiffres et la retenue, et soustrayez 10 (en mettant la retenue à 1) si le résultat vaut 10 ou plus. Rajoutez `0x30` et rangez le caractère dans un tampon que vous remplissez depuis la droite. Traitez un chiffre manquant dans le nombre le plus court comme 0.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Vérifiez chaque ligne avant d'additionner quoi que ce soit : chaque caractère jusqu'au saut de ligne doit être un chiffre, une ligne doit avoir au moins un et au plus 12 chiffres, et une ligne sans saut de ligne à la fin est aussi une erreur. Réservez 13 positions pour la somme : si la première n'est pas nulle, la réponse est `OVERFLOW`. À l'affichage, sautez les zéros en tête mais affichez toujours au moins un chiffre.

</details>
