<!-- source: quiz.md @ sha256:128a2b5483f9 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 14. Arithmétique compilée

Répondez d'abord, puis ouvrez la réponse.

**1. Que calcule `lea eax, [rdi+rdi*4]`, et lit-il la mémoire ?**

<details>
<summary>Réponse</summary>

Il calcule `5 * x` (`x + 4*x`) pour `x` dans `rdi`. Il ne lit pas la mémoire et ne modifie pas les indicateurs ; `lea` ne fait qu'évaluer l'expression d'adresse.

</details>

**2. Quelle expression C est `lea eax, [0+rdi*8]` suivi de `sub eax, edi` ?**

<details>
<summary>Réponse</summary>

`x * 7`, calculé comme `8 * x - x`.

</details>

**3. Un `x / 2` signé se compile en `mov eax, edi / shr eax, 31 / add eax, edi / sar eax`. Pourquoi pas seulement `sar` ?**

<details>
<summary>Réponse</summary>

C arrondit la division vers zéro, alors que `sar` arrondit vers moins l'infini. Pour les valeurs impaires négatives, les deux diffèrent d'une unité. `shr eax, 31` et `add` ajoutent 1 aux entrées négatives avant le décalage pour corriger l'arrondi.

</details>

**4. Vous trouvez `imul rax, rdx` après `mov edx, 3435973837`, puis `shr rax, 35`. Quel est le diviseur, et l'opération est-elle signée ?**

<details>
<summary>Réponse</summary>

Le diviseur est `2^35 / 3435973837`, soit environ 10 ; c'est une division par 10. Le décalage est `shr` et il n'y a pas de correction de signe : l'opération est non signée.

</details>

**5. Que produit la paire `cmp edi, esi` et `sbb eax, eax` ?**

<details>
<summary>Réponse</summary>

Un masque : que des uns quand `edi` est inférieur à `esi` en valeurs non signées (l'indicateur de retenue est positionné), et zéro sinon. C'est `-(a < b)` calculé sans saut.

</details>

**6. Citez trois différences d'instructions qui indiquent qu'une valeur est signée plutôt que non signée.**

<details>
<summary>Réponse</summary>

Trois au choix parmi : `jl`, `jg` et `setl` au lieu de `jb`, `ja` et `setb` ; `sar` au lieu de `shr` ; `movsx` au lieu d'un simple `mov` vers un registre 32 bits ; `cdq` avec `idiv` au lieu de `xor edx, edx` avec `div` ; `cmovl` ou `cmovge` au lieu de `cmovb` ou `cmovnb`.

</details>

**7. Un reste signé `x % 8` est compilé. Pourquoi n'est-ce pas un simple `and eax, 7` ?**

<details>
<summary>Réponse</summary>

Pour un `x` négatif, le résultat en C est négatif ou nul, alors que `and` avec 7 donne une valeur de 0 à 7. Le compilateur ajoute un biais avant le `and` et le retire ensuite, ce qui donne la séquence `sar 31`, `shr 29`, `add`, `and 7`, `sub`.

</details>
