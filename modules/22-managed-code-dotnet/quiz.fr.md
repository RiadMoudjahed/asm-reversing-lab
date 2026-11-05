<!-- source: quiz.md @ sha256:9c23cc07031e -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 22. Code managé : .NET

Répondez d'abord, puis ouvrez la réponse.

**1. Pourquoi un décompilateur retrouve-t-il en général plus de choses dans un assembly managé que dans un exécutable natif ?**

<details>
<summary>Réponse</summary>

L'assembly conserve dans ses métadonnées et dans l'IL les noms des types, des méthodes et des champs, leurs signatures typées et la structure du code, parce que l'environnement d'exécution en a besoin. Un exécutable natif contient des instructions machine dont les noms et les types ont été retirés, et le décompilateur doit les déduire.

</details>

**2. Citez trois signes, lors du triage, qu'un fichier est un assembly managé.**

<details>
<summary>Réponse</summary>

`file` indique `Mono/.Net assembly` ; la table des imports a une seule entrée de `mscoree.dll` (`_CorExeMain` pour un programme) ; le répertoire de données numéro 14 (l'en-tête CLR) n'est pas vide ; et les métadonnées commencent par la signature `BSJB` et une chaîne de version comme `v4.0.30319`.

</details>

**3. Pourquoi `strings` avec ses réglages par défaut ne montre-t-il pas les littéraux de chaîne d'un programme managé ?**

<details>
<summary>Réponse</summary>

Les littéraux sont stockés dans le tas `#US` sous forme de texte UTF-16. La recherche par défaut cherche des caractères d'un octet. Utilisez `strings -el` pour trouver du texte UTF-16, ou lisez le tas avec un script. Les noms de types, de méthodes et de champs sont dans `#Strings` en UTF-8 et apparaissent.

</details>

**4. Que fait la séquence d'IL `ldarg.0`, `ldfld`, `ldc.i4 499`, `add`, `stloc.0` ?**

<details>
<summary>Réponse</summary>

Elle empile `this`, le remplace par la valeur d'un champ, empile la constante 499, remplace les deux par leur somme et dépile le résultat dans la variable locale 0. C'est la façon d'écrire `x = field + 499` avec la pile d'évaluation.

</details>

**5. Une méthode décompilée utilise des variables nommées `num`, `num2` et `num3`. Où sont passés les noms d'origine, et comment les récupérer ?**

<details>
<summary>Réponse</summary>

Les noms des variables locales sont dans le fichier de débogage (PDB), pas dans l'assembly ; une version sans lui n'a que des emplacements numérotés et le décompilateur invente des noms. Si vous avez le PDB, décompilez avec l'option qui l'utilise (`-usepdb`). Sinon, nommez vous-même les variables d'après leur usage.

</details>

**6. Une classe de la sortie décompilée s'appelle `<>c__DisplayClass1_0`. Qu'est-ce que c'est ?**

<details>
<summary>Réponse</summary>

Une classe auxiliaire générée par le compilateur, non écrite par l'auteur. Celle-ci contient une variable locale qu'une lambda capture, et le corps de la lambda est une méthode de la classe. Les noms qui contiennent `<` et `>` signalent du code généré et se lisent comme le code qu'ils implémentent.

</details>

**7. Quand devez-vous lire l'IL plutôt que de vous fier au C# décompilé ?**

<details>
<summary>Réponse</summary>

Quand le code décompilé semble faux ou étrange, quand un détail décide de la réponse (une constante, une comparaison, l'ordre des opérations), et quand le code est assez obscurci pour que le décompilateur abandonne. Le décompilateur fait des choix ; l'IL est ce que l'environnement d'exécution exécute.

</details>

**8. Comment traitez-vous les chaînes qu'un obscurcisseur a chiffrées dans un assembly managé ?**

<details>
<summary>Réponse</summary>

Le tas `#US` a peu de littéraux et le code appelle une petite méthode qui renvoie une chaîne. Cette méthode est le décodeur : lisez son IL et réimplémentez-la, ou appelez-la depuis un banc de test dans le laboratoire isolé avec les arguments lus aux sites d'appel, comme dans le module 20.

</details>
