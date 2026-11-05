<!-- source: quiz.md @ sha256:f18a70512af2 -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 12. Formats exécutables : ELF et PE

Répondez d'abord, puis ouvrez la réponse.

**1. Quelle est la différence entre un décalage dans le fichier et une adresse virtuelle ?**

<details>
<summary>Réponse</summary>

Un décalage dans le fichier compte les octets depuis le début du fichier. Une adresse virtuelle est l'endroit où ces octets apparaissent dans la mémoire du processus après le chargement. Elles diffèrent à cause de l'alignement sur les pages et parce que certaines sections, comme `.bss`, occupent de la mémoire mais pas de place dans le fichier.

</details>

**2. Quelle partie d'un fichier ELF le chargeur utilise-t-il pour le projeter, et quelle partie utilisent des outils comme l'éditeur de liens et le débogueur ?**

<details>
<summary>Réponse</summary>

Le chargeur utilise les en-têtes de programme (segments). L'éditeur de liens, le débogueur et le rétro-ingénieur utilisent les en-têtes de section. Un fichier peut s'exécuter sans sa table des sections.

</details>

**3. Un segment `LOAD` a une taille dans le fichier de 0x22c et une taille en mémoire de 0x230. Que signifie la différence ?**

<details>
<summary>Réponse</summary>

Les 4 octets supplémentaires sont de la mémoire initialisée à zéro sans place dans le fichier : le `.bss`.

</details>

**4. Expliquez ce qui se passe au premier appel de `puts@plt` avec la liaison paresseuse.**

<details>
<summary>Réponse</summary>

Le relais saute à travers son emplacement de la GOT, qui pointe d'abord vers l'instruction suivante du relais. Cette instruction empile l'indice de relocalisation et saute au relais partagé, qui entre dans le résolveur du chargeur dynamique. Le résolveur trouve `puts`, écrit son adresse dans l'emplacement de la GOT et y saute. Les appels suivants vont directement à `puts`.

</details>

**5. Dans un fichier PE, la RVA du point d'entrée est 0x2150, et la section qui la contient a pour RVA 0x2000 et pour décalage brut 0xa00. Quel est le décalage dans le fichier ?**

<details>
<summary>Réponse</summary>

0x2150 - 0x2000 + 0xa00 = 0xb50.

</details>

**6. Quelles structures PE jouent le rôle de la PLT et de la GOT d'ELF ?**

<details>
<summary>Réponse</summary>

Le répertoire d'importation avec la table des noms d'importation et la table des adresses d'importation. Le chargeur remplit l'IAT avant le démarrage du programme, et le programme appelle à travers elle.

</details>

**7. Citez deux champs que le chargeur doit trouver dans un exécutable ELF 64 bits minimal.**

<details>
<summary>Réponse</summary>

Deux au choix parmi : le nombre magique et la classe, le type de machine, le point d'entrée, le décalage, la taille et le nombre des en-têtes de programme, et dans l'en-tête de programme le type, les indicateurs, le décalage, l'adresse virtuelle et les tailles du segment `LOAD`.

</details>
