<!-- source: README.md @ sha256:39876dd642a0 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 17. Archive des inconnues

Level 4. Durée : environ 4 heures. Architecture : x86-64 Linux. Modules : [17. Déchiffrer les formats de fichier](../../modules/17-deciphering-file-formats/README.fr.md)

## Scénario

Un outil d'empaquetage, `arkpack`, réunit plusieurs fichiers dans une seule archive. Personne n'a conservé la description du format de l'archive, et l'outil de dépaquetage est perdu. Un appareil embarqué doit maintenant lire ces archives et n'a de place que pour une petite routine écrite en assembleur. Vous retrouvez d'abord le format en étudiant le programme qui l'écrit. Vous écrivez ensuite le lecteur : lister les noms, extraire les contenus et refuser les archives endommagées.

## Objectifs

- Retrouver la structure d'un format de fichier (en-tête, table, enregistrements, noms, somme de contrôle) en lisant le programme qui l'écrit et en le testant sur des fichiers de votre cru.
- Reconnaître le codage appliqué aux données et la somme de contrôle, et reproduire les deux.
- Écrire en assembleur un lecteur qui ne se fie jamais aux décalages et aux tailles stockés dans le fichier.

## Partie 1 : Reverse

Le binaire est `bin/arkpack`. Lancez-le sous la forme `arkpack OUTPUT FILE...` et examinez la sortie avec un vidage hexadécimal.

```bash
printf 'AAA' > a.txt
printf 'hello world\n' > hello.txt
bin/arkpack t.ark a.txt hello.txt
xxd t.ark
objdump -d -M intel bin/arkpack
```

Trouvez ces cinq valeurs :

1. `E` : la taille en octets d'une entrée de la table qui décrit les fichiers stockés, en décimal.
2. `A` : l'alignement en octets du début des données de chaque fichier stocké (chaque décalage de données est un multiple de `A`), en décimal.
3. `R` : le nombre de bits dont la somme de contrôle est tournée vers la gauche pour chaque octet de données, en décimal.
4. `K` : la valeur de l'octet de clé que l'outil range dans l'en-tête d'une archive de 3 fichiers, en décimal.
5. `N` : le plus grand nombre de fichiers que l'outil accepte, en décimal.

```text
ARL{ark_<E>_<A>_<R>_<K>_<N>}
```

Par exemple, les valeurs 32, 8, 3, 100 et 20 donneraient `ARL{ark_32_8_3_100_20}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour implémenter trois fonctions dans la convention d'appel System V. L'archive est constituée des `n` octets situés à `a`.

```c-fragment
long ark_count(const unsigned char *a, size_t n);
long ark_name(const unsigned char *a, size_t n, long index, char *out, size_t cap);
long ark_extract(const unsigned char *a, size_t n, long index, unsigned char *out, size_t cap);
```

- `ark_count` renvoie le nombre de fichiers de l'archive, ou -1 si l'archive est invalide. Elle est invalide si elle est plus courte que l'en-tête, a une mauvaise valeur magique, ou si son en-tête décrit une table qui ne se trouve pas entièrement dans l'archive (elle doit commencer après l'en-tête et finir à la fin de l'archive ou avant) ou une zone de noms qui commence au-delà de la fin de l'archive.
- `ark_name` copie dans `out` le nom du fichier numéro `index` (compté à partir de 0), avec son octet nul final, et renvoie la longueur du nom sans cet octet. Elle renvoie -1 si l'archive est invalide ou si `index` n'est pas un numéro de fichier valide, -2 si le nom ne commence pas dans l'archive ou n'a pas d'octet nul final avant la fin de l'archive, et -3 si `cap` est inférieur à la longueur du nom plus un.
- `ark_extract` écrit dans `out` le contenu du fichier numéro `index` et renvoie sa taille en octets. Elle renvoie -1 si l'archive est invalide ou si `index` n'est pas un numéro de fichier valide, -2 si les données stockées ne se trouvent pas dans l'archive (elles doivent commencer après l'en-tête et finir à la fin de l'archive ou avant), -3 si `cap` est inférieur à la taille, et -4 si la somme de contrôle des octets décodés diffère de celle qui est stockée. Les vérifications se font dans cet ordre. En cas d'erreur, le contenu de `out` n'est pas examiné, mais rien ne doit être écrit au-delà de `cap` octets.

Tous les décalages et toutes les tailles du fichier sont des nombres non signés de 32 bits, et leurs sommes peuvent dépasser 32 bits. `index` est un nombre signé de 64 bits.

Les tests construisent des archives avec `arkpack` et les endommagent aussi de nombreuses façons. Pour chaque commande, ils comparent la valeur de retour et les octets écrits à ceux d'une référence.

## Règles

- Ne lisez jamais un octet en dehors des `n` octets de l'archive, quoi que dise l'en-tête. Les tests placent l'archive juste avant une page non projetée, et une lecture au-delà de sa fin interrompt l'exécution des tests par un signal.
- N'écrivez jamais au-delà de `cap` octets de `out`.
- Préservez les registres à sauvegarder par l'appelé `rbx`, `rbp` et `r12` à `r15`.
- Gardez la pile alignée à chaque `call`.

## Vérifier votre travail

```bash
make check EX=17 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte séparément de la Partie 1 et de la Partie 2. Sans `--flag`, il ne vérifie que la Partie 2. Chaque résultat d'un test de la Partie 2 est suivi de `ok` ou d'une liste de ce qui s'est mal passé : `overrun` ou `regs`.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
