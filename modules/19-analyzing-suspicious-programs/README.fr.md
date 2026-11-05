<!-- source: README.md @ sha256:cbf3cf5f6a4f -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 19. Analyser des programmes suspects

Durée : environ 3 heures. Prérequis : [18. Auditer des binaires à la recherche de bogues](../18-auditing-binaries/README.fr.md)

## Pourquoi c'est important

Un défenseur reçoit régulièrement un fichier dont quelqu'un se méfie : une pièce jointe, un téléchargement, un programme trouvé sur un serveur. Le travail consiste à répondre vite et en sécurité à quatre questions. Qu'est-ce que c'est ? Que ferait-il ? Comment le reconnaître ailleurs ? Que dire aux personnes qui doivent réagir ? Ce module enseigne ce travail du côté du défenseur. Il s'agit de reconnaître et de décrire des comportements afin de pouvoir les détecter. Il n'enseigne pas à écrire de tels logiciels. Tous les échantillons de ce cours sont des simulateurs inoffensifs qui affichent ce qu'ils feraient et rien d'autre ; aucun logiciel malveillant réel n'apparaît dans ce dépôt, et vous ne devez pas en apporter dans le laboratoire. Lisez [la déclaration d'éthique](../../ETHICS.md) avant de commencer.

## Concepts

### Pratique d'analyse sûre

Traitez tout programme inconnu comme dangereux tant que vous n'avez pas montré le contraire, et organisez le travail de façon qu'une erreur ne coûte rien.

| Règle | Pourquoi |
|---|---|
| N'exécutez jamais un programme inconnu sur votre propre ordinateur | Une seule exécution peut modifier des fichiers, installer une persistance ou envoyer vos données |
| Travaillez dans un environnement isolé : une machine virtuelle ou un conteneur sans réseau | Sans réseau, le programme ne peut ni recevoir d'instructions ni faire sortir des données |
| Prenez d'abord un instantané et restaurez-le ensuite | Vous revenez toujours à un état connu |
| Ne partagez rien avec l'hôte : ni dossiers partagés, ni presse-papiers, ni identifiants | Un échantillon ne doit pas pouvoir atteindre quoi que ce soit d'important |
| Gardez les échantillons inertes au repos : changez l'extension, rangez-les dans une archive, notez le hachage | Un double-clic par erreur ne fait rien |
| Notez ce que vous faites : commandes, heures, hachages, constats | Un rapport ne vaut que par les notes sur lesquelles il repose |
| N'utilisez que des simulateurs dans ce cours | Les simulateurs sont écrits pour être inoffensifs ; les échantillons réels demandent un environnement professionnel et une autorisation |

L'image du laboratoire s'exécute dans un conteneur qui ne voit que le dossier que vous montez. Pour une exécution plus stricte, montez le dossier en lecture seule et désactivez le réseau. Voici une commande pour l'hôte :

```bash
docker run --rm --network none --read-only -v "$PWD":/lab:ro -w /lab asm-reversing-lab \
    modules/19-analyzing-suspicious-programs/examples/build/19_sim
```

Même un programme inoffensif est vérifié avant qu'on lui fasse confiance. Le simulateur `examples/19_sim.c` promet de ne rien faire d'autre qu'afficher. Vous pouvez vérifier cette promesse de l'extérieur, sans regarder le code, avec `strace`, qui liste les appels système d'un programme :

```bash
make
strace -f -e trace=execve,connect,socket,clone,openat,unlink,write build/19_sim 2>&1 >/dev/null
```

```text
execve("build/19_sim", ["build/19_sim"], 0x7fff3dd23cc8 /* 15 vars */) = 0
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib/x86_64-linux-gnu/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
write(1, "[sim] simulator: no action is pe"..., 1018) = 1018
+++ exited with 0 +++
```

Le programme charge la bibliothèque C, écrit 1018 octets sur sa sortie standard et se termine. Il ne démarre aucun processus, n'établit aucune connexion, ne supprime rien et n'ouvre aucun fichier autre que ceux du chargeur. Un échantillon réel montrerait l'inverse, et c'est ce genre de preuve sur lequel reposent le triage et l'analyse de comportement.

### Triage : le premier regard, sans rien exécuter

Le triage décide jusqu'où aller. Rassemblez d'abord les faits peu coûteux. Le script `examples/19_triage.py` les rassemble pour les fichiers ELF et PE et n'exécute jamais le fichier :

```bash
python3 19_triage.py build/19_sim.exe
```

```text
file:     build/19_sim.exe
size:     251253 bytes
sha256:   656c3556a79829ebae38d56c31bc16b7f3b10174e89323862279217c015affe8
type:     PE (MZ header)
entropy:  5.89 bits per byte (above about 7.2 suggests packed or encrypted data)
imports:  48
    KERNEL32.dll!DeleteCriticalSection
    KERNEL32.dll!EnterCriticalSection
    KERNEL32.dll!GetLastError
    KERNEL32.dll!InitializeCriticalSection
    KERNEL32.dll!IsDBCSLeadByteEx
    KERNEL32.dll!LeaveCriticalSection
    KERNEL32.dll!MultiByteToWideChar
    KERNEL32.dll!SetUnhandledExceptionFilter
    KERNEL32.dll!Sleep
    KERNEL32.dll!TlsGetValue
    KERNEL32.dll!VirtualProtect
    KERNEL32.dll!VirtualQuery
    ... and 36 more (the C runtime and basic KERNEL32 functions)
strings worth a second look:
    [registry path] HKCU\Software\Microsoft\Windows\CurrentVersion\Run
    [command line] schtasks /create /tn "%s" /tr "%s" /sc daily /st 09:00
    [url or host] updates.contoso-cdn.test
    [windows path] C:\Users\Public\upd\updsvc.exe
    [process name] updsvc.exe
    [process name] explorer.exe
```

| Fait | Ce qu'il vous apprend |
|---|---|
| Le hachage | Un nom pour le fichier. Cherchez-le dans vos archives et dans les bases publiques ; signalez-le. Des compilations différentes ont des hachages différents : une correspondance prouve l'identité, une absence de correspondance ne prouve rien |
| Le type et la taille | Quels outils s'appliquent, et si la taille correspond à l'usage annoncé |
| L'entropie | Près de 8 bits par octet signifie un contenu compressé ou chiffré, donc souvent un empaquetage ; le code et le texte ordinaires restent bien en dessous. Regardez aussi l'entropie de chaque section |
| Les importations | Les fonctions que le programme demande au système. Une liste d'importations minuscule peut signifier que le programme résout les fonctions lui-même ; une liste avec des fonctions de processus, de mémoire, de registre et de réseau dit ce qu'il peut faire |
| Les chaînes | Chemins, clés, commandes, hôtes. C'est la preuve la moins chère et souvent la meilleure de l'usage |

Remarquez ce que montre ce PE. Ses importations sont seulement l'environnement d'exécution C et des fonctions système de base, parce que le simulateur n'appelle rien de dangereux. Son but se voit dans les seules chaînes : un emplacement de persistance, une commande de tâche planifiée, un chemin de dépôt, un nom d'hôte et un nom de processus.

### Indicateurs statiques

Un **indicateur** est un fait observable qui désigne un programme, une famille ou un comportement. Les indicateurs statiques viennent du fichier sans l'exécuter. Types utiles :

| Type | Exemples | Précaution |
|---|---|---|
| Hachages | SHA-256 du fichier | Se brisent à la moindre modification du fichier |
| Chaînes | Chemins du registre, lignes de commande, noms de fichiers, noms d'hôtes, noms de mutex, noms de tubes, chaînes d'agent utilisateur | Les chaînes courantes (les clés `Run` apparaissent dans les outils de documentation) ont besoin du soutien d'autres chaînes |
| Importations et exportations | Ensembles de fonctions qui, ensemble, suggèrent une capacité | Les logiciels légitimes utilisent les mêmes fonctions |
| Structure | Noms et tailles de sections, point d'entrée dans une section inhabituelle, entropie élevée, une ressource qui est un exécutable | Les compilateurs et les installeurs produisent aussi des dispositions étranges |
| Motifs de code | Une boucle de décodage, une suite d'appels, une table de constantes | Demandent de la rétro-ingénierie pour être confirmés |

Un seul indicateur condamne rarement. Plusieurs qui s'accordent le font. Notez toujours à quel point chaque indicateur est étayé et avec quelle probabilité il apparaîtrait dans des fichiers innocents.

### Indicateurs de comportement

Un **indicateur de comportement** vient de l'observation de ce que fait le programme dans le laboratoire isolé. Regardez le système de fichiers (ce qui est créé, modifié ou supprimé, et où), les processus (ce qui est lancé, avec quelle ligne de commande), le registre sous Windows (clés et valeurs écrites), le réseau (tentatives de connexion, même si elles échouent, et les noms recherchés) et la mémoire (autres processus ouverts, mémoire allouée en eux). Sous Linux, `strace -f` montre les appels système ; sous Windows, les moniteurs de processus, de fichiers, de registre et de réseau font le même travail. Le simulateur affiche directement son comportement :

```bash
make run
```

```text
[sim] simulator: no action is performed, the steps below are only printed
[sim] would read the computer name, the user name and the OS version
[sim] would list the running processes
[sim] would open the key HKCU\Software\Microsoft\Windows\CurrentVersion\Run
[sim] would set the value "Updater" to "C:\Users\Public\upd\updsvc.exe"
[sim] would run: schtasks /create /tn "GoogleUpdateTaskMachineQC" /tr "C:\Users\Public\upd\updsvc.exe" /sc daily /st 09:00
[sim] would open the process explorer.exe with write access
[sim] would allocate 4096 bytes of executable memory in it
[sim] would write the payload into that memory
[sim] would start a remote thread at the payload
[sim] would open the access token of a system process
[sim] would duplicate the token as a primary token
[sim] would start C:\Users\Public\upd\updsvc.exe with the duplicated token
[sim] would send an HTTP GET to http://updates.contoso-cdn.test:8443/v2/check?id=<host id> every 60 seconds
[sim] would write the reply to C:\Users\Public\upd\updsvc.exe
```

L'ordre est instructif. L'ordre des actions d'un échantillon réel se voit aussi dans le code : dans cette compilation, `main` appelle six fonctions l'une après l'autre, et les noms les trahissent (un échantillon réel dépouillé n'aurait pas de noms, mais le même ordre et les mêmes arguments de chaîne) :

