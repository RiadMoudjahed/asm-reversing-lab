<!-- source: quiz.md @ sha256:62ae4799ebaf -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 09. Ce qu'est la rétro-ingénierie

Répondez d'abord, puis ouvrez la réponse.

**1. Quel est le but de la rétro-ingénierie, et en quoi diffère-t-elle de la réécriture d'un programme ?**

<details>
<summary>Réponse</summary>

Le but est de comprendre : retrouver le fonctionnement d'un programme à partir du programme lui-même. Réécrire produit du code nouveau ; cela peut découler de la compréhension, mais n'est pas l'objectif.

</details>

**2. Citez trois usages légitimes de la rétro-ingénierie.**

<details>
<summary>Réponse</summary>

Trois parmi : l'interopérabilité, l'audit de sécurité, l'analyse de logiciels malveillants, la récupération d'un savoir perdu, le débogage sans source, l'apprentissage de la façon dont un compilateur traduit le code.

</details>

**3. Quelles trois choses devez-vous vérifier avant d'analyser un programme qui n'est pas le vôtre ?**

<details>
<summary>Réponse</summary>

À qui il appartient, ce que sa licence ou son contrat dit de l'analyse, et ce que dit la loi de votre pays.

</details>

**4. Donnez un point fort et un point faible de l'analyse statique.**

<details>
<summary>Réponse</summary>

Point fort : elle est sûre et voit tout le code, y compris les chemins qui ne s'exécutent jamais. Point faible : elle ne voit pas les valeurs calculées à l'exécution, et le code obscurci est difficile à lire.

</details>

**5. Pourquoi n'effectue-t-on jamais d'analyse dynamique sur la machine qui contient vos fichiers personnels ?**

<details>
<summary>Réponse</summary>

Parce qu'elle exécute du code que vous ne comprenez pas encore entièrement. Utilisez un environnement jetable, comme le conteneur du laboratoire de ce cours ou, pour de vrais échantillons, une machine virtuelle sans réseau et avec un instantané.

</details>

**6. Mettez ces étapes dans l'ordre : lire et nommer, triage, rendre compte, observer, tester l'idée, formuler une question, trouver le code.**

<details>
<summary>Réponse</summary>

Triage, observer, formuler une question, trouver le code, lire et nommer, tester l'idée, rendre compte.

</details>

**7. Après le triage d'un fichier, vous voyez `statically linked, not stripped` et la chaîne `NASM`. Que pouvez-vous conclure, et que ne le pouvez-vous pas ?**

<details>
<summary>Réponse</summary>

Vous pouvez conclure que le fichier n'a aucune dépendance envers des bibliothèques partagées, que les noms de symboles sont toujours présents et qu'il a probablement été assemblé avec NASM. Vous ne pouvez pas conclure ce que fait le programme ; les chaînes et les en-têtes sont des indices que l'analyse du code ou l'observation doivent confirmer.

</details>
