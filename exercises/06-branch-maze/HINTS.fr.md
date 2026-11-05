<!-- source: HINTS.md @ sha256:36d540613cb0 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 06

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Dans `main`, regardez ce qui se passe après la conversion du troisième nombre. Chaque contrôle est une comparaison suivie d'un saut conditionnel vers le code qui affiche `wall`. Comptez les contrôles.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Associez chaque comparaison au saut qui la suit. `ja` et `jb` sont non signés ; `jl` et `jg` sont signés ; `jns` teste le bit de signe. Un `lea` ou un `sub` suivi d'un `cmp` non signé est un contrôle de plage : soustraire la borne inférieure transforme « entre le minimum et le maximum » en une seule comparaison non signée.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Le premier contrôle est un contrôle de plage sur A. Le deuxième ne garde que certains bits de A avec `and`. Le troisième est le même genre de contrôle de plage sur B. Les trois derniers concernent C : son signe, son octet de poids faible (une comparaison sur 8 bits) et une borne inférieure signée. Déterminez l'ensemble des valeurs que chaque contrôle autorise, faites l'intersection et prenez la plus petite valeur de chaque nombre.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Écrivez les mêmes contrôles que le binaire, dans le même ordre. Chaque saut d'échec peut aller vers une étiquette commune qui renvoie 0.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Une condition `low <= x <= high` devient une soustraction et une comparaison non signée : `lea eax, [rdi - low]`, `cmp eax, high - low`, `ja no`. Utilisez `test` et `jns` pour le signe, et `cmp dl, value` pour ne regarder que l'octet de poids faible.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Choisissez chaque saut d'après le type du contrôle : `ja` pour « supérieur » non signé, `jl` pour « inférieur » signé. Un nombre négatif paraît énorme lorsqu'on le compare en non signé : un test sur le signe ou sur une borne signée demande donc un saut signé. Mettez `mov eax, 1` après le dernier contrôle et `xor eax, eax` à l'étiquette d'échec commune.

</details>
