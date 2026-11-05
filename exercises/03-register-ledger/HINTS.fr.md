<!-- source: HINTS.md @ sha256:fb6bdd17528a -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 03

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Posez un point d'arrêt sur chacune des quatre marques et regardez les registres à chaque arrêt. L'instruction qui compte est celle qui se trouve juste au-dessus de la marque où vous vous êtes arrêté.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Un point d'arrêt sur une étiquette s'arrête avant l'instruction de cette étiquette : les registres et les indicateurs montrent donc le résultat de l'instruction du dessus. Utilisez `p/d $rax` pour afficher `rax` en décimal. Les indicateurs que GDB ne liste pas sont à zéro. Le module 02 explique ce que signifie chaque indicateur.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

À `mark1`, `rax` contient le résultat d'une multiplication. À `mark2`, une soustraction a emprunté et produit un résultat négatif. À `mark3`, une addition 32 bits est repassée à zéro. À `mark4`, une addition 64 bits a débordé en tant que nombre signé. Traduisez chaque ensemble de noms d'indicateurs en quatre chiffres dans l'ordre CF, ZF, SF, OF.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

L'opération elle-même positionne les indicateurs. Capturez-les juste après, avant qu'une autre instruction arithmétique ne les modifie.

</details>

<details>
<summary>Niveau 2 : direction</summary>

`setc`, `setz`, `sets` et `seto` copient chacune un indicateur dans un registre de 8 bits. `mov` et `movzx` ne modifient pas les indicateurs, mais `add`, `or` et `shl` le font : exécutez donc les quatre instructions `set` avant de combiner les octets.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Élargissez chaque octet avec `movzx`, décalez-le jusqu'à la position de son bit avec `shl` (ZF de 1, SF de 2, OF de 3) et combinez les valeurs avec `or`. Rangez le résultat avant de collecter les indicateurs ; une écriture avec `mov` laisse les indicateurs intacts. Les deux fonctions peuvent partager le code qui collecte les indicateurs : laissez `ledger_sub` y entrer après sa propre écriture.

</details>
