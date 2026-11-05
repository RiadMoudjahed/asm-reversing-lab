<!-- source: quiz.md @ sha256:a31f786f90fe -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 17. Déchiffrer les formats de fichier

Répondez d'abord, puis ouvrez la réponse.

**1. Un fichier commence par les octets `50 4b 03 04` et les mêmes quatre octets commencent tous vos échantillons. Qu'est-ce que c'est, et que cela vous permet-il de faire ?**

<details>
<summary>Réponse</summary>

Une valeur magique, un identifiant fixe au décalage 0. Elle permet à un lecteur de reconnaître le format et de refuser les autres fichiers, et elle vous permet de chercher dans un programme le code qui la vérifie, comme une comparaison avec cette constante ou avec une chaîne de ces octets.

</details>

**2. Vous changez un paramètre en fabriquant un fichier et le vidage diffère à deux endroits : quatre octets près du début et les quatre derniers octets. Que concluez-vous ?**

<details>
<summary>Réponse</summary>

Les quatre octets près du début contiennent le paramètre que vous avez changé, et les quatre derniers dépendent du reste du fichier, ce sont donc probablement une somme de contrôle ou un condensé. L'expérience montre aussi que la somme de contrôle couvre au moins la région qui contient le paramètre.

</details>

**3. Le code assemble une valeur à partir de quatre octets avec des chargements `movzx`, des `shl` de 8, 16 et 24 et des `or`, l'octet à l'adresse la plus basse n'ayant aucun décalage. Quel est l'ordre des octets ?**

<details>
<summary>Réponse</summary>

Petit-boutiste (little endian) : l'octet à l'adresse la plus basse est le moins significatif. Un lecteur gros-boutiste décalerait le premier octet de 24.

</details>

**4. Vous trouvez une boucle qui remplit 256 entrées de table avec huit tours de `shr 1` et un xor conditionnel avec 0xedb88320. Qu'est-ce que c'est, et quels trois détails devez-vous encore établir ?**

<details>
<summary>Réponse</summary>

Elle construit la table d'un CRC-32 (le polynôme réfléchi 0xedb88320). Il reste à établir la valeur initiale de la somme cumulée, si le résultat est complémenté à la fin, et la plage d'octets que la somme de contrôle couvre. Ils changent le résultat, donc chacun doit être pris dans le code puis confirmé sur un échantillon.

</details>

**5. Comment trouvez-vous le code qui lit un fichier quand le programme est dépouillé de ses symboles ?**

<details>
<summary>Réponse</summary>

Partez de ce qui survit à la suppression des symboles : les messages d'erreur et autres chaînes (suivez leurs références croisées jusqu'aux conditions qui y sautent), les fonctions importées comme `fopen`, `fread` et `memcmp`, et les constantes comme les étiquettes, les valeurs magiques et les polynômes. Les tailles passées à `fread` donnent les tailles du champ ou de la structure suivante.

</details>

**6. Pourquoi une spécification n'est-elle pas terminée tant que vous n'avez pas écrit un analyseur d'après elle et comparé cet analyseur à l'original ?**

<details>
<summary>Réponse</summary>

La spécification est un ensemble d'affirmations, et écrire du code d'après elle oblige à préciser chacune. Comparer l'analyseur à l'original sur des fichiers valides et endommagés montre les affirmations fausses ou manquantes, comme l'ordre des vérifications, les limites, les étiquettes ignorées et les octets après la fin des données.

</details>

**7. Un lecteur accepte un fichier qui a des octets en plus après le dernier bloc. Est-ce un bogue de votre analyseur s'il accepte aussi le fichier ?**

<details>
<summary>Réponse</summary>

Non. La spécification doit consigner que l'original ne regarde pas les octets après le bloc de fin, et un analyseur équivalent se comporte de même. Si le format est utilisé plus tard par un lecteur plus strict, c'est une autre question, et la spécification doit la mentionner comme une observation.

</details>

**8. Pourquoi modifiez-vous d'abord le champ et la somme de contrôle en dernier quand vous construisez un fichier de test ?**

<details>
<summary>Réponse</summary>

La somme de contrôle dépend du champ. Si vous corrigez d'abord la somme de contrôle puis changez le champ, le lecteur s'arrête à la vérification de la somme et n'atteint jamais le code qui traite le champ que vous vouliez tester.

</details>
