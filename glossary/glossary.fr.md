<!-- source: glossary.md @ sha256:567da3b1afab -->
[English](glossary.md) | [العربية](glossary.ar.md) | [Deutsch](glossary.de.md) | [Español](glossary.es.md) | [Français](glossary.fr.md)

# Glossaire

Termes essentiels utilisés dans tout le cours. Des entrées sont ajoutées à mesure que les modules introduisent du vocabulaire.

**adresse mémoire (memory address)**: Un nombre qui identifie un octet en mémoire.

**analyse de vivacité (liveness analysis)**: L'analyse de flot de données qui détermine quelles valeurs sont lues plus tard ; les affectations à des valeurs jamais lues sont mortes et peuvent être supprimées.

**analyse dynamique (dynamic analysis)**: L'étude d'un programme en l'exécutant et en observant son comportement, généralement dans un débogueur.

**analyse statique (static analysis)**: L'étude d'un programme sans l'exécuter.

**aplatissement du flux de contrôle (control-flow flattening)**: Un obscurcissement qui remplace le graphe d'une fonction par une boucle avec un répartiteur et une variable d'état qui contient le numéro du bloc suivant.

**appel système (system call)**: Une demande d'un programme au noyau du système d'exploitation pour effectuer une opération privilégiée.

**arc de retour (back edge)**: Un arc du graphe de flot de contrôle d'un bloc vers un bloc qui le domine ; il signale une boucle.

**assembleur (assembler)**: Un programme qui traduit le code source en langage d'assemblage en code machine stocké dans un fichier objet.

**assembly (.NET)**: Un programme ou une bibliothèque .NET : un fichier PE qui contient du langage intermédiaire et des métadonnées.

**binaire sans symboles (stripped binary)**: Un exécutable dont les noms de symboles et les informations de débogage ont été supprimés.

**bloc (chunk)**: Une partie étiquetée d'un fichier : une étiquette qui dit ce que c'est, une longueur et une charge utile.

**bloc de base (basic block)**: Une suite d'instructions avec une entrée et une sortie : le contrôle entre à la première instruction et sort après la dernière.

**boucle de répartition (dispatch loop)**: La boucle centrale d'un interpréteur : elle récupère un code opération, choisit son gestionnaire et l'exécute, puis recommence.

**boutisme (endianness)**: L'ordre dans lequel les octets d'une valeur multi-octets sont stockés en mémoire.

**bytecode (bytecode)**: Un format compact d'instructions qu'un interpréteur lit et exécute ; pour la machine qui l'exécute, ce sont des données et non du code machine.

**cadre de pile (stack frame)**: La partie de la pile qui appartient à un appel de fonction.

**chaîne de pile (stack string)**: Une chaîne que le code construit octet par octet à partir de constantes, de sorte qu'elle n'existe jamais d'un seul tenant dans le fichier.

**code managé (managed code)**: Du code qui s'exécute sous un environnement d'exécution comme .NET, qui le charge, le compile et gère sa mémoire.

**code opération (opcode)**: Le nombre d'une instruction qui sélectionne l'opération à effectuer.

**compilation juste-à-temps (just-in-time compilation)**: La traduction du langage intermédiaire en code machine pendant l'exécution du programme, méthode par méthode, à la première utilisation de la méthode.

