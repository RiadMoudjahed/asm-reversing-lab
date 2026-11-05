<!-- source: quiz.md @ sha256:016737cf52bb -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 05. Arithmétique et logique

**1. Après `add rax, 1` avec `rax = 0x7FFFFFFFFFFFFFFF`, lequel de CF et OF est positionné ?**

<details>
<summary>Réponse</summary>

OF seulement. En addition non signée, rien ne sort du registre, mais en addition signée le résultat passe de la plus grande valeur positive à la plus négative.

</details>

**2. Pourquoi `cqo` doit-il précéder `idiv` ?**

<details>
<summary>Réponse</summary>

`idiv` divise la valeur de 128 bits `rdx:rax`. `cqo` remplit `rdx` avec l'extension de signe de `rax` ; sans lui, `rdx` contient une valeur sans rapport et le résultat est faux.

</details>

**3. Qu'écrit `mul rsi` en plus de `rax` ?**

<details>
<summary>Réponse</summary>

`rdx`. Le produit de 128 bits est rangé dans `rdx:rax` : toute valeur que vous gardiez dans `rdx` est donc écrasée.

</details>

**4. `-17 / 5` est calculé avec `idiv`. Quels sont le quotient et le reste ?**

<details>
<summary>Réponse</summary>

Quotient -3 et reste -2. `idiv` arrondit vers zéro et le reste prend le signe du dividende.

</details>

**5. Quelle est la différence entre `sar` et `shr` ?**

<details>
<summary>Réponse</summary>

`shr` insère des zéros (décalage logique, pour les valeurs non signées). `sar` recopie le bit de signe (décalage arithmétique, pour les valeurs signées) : un nombre négatif reste donc négatif.

</details>

**6. Une fonction compilée contient `shr rax, 3`. Qu'a probablement fait le source C ?**

<details>
<summary>Réponse</summary>

Divisé une valeur non signée par 8.

</details>

**7. Pourquoi `mov [rdx], rdi` peut-il se trouver entre `add rdi, rsi` et `seto al` ?**

<details>
<summary>Réponse</summary>

`mov` ne modifie aucun indicateur : OF décrit donc toujours le `add` lorsque `seto` le lit.

</details>

**8. Pourquoi `daa` ne s'assemble-t-il pas en code 64 bits ?**

<details>
<summary>Réponse</summary>

Les instructions d'ajustement décimal ne sont pas disponibles en mode 64 bits. NASM signale `error: instruction not supported in 64-bit mode`.

</details>
