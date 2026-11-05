<!-- source: quiz.md @ sha256:0a87bd2a93a8 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 11. La boîte à outils du rétro-ingénieur

Répondez d'abord, puis ouvrez la réponse.

**1. Quel outil vous dit de quel type de fichier il s'agit, et lequel affiche le texte lisible qu'il contient avec ses décalages ?**

<details>
<summary>Réponse</summary>

`file` identifie le type, et `strings -t x` affiche le texte avec les décalages en hexadécimal.

</details>

**2. Dans la liste des sections, que signifient les indicateurs `AX` et `WA` ?**

<details>
<summary>Réponse</summary>

`AX` signifie alloué et exécutable, donc du code comme `.text`. `WA` signifie inscriptible et alloué, donc des données comme `.data` et `.bss`.

</details>

**3. Convertissez `movzbl %al,%eax` et `lea 0xea9(%rip),%rsi` en syntaxe Intel.**

<details>
<summary>Réponse</summary>

`movzx eax, al` et `lea rsi, [rip + 0xea9]`. Inversez les opérandes, retirez `%` et `$`, placez le déplacement entre crochets et remplacez le suffixe de taille par un mnémonique explicite ou un préfixe de taille.

</details>

**4. Que liste `axt @ sym.check` dans Rizin, et pourquoi est-ce utile ?**

<details>
<summary>Réponse</summary>

Il liste les références croisées vers `sym.check`, c'est-à-dire les endroits qui l'appellent ou la référencent. Il répond à « qui utilise ceci ? », la manière habituelle de passer d'une chaîne ou d'une fonction au code qui compte.

</details>

**5. Pourquoi comparer la sortie du décompilateur avec les instructions ?**

<details>
<summary>Réponse</summary>

Le décompilateur reconstruit du C à partir du code machine et peut se tromper sur les types, les formes de boucle et les limites des variables. Les instructions font foi.

</details>

**6. À un point d'arrêt sur la première instruction d'une fonction, où se trouvent les deux premiers arguments entiers en code System V 64 bits, et comment affiche-t-on un argument de type chaîne dans GDB ?**

<details>
<summary>Réponse</summary>

Dans `rdi` et `rsi`. On affiche un argument chaîne avec `x/s $rdi`.

</details>

**7. Un programme compare votre entrée à une chaîne fixe. Citez deux moyens rapides de connaître cette chaîne sans lire le code.**

<details>
<summary>Réponse</summary>

`ltrace` montre les arguments de `strcmp`, et `strings` liste le texte stocké dans le fichier. Aucun ne fonctionne quand la chaîne est construite ou décodée à l'exécution.

</details>
