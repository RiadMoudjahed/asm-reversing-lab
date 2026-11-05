<!-- source: HINTS.md @ sha256:2278a21c0cab -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 11

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

`objdump -p` affiche les valeurs de l'en-tête facultatif, la table des sections et les tables d'importation. Le script du module 12 affiche la table des sections avec les RVA et les décalages dans le fichier.

</details>

<details>
<summary>Niveau 2 : direction</summary>

La base d'image, la RVA du point d'entrée et le nombre de sections sont des champs des en-têtes. Pour `O`, trouvez la section dont la plage de RVA contient 0x12348 et appliquez : décalage dans le fichier = RVA - RVA de la section + décalage brut de la section. Le script du module 12 n'affiche que les premiers noms de fonction de chaque bibliothèque ; pour `K`, comptez tous les noms de la liste d'importation de `KERNEL32.dll`, par exemple avec `objdump -p`.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Cherchez dans la liste des sections celle qui commence à 0x12348 ou juste en dessous, puis vérifiez que 0x12348 est inférieur à son début plus sa taille brute. Le résultat est en hexadécimal et s'écrit sans `0x`. Le nombre de bibliothèques importées est le nombre de lignes `DLL Name`. La valeur du point d'entrée est une RVA, pas une adresse : n'ajoutez pas la base d'image.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Commencez par `pe_rva_to_offset` : elle n'a besoin que de registres volatils et d'aucun appel. Écrivez-la comme une boucle sur la table des sections, testez la RVA contre chaque section et renvoyez `0xFFFFFFFF` après la boucle. Écrivez ensuite `pe_entry_offset` en vous appuyant dessus.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Un en-tête de section fait 40 octets. Les champs dont vous avez besoin sont l'adresse virtuelle, la taille des données brutes et le pointeur vers les données brutes ; lisez leurs positions dans la table des sections que vous avez affichée à la Partie 1. Pour le test non signé, soustrayez l'adresse virtuelle de la RVA et comparez le résultat à la taille des données brutes avec un saut non signé : si la RVA est en dessous de la section, la soustraction déborde vers un nombre énorme et la comparaison échoue d'elle-même. Dans `pe_entry_offset`, l'en-tête facultatif commence 24 octets après la signature NT, la RVA du point d'entrée s'y trouve au décalage 16, et la table des sections commence après l'en-tête facultatif, dont la taille est un champ de l'en-tête de fichier.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

`pe_entry_offset` fait un appel, elle a donc besoin d'un cadre : sauvegardez l'adresse des en-têtes NT dans `rbx` (empilez-le d'abord), réservez au moins 32 octets d'espace d'ombre et assurez-vous que `rsp` est un multiple de 16 à l'appel. Après un `push`, la pile est alignée ; retranchez donc un multiple de 16. Gardez la RVA du point d'entrée dans un emplacement de pile au-dessus de l'espace d'ombre, car `log_rva` détruit `rcx` et tous les autres registres volatils. Si le vérificateur affiche `align`, la taille du cadre est fausse ; `regs` signifie qu'un registre non volatil a été modifié sans être restauré (pensez à `rsi` et `rdi`) ; `calls` signifie que `log_rva` n'a pas été appelée exactement une fois avec la RVA du point d'entrée.

</details>
