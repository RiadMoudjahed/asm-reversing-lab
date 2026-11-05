<!-- source: HINTS.md @ sha256:6f1c42f8454d -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 19

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Exécutez le programme et lisez les lignes de journal, puis comparez-les à ce que trouve `strings` : les lignes contiennent des marqueurs là où devraient se trouver les vraies données. Cherchez la fonction appelée une fois pour chaque marqueur et la table des données en lecture seule dans laquelle elle lit. La table n'est pas du texte. Cherchez la boucle de la fonction : elle est courte, et chaque opération qu'elle contient vous est connue depuis le module 20.

</details>

<details>
<summary>Niveau 2 : direction</summary>

La boucle conserve un octet d'état qui change à chaque position, et l'octet en clair est l'octet encodé combiné avec cet état. Notez les opérations qui modifient l'état : une rotation, une addition de la position, une addition d'une constante. Les deux premiers octets de chaque entrée ne sont pas du texte encodé : l'un est le nombre d'octets qui suivent, l'autre initialise l'état. Pour les techniques, déterminez ce qu'est chaque texte décodé (un nom, un chemin, un nom de fichier, un domaine) et cherchez le comportement dans la matrice ATT&CK pour Windows : services, ordre de recherche des bibliothèques, protocoles de la couche application. Pour la règle, utilisez ce qui se trouve vraiment dans le fichier et non ce qu'il affiche : comparez le texte en clair de l'échantillon aux programmes leurres et choisissez des chaînes que seul l'échantillon possède.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Décodez à la main la première entrée avec un script qui suit exactement la boucle, en arithmétique sur 8 bits : faites tourner l'état de trois positions vers la gauche, ajoutez la position, ajoutez la constante et combinez par xor. Quand la première entrée décodée se lit comme un nom de service, la règle est correcte ; le même script donne toutes les autres. Comptez les entrées d'après la table ou d'après le nombre d'appels de la fonction ; prenez les longueurs dans le premier octet de chaque entrée. Pour la règle, les leurres contiennent les mots du sujet mais pas les lignes de journal des comportements, et le programme de mise à jour contient le même décodeur avec d'autres données : une règle fondée sur le seul décodeur correspond au programme de mise à jour, et une règle fondée sur un mot courant correspond à l'exécution à blanc. Combinez plusieurs modèles de lignes de journal de l'échantillon, exigez le format de l'exécutable et ajoutez les premiers octets de la première entrée encodée.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Faites d'abord les deux contrôles de taille, dans l'ordre de la spécification, avant de lire ou d'écrire le moindre octet de l'entrée. Écrivez ensuite la boucle du décodeur que vous avez trouvé dans la partie 1 : l'état tient sur un octet, travaillez donc dans un registre de 8 bits.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Lisez la longueur dans le premier octet de l'entrée et calculez les octets nécessaires comme la longueur plus 2 ; comparez avec `avail`. La place nécessaire dans `out` est la longueur plus 1 ; comparez avec `cap`. Dans la boucle, gardez la position dans un registre de 64 bits, utilisez son octet de poids faible pour l'addition à l'état et lisez l'octet encodé à l'adresse de l'entrée plus la position plus 2. Après la boucle, stockez l'octet nul et renvoyez la longueur. Les registres `rax`, `rcx`, `rdx`, `rsi`, `rdi` et `r8` à `r11` peuvent être modifiés librement.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Le corps de la boucle comporte quatre étapes : faire tourner l'octet d'état de 3 positions vers la gauche (`rol` sur un registre de 8 bits), ajouter l'octet de poids faible de la position, ajouter la constante que vous avez trouvée et combiner le résultat par xor avec l'octet encodé. La valeur initiale de l'état est le deuxième octet de l'entrée. Si un test avec une entrée coupée se termine par un signal, le contrôle de `avail` est faux ou arrive trop tard ; si un test avec un petit `cap` échoue, vérifiez que les deux codes d'erreur sont renvoyés dans l'ordre de la spécification et que rien n'est écrit avant les contrôles.

</details>
