<!-- source: quiz.md @ sha256:12817111b472 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 06. Conditions, branchements et boucles

**1. Quelle est la différence entre `cmp rdi, rsi` et `sub rdi, rsi` ?**

<details>
<summary>Réponse</summary>

Les deux calculent `rdi - rsi` et positionnent les mêmes indicateurs. `cmp` jette le résultat ; `sub` le range dans `rdi`.

</details>

**2. Après `cmp rax, rbx`, quel saut est pris lorsque `rax` est supérieur à `rbx` en nombres signés, et lequel en nombres non signés ?**

<details>
<summary>Réponse</summary>

`jg` en signé et `ja` en non signé.

</details>

**3. Pourquoi `max_signed(-1, 1)` renvoie-t-il 1 alors que `max_unsigned(-1, 1)` renvoie 18446744073709551615 ?**

<details>
<summary>Réponse</summary>

Les bits de -1 sont tous à un. En signé, c'est plus petit que 1 ; en non signé, c'est la plus grande valeur possible. `cmovl` lit les indicateurs du signé et `cmovb` ceux du non signé.

</details>

**4. Laquelle exécute le corps au moins une fois à coup sûr, une boucle while ou une boucle do-while ?**

<details>
<summary>Réponse</summary>

Une boucle do-while, car son test vient après le corps.

</details>

**5. Pourquoi `xor eax, eax` doit-il précéder `cmp` et non se trouver entre `cmp` et `setg al` ?**

<details>
<summary>Réponse</summary>

`xor` modifie les indicateurs. Placé après `cmp`, il remplace le résultat de la comparaison avant que `setg` ne le lise.

</details>

**6. Que fait `test rdi, rcx` lorsque `rcx` vaut `rdi - 1` ?**

<details>
<summary>Réponse</summary>

Elle calcule `rdi AND (rdi - 1)` uniquement pour ses indicateurs. Le résultat est nul lorsque `rdi` a au plus un bit positionné : `setz` indique donc ensuite une puissance de deux (une fois zéro exclu séparément).

</details>

**7. Une fonction compilée termine une boucle par `cmp rax, rsi` suivi d'un `jne` vers le début du corps. Que révèle cela sur son optimisation ?**

<details>
<summary>Réponse</summary>

Le compilateur a déplacé le test de la boucle vers le bas (inversion de boucle) et réécrit le compteur, avec une garde avant la boucle pour le cas où elle ne doit pas s'exécuter du tout.

</details>

**8. Que se passe-t-il si `loop` s'exécute avec `rcx` égal à 0 ?**

<details>
<summary>Réponse</summary>

`loop` soustrait 1 d'abord : `rcx` repasse à une énorme valeur non nulle et la boucle s'exécute 2^64 fois.

</details>
