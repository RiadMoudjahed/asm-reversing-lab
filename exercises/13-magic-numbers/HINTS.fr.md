<!-- source: HINTS.md @ sha256:cbf7ef633e3e -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 13

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Cherchez les instructions `imul` qui multiplient par une grande constante, chacune suivie d'un décalage à droite. Il y a quatre telles paires dans `mix`, une par division. Notez chaque constante et son décalage dans un tableau.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Le diviseur est 2 puissance le décalage total, divisé par la constante, arrondi à l'entier le plus proche. Le décalage total compte chaque décalage à droite qui suit la multiplication, y compris un `shr` de 32 suivi d'un plus petit. Exécutez `14_magic.py` du module 14 pour vérifier votre lecture. Pour `K` et `M`, regardez les instructions `lea` qui suivent chaque quotient : `lea r, [x+x*4]` est une multiplication par 5, `lea r, [x+x*2]` par 3, `lea r, [x+x*8]` par 9.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

L'une des séquences signées charge une constante que le désassembleur affiche avec beaucoup de chiffres `f`, comme `0xffffffff88888889`. C'est une constante 32 bits dont le bit de poids fort est à 1, étendue avec signe ; le vrai nombre magique est constitué des 32 bits bas lus comme positif, et le `add` supplémentaire de `x` après le `shr 32` compense le signe. Le reste non signé se calcule comme la valeur moins le quotient fois le diviseur, et le quotient fois le diviseur est un `lea` ; le facteur `M` multiplie le résultat de cette soustraction. Pour vérifier une formule candidate, exécutez `bin/magic` sur quelques entrées et comparez.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Écrivez la formule avec les quatre diviseurs trouvés. Écrivez ensuite un bloc pour chaque terme. Chaque bloc transforme une division par une constante en `x * M` suivi d'un décalage à droite, avec `M` et le décalage tirés du binaire.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Utilisez des registres 64 bits pour les produits : `mov eax, edi` étend `x` avec des zéros pour les termes non signés, et `movsxd rax, edi` l'étend avec signe pour les termes signés. Une multiplication par une constante 32 bits dont le bit de poids fort est à 1 demande que la constante soit chargée comme valeur positive 64 bits, par exemple avec `mov edx, constant`. Le reste est `x - q * d`. Pour les termes signés, décalez arithmétiquement (`sar`) et ajoutez 1 au quotient quand `x` est négatif ; la valeur `x >> 31` vaut -1 pour `x` négatif, retranchez-la donc.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Pour le quotient non signé, `(x * M) >> k` suffit avec la constante et le décalage du binaire. Le quotient et le reste signés demandent la correction pour les entrées négatives : si le quotient est trop petit d'une unité pour `x` négatif, vous avez oublié de retrancher `x >> 31`. Si seules les entrées à partir de 2^31 échouent, les termes non signés ont été traités avec une extension de signe. Si le vérificateur dit que l'instruction `div` a été trouvée, remplacez-la par la multiplication ; vous pouvez combiner tous les termes avec `lea` à la fin.

</details>
