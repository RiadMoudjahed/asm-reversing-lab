<!-- source: quiz.md @ sha256:2b0e947aeaa7 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 04. Déplacer des données et modes d'adressage

**1. Pourquoi `mov [rdi], 1` ne s'assemble-t-il pas, et comment le corriger ?**

<details>
<summary>Réponse</summary>

Rien n'indique la largeur de l'écriture : une constante n'a pas de taille et `[rdi]` non plus. Ajoutez un mot-clé de taille, par exemple `mov dword [rdi], 1`.

</details>

**2. Que signifie habituellement `[rdi + rsi*4]` dans du code compilé ?**

<details>
<summary>Réponse</summary>

L'élément `rsi` d'un tableau de valeurs de 4 octets dont l'adresse est dans `rdi`, comme `a[i]` pour un tableau d'`int`.

</details>

**3. Quelle est la différence entre `lea rax, [rdi + 8]` et `mov rax, [rdi + 8]` ?**

<details>
<summary>Réponse</summary>

`lea` range l'adresse `rdi + 8` dans `rax` et ne lit pas la mémoire. `mov` lit 8 octets à cette adresse et range la valeur.

</details>

**4. Quelle instruction charge un octet signé et l'étend sur 32 bits ?**

<details>
<summary>Réponse</summary>

`movsx eax, byte [rdi]`. `movzx` traiterait l'octet comme non signé.

</details>

**5. Une fonction compilée contient `shl rsi, 4` suivi de `mov rax, [rdi + rsi + 8]`. Que suggère cela sur le source C ?**

<details>
<summary>Réponse</summary>

Qu'il indexe un tableau de structures de 16 octets (le décalage de 4 bits multiplie l'index par 16) et lit un champ au décalage 8 de l'élément sélectionné.

</details>

**6. Pourquoi n'existe-t-il pas de `movzx rax, eax` ?**

<details>
<summary>Réponse</summary>

Parce qu'une écriture 32 bits ordinaire, comme `mov eax, eax`, met déjà à zéro les 32 bits hauts du registre 64 bits : elle étend donc par des zéros d'elle-même.

</details>

**7. Les octets `34 12` sont lus comme un mot sur x86. Quelle valeur obtient-on ?**

<details>
<summary>Réponse</summary>

`0x1234`. x86 est petit-boutiste : le premier octet est donc l'octet de poids faible.

</details>
