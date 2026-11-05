<!-- source: HINTS.md @ sha256:796d52974ca9 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 01

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

`strace` affiche chaque appel système avec ses arguments et son résultat. Regardez le dernier nombre après chaque signe `=` et le dernier argument de chaque appel.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Dans le listing d'`objdump`, chaque instruction `syscall` est précédée d'instructions `mov` qui chargent `eax`, `edi`, `rsi` et `edx`. `eax` contient le numéro d'appel. Pour `write`, `edi` est le descripteur de fichier et `edx` le nombre d'octets. Les nombres du listing sont en hexadécimal.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Il y a trois instructions `syscall`. Les deux premières chargent la même valeur dans `eax`, qui est le numéro de `write`. Elles diffèrent par `edi`. Convertissez chaque valeur de `edx` de l'hexadécimal en décimal. Le troisième appel est `exit` ; la valeur chargée dans `edi` avant lui est le code de sortie.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Votre programme effectue les trois mêmes appels que l'original, dans le même ordre. Notez-les d'abord, avec leurs arguments, avant d'écrire du code.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Placez les deux textes dans `.rodata` avec `db`. L'octet `10` est un saut de ligne. Laissez NASM calculer chaque longueur avec `$ - label`, placé juste après le texte. Utilisez `lea rsi, [label]` pour charger une adresse.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Chaque `write` met dans `eax` le numéro d'appel, dans `edi` le descripteur, dans `rsi` l'adresse du texte et dans `edx` la longueur, puis exécute `syscall`. `exit` met `eax` à 60 et `edi` au code de sortie. Si le vérificateur indique que le programme a été terminé par le signal 11, le programme a continué au-delà de sa dernière instruction parce que l'appel `exit` manque.

</details>
