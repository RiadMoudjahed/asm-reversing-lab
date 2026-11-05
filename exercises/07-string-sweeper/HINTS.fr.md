<!-- source: HINTS.md @ sha256:c524387281a2 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 07

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Ouvrez les six routines une à une. Chacune est courte et contient une instruction de chaîne avec `rep`, `repz` ou `repnz`. Notez quelle instruction et quel préfixe chacune utilise.

</details>

<details>
<summary>Niveau 2 : direction</summary>

`rep stos` remplit la mémoire. `rep movs` la copie. `repz cmps` compare deux blocs tant qu'ils sont égaux. `repnz scas` cherche l'octet contenu dans `al`. Deux routines utilisent `repnz scas` : l'une commence avec `al` à zéro et transforme le `rcx` final en longueur, l'autre commence avec `al` pris dans un argument et renvoie un pointeur. Deux routines copient : l'une ne va que vers l'avant, l'autre contient `std` et `cld`.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

La routine avec `std` et `cld` est la copie sûre en cas de chevauchement. Celle qui copie un quadruple mot à la fois puis le reste octet par octet est la copie simple. La routine `repz cmps` soustrait deux octets à la fin, ce qui donne son résultat à une comparaison. La routine qui renvoie `rdi - 1` après une correspondance est la recherche ; celle qui contient `not rcx` est la longueur. Utilisez les numéros des noms de symboles pour construire le drapeau.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Chaque routine est courte. Préparez `rsi`, `rdi` et `rcx` (et `al` là où un octet intervient), puis laissez une seule instruction `rep` faire le travail.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Pour `my_strlen`, partez de `al` à zéro et de `rcx` à -1, exécutez `repnz scasb` et transformez le `rcx` restant en longueur. Pour `my_memchr`, vérifiez d'abord si `n` vaut 0, puis exécutez `repnz scasb` et regardez l'indicateur de zéro. Pour `my_memmove`, comparez les adresses : si la destination est au-dessus de la source et que les blocs se chevauchent, positionnez `std` et copiez depuis le dernier octet.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Après `repnz scasb` sur `n` octets, l'indicateur de zéro n'est positionné que si une correspondance a été trouvée, et `rdi` se trouve un octet après la correspondance. `not rcx` après le balayage compte aussi le terminateur : retranchez un. Ne copiez vers l'arrière que si `rdi` est au-dessus de `rsi` et que `rdi` est en dessous de `rsi + n`, avec `std`, en partant de `rsi + n - 1` et `rdi + n - 1`, et terminez par `cld`. Si le vérificateur indique que l'indicateur de direction est resté positionné, il manque un `cld` sur un chemin.

</details>
