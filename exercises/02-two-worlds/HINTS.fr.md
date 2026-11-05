<!-- source: HINTS.md @ sha256:e1b217e81ed4 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 02

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

`strace` affiche le nom de chaque appel, mais seul le désassemblage affiche le numéro. Ouvrez les deux listings côte à côte et regardez ce que charge `eax`.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Chaque entrée dans le noyau (`int 0x80` ou `syscall`) est précédée d'un `mov eax, <number>`. Les nombres sont en hexadécimal : convertissez-les en décimal avant de les chercher dans l'aide-mémoire. L'appel qui n'est ni `write` ni `exit` est `sync`.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Chaque binaire a trois entrées dans le noyau. Lisez la valeur de `eax` avant la deuxième pour le numéro de `sync`, et la valeur de `eax` avant la troisième pour le numéro d'`exit`. Faites-le pour le listing 32 bits puis pour le listing 64 bits. Les deux numéros de `sync` diffèrent, de même que les deux numéros d'`exit`.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Tout ce qui, dans le programme de départ, est propre au mode 32 bits doit changer : la directive de mode, l'instruction qui entre dans le noyau, les numéros d'appel et les registres qui portent les arguments.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Servez-vous de ce que vous avez trouvé en partie 1 : les numéros d'appel 64 bits et les registres d'arguments 64 bits de l'aide-mémoire. Pensez à `default rel` et à `lea` pour charger l'adresse du texte.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Pour `write` en code 64 bits, `rdi` contient le descripteur, `rsi` l'adresse et `rdx` la longueur. `sync` ne prend aucun argument. Pour `exit`, `rdi` contient le code de sortie. Si le vérificateur signale une instruction interdite, une copie de `int 0x80` reste dans le fichier. S'il liste des appels système différents de ceux attendus, un numéro d'appel est faux.

</details>
