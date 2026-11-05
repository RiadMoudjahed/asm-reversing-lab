<!-- source: quiz.md @ sha256:314c7156006a -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 02. Le processeur et ses registres

**1. `rax` contient `0x1122334455667788`. Après `mov al, 0xFF`, que contient-il ?**

<details>
<summary>Réponse</summary>

`0x11223344556677FF`. Une écriture 8 bits ne modifie que les bits 0 à 7.

</details>

**2. `rax` contient `0x1122334455667788`. Après `mov eax, 1`, que contient-il ?**

<details>
<summary>Réponse</summary>

`0x0000000000000001`. Une écriture 32 bits met à zéro les 32 bits hauts.

</details>

**3. Quel indicateur signale un débordement d'addition non signée, et lequel un débordement signé ?**

<details>
<summary>Réponse</summary>

CF (retenue) pour le non signé ; OF (débordement) pour le signé.

</details>

**4. Après `mov al, 127` et `add al, 1`, CF est-il positionné ? OF l'est-il ?**

<details>
<summary>Réponse</summary>

CF est à zéro (127 + 1 = 128 tient dans un octet non signé). OF est positionné (128 ne tient pas dans un octet signé ; le résultat se lit -128).

</details>

**5. Quel est l'effet de `cmp al, 5` sur `al` ?**

<details>
<summary>Réponse</summary>

Aucun. Elle calcule `al - 5`, jette le résultat et ne conserve que les indicateurs.

</details>

**6. Quel registre contient l'adresse de l'instruction suivante ?**

<details>
<summary>Réponse</summary>

`rip` en code 64 bits, `eip` en code 32 bits.

</details>

**7. Que fait l'instruction `mov eax, eax` en code 64 bits ?**

<details>
<summary>Réponse</summary>

Elle étend `eax` par des zéros jusqu'à `rax`, parce que l'écriture 32 bits met à zéro les 32 bits hauts. La valeur visible dans `eax` ne change pas.

</details>
