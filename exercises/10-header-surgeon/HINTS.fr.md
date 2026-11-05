<!-- source: HINTS.md @ sha256:25348698dd82 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 10

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Exécutez `readelf -h`, `readelf -l`, `readelf -S` et `readelf -d` sur le fichier. Chaque valeur dont vous avez besoin figure dans l'un de ces quatre listings.

</details>

<details>
<summary>Niveau 2 : direction</summary>

La base d'image est la `VirtAddr` de la première entrée `LOAD`. Le point d'entrée est une adresse virtuelle ; la première entrée `LOAD` a le décalage 0 dans le fichier, donc le décalage dans le fichier est le point d'entrée moins la base. Le nombre d'en-têtes de programme est dans l'en-tête ELF. `MemSiz` et `FileSiz` sont des colonnes des entrées `LOAD`. Les noms de section que vous ne reconnaissez pas sont rares ; cherchez-en un dans la liste des sections. Les entrées `NEEDED` sont dans la section dynamique.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Utilisez l'entrée `LOAD` avec les indicateurs `RW` pour `M` : soustrayez les deux tailles. Pour `V`, la section qui ne figure pas dans la liste habituelle de `.text`, `.data`, `.rodata`, `.bss` et ainsi de suite a une taille dans la colonne `Size` ; elle est en hexadécimal, convertissez-la donc. Il y a deux bibliothèques `NEEDED` ; l'une est la bibliothèque C `libc.so.6`, l'autre donne `L`.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Écrivez le fichier dans l'ordre où il apparaît sur le disque : en-tête ELF, en-têtes de programme, code, remplissage, données. Utilisez `org` avec la base d'image pour que les étiquettes aient les bonnes adresses virtuelles, et `$$` pour calculer les décalages dans le fichier.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Il vous faut trois en-têtes de programme : un `PT_LOAD` pour les en-têtes et le code (indicateurs 5), un `PT_LOAD` pour le message (indicateurs 6) et un `PT_GNU_STACK` (indicateurs 6). L'en-tête ELF fait 64 octets et chaque en-tête de programme 56. Placez le message à un décalage de fichier comme 0x200, remplissez le fichier jusque-là avec `times` et donnez au segment une adresse virtuelle ayant le même reste modulo 0x1000 : par exemple la base d'image plus 0x1000 plus 0x200.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Dans l'en-tête ELF, mettez `e_phoff` à 64, `e_phentsize` à 56 et `e_phnum` à 3, et laissez `e_shoff`, `e_shentsize`, `e_shnum` et `e_shstrndx` à zéro. Pour le premier segment, utilisez `p_offset` 0, `p_vaddr` égal à la base et `p_filesz` égal à la fin du code. Le code fait `write` avec l'adresse des données dans `esi`, puis `exit`. Si le vérificateur dit que le décalage et l'adresse doivent concorder modulo `p_align`, soustrayez-les et vérifiez que la différence est un multiple de 0x1000. Si le programme meurt avec le signal 11, le point d'entrée ou l'adresse du message est en dehors des plages projetées.

</details>
