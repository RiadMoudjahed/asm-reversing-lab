<!-- source: README.md @ sha256:091712ebb014 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 19. Bureau de triage

Niveau 5. Durée : environ 5 heures. Architecture : x86-64 Linux et Windows. Modules : [19. Analyser des programmes suspects](../../modules/19-analyzing-suspicious-programs/README.fr.md), [20. Lire du code obscurci](../../modules/20-reading-obfuscated-code/README.fr.md)

## Scénario

L'équipe de sécurité d'une petite entreprise a reçu, à la suite d'une alerte sur un poste de travail, un programme nommé `dispatch`, avec une version Windows du même programme, `dispatch.exe`. Personne ne sait ce qu'il fait. Il doit être évalué avant la fin de la journée : que fait-il, comment l'équipe peut-elle retrouver d'autres copies, et comment peut-elle lire sa configuration sans rien exécuter de risqué ? Le programme de cet exercice est un simulateur écrit pour le cours. Il affiche seulement ce qu'il ferait et n'effectue aucune action. Vous rédigez les notes de triage et la règle de détection, puis vous écrivez la routine qui décode sa configuration.

## Objectifs

- Évaluer un programme inconnu dans un ordre fixe : identification, chaînes, imports, comportement et décodage des données cachées.
- Trouver une configuration encodée, retrouver la règle de décodage à partir du code et décoder chaque entrée.
- Associer le comportement observé à des identifiants de techniques ATT&CK et indiquer ce que les preuves permettent d'affirmer.
- Écrire une règle YARA qui trouve l'échantillon et ne se déclenche pas sur des programmes qui lui ressemblent seulement.
- Réimplémenter le décodeur en assembleur afin de pouvoir l'appliquer aux configurations d'autres échantillons.

## Partie 1 : Reverse

Les fichiers sont `bin/dispatch` (Linux) et `bin/dispatch.exe` (Windows). Ils affichent quelques lignes de journal et se terminent. Les exécuter est sans danger, et c'est par là qu'il faut commencer, mais les lignes de journal ne contiennent que des marqueurs à la place des données intéressantes : les données elles-mêmes sont stockées encodées et ne sont décodées qu'en mémoire.

```bash
bin/dispatch
file bin/dispatch bin/dispatch.exe
strings -n 6 bin/dispatch
```

Suivez le triage du module 19 et les méthodes de décodage du module 20. Trouvez la table qui contient les entrées encodées, retrouvez la règle de décodage à partir du code et décodez-les toutes. Préparez ensuite ces valeurs et ces fichiers :

1. `N` : le nombre d'entrées de la configuration encodée, en décimal.
2. `L` : la longueur en octets de l'entrée décodée la plus longue, en décimal.
3. Les identifiants ATT&CK de trois comportements, chacun écrit comme l'identifiant complet de la technique ou de la sous-technique (un `T`, quatre chiffres, un point et trois chiffres), mais sans le `T` ni le point dans le flag : la création d'un service, le dépôt d'une bibliothèque à côté d'un programme de confiance pour que le programme la charge, et l'utilisation de noms DNS comme support de données.
4. Une règle YARA dans `starter/rule.yar`. Elle doit correspondre à `bin/dispatch` et à `bin/dispatch.exe`, et ne pas correspondre aux autres programmes que le vérificateur essaie : trois programmes qui évoquent les mêmes sujets de façon inoffensive (un texte d'aide, une exécution à blanc, un programme de mise à jour qui utilise la même routine de décodage pour des textes inoffensifs) dans les deux formats, et un programme système. Une règle qui se déclencherait sur l'un d'eux noierait les analystes sous les fausses alertes.

Le flag combine les trois premières valeurs dans cet ordre : `N`, `L`, puis les trois identifiants (service, chargement latéral, DNS) sans le `T` ni le point.

```text
ARL{triage_<N>_<L>_<service>_<sideload>_<dns>}
```

Les valeurs 3 et 20 et les identifiants T1234.567, T1234.568 et T1234.569 donneraient par exemple `ARL{triage_3_20_1234567_1234568_1234569}`.

Les lignes de journal décrivent aussi deux autres comportements que vous devez associer dans vos notes, bien que le flag ne les demande pas : comment le programme s'informe sur le système qui l'entoure et ce qu'il fait des traces qu'il laisse. Rédiger les notes fait partie de l'exercice. Le vérificateur ne les lit pas ; le corrigé détaillé de `solutions/19/` en contient un modèle pour vous comparer ensuite.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour implémenter une fonction selon la convention d'appel System V :

```c-fragment
long decode_record(const unsigned char *rec, size_t avail, char *out, size_t cap);
```

`rec` pointe vers une entrée encodée dans la structure que vous avez trouvée dans la partie 1, et `avail` est le nombre d'octets que vous pouvez lire à partir de `rec`. La fonction écrit le texte décodé dans `out`, suivi d'un octet nul, et renvoie la longueur du texte. Les règles, dans l'ordre où elles s'appliquent :

- Si `avail` est inférieur à 2, ou inférieur à 2 plus la longueur stockée dans le premier octet de l'entrée, l'entrée n'est pas entièrement contenue dans les octets lisibles : renvoyez -1 et n'écrivez rien.
- Si `cap` est inférieur à la longueur du texte plus un, la sortie ne tient pas : renvoyez -2 et n'écrivez rien.
- Sinon, décodez l'entrée exactement comme le fait le simulateur, écrivez le texte et l'octet nul final, et renvoyez la longueur du texte. Une entrée de longueur 0 donne un texte vide, l'octet nul et la valeur de retour 0.

Les tests comprennent les entrées de l'échantillon, les mêmes entrées coupées à chaque position, des entrées aux longueurs et aux graines limites, et des entrées aléatoires. Ils comparent la valeur de retour et la sortie à celles d'une référence.

## Règles

- Ne lisez jamais un octet situé à `rec + avail` ou au-delà. Les tests placent l'entrée juste avant une page non mappée, et une lecture au-delà de la fin termine l'exécution des tests par un signal.
- N'écrivez jamais au-delà des `cap` octets de `out`, et n'écrivez rien lorsque vous renvoyez une erreur.
- Préservez les registres que l'appelé doit conserver : `rbx`, `rbp` et `r12` à `r15`.
- Gardez la pile alignée à chaque `call`.

## Vérifier votre travail

```bash
make check EX=19 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte séparément de la partie 1 et de la partie 2. La partie 1 n'est réussie que si le flag est correct et si la règle YARA de `starter/rule.yar` est correcte ; la sortie du vérificateur nomme chaque programme sur lequel la règle donne la mauvaise réponse. Sans `--flag`, il ne vérifie que la partie 2.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
