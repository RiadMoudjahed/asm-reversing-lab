<!-- source: quiz.md @ sha256:093870c388f5 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 23. Les décompilateurs et leurs limites

Répondez d'abord, puis ouvrez la réponse.

**1. Pourquoi un décompilateur ne défait-il pas le compilateur ?**

<details>
<summary>Réponse</summary>

Le compilateur a jeté les noms, les types, les commentaires, la structure des boucles et le choix des constructions du source. Le décompilateur ne peut que construire un programme plausible au même comportement, en devinant là où l'information manque.

</details>

**2. Nommez les étapes par lesquelles passe un décompilateur.**

<details>
<summary>Réponse</summary>

Désassemblage et graphe de flot de contrôle, élévation (instructions vers opérations sur des variables, indicateurs vers conditions), analyse de flot de données (supprimer les affectations que personne ne lit, fondre les valeurs en expressions, retrouver les variables locales), récupération des types, structuration (reconstruire `if` et boucles, avec `goto` là où cela échoue) et affichage.

</details>

**3. Qu'est-ce qu'un arc de retour, et que dit-il à un décompilateur ?**

<details>
<summary>Réponse</summary>

Un arc d'un bloc vers un bloc qui le domine, c'est-à-dire qui se trouve sur tous les chemins de l'entrée au premier bloc. Il signale une boucle, la cible étant l'en-tête de la boucle.

</details>

**4. Quel problème rencontre un décompilateur quand un bloc modifie un registre que lit la condition de branchement du bloc lui-même ?**

<details>
<summary>Réponse</summary>

La condition est construite à partir des valeurs que les registres avaient au début du bloc, mais dans le texte affiché les affectations viennent d'abord. Si elles sont affichées dans cet ordre, la condition utilise les nouvelles valeurs. Le décompilateur doit d'abord sauvegarder la condition dans une variable ou ordonner les instructions pour que chacune lise d'anciennes valeurs. Une erreur ici change le sens sans changer la façon dont le texte se lit.

</details>

**5. Le code compilé lit un octet avec `movsx`. Un listing montre `p[0]` pour un pointeur `unsigned char`, comme un simple `int`. Qu'est-ce qui est faux, et comment l'avez-vous vu ?**

<details>
<summary>Réponse</summary>

`movsx` est une extension de signe, donc l'octet est lu comme un nombre signé (une conversion vers `signed char`), et pour 0x80 la valeur est -128. Un simple `p[0]` d'un `unsigned char` est étendu par des zéros et donne 128. Le mnémonique du désassemblage le montre.

</details>

**6. Pourquoi `x & (x - 1) == 0` n'est-il pas identique à `(x & (x - 1)) == 0` en C ?**

<details>
<summary>Réponse</summary>

La comparaison `==` lie plus fort que le `&` bit à bit, donc la première signifie `x & ((x - 1) == 0)`. Supprimer des parenthèses en réécrivant ou en simplifiant la sortie d'un décompilateur peut changer la fonction sans que cela se voie.

</details>

**7. Une lecture d'une fonction ne diffère du binaire que sur 3 des 215 entrées de test. Qu'est-ce que cela montre sur les tests ?**

<details>
<summary>Réponse</summary>

Un test ne vaut que par ses entrées. La lecture fausse concorde sur toutes les entrées qui ne sont pas des puissances de deux, et l'ensemble de tests en contient peu. Incluez les valeurs limites et les cas particuliers que met en évidence la logique, pas seulement des valeurs aléatoires.

</details>

**8. Quand devez-vous lire les instructions plutôt que le code décompilé ?**

<details>
<summary>Réponse</summary>

Pour les petites routines où chaque opération, largeur et indicateur compte (sommes de contrôle, chiffrements, astuces de bits), l'assembleur écrit à la main et les instructions sans équivalent en C, le flot de contrôle inhabituel (tables de sauts non trouvées, une machine virtuelle), les sorties avec beaucoup d'instructions `goto` ou des types changeants, et quand le décompilateur échoue ou qu'un résultat semble faux.

</details>
