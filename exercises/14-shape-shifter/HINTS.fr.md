<!-- source: HINTS.md @ sha256:050dd4914a1f -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 14

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Trouvez l'appel à `operator new` dans la fonction qui traite chaque variante de kind. Juste après chaque appel, le code range une adresse constante dans le premier mot du nouvel objet : cette adresse est un pointeur de vtable, et chaque classe a le sien.

</details>

<details>
<summary>Niveau 2 : direction</summary>

L'argument de `operator new` est la taille de l'objet. Les autres rangements qui suivent l'appel sont les champs, chacun à un décalage par rapport au nouvel objet. Le pointeur de vtable pointe 16 octets à l'intérieur d'une table de `.rodata` ; lisez trois adresses de 8 octets à partir du pointeur. Dans la boucle principale, les trois appels virtuels utilisent les décalages 0, 8 et 16 à partir du pointeur de vtable. Le compilateur évalue les arguments de l'appel d'affichage de droite à gauche ; les appels apparaissent donc dans l'ordre 16, 8, 0 alors que le programme affiche les résultats des emplacements 0, 1 et 2.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Écrivez les quatre tables côte à côte, trois adresses chacune, et marquez les égales. Deux classes partagent une entrée quand c'est la même adresse dans la même ligne. Le nombre d'adresses différentes est le nombre d'entrées différentes dans toute la table. La classe aux plus grands objets est celle dont l'argument de `operator new` est le plus grand, et son dernier champ est le rangement au plus grand décalage dans ce code de constructeur. N'oubliez pas que tout objet commence par le pointeur de vtable ; le premier champ est donc au décalage 8.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

`gadget_init` a besoin du pointeur de vtable, elle appelle donc `gadget_vptr(kind)` ; gardez ses autres arguments dans des registres à sauvegarder par l'appelé pendant cet appel. Rangez ensuite les champs aux décalages trouvés à la Partie 1.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Les constructeurs lus à la Partie 1 montrent quelles classes rangent une valeur au décalage 12 et lesquelles en rangent une au décalage 16 : trois classes ont un deuxième champ et une classe a aussi un troisième. Pour `gadget_call`, chargez le pointeur de vtable depuis le premier mot de l'objet, ajoutez 8 fois le numéro d'emplacement et appelez l'adresse qui s'y trouve. L'objet reste dans `rdi` ; déplacez d'abord `arg` dans `esi`.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Gardez la pile alignée : à l'entrée d'une fonction, `rsp` vaut 8 modulo 16 ; `gadget_call` doit donc retrancher 8 (ou empiler un registre) avant son `call`, et `gadget_init` doit empiler un nombre impair de registres. Étendez avec signe le numéro d'emplacement avant de l'utiliser comme indice. Si le vérificateur affiche `vptr`, le premier mot de l'objet est faux ; `call` signifie que votre appel ne concorde pas avec l'appel virtuel de la bibliothèque (mauvais emplacement, mauvais facteur ou mauvais argument) ; `canary` signifie que vous avez écrit trop loin ; `regs` signifie qu'un registre à sauvegarder par l'appelé a été modifié.

</details>
