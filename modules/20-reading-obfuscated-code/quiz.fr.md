<!-- source: quiz.md @ sha256:a90892d38baa -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 20. Lire du code obscurci

Répondez d'abord, puis ouvrez la réponse.

**1. Pourquoi le code obscurci peut-il toujours être compris en principe ?**

<details>
<summary>Réponse</summary>

L'obscurcissement change la difficulté de lire le code, pas ce qu'il fait. Le processeur doit toujours exécuter le comportement réel, qui peut donc être retrouvé en lisant, en émulant de petites parties ou en suivant les données.

</details>

**2. Un programme contacte manifestement un serveur, mais `strings` ne montre aucun nom d'hôte. Que cherchez-vous ?**

<details>
<summary>Réponse</summary>

Des chaînes encodées : une table d'octets d'aspect aléatoire, une petite boucle avec un `xor`, une fonction appelée avec une destination, une source, une longueur et une clé, ou une suite d'écritures d'octets de constantes imprimables (une chaîne de pile). Le texte est introuvable, vous trouvez donc le code qui le produit.

</details>

**3. Qu'est-ce qui distingue, dans le désassemblage, un décodeur à clé fixe d'un décodeur à clé tournante ?**

<details>
<summary>Réponse</summary>

Un décodeur à clé fixe applique un xor à chaque octet avec la même valeur. Dans un décodeur à clé tournante, le registre de clé est mis à jour dans la boucle (par exemple `lea ecx,[rcx+rcx*4+3]`, soit `k * 5 + 3`), de sorte que chaque octet utilise une clé différente et il vous faut la règle et la valeur de départ.

</details>

**4. Comment obtenez-vous le texte clair d'une chaîne encodée sans exécuter le programme ?**

<details>
<summary>Réponse</summary>

Trouvez le décodeur et un appel vers lui, lisez les arguments (destination, octets encodés, longueur, clé) dans les instructions avant l'appel, puis reproduisez la règle dans un script ou émulez le décodeur avec un émulateur. Pour une clé d'un octet, vous pouvez aussi faire un xor du premier octet encodé avec la première lettre attendue, comme le `h` de `http`.

</details>

**5. Qu'est-ce qu'un prédicat opaque, et comment en reconnaître un ?**

<details>
<summary>Réponse</summary>

Une condition qui a toujours la même valeur, bien que ce soit difficile à voir d'un coup d'œil, de sorte qu'un côté de la branche est du code mort. C'est en général une identité arithmétique, comme le fait que le produit d'un nombre et de son successeur est pair. Testez-la en écrivant l'identité ou en émulant sur plusieurs entrées.

</details>

**6. À quoi ressemble une fonction dont le flux de contrôle est aplati, et comment retrouver son graphe d'origine ?**

<details>
<summary>Réponse</summary>

Un bloc central (une table de sauts ou une chaîne de comparaisons sur une variable) vers lequel tous les autres blocs reviennent, et des blocs qui rangent des constantes dans cette variable. Lisez les constantes rangées par chaque bloc pour trouver ses successeurs, ou exécutez la fonction pour plusieurs entrées et notez les valeurs rangées dans la variable d'état ; les suites sont des chemins dans le graphe d'origine.

</details>

**7. Comment reconnaissez-vous une vérification de débogueur lors du triage, sans exécuter le programme ?**

<details>
<summary>Réponse</summary>

À ses importations, ses chaînes et ses motifs : un appel à une fonction qui échoue si un traceur est attaché (`ptrace` avec la requête qui demande à être tracé, `IsDebuggerPresent`), la chaîne `/proc/self/status` avec une comparaison de `TracerPid` avec 0, deux lectures d'une horloge autour d'un bloc, ou des recherches d'outils d'analyse. Notez l'emplacement, le test et ce que le programme fait de différent.

</details>

**8. Pourquoi l'analyse statique et l'émulation sont-elles moins affectées par les vérifications d'analyse que l'exécution du programme ?**

<details>
<summary>Réponse</summary>

Les vérifications se déclenchent quand le programme s'exécute au sens normal sous observation. Lire le code ou émuler une routine n'exécute pas le programme, donc les vérifications ne se déclenchent jamais. S'il faut observer une exécution, indiquez dans le rapport qu'une vérification est présente et que l'observation peut différer d'une exécution normale.

</details>
