<!-- source: HINTS.md @ sha256:7d4b755cfc60 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 12

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Désassemblez `dispatch`. Ses premières instructions sont la vérification de bornes et l'ajustement de l'indice, puis il y a un `jmp` indirect qui lit dans une table de `.rodata`.

</details>

<details>
<summary>Niveau 2 : direction</summary>

L'instruction qui précède la vérification de bornes retranche une constante de la commande : cette constante est la plus petite commande. La constante de la comparaison est l'indice le plus grand ; le nombre d'entrées est un de plus. Affichez la table avec `objdump -s -j .rodata` : chaque entrée est une adresse de 8 octets en petit-boutiste, lisez donc les octets à l'envers. La première entrée correspond à la plus petite commande, la suivante à la commande suivante, et ainsi de suite.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Écrivez les entrées dans une liste avec le numéro de commande à côté, puis cherchez les adresses répétées. Le code par défaut est la cible du `ja` qui suit la vérification de bornes ; les entrées qui lui sont égales sont des commandes sans cas. Certaines entrées pointent au milieu du code d'une autre entrée : c'est un cas qui se poursuit dans le suivant. Ce sont des adresses différentes, et toutes comptent dans `B`. Pour vérifier, exécutez `bin/switchboard` avec une commande de chaque groupe.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Écrivez d'abord, pour chaque commande de 3 à 13, ce que `dispatch` renvoie en fonction de `acc`. Utilisez le programme ou lisez les blocs de code. Regroupez les commandes qui partagent du code.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Calculez `edi - L` dans un autre registre, comparez-le à l'indice le plus grand avec `ja` vers le cas par défaut, chargez l'adresse de la table avec `lea` et faites `jmp [reg + index*8]`. Placez la table dans `.rodata` sous forme d'entrées `dq label`. Écrivez chaque bloc de façon qu'il calcule son résultat dans `eax` et retourne ; pour un cas qui se poursuit dans le suivant, placez son code juste avant le bloc suivant.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Les étiquettes locales de NASM appartiennent à la dernière étiquette non locale ; référez-vous-y donc dans la table par leur nom complet, comme `run_cmd.c3`. Le cas par défaut renvoie 0 parce que `dispatch` met `acc` à 0xFFFFFFFF et ajoute un. Une commande renvoie `acc - 2` sans ajouter un ; comparez la dernière étape de chaque bloc à la sortie du programme. Si le vérificateur dit qu'une commande supérieure à 13 donne un mauvais résultat, votre borne est trop petite d'une unité ; si le programme plante, votre vérification de bornes est une comparaison signée.

</details>
