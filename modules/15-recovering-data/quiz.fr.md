<!-- source: quiz.md @ sha256:f9784c17b4a4 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 15. Récupérer les données

Répondez d'abord, puis ouvrez la réponse.

**1. Le code contient `mov eax, DWORD PTR [rdi+rsi*4]`. Que savez-vous de `rdi` et de `rsi` ?**

<details>
<summary>Réponse</summary>

`rdi` est la base d'un tableau d'éléments de 4 octets, probablement des `int`, et `rsi` est l'indice (ici sous forme de valeur 64 bits). Le facteur 4 et le chargement de 4 octets s'accordent sur la taille de l'élément.

</details>

**2. Qu'est-ce que `[rdi+4+rsi*8]` vous apprend que `[rdi+rsi*8]` ne vous apprendrait pas ?**

<details>
<summary>Réponse</summary>

Le pas 8 est la taille d'un élément d'un tableau de structures, et le 4 est le décalage d'un champ à l'intérieur de chaque élément. Le code lit le champ au décalage 4 de l'élément `rsi`.

</details>

**3. Une fonction lit à partir de `rdi` les décalages 0 (4 octets), 8 (8 octets) et 16 (1 octet). Que pouvez-vous dire de la disposition, et des décalages 4 et 9 à 15 ?**

<details>
<summary>Réponse</summary>

C'est une structure avec au moins un champ de la taille d'un int en 0, un champ de 8 octets en 8 et un octet en 16. Les décalages 4 à 7 sont probablement du remplissage parce qu'un champ de 8 octets est aligné sur 8, et 9 à 15 sont du remplissage ou des champs que cette fonction n'utilise pas. La taille totale est probablement 24, arrondie à un multiple de 8.

</details>

**4. Une boucle fait `mov rdi, [rdi+8]` et compare `rdi` à zéro. De quelle structure de données s'agit-il ?**

<details>
<summary>Réponse</summary>

D'une liste chaînée : le registre de base est rechargé depuis le nœud sur lequel il pointe, et le pointeur nul termine la boucle. Le pointeur vers le nœud suivant est au décalage 8.

</details>

**5. Comment distinguez-vous dans le code machine une recherche dans un arbre binaire d'un parcours de liste ?**

<details>
<summary>Réponse</summary>

Une recherche dans un arbre charge l'un de deux champs de pointeur (par exemple aux décalages 8 et 16), choisi par une comparaison de la clé recherchée avec la clé du nœud. Un parcours de liste charge toujours le même champ.

</details>

**6. Quel est le premier champ d'un objet C++ qui a des fonctions virtuelles, et comment un appel virtuel l'utilise-t-il ?**

<details>
<summary>Réponse</summary>

Le vptr, au décalage 0. Un appel virtuel charge le vptr depuis l'objet (`mov rax, [rdi]`), puis appelle une adresse rangée dans la vtable à un emplacement fixe (`call [rax+8*slot]`), en passant le pointeur d'objet comme premier argument.

</details>

**7. Deux classes ont des vtables qui ne diffèrent que par un emplacement. Que cela vous apprend-il ?**

<details>
<summary>Réponse</summary>

Une classe dérive de l'autre, et l'emplacement différent est une fonction virtuelle que la classe dérivée redéfinit. Tous les autres emplacements contiennent les mêmes adresses de fonctions parce qu'elles sont héritées.

</details>
