<!-- source: README.md @ sha256:ffb09f7f884f -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 14. Métamorphe

Level 4. Durée : environ 4 heures. Architecture : x86-64 Linux (C++). Modules : [15. Récupérer les données](../../modules/15-recovering-data/README.fr.md)

## Scénario

Un programme C++, `gadgets`, construit des objets de plusieurs classes apparentées et appelle leurs fonctions virtuelles. Ses symboles et ses informations de type à l'exécution ont été supprimés, et le source est perdu. Un greffon écrit en assembleur doit créer lui-même des objets de ces classes et appeler leurs fonctions virtuelles ; il doit donc reproduire leur disposition exacte et utiliser correctement leurs vtables. Vous retrouvez d'abord la hiérarchie de classes. Vous écrivez ensuite le greffon.

## Objectifs

- Retrouver les classes dans un binaire C++ dépouillé (stripped) à partir des constructeurs qui rangent un pointeur de vtable.
- Retrouver la disposition des objets, le contenu des vtables et les relations d'héritage.
- Construire des objets à la main et appeler des fonctions virtuelles par une vtable depuis l'assembleur.

## Partie 1 : Reverse

Le binaire est `bin/gadgets`. Il lit des lignes `kind a b c m`, crée un objet de la classe choisie par `kind` et affiche trois nombres. Une fonction avec un `switch` sur `kind` crée les objets ; elle appelle `operator new` pour chaque classe et range le pointeur de vtable et les champs.

```bash
printf '2 7 3 800 10\n' | bin/gadgets
objdump -d -M intel bin/gadgets
objdump -s -j .rodata bin/gadgets
```

Aucune des classes n'a de destructeur virtuel et le programme a été construit sans informations de type à l'exécution ; chaque vtable commence donc par deux mots nuls puis ne contient plus que les adresses des fonctions virtuelles. Le pointeur de vtable rangé dans un objet pointe juste après ces deux mots.

Trouvez ces six valeurs :

1. `N` : le nombre de classes, c'est-à-dire le nombre de vtables différentes, en décimal.
2. `S` : le nombre de fonctions virtuelles de chaque vtable, en décimal.
3. `Z` : la taille en octets des objets de la classe dont les objets sont les plus grands, en décimal.
4. `P` : le décalage en octets du dernier champ des objets de cette classe la plus grande, en décimal.
5. `E` : le nombre de paires de classes qui contiennent la même adresse de fonction dans le même emplacement, en comptant chaque paire et chaque emplacement une fois, en décimal.
6. `F` : le nombre d'adresses de fonction différentes dans toutes les vtables ensemble, en décimal.

```text
ARL{shape_<N>_<S>_<Z>_<P>_<E>_<F>}
```

Par exemple, les valeurs 3, 2, 32, 24, 1 et 5 donneraient `ARL{shape_3_2_32_24_1_5}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour implémenter deux fonctions dans la convention d'appel System V :

```c
void gadget_init(void *mem, int kind, int a, int b, int c);
int  gadget_call(const void *obj, int slot, int arg);

/* Provided by the tests. */
const void *gadget_vptr(int kind);
```

- `gadget_init` construit un objet de la classe `kind` dans le tampon `mem`. `kind` est la valeur sur laquelle la fonction créatrice d'objets de `bin/gadgets` fait son switch. Le premier mot de l'objet est `gadget_vptr(kind)`. Les valeurs `a`, `b` et `c` remplissent les champs de données de la classe dans l'ordre où ils apparaissent dans l'objet, en commençant par le plus petit décalage ; les valeurs pour lesquelles la classe n'a pas de champ sont ignorées.
- `gadget_call` appelle une fonction virtuelle d'un objet par sa propre vtable : l'emplacement 0 est la première fonction de la vtable, l'emplacement 1 la suivante et l'emplacement 2 la dernière. Elle passe l'objet comme premier argument et `arg` comme second, et renvoie le résultat.

Les tests construisent des objets avec votre `gadget_init`, puis comparent ce que renvoient les appels virtuels propres à la bibliothèque avec les résultats de `bin/gadgets`.

## Règles

- Utilisez la vtable : `gadget_call` doit charger le pointeur de vtable depuis l'objet et appeler par son intermédiaire.
- N'écrivez que les octets qui appartiennent à l'objet ; les tests vérifient que rien ne change après les 32 premiers octets du tampon.
- Préservez les registres à sauvegarder par l'appelé `rbx`, `rbp` et `r12` à `r15`.
- N'appelez rien d'autre que `gadget_vptr`.

## Vérifier votre travail

```bash
make check EX=14 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte séparément de la Partie 1 et de la Partie 2. Sans `--flag`, il ne vérifie que la Partie 2. Chaque test de la Partie 2 affiche trois résultats suivis de `ok` ou d'une liste de ce qui s'est mal passé : `vptr`, `call`, `canary` ou `regs`.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