**complément à deux (two's complement)**: La représentation standard des entiers signés : pour opposer un nombre, on inverse ses bits puis on ajoute un.

**convention d'appel (calling convention)**: La convention qui précise où l'appelant place les arguments, où l'appelé renvoie son résultat et quels registres chaque partie doit préserver.

**descripteur opaque (opaque handle)**: Un pointeur qu'une bibliothèque renvoie et attend en retour, dont l'appelant ne lit ni n'écrit jamais le contenu ; il identifie une ressource qui doit être libérée.

**débogueur (debugger)**: Un outil qui exécute un programme sous contrôle afin de le mettre en pause, de l'exécuter pas à pas et d'inspecter les registres et la mémoire.

**débordement d'entier (integer overflow)**: Un résultat arithmétique qui ne tient pas dans son type et reboucle, de sorte qu'une taille ou un indice devient bien plus petit que prévu.

**décompilateur (decompiler)**: Un outil qui reconvertit le code machine en code source approximatif de haut niveau, par exemple en C.

**désassembleur (disassembler)**: Un outil qui reconvertit les octets du code machine en instructions d'assemblage.

**erreur d'un (off-by-one error)**: Une limite fausse d'une unité, comme une vérification qui accepte une longueur égale à la capacité alors que la copie a aussi besoin de place pour un terminateur.

**exportation (export)**: Une fonction ou une variable qu'une bibliothèque met à la disposition d'autres programmes et qui figure dans sa table de symboles dynamiques.

**graphe de flot de contrôle (control-flow graph)**: Les blocs d'une fonction comme nœuds et les sauts entre eux comme arcs.

**indicateur (flag)**: Un bit du registre d'indicateurs qui enregistre une propriété du dernier résultat, comme zéro ou retenue.

**indicateur (indicator)**: Un fait observable, comme une chaîne, un hachage, une fonction importée ou un comportement, qui désigne un programme, une famille ou une activité (ce n'est pas l'indicateur du processeur).

**instruction**: Une commande unitaire que le processeur peut exécuter, composée d'un code opération et de zéro, un ou plusieurs opérandes.

**interpréteur (interpreter)**: Un programme qui lit des instructions dans un autre format, comme le bytecode, et les exécute une par une.

**langage intermédiaire (intermediate language)**: Le jeu d'instructions que produisent les compilateurs managés, aussi appelé IL ou CIL ; un environnement d'exécution le compile en code machine quand il en a besoin.

**machine virtuelle (virtual machine)**: Un programme qui se comporte comme un processeur, avec son propre compteur ordinal, son stockage et son jeu d'instructions, et qui exécute du bytecode.

**mode d'adressage (addressing mode)**: La règle qu'utilise une instruction pour indiquer où se trouve un opérande : dans l'instruction elle-même, dans un registre ou à une adresse mémoire calculée.

**métadonnées (metadata)**: Les tables et les tas d'un assembly managé qui décrivent ses types, ses méthodes, ses champs, ses références et ses littéraux.

**note de détection (detection note)**: Une courte description, destinée aux défenseurs, d'un comportement à rechercher : ce qui est observé et où, pourquoi c'est suspect, ce qui y ressemble mais est inoffensif, et à quelle technique il correspond.

**obscurcissement (obfuscation)**: Modifier un programme pour le rendre plus difficile à lire sans changer ce qu'il fait.

**opérande (operand)**: Une valeur sur laquelle travaille une instruction : une constante, un registre ou un emplacement mémoire.

**paramètre de sortie (output parameter)**: Un argument pointeur à travers lequel une fonction ne fait qu'écrire, afin que l'appelant reçoive un résultat dans une variable qui lui appartient.

**persistance (persistence)**: Un moyen pour un programme de redémarrer après un redémarrage ou une fermeture de session, comme une valeur Run du registre ou une tâche planifiée.

**pile (stack)**: Une zone de mémoire utilisée comme structure dernier entré, premier sorti pour les adresses de retour, les arguments et les variables locales.

**point d'arrêt (breakpoint)**: Un repère qui fait s'arrêter le programme dans le débogueur lorsque l'exécution atteint une adresse choisie.

**point d'entrée (entry point)**: L'adresse de la première instruction exécutée au démarrage d'un programme.

**prologue**: Les instructions au début d'une fonction qui sauvegardent des registres et mettent en place son cadre de pile.

**prédicat opaque (opaque predicate)**: Une condition qui a toujours la même valeur mais qu'il est difficile de voir d'un coup d'œil, de sorte qu'un côté de la branche est du code mort.

**puits (sink)**: Un endroit d'un programme où une valeur décide de la quantité de mémoire touchée, comme une copie, un indice, une allocation ou une libération.

**registre (register)**: Un petit emplacement de stockage très rapide à l'intérieur du processeur.

**règle YARA (YARA rule)**: Une règle avec des chaînes et une condition qui compare des fichiers à des motifs et sert à trouver des programmes d'un même genre.

**rétro-ingénierie**: Retrouver le fonctionnement d'un programme ou d'un produit à partir du produit fini lui-même, sans ses documents de conception.

**sanitizer (sanitizer)**: Une fonction du compilateur qui ajoute des vérifications à un programme et l'arrête avec un rapport au premier accès mémoire incorrect.

**signature (signature)**: Le type de retour, le nom et les types des paramètres d'une fonction ; pour une fonction non documentée, elle est retrouvée d'après la façon dont son code utilise les registres.

**somme de contrôle (checksum)**: Une valeur calculée à partir des octets d'un fichier ou d'un message qui permet à un lecteur de détecter des dégâts ou des modifications.

**triage**: Un premier examen rapide d'un fichier, sans l'exécuter, pour identifier ce qu'il est et décider où chercher ensuite.

**utilisation après libération (use after free)**: Utiliser de la mémoire à travers un pointeur après que la mémoire a été libérée.

**valeur magique (magic value)**: Un identifiant fixe au début d'un format de fichier qui dit à un lecteur de quel format il s'agit.

**éditeur de liens (linker)**: Un programme qui assemble des fichiers objets et des bibliothèques en un exécutable et résout les références entre symboles.

**élévation (lifting)**: L'étape d'un décompilateur qui convertit les instructions machine en opérations sur des variables, et les indicateurs en conditions.

**épilogue (epilogue)**: Les instructions à la fin d'une fonction qui restaurent les registres sauvegardés et la pile, puis retournent à l'appelant.
