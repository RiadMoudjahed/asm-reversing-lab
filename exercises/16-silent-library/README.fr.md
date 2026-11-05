<!-- source: README.md @ sha256:b193d6bb0921 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 16. Bibliothèque silencieuse

Level 4. Durée : environ 4 heures. Architecture : x86-64 Linux. Modules : [16. Interfaces non documentées](../../modules/16-undocumented-interfaces/README.fr.md)

## Scénario

Une bibliothèque partagée, `libsilent.so`, est arrivée avec un programme aujourd'hui retiré. Il n'y a ni en-tête, ni manuel, ni source. Une autre équipe a besoin de deux choses. D'abord, un nouvel outil écrit en assembleur doit appeler la bibliothèque pour calculer son condensé (digest) d'un tampon, ce qui exige d'appeler les bonnes fonctions, dans le bon ordre, avec les bons types. Ensuite, la fonction d'empaquetage de la bibliothèque doit être remplacée par votre propre code, avec exactement les mêmes résultats, afin de pouvoir se passer de la bibliothèque dans un déploiement. Vous retrouvez d'abord l'interface. Vous écrivez ensuite les deux morceaux.

## Objectifs

- Lister les exportations d'une bibliothèque et retrouver la signature et le comportement de chaque fonction à partir de son code.
- Trouver la disposition de l'état qu'une fonction crée et que les autres utilisent.
- Appeler des fonctions de la bibliothèque depuis l'assembleur avec les bons types et dans le bon ordre, et libérer ce que l'on acquiert.
- Réimplémenter une fonction de façon qu'elle corresponde à l'originale pour toute entrée, y compris les cas d'échec.

## Partie 1 : Reverse

La bibliothèque est `bin/libsilent.so`. Elle exporte des fonctions qui créent et libèrent un état, y injectent des octets, en lisent un condensé, comptent les états ouverts et empaquettent des entiers en octets.

```bash
nm -D --defined-only bin/libsilent.so
objdump -d -M intel bin/libsilent.so
```

Écrivez un petit programme C avec les prototypes retrouvés et liez-le à la bibliothèque pour tester vos hypothèses, comme dans la leçon.

Trouvez ces cinq valeurs :

1. `S` : la taille en octets du bloc d'état qu'alloue la fonction de création, en décimal.
2. `C` : le décalage en octets du champ de 64 bits de l'état qui compte combien d'octets ont été injectés, en décimal.
3. `F` : le nombre de bits dont la fonction d'injection fait tourner vers la gauche le second champ de 32 bits de l'état à chaque octet, en décimal.
4. `G` : le nombre de bits dont la fonction qui lit le condensé fait tourner vers la gauche le second champ de 32 bits, en décimal.
5. `B` : le nombre d'octets que la fonction d'empaquetage écrit pour les quatre entiers 0, -1, 300 et -70000 quand la zone de sortie est assez grande, en décimal.

```text
ARL{silent_<S>_<C>_<F>_<G>_<B>}
```

Par exemple, les valeurs 24, 16, 3, 9 et 11 donneraient `ARL{silent_24_16_3_9_11}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour implémenter deux fonctions dans la convention d'appel System V :

```c-fragment
unsigned digest_buffer(const unsigned char *p, size_t n, unsigned seed);
int      pack_values(const int *vals, size_t n, unsigned char *out, size_t cap);
```

- `digest_buffer` renvoie le condensé des `n` octets situés à `p` pour la graine `seed` donnée. Elle doit obtenir le résultat en appelant la bibliothèque : créer un état à partir de la graine, injecter les octets, lire le condensé et libérer l'état.
- `pack_values` fait ce que fait la fonction d'empaquetage de la bibliothèque, sans l'appeler : pour les mêmes `vals`, `n`, `out` et `cap`, elle renvoie la même valeur et laisse les mêmes octets dans `out`, y compris lorsque les données ne tiennent pas.

Les tests appellent les deux fonctions avec de nombreuses entrées et comparent les résultats avec la bibliothèque. Pour `pack_values`, ils comparent la valeur de retour et chaque octet d'une zone de sortie longue de `cap` + 4 octets, remplie avec la valeur 0xcc avant l'appel.

## Règles

- Liez la bibliothèque avec les signatures retrouvées ; les symboles que vous pouvez appeler sont les fonctions de création, d'injection, de lecture et de libération.
- N'appelez pas la fonction d'empaquetage, et n'appelez aucune autre fonction dans `pack_values`.
- Libérez chaque état exactement une fois. Les tests demandent à la bibliothèque combien d'états sont ouverts après chaque appel.
- Préservez les registres à sauvegarder par l'appelé `rbx`, `rbp` et `r12` à `r15`.
- Gardez la pile alignée à chaque `call`.

## Vérifier votre travail

```bash
make check EX=16 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte séparément de la Partie 1 et de la Partie 2. Sans `--flag`, il ne vérifie que la Partie 2. Chaque résultat d'un test de la Partie 2 est suivi de `ok` ou d'une liste de ce qui s'est mal passé : `handle` ou `regs`. Le vérificateur signale aussi une violation des règles si votre fichier objet n'a pas de référence à l'une des quatre fonctions obligatoires de la bibliothèque ou s'il fait référence à la fonction d'empaquetage.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
