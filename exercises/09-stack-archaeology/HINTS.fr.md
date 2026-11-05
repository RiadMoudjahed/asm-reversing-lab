<!-- source: HINTS.md @ sha256:450b94c69311 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 09

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Lisez les premières et les dernières instructions de `chain`. Le prologue empile des registres et déplace `rsp` ; l'épilogue défait les deux dans l'ordre inverse. Trouvez l'instruction qui prépare le deuxième argument de `note`, car elle montre où commence la table.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Comptez les instructions `push` avant le `sub rsp`. Chacune fait 8 octets. Un niveau de récursion est l'adresse de retour (8 octets) plus les push plus la quantité retranchée. La table commence à l'adresse passée à `note` ; l'adresse de retour sauvegardée se trouve au-dessus des push. Les écritures dans `[rsp]`, `[rsp + 8]` et `[rsp + 16]` montrent quelle partie de l'espace retranché est utilisée.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Deux registres sont empilés, donc les registres sauvegardés occupent 16 octets. La valeur retranchée est en hexadécimal dans le listing ; convertissez-la. La table utilise trois entrées de 8 octets à partir de `rsp`. L'adresse de retour se trouve au-dessus de l'espace retranché et des registres empilés : additionnez la quantité retranchée et les octets empilés pour obtenir sa distance à `rsp`. L'espace inutilisé est la quantité retranchée moins 24. Pour vérifier vos nombres, exécutez le programme sous `gdb`, arrêtez-vous deux fois sur `note` et soustrayez les deux valeurs de `rsp`.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Écrivez d'abord sur papier les formules de `chain`. Listez ensuite les valeurs dont vous avez besoin au retour de `note` et au retour de l'appel récursif, et décidez où vit chacune.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Après `note`, `rdi` et `rsi` sont perdus ; gardez donc `depth` et `seed` dans des registres à sauvegarder par l'appelé, que vous devez empiler avec `push` et restaurer avec `pop`. La table peut vivre sur la pile et y rester pendant les deux appels. À l'entrée, `rsp` vaut 8 modulo 16 ; chaque `push` change cela, et `sub rsp` doit le ramener à un multiple de 16 avant chaque `call`.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Empilez deux registres à sauvegarder par l'appelé, puis retranchez 24 octets pour la table. L'entrée est à 8 modulo 16, les push la gardent à 8 modulo 16, et 24 octets de plus en font un multiple de 16. Ce niveau utilise 8 + 16 + 24 octets. Si le vérificateur signale `align`, le total des push et de la soustraction est faux. S'il signale `regs`, un registre à sauvegarder par l'appelé est modifié sans être restauré, ou restauré dans le mauvais ordre. S'il signale `stack`, le cadre est plus grand que `L`.

</details>
