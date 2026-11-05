<!-- source: quiz.md @ sha256:d1015bcb390d -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 07. Chaînes et instructions de chaîne

**1. Quels registres `rep movsb` utilise-t-il et que contient chacun ?**

<details>
<summary>Réponse</summary>

`rsi` contient l'adresse source, `rdi` l'adresse de destination et `rcx` le nombre d'octets à copier. Les deux pointeurs avancent d'un octet à chaque passage.

</details>

**2. Que contrôle l'indicateur de direction, et quelle valeur doit-il avoir au retour d'une fonction ?**

<details>
<summary>Réponse</summary>

Il décide si les instructions de chaîne font monter leurs pointeurs (DF = 0, `cld`) ou descendre (DF = 1, `std`). L'ABI System V exige qu'il soit à zéro au retour d'une fonction.

</details>

**3. Quelle est la différence entre `repe` et `repne` ?**

<details>
<summary>Réponse</summary>

`repe` répète tant que la dernière comparaison était égale (ZF = 1) et que `rcx` est supérieur à zéro. `repne` répète tant qu'elle était différente (ZF = 0) et que `rcx` est supérieur à zéro.

</details>

**4. Si `repne scasb` se termine avec `rcx` égal à 0 et ZF égal à 0, qu'est-ce que cela signifie pour une implémentation de `memchr` ?**

<details>
<summary>Réponse</summary>

Tous les octets ont été comparés et aucun ne correspondait : l'octet n'a pas été trouvé et la fonction renvoie NULL.

</details>

**5. Pourquoi `my_memmove` copie-t-il vers l'arrière lorsque la destination est au-dessus de la source et la chevauche ?**

<details>
<summary>Réponse</summary>

Une copie vers l'avant écraserait des octets de la source avant de les lire. Partir du dernier octet lit chaque octet avant qu'il ne puisse être écrasé.

</details>

**6. Que fait `mov ecx, 0x40` suivi de `rep stos DWORD` avec `eax` égal à 0 ?**

<details>
<summary>Réponse</summary>

Il met à zéro 0x40 fois 4 = 256 octets à partir de `[rdi]`, ce qui correspond à `memset(p, 0, 256)`.

</details>

**7. Que deviennent les indicateurs lorsque `repe cmpsb` s'exécute avec `rcx` égal à 0 ?**

<details>
<summary>Réponse</summary>

Rien n'est exécuté et les indicateurs gardent leurs valeurs précédentes. C'est pourquoi `my_memcmp` positionne ZF avec `xor eax, eax` avant l'instruction, afin qu'une longueur nulle compte comme égale.

</details>

**8. Dans `memcmp(a, b, n)`, `a` arrive dans `rdi` et `b` dans `rsi`. Quels octets soustrayez-vous pour produire le résultat après une différence, et dans quel ordre ?**

<details>
<summary>Réponse</summary>

L'octet de `a` moins l'octet de `b`, c'est-à-dire `[rdi - 1]` moins `[rsi - 1]` (les deux pointeurs ont déjà dépassé les octets différents). L'ordre inverse renverse le signe du résultat.

</details>
