<!-- source: quiz.md @ sha256:d4df31272239 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 08. La pile, les procédures et les conventions d'appel

**1. Que fait `call target` et que fait `ret` ?**

<details>
<summary>Réponse</summary>

`call` empile l'adresse de l'instruction suivante et saute à `target`. `ret` dépile une adresse dans `rip`.

</details>

**2. Dans une fonction à cadre standard, où se trouvent la première variable locale et l'adresse de retour par rapport à `rbp` ?**

<details>
<summary>Réponse</summary>

Les variables locales sont à des décalages négatifs, par exemple `[rbp - 8]`. Le `rbp` sauvegardé est à `[rbp]` et l'adresse de retour à `[rbp + 8]`.

</details>

**3. Quels registres portent les quatre premiers arguments entiers en System V AMD64 et en Microsoft x64 ?**

<details>
<summary>Réponse</summary>

System V : `rdi`, `rsi`, `rdx`, `rcx`. Microsoft x64 : `rcx`, `rdx`, `r8`, `r9`.

</details>

**4. Quelle est la différence entre cdecl et stdcall, et comment les distingue-t-on dans une fonction appelée ?**

<details>
<summary>Réponse</summary>

En cdecl, l'appelant retire les arguments de la pile ; en stdcall, c'est l'appelé. Une fonction stdcall se termine par `ret N`, une fonction cdecl par un simple `ret`.

</details>

**5. À la première instruction d'une fonction System V, où se trouve le septième argument entier ?**

<details>
<summary>Réponse</summary>

À `[rsp + 8]`. L'adresse de retour est à `[rsp]` et les arguments de pile la suivent.

</details>

**6. Pourquoi `factorial` sauvegarde-t-elle et restaure-t-elle `rbx` ?**

<details>
<summary>Réponse</summary>

Elle utilise `rbx` pour garder `n` à travers l'appel récursif, et `rbx` est sauvegardé par l'appelé : la valeur d'origine doit donc être préservée pour l'appelant. Le `push` maintient aussi `rsp` aligné pour l'appel.

</details>

**7. Quelle valeur `rsp` doit-il avoir au moment d'un `call` en code 64 bits, et comment le corriger dans une fonction qui n'a rien empilé ?**

<details>
<summary>Réponse</summary>

Un multiple de 16. À l'entrée de la fonction, `rsp + 8` est un multiple de 16 : soustrayez donc 8 (`sub rsp, 8`) avant l'appel, ou empilez un registre.

</details>

**8. Qu'est-ce que l'espace d'ombre (shadow space) ?**

<details>
<summary>Réponse</summary>

En Microsoft x64, 32 octets que l'appelant réserve au-dessus de l'adresse de retour à chaque appel, où l'appelé peut ranger les quatre arguments passés par registre.

</details>

**9. Pourquoi une pile mal alignée peut-elle passer inaperçue dans un appel et planter dans un autre ?**

<details>
<summary>Réponse</summary>

Seuls certains chemins de code utilisent des instructions qui exigent un alignement sur 16 octets, comme les écritures SSE alignées. Un appel à `printf` sans argument vectoriel les évite ; un appel avec un argument à virgule flottante les exécute et provoque une faute.

</details>
