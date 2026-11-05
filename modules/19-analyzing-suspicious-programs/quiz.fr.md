<!-- source: quiz.md @ sha256:ce54d557ec3c -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 19. Analyser des programmes suspects

Répondez d'abord, puis ouvrez la réponse.

**1. Citez quatre précautions pour analyser un programme inconnu.**

<details>
<summary>Réponse</summary>

Travailler dans un environnement isolé (machine virtuelle ou conteneur) sans réseau, prendre d'abord un instantané et le restaurer ensuite, ne rien partager avec l'hôte (dossiers, presse-papiers, identifiants) et garder les échantillons inertes au repos (extension changée ou archive) en notant leurs hachages. Noter les commandes et les constats fait aussi partie du travail.

</details>

**2. Un simulateur affirme qu'il se contente d'afficher. Comment pouvez-vous le vérifier sans lire son code ?**

<details>
<summary>Réponse</summary>

Exécutez-le dans le laboratoire sous un traceur d'appels système comme `strace -f` et regardez les appels : un programme qui se contente d'afficher charge ses bibliothèques, écrit sur sa sortie et se termine. Toute création de processus, connexion réseau, suppression de fichier ou accès inattendu à un fichier apparaîtrait dans la trace.

</details>

**3. Que suggère une entropie élevée pour un exécutable, et pourquoi n'est-ce pas une preuve ?**

<details>
<summary>Réponse</summary>

Un contenu proche de 8 bits par octet signifie que les données sont compressées ou chiffrées, ce qui indique souvent un programme empaqueté. Ce n'est pas une preuve, car des ressources compressées, des installeurs et des logiciels légitimes protégés ont aussi une entropie élevée, et un échantillon peut être empaqueté de façon à garder une entropie plus basse.

</details>

**4. Pourquoi un chemin du registre comme `CurrentVersion\Run` trouvé dans un fichier ne suffit-il pas à le dire malveillant, et que faut-il ajouter ?**

<details>
<summary>Réponse</summary>

Les outils de documentation, les installeurs et les utilitaires légitimes mentionnent ou utilisent le même chemin. Ajoutez d'autres preuves qui s'accordent : une commande de tâche planifiée, un chemin de dépôt dans un dossier public, un hôte réseau, l'ordre des comportements. Un groupe d'indicateurs vaut plus qu'un seul.

</details>

**5. Quel identifiant de technique couvre un programme qui ajoute une valeur sous une clé Run, et lequel couvre un programme qui crée une tâche planifiée ?**

<details>
<summary>Réponse</summary>

T1547.001 (Registry Run Keys / Startup Folder) pour la valeur de la clé Run, et T1053.005 (Scheduled Task) pour la tâche planifiée. Ce sont deux techniques de persistance.

</details>

**6. Quelle suite d'appels d'API est caractéristique de l'injection classique de DLL, et à quel identifiant de technique correspond-elle ?**

<details>
<summary>Réponse</summary>

`OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory` et `CreateRemoteThread`, le thread démarrant souvent à l'adresse de `LoadLibrary` et la mémoire écrite contenant le chemin d'une bibliothèque. Elle correspond à T1055.001 (DLL injection) sous T1055 (Process Injection). Les appels seuls ne sont pas une preuve, car les débogueurs et d'autres outils légitimes les utilisent aussi.

</details>

**7. Une règle YARA se compose d'une seule chaîne courante et reconnaît votre échantillon. Que faire avant de l'utiliser ?**

<details>
<summary>Réponse</summary>

La tester sur des fichiers propres, car une chaîne courante reconnaîtra des programmes légitimes (un faux positif). La rendre ensuite spécifique : exiger plusieurs chaînes distinctives ensemble, ajouter une vérification du type de fichier comme le nombre magique MZ ou ELF, et tester de nouveau sur l'échantillon et sur la collection propre.

</details>

**8. Que contient une bonne note de détection ?**

<details>
<summary>Réponse</summary>

Le comportement, l'identifiant de technique, où on peut l'observer (la source d'événements), pourquoi il est suspect, quelle activité légitime lui ressemble (faux positifs) et que faire en réponse. Sans source d'événements nommée, personne ne peut agir dessus.

</details>