```text
00000000004013da <main>:
  4013e2:  mov    edi,0x402300
  4013e7:  call   puts                      ; the banner
  4013ec:  call   sim_discover
  4013f1:  call   sim_persist_runkey
  4013f6:  call   sim_persist_task
  4013fb:  call   sim_inject
  401400:  call   sim_token
  401405:  call   sim_beacon
```

### Correspondance avec ATT&CK

MITRE ATT&CK est un catalogue public des techniques que les attaquants sont connus pour utiliser. Il les organise par **tactique**, l'objectif (persistance, évasion des défenses, commande et contrôle), et donne à chaque **technique** un numéro, comme T1547, avec des **sous-techniques** en dessous, comme T1547.001. Rattacher vos observations à ces numéros donne à tous le même vocabulaire pour les rapports, les détections et les questions de couverture. Rattachez des comportements, pas des outils : demandez ce que fait le programme et trouvez la technique qui le décrit, puis lisez la description officielle pour confirmer qu'elle convient et voir comment la technique se détecte.

| Observation | Technique | Tactique |
|---|---|---|
| Une valeur écrite sous une clé `Run` du registre qui lance un programme à l'ouverture de session | T1547.001 Registry Run Keys / Startup Folder | Persistance |
| Une tâche planifiée créée avec `schtasks` ou les interfaces du planificateur de tâches | T1053.005 Scheduled Task | Persistance, exécution |
| Un programme qui écrit dans la mémoire d'un autre processus et y démarre un thread | T1055 Process Injection (T1055.001 DLL injection, T1055.002 portable executable injection, T1055.003 thread execution hijacking, T1055.004 asynchronous procedure call, T1055.012 process hollowing) | Évasion des défenses, élévation de privilèges |
| Un jeton dupliqué ou emprunté à un autre processus et utilisé pour lancer un processus | T1134 Access Token Manipulation (T1134.001 token impersonation or theft, T1134.002 create process with token) | Évasion des défenses, élévation de privilèges |
| Un programme qui fait charger sa propre bibliothèque à un programme de confiance | T1574 Hijack Execution Flow (T1574.001 DLL search order hijacking, T1574.002 DLL side-loading) | Persistance, élévation de privilèges, évasion des défenses |
| Lecture du nom de la machine, de l'utilisateur et de la version du système | T1082 System Information Discovery | Découverte |
| Liste des processus en cours | T1057 Process Discovery | Découverte |
| Requêtes HTTP vers un serveur à intervalles | T1071.001 Application Layer Protocol: Web Protocols | Commande et contrôle |
| Téléchargement d'un fichier à exécuter plus tard | T1105 Ingress Tool Transfer | Commande et contrôle |
| Commandes exécutées par l'interpréteur de commandes | T1059.003 Windows Command Shell | Exécution |
| Chaînes ou code encodés et décodés à l'exécution | T1027 Obfuscated Files or Information, T1140 Deobfuscate/Decode Files or Information | Évasion des défenses |

