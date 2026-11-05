<!-- source: HINTS.md @ sha256:7cf9300fb009 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 08

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Regardez la dernière instruction de chaque fonction et l'endroit où elle lit ses arguments. Dans le binaire 32 bits, regardez aussi ce que fait l'appelant juste après chaque `call`.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Un `ret` suivi d'un nombre retire ce nombre d'octets d'arguments, donc la fonction n'est pas cdecl. Sans nombre, l'appelant doit ajuster `esp` après l'appel, ou la fonction ne prend aucun argument sur la pile. Une fonction qui lit `ecx` ou `edx` avant de les écrire prend des arguments en registres. Dans le binaire 64 bits, comparez les registres lus par `g1` et `g2`, et regardez le `sub rsp` de l'appelant avant `g1`.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Deux des fonctions 32 bits se terminent par `ret` avec un nombre : celle qui retire douze octets prend trois arguments sur la pile (stdcall), celle qui en retire quatre prend un seul argument sur la pile parce que deux autres arrivent en registres (fastcall). Les deux autres se terminent par un `ret` simple et leurs appelants nettoient (cdecl). Dans le binaire 64 bits, la fonction qui range `rcx` et `rdx` dans la zone au-dessus de son adresse de retour utilise l'espace d'ombre (Microsoft x64) ; l'autre lit `rdi`, `rsi` et `rdx` (System V).

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Écrivez d'abord sur papier la formule de chaque fonction. Décidez ensuite, pour chaque convention, où se trouvent les trois arguments à l'entrée.

</details>

<details>
<summary>Niveau 2 : direction</summary>

À l'entrée, `[esp]` contient l'adresse de retour. Pour les deux conventions de pile, les arguments commencent en `[esp + 4]`. Pour fastcall, `a` est dans `ecx` et `b` dans `edx`, et seul `c` est en `[esp + 4]`. Utilisez `ret N` là où l'appelé doit retirer des octets.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

La fonction stdcall retire 12 octets (trois arguments) et la fonction fastcall retire 4 octets (un argument). La fonction cdecl se termine par un `ret` simple. Si le vérificateur affiche `stack` au lieu de `ok`, le nombre d'octets d'un `ret` est faux, ou un `ret` simple est utilisé là où un nombre est nécessaire. Si le programme plante, la fonction a retiré trop d'octets et l'appelant est revenu à une mauvaise adresse.

</details>
