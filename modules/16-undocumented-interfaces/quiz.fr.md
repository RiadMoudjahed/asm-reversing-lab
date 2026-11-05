<!-- source: quiz.md @ sha256:d39f86564105 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 16. Interfaces non documentées

Répondez d'abord, puis ouvrez la réponse.

**1. Une fonction lit `edi`, `rsi` et `edx` avant de les écrire, et se termine par `mov eax, ...`. Que pouvez-vous dire au plus de sa signature ?**

<details>
<summary>Réponse</summary>

Trois paramètres : une valeur de 32 bits, une valeur de 64 bits (un pointeur ou un `size_t`) et une valeur de 32 bits, dans cet ordre, et un résultat de 32 bits. Que les valeurs de 32 bits soient signées dépend de la façon dont elles sont comparées ou étendues, et que `rsi` soit un pointeur dépend de son déréférencement.

</details>

**2. Le code fixe `rax` par `mov rax, 0xffffffffffffffff` sur ses chemins d'échec. Que vous apprend-il ?**

<details>
<summary>Réponse</summary>

Le résultat est une valeur signée de 64 bits, comme `long`, `ssize_t` ou un entier de la taille d'un pointeur, et -1 est un code d'erreur. Un `int` de 32 bits utiliserait `mov eax, -1` ou `or eax, 0xffffffff`.

</details>

**3. Comment reconnaissez-vous un paramètre de sortie ?**

<details>
<summary>Réponse</summary>

Un argument pointeur à travers lequel la fonction ne fait qu'écrire : des écritures dans `[reg]` ou `[reg+N]` et aucun chargement depuis lui avant les écritures. L'appelant passe l'adresse d'une variable et y lit ensuite la valeur.

</details>

**4. Une fonction de la bibliothèque renvoie un pointeur obtenu par `calloc`. L'appelant ne lit ni n'écrit jamais à travers lui et le passe seulement à d'autres fonctions de la bibliothèque, dont l'une se termine par `free`. Qu'est-ce que ce pointeur, et quelles règles d'emploi en découlent ?**

<details>
<summary>Réponse</summary>

Un descripteur opaque. Chaque descripteur que renvoie une fonction d'ouverture doit être fermé exactement une fois, ne doit pas servir après sa fermeture, et un résultat nul de la fonction d'ouverture doit être vérifié.

</details>

**5. `subsd xmm1, xmm0` et `mulsd xmm1, xmm2` apparaissent, et `xmm0` contient le résultat. Quels registres sont des arguments, et de quel type ?**

<details>
<summary>Réponse</summary>

`xmm0`, `xmm1` et `xmm2` sont lus avant d'être écrits, il y a donc trois arguments `double` (`sd` signifie scalaire double ; `ss` serait `float`), et le type de retour est `double`. Les registres entiers n'interviennent pas.

</details>

**6. Vous avez testé une hypothèse avec trois entrées et toutes ont correspondu. Est-elle confirmée ?**

<details>
<summary>Réponse</summary>

Pas encore. L'accord sur des entrées typiques est un indice faible. Testez les limites (entrée vide ou d'un élément, zéro, les plus grandes et plus petites valeurs, un tampon qui tient juste et un autre trop petit d'un octet) et comparez avec un modèle indépendant de la règle, écrit avant l'exécution.

</details>

**7. Pourquoi le programme de test affiche-t-il les trois valeurs de `bb_next` dans trois instructions séparées ?**

<details>
<summary>Réponse</summary>

Le C ne définit pas l'ordre d'évaluation des arguments d'un appel. Dans un seul `printf` avec trois appels, les valeurs peuvent apparaître dans n'importe quel ordre, et la sortie ne correspondrait plus à ce que fait la bibliothèque.

</details>

**8. Une fonction lit `[rdi]` avant de vérifier que son argument de nombre est non nul. Qu'est-ce que cela ajoute à l'interface ?**

<details>
<summary>Réponse</summary>

Une règle d'emploi : le nombre doit valoir au moins 1, sinon la fonction lit une mémoire qui ne lui appartient pas. Une signature ne peut pas l'exprimer, elle va donc dans le commentaire de l'en-tête.

</details>
