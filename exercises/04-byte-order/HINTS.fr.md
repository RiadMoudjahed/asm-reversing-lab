<!-- source: HINTS.md @ sha256:555e8220e82a -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 04

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Regardez comment chaque champ est chargé avant d'être ajouté au total. Certains chargements sont suivis d'une instruction supplémentaire, d'autres non.

</details>

<details>
<summary>Niveau 2 : direction</summary>

`bswap` inverse les quatre octets d'un registre. `rol dx, 8` inverse les deux octets de `dx`. Un chargement sans instruction supplémentaire conserve l'ordre de la mémoire, qui est petit-boutiste sur x86. La constante du `cmp` vous permet de vérifier votre décodage : les quatre valeurs doivent s'additionner pour la donner.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Le champ A fait 4 octets au décalage 0 et il est échangé : il est donc gros-boutiste. Le champ B fait 2 octets au décalage 4 et n'est pas échangé. Le champ C fait 2 octets au décalage 6 et il est échangé. Le champ D fait 4 octets au décalage 8 et n'est pas échangé. Prenez les octets du vidage de `.rodata`, inversez ceux qui sont échangés et lisez chaque résultat comme un nombre. Convertissez l'hexadécimal en décimal là où la question demande du décimal.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Sur x86, un chargement simple lit déjà correctement les valeurs petit-boutistes. Seuls les champs gros-boutistes demandent du travail.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Pour construire une valeur gros-boutiste, partez du premier octet, décalez-le de 8 vers la gauche, combinez l'octet suivant avec `or`, et recommencez. Utilisez `movzx` pour charger chaque octet afin que les bits hauts du registre soient à zéro. Charger un double mot entier puis le corriger lirait les bons octets, mais demanderait `bswap`, ce qui n'est pas autorisé.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

`load_le16` tient en une instruction : `movzx eax, word [rdi]`. Pour `decode_record`, construisez `version` et `flags` avec le schéma décalage puis `or`, et rangez-les avec une écriture de 4 octets et une de 2 octets. Copiez `port` et `timeout_ms` avec de simples chargements et écritures de même largeur. Une écriture plus large que le champ écrase son voisin, et un chargement plus large que le champ peut lire au-delà de la fin de l'entrée ; les deux font échouer le vérificateur.

</details>