Pour le simulateur, la sortie ci-dessus correspond à T1082 et T1057 (les lignes de découverte), T1547.001 (la clé et la valeur), T1053.005 (la commande `schtasks`), T1055 (ouvrir un processus, y allouer de la mémoire, y écrire, démarrer un thread distant : la famille d'injection décrite dans le tableau ci-dessous), T1134 (les lignes du jeton) et T1071.001 (la requête HTTP périodique). Écrivez la correspondance dans un tableau à côté de la preuve de chaque ligne. Si un comportement convient à plusieurs techniques, indiquez la plus proche et dites pourquoi.

### Motifs d'appels d'API derrière les techniques

Sous Windows, un programme atteint le système par des fonctions d'API, et chaque technique demande un groupe caractéristique d'entre elles. Connaître les groupes permet de reconnaître la technique dans une liste d'importations, dans l'ordre des appels du désassemblage ou dans le journal d'un moniteur. Ce sont des motifs de reconnaissance et de détection. Les logiciels légitimes, comme les installeurs, les débogueurs, les outils de sécurité et les utilitaires système, utilisent bon nombre des mêmes fonctions : un motif est donc une raison de regarder de plus près, jamais un verdict.

| Technique | Appels caractéristiques, dans l'ordre habituel | Ce qu'il faut aussi chercher |
|---|---|---|
| Clé Run du registre (T1547.001) | `RegCreateKeyEx` ou `RegOpenKeyEx`, puis `RegSetValueEx` avec un chemin comme donnée | Le chemin de la clé (`...\CurrentVersion\Run`, `RunOnce`) et un chemin vers un fichier dans un dossier où l'utilisateur peut écrire |
| Tâche planifiée (T1053.005) | `CreateProcess` ou `ShellExecute` pour `schtasks /create`, ou `CoCreateInstance` pour les interfaces du planificateur de tâches | La ligne de commande avec `/tr`, `/sc` et `/tn`, et un nom de tâche qui imite un produit connu |
| Injection de DLL (T1055.001) | `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory`, `CreateRemoteThread` (l'adresse de départ est souvent celle de `LoadLibrary`) | Le nom du processus cible et un chemin de bibliothèque dans la mémoire écrite |
| Injection de code (T1055.002) | Les trois mêmes premiers appels, suivis d'un thread dont l'adresse de départ est dans la mémoire allouée | Des permissions d'exécution (`PAGE_EXECUTE_READWRITE`) sur l'allocation |
| Détournement de thread (T1055.003) | `OpenThread`, `SuspendThread`, `GetThreadContext`, `SetThreadContext`, `ResumeThread` | Un pointeur d'instruction modifié dans le contexte |
| Injection par APC (T1055.004) | `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory`, `QueueUserAPC` | Un thread dans un état alertable |
| Évidement de processus (T1055.012) | `CreateProcess` avec l'indicateur de suspension, un retrait de la projection de l'image d'origine (`NtUnmapViewOfSection`), `WriteProcessMemory`, `SetThreadContext`, `ResumeThread` | Un processus dont l'image sur disque diffère de ce qui s'exécute |
| Manipulation de jetons (T1134) | `OpenProcessToken`, `DuplicateTokenEx`, puis `ImpersonateLoggedOnUser`, `SetThreadToken` ou `CreateProcessWithTokenW` ; `AdjustTokenPrivileges` pour activer un privilège | La demande d'un privilège de débogage ou d'usurpation |
| Détournement de l'ordre de recherche des DLL et chargement latéral (T1574.001, T1574.002) | `LoadLibrary` avec un nom de bibliothèque nu, un exécutable lancé depuis un dossier qui contient aussi une bibliothèque au nom attendu | Une nouvelle bibliothèque écrite à côté d'un exécutable de confiance dans un dossier où l'utilisateur peut écrire |

Sous Linux, les familles analogues utilisent d'autres appels (`ptrace` et `process_vm_writev` pour écrire dans un autre processus, `LD_PRELOAD` et les chemins de recherche de bibliothèques pour le détournement, des entrées cron et des unités systemd pour la persistance), et la méthode est la même : regrouper les appels, les ordonner et trouver la preuve dans les chaînes et les arguments.

### Notes de détection

Une **note de détection** dit aux personnes qui surveillent les systèmes ce qu'il faut chercher. Pour chaque comportement, notez ce qui est observé et où, pourquoi c'est suspect, quelle activité légitime y ressemble et à quelle technique il correspond. Un court exemple pour le simulateur :

```text
Behavior:     A value is added under HKCU\Software\Microsoft\Windows\CurrentVersion\Run
              that points to a program in C:\Users\Public.
Technique:    T1547.001 Registry Run Keys / Startup Folder
Where to see: Registry auditing and endpoint monitoring events for registry value
              writes, the process that made the write, the file at the path.
Why suspicious: Run values normally point to program folders; a path in a public,
              user-writable folder is unusual.
False positives: Installers of user-level applications also add Run values. Check
              the signer and the folder.
Response:     Collect the file, hash it, check where it came from, remove the value
              after the evidence is saved.
```

Sources courantes d'événements pour de telles notes : événements de création de processus (avec la ligne de commande complète), événements d'écriture dans le registre, événements de création de tâches du planificateur de tâches, création de threads distants et événements d'accès aux processus des moniteurs de poste, et événements de création de fichiers. Nommez la source dans la note, car une détection que personne ne peut voir ne vaut rien.

### Écrire une règle YARA

YARA compare des fichiers à des motifs. Une règle a un nom, des métadonnées, des chaînes et une condition :

```text
rule name
{
    meta:      description, author, references
    strings:   $a = "text"  $b = { 6A 40 68 ?? ?? }  $c = /regex/
    condition: boolean expression over the strings and the file
}
```

Le fichier `examples/19_sim.yar` contient deux versions d'une règle pour le simulateur. La première est facile à écrire :

```text
rule sim_updater_v1_weak
{
    strings:
        $run = "CurrentVersion\\Run"
    condition:
        $run
}
```

Elle reconnaît le simulateur, et aussi le programme auxiliaire inoffensif (`19_helper.c`), dont le seul tort est que son texte d'aide mentionne le même chemin du registre. Tester une règle sur des fichiers propres est aussi important que la tester sur l'échantillon :

```bash
for f in build/19_sim build/19_sim.exe build/19_helper build/19_helper.exe /bin/ls; do yara 19_sim.yar $f; done
```

```text
sim_updater_v1_weak build/19_sim
sim_updater_v2 build/19_sim
sim_updater_v1_weak build/19_sim.exe
sim_updater_v2 build/19_sim.exe
sim_updater_v1_weak build/19_helper
sim_updater_v1_weak build/19_helper.exe
```

La version 1 se déclenche sur les deux programmes auxiliaires, ce qui est un faux positif. La version 2 exige plusieurs chaînes distinctives ensemble et une vérification du type de fichier :

```text
rule sim_updater_v2
{
    meta:
        description = "Harmless updater simulator from module 19"
        technique = "T1547.001, T1053.005, T1055, T1134, T1071.001"
    strings:
        $runkey = "CurrentVersion\\Run"
        $task   = "schtasks /create /tn" ascii
        $drop   = "\\Users\\Public\\upd\\updsvc.exe" ascii
        $host   = "updates.contoso-cdn.test" ascii
        $tag    = "[sim] would" ascii
    condition:
        (uint32(0) == 0x464c457f or uint16(0) == 0x5a4d) and
        3 of ($runkey, $task, $drop, $host) and $tag
}
```

Elle ne reconnaît que les deux compilations du simulateur. L'option `-s` montre quelles chaînes ont correspondu et où, ce qui est la façon de déboguer une règle :

```bash
yara -s 19_sim.yar build/19_sim
```

```text
sim_updater_v1_weak build/19_sim
0x2150:$run: CurrentVersion\Run
sim_updater_v2 build/19_sim
0x2150:$runkey: CurrentVersion\Run
0x21ad:$task: schtasks /create /tn
0x216a:$drop: \Users\Public\upd\updsvc.exe
0x2071:$host: updates.contoso-cdn.test
0x2004:$tag: [sim] would
```

Conseils pour les règles :

- Visez ce qui est distinctif : chemins inhabituels, noms, lignes de commande, constantes, courtes suites de code avec jokers. Évitez les chaînes que possède tout programme du même genre.
- Combinez les chaînes avec `N of`, `all of` ou `and`, et ajoutez une vérification du type de fichier avec `uint16(0) == 0x5a4d` (les lettres MZ) pour PE ou `uint32(0) == 0x464c457f` pour ELF.
- Ajoutez `ascii`, `wide` ou les deux quand le fichier peut stocker le texte sous l'une ou l'autre forme.
- Testez sur une collection de fichiers propres et sur plusieurs échantillons. Notez à quoi sert chaque règle.
- Une règle sur des chaînes cesse de fonctionner quand l'auteur change les chaînes. Préférez quelques caractéristiques indépendantes à une seule.
- Mettez les numéros de technique dans les métadonnées pour que la règle puisse être retrouvée par couverture.

## Essayez

Lancez les commandes du laboratoire depuis `modules/19-analyzing-suspicious-programs/examples`.

1. Construisez avec `make`. Lancez le script de triage sur `build/19_sim`, sur `build/19_sim.exe` et sur `/bin/ls`. Comparez les importations, l'entropie et les chaînes.
2. Lancez `strace -f -c build/19_sim` et lisez le résumé des appels système. Quels appels attendriez-vous d'un programme qui se rendrait vraiment persistant et se connecterait ?
3. Faites un tableau à partir de `make run` : une ligne pour chaque ligne `[sim]` avec l'identifiant de technique, la tactique et la preuve. Vérifiez chaque identifiant dans la description de la technique sur le site d'ATT&CK.
4. Écrivez une règle YARA qui reconnaît `19_sim` et `19_sim.exe` mais pas `19_helper`, avec des chaînes différentes de celles de `19_sim.yar`. Testez-la avec `yara -s`.
5. Rédigez une note de détection pour le comportement de la tâche planifiée dans le format ci-dessus.
6. Expliquez pourquoi une règle qui reconnaît les chaînes `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory` et `CreateRemoteThread` reconnaîtrait aussi certains programmes légitimes, et ce que vous pourriez ajouter à la condition pour la rendre plus spécifique.

## Pièges courants

- **Exécuter d'abord.** N'exécutez rien avant les vérifications peu coûteuses, et exécutez-le seulement dans le laboratoire isolé.
- **Se fier à l'étiquette.** Un nom de fichier, une icône ou une signature ne prouvent pas ce que fait le programme.
- **Lire un indicateur comme une preuve.** Un chemin du registre dans une chaîne est de la documentation dans un fichier et de la persistance dans un autre. Cherchez un groupe.
- **Rattacher des outils au lieu de comportements.** La question est ce que fait le programme, pas comment il s'appelle.
- **Écrire une règle qui ne convient qu'à l'échantillon.** Une règle fondée sur un hachage ou une seule chaîne casse à la compilation suivante. Testez-la pour les faux positifs et la robustesse.
- **Sauter les notes.** Sans commandes, heures et hachages, personne ne peut répéter ni vérifier les constats.

## Résumé

- N'analysez les programmes inconnus que dans un environnement isolé, sans réseau et avec un instantané, et vérifiez même un simulateur inoffensif de l'extérieur avec une trace des appels système.
- Le triage rassemble hachages, type, entropie, importations et chaînes sans exécuter le fichier et décide jusqu'où aller.
- Les indicateurs statiques viennent du fichier et les indicateurs de comportement de l'observation de son exécution ; plusieurs qui s'accordent valent plus qu'un seul.
- ATT&CK donne un vocabulaire commun : rattachez chaque comportement observé à une tactique et à une technique, avec la preuve.
- Chaque technique a un groupe caractéristique d'appels d'API, qui est une raison de regarder de plus près et jamais un verdict.
- Les notes de détection et les règles YARA transforment les constats en quelque chose que d'autres peuvent utiliser, et toutes deux doivent être testées sur des fichiers propres.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 20, la lecture du code obscurci.
- Exercices qui utilisent ce module: [Exercice 19](../../exercises/19-triage-desk/README.fr.md).
