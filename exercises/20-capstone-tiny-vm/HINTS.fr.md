<!-- source: HINTS.md @ sha256:881a006fd716 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 20

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Commencez par la répartition, comme dans le module 21. Trouvez l'endroit où le programme lit le premier octet d'une instruction puis saute à travers une table. Avant le saut, l'octet passe par autre chose : cherchez une lecture dans une petite table indexée par l'octet. Dans les données en lecture seule, trois tables sont proches les unes des autres : une de 256 octets, une avec un octet par instruction et une avec des adresses de code. Extrayez-les.

</details>

<details>
<summary>Niveau 2 : direction</summary>

La table de 256 octets transforme un octet de code opération en un petit nombre, avec 0 pour un octet qui n'est pas une instruction. Chaque entrée non nulle est un code opération valide, et l'entrée est l'indice du gestionnaire dans la table des adresses de code. La table avec un octet par instruction contient la longueur. Pour chaque gestionnaire, trouvez ce qu'il lit dans les octets de l'instruction et quelles parties de l'état il modifie. L'état est une structure sur la pile de la fonction principale ; déduisez sa disposition des décalages que les gestionnaires utilisent et notez-la. Pour les champs de registre, regardez les décalages et les masques appliqués à l'octet qui suit le code opération, et l'ordre des deux octets de 16 bits dans les gestionnaires qui prennent un nombre ou une cible.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Associez les gestionnaires aux noms d'après ce qu'ils font : le gestionnaire qui range l'adresse de retour et saute est `call`, celui qui fait l'appel de sortie avec un format de caractère est `outc`, et ainsi de suite. Trois des gestionnaires arithmétiques ne diffèrent que par l'opération. Lisez avec soin les mises à jour des indicateurs : certaines instructions positionnent les deux, certaines un seul, et les décalages laissent les indicateurs intacts dans un cas. Pour `I`, écrivez le désassembleur à partir de votre table et comptez les lignes de `bin/sample.tvm` ; la longueur du code est dans l'en-tête, et une instruction est aussi longue que le dit son entrée dans la table des longueurs. Pour `N`, comptez les entrées non nulles de la table de 256 octets. Pour `C`, trouvez l'indice du gestionnaire qui range une adresse de retour et cherchez quel octet de la table de 256 octets contient cet indice.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Faites les gestionnaires un par un et testez chacun, pour que le rapport du vérificateur reste court. Les registres de la machine forment un tableau au début de la structure ; un champ de registre devient donc un décalage en le multipliant par 4. La première chose que fait chaque gestionnaire est de décomposer l'octet qui suit le code opération. Pour le programme, voyez la mémoire comme un tableau de douze octets et écrivez les nombres avec une boucle.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Prenez les numéros de registre dans les quartets de l'octet qui suit le code opération, avec le masque que l'original applique. Utilisez `setcc` pour transformer une comparaison en un 0 ou un 1 que vous rangez dans un indicateur. Lisez `shl` et `shr` dans l'original pour l'indicateur de retenue : ce n'est pas le même bit dans les deux, et un compteur de zéro est un cas particulier. Pour `call` et `ret`, suivez la profondeur : l'adresse de retour va dans l'emplacement à la profondeur courante, et `ret` la reprend dans l'emplacement du dessous. Le vérificateur affiche les champs qui diffèrent, avec la valeur avant l'instruction, ce qui vous dit ce que vous avez fait et ce que vous avez raté. Pour le programme : une boucle qui additionne les deux derniers nombres et range chacun, puis un sous-programme qui parcourt la mémoire vers le bas et affiche.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Pour `cmp`, l'indicateur de retenue est positionné quand la première valeur est plus petite que la seconde comme nombres non signés, et l'indicateur de zéro quand elles sont égales ; rien n'est rangé. Pour `mul`, seuls les 32 bits de poids faible sont conservés et l'indicateur de retenue est mis à zéro. Pour `ldb` et `stb`, seul l'octet de poids faible du registre d'adresse est utilisé, et `stb` ne range que l'octet de poids faible de l'autre registre. Pour `shl`, l'indicateur de retenue est le dernier bit qui sort par le haut ; pour `shr`, le dernier bit qui sort par le bas. Le compteur ordinal que range `call` est celui qui a déjà été avancé. Pour le programme : la machine démarre avec tous les registres à zéro, donc un registre que vous ne positionnez jamais peut servir de constante 0 dans une comparaison ; comparez le compteur avec lui pour arrêter la boucle d'affichage.

</details>
