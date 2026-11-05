<!-- source: README.md @ sha256:e0ff7aa3c115 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 18. Rapport de chasse aux bogues

Level 4. Durée : environ 4 heures. Architecture : x86-64 Linux. Modules : [18. Auditer des binaires à la recherche de bogues](../../modules/18-auditing-binaries/README.fr.md)

## Scénario

Un petit programme, `recparse`, lit un fichier d'enregistrements provenant d'un appareil de surveillance et affiche un résumé. Il doit être examiné avant d'être utilisé sur des fichiers venus de l'extérieur. Le source n'est pas disponible. Vous auditez le binaire, trouvez ses défauts de sûreté mémoire et rédigez un rapport pour ses mainteneurs. Vous écrivez ensuite en assembleur la routine d'analyse corrigée, afin que les mainteneurs puissent remplacer la routine fautive.

## Objectifs

- Auditer un binaire dépouillé de ses symboles : trouver les puits, remonter les tailles et les indices jusqu'à l'entrée et lire chaque vérification.
- Identifier des défauts de plusieurs classes et énoncer précisément la cause de chacun.
- Rédiger un rapport exploitable par le propriétaire du code, sans rien exploiter.
- Écrire en assembleur un analyseur qui refuse les entrées mal formées sans jamais toucher à de la mémoire qu'il ne doit pas toucher.

## Partie 1 : Reverse

Le binaire est `bin/recparse`. Il lit le fichier nommé sur sa ligne de commande et affiche un résumé des enregistrements qu'il contient.

```bash
bin/recparse sample.rec
objdump -d -M intel bin/recparse
```

Le programme est volontairement défectueux. Ne le lancez que sur des fichiers que vous avez faits vous-même, et utilisez le débogueur pour l'arrêter avant une opération suspecte au lieu de le laisser dépasser la fin d'un tampon. Auditez le code comme le décrit le module 18. Le programme a six défauts de sûreté mémoire : un dans la façon de passer d'un enregistrement au suivant, et un dans chacun des cinq types d'enregistrement. Trouvez-les.

Notez ces cinq valeurs :

1. `N` : la plus grande longueur de nom qu'accepte le type d'enregistrement des noms, en décimal.
2. `V` : le plus petit octet de longueur, en décimal, pour lequel la vérification du type d'enregistrement des listes de nombres laisse passer une taille impossible.
3. `T` : la plus grande valeur d'étiquette qu'accepte le type d'enregistrement des étiquettes, en décimal.
4. `B` : le plus petit nombre d'éléments, en décimal, pour lequel la taille calculée dans le type d'enregistrement des blocs de données ne correspond plus au nombre d'octets copiés ensuite.
5. `U` : le numéro de type, en décimal, du type d'enregistrement qui libère de la mémoire qui est réutilisée après.

```text
ARL{bughunt_<N>_<V>_<T>_<B>_<U>}
```

Par exemple, les valeurs 8, 200, 4, 32 et 3 donneraient `ARL{bughunt_8_200_4_32_3}`.

Rédigez ensuite un rapport. Créez un fichier `REPORT.md` et donnez à chacun des six défauts une entrée au format du module 18 : titre, classe, emplacement, gravité, cause, preuve, impact, correctif et test. Le vérificateur ne lit pas le rapport. Le corrigé dans `solutions/18/` contient un rapport modèle avec lequel vous pourrez comparer ensuite.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour implémenter une fonction dans la convention d'appel System V :

```c-fragment
struct out {
    char     name[16];
    int64_t  sum;
    uint32_t tags[8];
    uint32_t blob_sum;
    uint32_t pad;
};

long parse_records(const unsigned char *buf, size_t n, struct out *o);
```

Le tampon est constitué des `n` octets situés à `buf`. La structure `o` est mise à zéro avant l'appel. Un enregistrement est un octet de type, un octet de longueur et autant d'octets de charge utile. La fonction traite les enregistrements l'un après l'autre jusqu'à trouver l'enregistrement de type 0, qui termine la liste. Elle renvoie le nombre d'enregistrements avant celui-là, ou un code d'erreur négatif. Les règles, dans l'ordre où elles s'appliquent à chaque enregistrement :

- S'il reste moins de 2 octets, ou moins de `length` octets après les deux octets d'en-tête, l'enregistrement est tronqué : renvoyez -1. Cela vaut aussi pour l'enregistrement de type 0, et un tampon qui se termine avant un enregistrement de type 0 est tronqué.
- Le type 0 termine la liste. Les octets qui suivent sont ignorés, et sa charge utile, s'il y en a une, est sautée.
- Le type 1 fixe le nom. Il exige une longueur d'au plus 15, sinon renvoyez -2. Le champ du nom devient exactement les octets de la charge utile suivis d'octets nuls jusqu'à 16 octets au total.
- Le type 2 ajoute des nombres à `sum`. La longueur doit être un multiple de 4 et d'au plus 32, sinon renvoyez -3. Chaque groupe de 4 octets de charge utile est un nombre signé de 32 bits, octet de poids faible d'abord, et est ajouté à `sum`.
- Le type 3 compte des étiquettes. Chaque octet de charge utile doit valoir au plus 7, sinon renvoyez -4. Pour chaque octet, `tags[byte]` est incrémenté.
- Le type 4 fixe un bloc de données. Il exige une longueur d'au moins 1 égale à 1 plus 4 fois la valeur du premier octet de la charge utile, sinon renvoyez -5. `blob_sum` devient la somme des octets de la charge utile après le premier, de sorte qu'un bloc ultérieur remplace un bloc antérieur.
- Le type 5 abandonne le bloc de données. Il exige une longueur de 0, sinon renvoyez -5. `blob_sum` devient 0.
- Tout autre type : renvoyez -6.

Quand la valeur de retour est négative, le contenu de `o` n'est pas examiné, mais rien ne doit être écrit en dehors de `o` et le champ `pad` doit rester nul.

Les tests construisent de nombreux tampons, des valides aux tronqués, endommagés et aléatoires, et comparent la valeur de retour et chaque champ avec ceux d'une référence.

## Règles

- Ne lisez jamais un octet en dehors des `n` octets du tampon, quoi que disent les enregistrements. Les tests placent le tampon juste avant une page non projetée, et une lecture au-delà de sa fin interrompt l'exécution des tests par un signal.
- N'écrivez jamais en dehors des 64 octets de `o`, et laissez `pad` nul.
- Préservez les registres à sauvegarder par l'appelé `rbx`, `rbp` et `r12` à `r15`.
- Gardez la pile alignée à chaque `call`.

## Vérifier votre travail

```bash
make check EX=18 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte séparément de la Partie 1 et de la Partie 2. Sans `--flag`, il ne vérifie que la Partie 2. Chaque résultat d'un test de la Partie 2 se termine par `ok` ou par une liste de ce qui s'est mal passé : `overrun` ou `regs`.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
