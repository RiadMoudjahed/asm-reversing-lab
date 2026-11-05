<!-- source: quiz.md @ sha256:9a14aab0c7f9 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 13. Reconnaître les structures de code

Répondez d'abord, puis ouvrez la réponse.

**1. Qu'est-ce qu'une fonction feuille, et pourquoi le code optimisé d'une telle fonction n'a-t-il souvent pas de prologue ?**

<details>
<summary>Réponse</summary>

Une fonction feuille ne fait aucun appel. Elle n'a pas besoin de registres sauvegardés au travers d'appels et, si elle n'a pas besoin de mémoire pour des variables locales, pas de cadre non plus ; le compilateur peut donc omettre entièrement le prologue et l'épilogue.

</details>

**2. Le source dit `if (x > 0) { A }`. Le listing contient `cmp` suivi de `jle`. Où `jle` saute-t-il, et pourquoi cette condition ?**

<details>
<summary>Réponse</summary>

Il saute par-dessus le bloc A jusqu'au code qui suit le `if`. Le compilateur saute quand la condition est fausse ; le saut teste donc l'opposé de `x > 0`.

</details>

**3. Quelle est la signature d'une boucle, et en quoi la disposition d'une boucle `for` optimisée diffère-t-elle d'une boucle non optimisée ?**

<details>
<summary>Réponse</summary>

Un saut conditionnel vers l'arrière. La boucle non optimisée commence par un `jmp` vers un test placé en bas. La boucle optimisée vérifie une fois si elle s'exécute, puis a le corps suivi du test en bas, de sorte que chaque itération coûte un saut.

</details>

**4. Deux `cmp` suivis de sauts conditionnels, tous deux vers la même étiquette qui met le résultat à 0. De quelle construction du source s'agit-il ?**

<details>
<summary>Réponse</summary>

Une condition avec `&&` : le code n'atteint le chemin de succès que si aucun test n'échoue.

</details>

**5. Comment distinguez-vous une table de sauts d'une chaîne de comparaisons, et où trouvez-vous les cibles des cas ?**

<details>
<summary>Réponse</summary>

Une table de sauts a une vérification de bornes (`cmp reg, N / ja default`) suivie d'un `jmp` indirect via `[reg*8 + table]`. La table est dans `.rodata` et contient une adresse par valeur de cas, dans l'ordre, avec `N` + 1 entrées. Une chaîne de comparaisons est une suite de paires `cmp` et `je` sans saut indirect.

</details>

**6. Une fonction se termine par `jmp other_function` au lieu de `call other_function; ret`. De quoi s'agit-il, et quel effet cela a-t-il sur un graphe d'appels ?**

<details>
<summary>Réponse</summary>

Un appel terminal. L'appelé revient directement à l'appelant de l'appelant ; la fonction n'apparaît donc pas dans les traces de pile et le graphe d'appels peut la montrer sans appelants.

</details>

**7. Dans du code optimisé, vous trouvez `cmp edi, esi`, puis `mov eax, esi`, puis `cmovge eax, edi`. Quelle expression C est-ce ?**

<details>
<summary>Réponse</summary>

Le maximum de deux valeurs signées : `a >= b ? a : b`, avec `a` dans `edi` et `b` dans `esi`.

</details>
