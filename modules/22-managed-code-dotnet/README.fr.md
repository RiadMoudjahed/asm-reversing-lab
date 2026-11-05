<!-- source: README.md @ sha256:af16eef57f28 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 22. Code managé : .NET

Durée : environ 2 heures. Prérequis : [21. Machines virtuelles et bytecode](../21-virtual-machines-bytecode/README.fr.md)

## Pourquoi c'est important

Une grande partie des logiciels que vous rencontrerez n'est pas compilée en code machine. Les programmes écrits pour .NET sont compilés dans un langage intermédiaire qu'un environnement d'exécution traduit pendant l'exécution du programme. Un tel fichier conserve l'essentiel de ce que l'auteur a écrit : les noms des types, des méthodes et des champs, les signatures et la structure du code. Cela rend le code managé bien plus facile à lire que le code natif, et c'est pourquoi un décompilateur peut souvent restituer un source proche de l'original. Cela signifie aussi que les habitudes de l'analyse native ne se transposent pas telles quelles : il n'y a pas de registres à suivre ni de cadres de pile à reconstruire, et les indices utiles se trouvent dans des tables de métadonnées et dans un jeu d'instructions de machine à pile. Ce module enseigne comment un programme managé est organisé, comment lire ses métadonnées et son code intermédiaire, comment utiliser un décompilateur et ce que le décompilateur réussit ou rate. Tous les exemples sont des programmes inoffensifs écrits pour la leçon.

## Concepts

### Code managé et code natif

Le tableau indique ce qui change quand la cible est managée.

| | Code natif | Code managé (.NET) |
|---|---|---|
| Ce que contient le fichier | Des instructions machine pour un processeur | Du langage intermédiaire (IL, aussi appelé CIL) et des métadonnées, pour n'importe quel processeur |
| Qui l'exécute | Le processeur, une fois le fichier mappé par le chargeur | Un environnement d'exécution qui charge le fichier, compile l'IL en code machine à la première exécution d'une méthode (compilation juste-à-temps) et exécute le résultat |
| Noms | Supprimés par le dépouillement | Conservés dans les métadonnées, parce que l'environnement d'exécution en a besoin pour les types, les méthodes et les champs |
| Types | Retrouvés par l'analyse | Enregistrés : chaque méthode a une signature typée, chaque variable locale a un type |
| Mémoire | Le programme la gère | L'environnement d'exécution la gère (un ramasse-miettes) ; il n'y a pas de libération explicite |
| Lecture | Désassembleur, puis raisonnement sur les registres et la pile | Explorateur de métadonnées, listing d'IL et décompilateur qui fonctionne bien |

Certains programmes managés sont compilés à l'avance en code machine, et certains contiennent des parties natives qu'ils appellent. Ces cas demandent les méthodes natives des modules précédents. Ce module traite le cas courant d'un assembly d'IL pur.

### Assemblies et conteneur de fichier

Un programme ou une bibliothèque .NET est un assembly. Sous Windows, c'est un fichier PE comme un autre, avec un petit ajout : un pointeur vers un en-tête CLR, qui contient la version de l'environnement d'exécution et l'emplacement des métadonnées. Le fichier se reconnaît facilement lors du triage :

```bash
make
file build/Parcels.dll
python3 22_meta.py build/Parcels.dll
```

```text
build/Parcels.dll: PE32 executable (console) Intel 80386 Mono/.Net assembly, for MS Windows, 3 sections
CLR header: rva 0x2008, size 72
  runtime version 2.5, flags 0x1, entry point token 0x06000006
metadata: signature BSJB, version string v4.0.30319
  stream #~       offset 0x0006c  size 816
  stream #Strings offset 0x0039c  size 892
  stream #US      offset 0x00718  size 184
  stream #GUID    offset 0x007d0  size 16
  stream #Blob    offset 0x007e0  size 332
table rows: Module 1, TypeRef 19, TypeDef 8, Field 8, MethodDef 8, Param 7, MemberRef 19, CustomAttribute 7, Assembly 1, AssemblyRef 2
user strings (#US heap):
  +0x002  ' parcel, '
  +0x016  ' g, cost '
  +0x02a  'NE'
  +0x030  'NW'
  +0x036  'SE'
  +0x03c  'north'
  +0x048  'south'
  +0x054  'east'
  +0x05e  'west'
  +0x068  'northeast'
  +0x07c  'northwest'
  +0x090  'southeast'
  +0x0a4  'unknown'
  +0x0b4  ','
```

Les signes d'un assembly managé sont : `file` indique `Mono/.Net assembly`, la table des imports a une seule entrée de `mscoree.dll` (`_CorExeMain` pour un programme), le répertoire de données numéro 14 n'est pas vide, et les métadonnées commencent par la signature `BSJB` suivie d'une chaîne de version. Le script `examples/22_meta.py` lit ces structures avec une bibliothèque PE et rien d'autre.

### Métadonnées

Les métadonnées sont un ensemble de tables et de tas (les flux de la sortie ci-dessus). Les tables décrivent ce que contient l'assembly :

| Table | Contient | Lignes dans l'exemple |
|---|---|---|
| TypeDef | Les types définis ici : classes, structures, énumérations | 8 |
| MethodDef | Les méthodes définies ici, avec leur signature et l'emplacement de leur IL | 8 |
| Field | Les champs définis ici, avec leurs types | 8 |
| TypeRef, MemberRef | Les types et les membres que l'assembly utilise depuis d'autres assemblies | 19 et 19 |
| AssemblyRef | Les autres assemblies qu'il référence | 2 |
| CustomAttribute | Les attributs attachés aux types et aux méthodes, dont beaucoup ajoutés par le compilateur | 7 |

Les tas contiennent les données vers lesquelles pointent les tables. `#Strings` contient les noms (types, méthodes, champs) en UTF-8, de sorte qu'une exécution simple de `strings` les trouve, comme elle a trouvé `WriteLine`, `AppendFormatted` et `<>c__DisplayClass1_0` dans l'exemple. `#US` contient les littéraux de chaîne du programme, les chaînes utilisateur, en UTF-16, et c'est pourquoi les littéraux d'un programme managé n'apparaissent pas dans la sortie de `strings` avec ses réglages par défaut. Utilisez `strings -el` pour voir le texte UTF-16, ou lisez le tas avec le script comme ci-dessus. `#Blob` contient les signatures et les données constantes, et `#GUID` contient des identifiants.

Trois conséquences comptent pour l'analyse. D'abord, les références à d'autres assemblies montrent ce que le programme peut faire : une référence aux bibliothèques réseau, de fichiers ou de processus est l'équivalent managé d'une table des imports, et la liste des lignes `MemberRef` nomme les méthodes exactes appelées. Ensuite, comme les noms sont des données de `#Strings`, un programme qui veut cacher son but renomme ses types et ses méthodes avec des noms courts sans signification ; cela change la lecture mais pas ce qu'il fait. Enfin, le compilateur ajoute beaucoup de choses que l'auteur n'a pas écrites, et leurs noms le révèlent : un nom qui commence par `<` ou contient `>` est généré, par exemple `<>c__DisplayClass1_0` et `<Surcharges>b__0` dans l'exemple, qui sont la classe auxiliaire et la méthode d'une lambda.

### Le langage intermédiaire

L'IL est le jeu d'instructions d'une machine à pile, ce qui le rapproche de la machine du module 21. Les opérandes sont empilés sur une pile d'évaluation, une instruction prend ses entrées au sommet de la pile et y dépose son résultat, et les instructions n'ont pas d'opérandes de registre. Les arguments et les variables locales vivent dans des emplacements numérotés et sont déplacés vers la pile et en sens inverse. Le code de la méthode `Cost` du programme d'exemple commence ainsi (le listing complet est produit dans la section suivante) :

```text
IL_0000: ldarg.0
IL_0001: ldfld int32 Parcels.Parcel::weightGrams
IL_0006: ldc.i4 499
IL_000b: add
IL_000c: ldc.i4 500
IL_0011: div
IL_0012: stloc.0
```

| Instruction | Effet | Pile après |
|---|---|---|
| `ldarg.0` | Empile l'argument 0, qui est `this` dans une méthode d'instance | this |
| `ldfld ...::weightGrams` | Dépile l'objet, empile la valeur de son champ | weightGrams |
| `ldc.i4 499` | Empile la constante de 32 bits 499 | weightGrams, 499 |
| `add` | Dépile deux valeurs, empile leur somme | weightGrams + 499 |
| `ldc.i4 500` | Empile 500 | weightGrams + 499, 500 |
| `div` | Dépile deux valeurs, empile le quotient | le quotient |
| `stloc.0` | Dépile dans la variable locale 0 | (vide) |

C'est la ligne `int units = (weightGrams + 499) / 500;` du source. Les groupes d'instructions que vous verrez le plus souvent :

| Groupe | Instructions | Signification |
|---|---|---|
| Chargement et rangement | `ldarg`, `ldloc`, `stloc`, `ldfld`, `stfld`, `ldc.i4`, `ldstr` | Déplacer arguments, variables locales, champs, constantes et littéraux de chaîne vers la pile et depuis elle |
| Arithmétique | `add`, `sub`, `mul`, `div`, `rem`, `neg`, `and`, `or`, `xor`, `shl`, `shr` | Opèrent sur la ou les deux valeurs du sommet |
| Branchements | `br`, `brtrue`, `brfalse`, `beq`, `bgt`, `blt`, `switch` | Sauter, sans condition ou selon une condition ; les cibles sont des décalages d'IL |
| Appels | `call`, `callvirt`, `newobj`, `ret` | Appeler une méthode (le jeton de métadonnées dit laquelle), créer un objet, retourner |
| Conversions et tableaux | `conv.i8`, `ldelem`, `stelem`, `ldlen` | Changer la largeur d'un nombre, accéder aux tableaux |

Le suffixe `.s` (par exemple `ldloc.s`, `br.s`) désigne une forme courte avec un opérande d'un octet, et `ldarg.0`, `ldc.i4.1` et les autres sont des formes courtes avec l'opérande dans le code opération. Elles ne changent pas le sens. L'IL porte des types : les variables locales ont des types déclarés, et `ldfld` et `call` portent le nom complet du champ ou de la méthode. Dans une version sans informations de débogage, les variables locales n'ont pas de noms, seulement des numéros d'emplacement.

#### Lire une méthode

La méthode `Cost` complète est ci-dessous, produite par le mode IL du décompilateur. Lisez-la en gardant la structure du source à l'esprit :

```bash
ilspycmd -il -t Parcels.Parcel build/Parcels.dll 2>/dev/null | sed -n '/instance int32 Cost/,/end of method Parcel::Cost/p'
```

```text
        instance int32 Cost () cil managed 
    {
        // Method begins at RVA 0x2068
        // Header size: 12
        // Code size: 85 (0x55)
        .maxstack 2
        .locals init (
            [0] int32,
            [1] int32,
            [2] int32,
            [3] int32,
            [4] valuetype Parcels.Zone,
            [5] int32
        )

        IL_0000: ldarg.0
        IL_0001: ldfld int32 Parcels.Parcel::weightGrams
        IL_0006: ldc.i4 499
        IL_000b: add
        IL_000c: ldc.i4 500
        IL_0011: div
        IL_0012: stloc.0
        IL_0013: ldarg.0
        IL_0014: ldfld valuetype Parcels.Zone Parcels.Parcel::zone
        IL_0019: stloc.s 4
        IL_001b: ldloc.s 4
        IL_001d: ldc.i4.1
        IL_001e: beq.s IL_0027

        IL_0020: ldloc.s 4
        IL_0022: ldc.i4.2
        IL_0023: beq.s IL_002b

        IL_0025: br.s IL_002f

        IL_0027: ldc.i4.3
        IL_0028: stloc.3
        IL_0029: br.s IL_0032

        IL_002b: ldc.i4.5
        IL_002c: stloc.3
        IL_002d: br.s IL_0032

        IL_002f: ldc.i4.s 9
        IL_0031: stloc.3

        IL_0032: ldloc.3
        IL_0033: stloc.1
        IL_0034: ldc.i4.0
        IL_0035: stloc.2
        IL_0036: ldc.i4.0
        IL_0037: stloc.s 5
        IL_0039: br.s IL_004e
        // loop start (head: IL_004e)
            IL_003b: ldloc.2
            IL_003c: ldloc.1
            IL_003d: add
            IL_003e: stloc.2
            IL_003f: ldloc.s 5
            IL_0041: ldc.i4.4
            IL_0042: blt.s IL_0048

            IL_0044: ldloc.2
            IL_0045: ldc.i4.1
            IL_0046: add
            IL_0047: stloc.2

            IL_0048: ldloc.s 5
            IL_004a: ldc.i4.1
            IL_004b: add
            IL_004c: stloc.s 5

            IL_004e: ldloc.s 5
            IL_0050: ldloc.0
            IL_0051: blt.s IL_003b
        // end loop

        IL_0053: ldloc.2
        IL_0054: ret
    } // end of method Parcel::Cost
```

La structure est visible même sans le source. La variable locale 0 reçoit le nombre d'unités. Les deux comparaisons de la valeur de zone avec 1 et 2 sélectionnent les constantes 3 et 5, et le cas restant range 9 : c'est un choix à trois voies avec les valeurs de zone comme clés, rangé dans la variable 3 et copié dans la variable 1, le tarif. Le saut en IL_0039 va au test de la boucle en IL_004e, qui compare la variable 5 à la variable 0 et revient en IL_003b tant que le compteur est plus petit. Le corps ajoute le tarif (variable 1) au total (variable 2), et le `blt.s` en IL_0042 saute l'incrément supplémentaire tant que le compteur est inférieur à 4. La variable 2 est renvoyée. Les emplacements ne disent pas comment s'appelle chaque variable ; les rôles se déduisent de l'usage, comme vous nommiez les registres en code natif.

La ligne `.maxstack 2` indique la profondeur maximale de la pile d'évaluation, ce qui permet de vérifier votre lecture : aucune séquence de la méthode ne laisse plus de deux valeurs sur la pile.

### Décompiler avec ILSpy

Un décompilateur retraduit l'IL dans un langage de haut niveau, en général C#. L'image du laboratoire contient `ilspycmd`, la forme en ligne de commande d'ILSpy ; l'ILSpy graphique donne les mêmes résultats avec un arbre des types et un accès direct aux définitions. Commandes utiles :

| Commande | Sortie |
|---|---|
| `ilspycmd Assembly.dll` | L'assembly entier en C# sur la sortie standard |
| `ilspycmd -l c Assembly.dll` | Une liste des classes |
| `ilspycmd -t Namespace.Type Assembly.dll` | Un type |
| `ilspycmd -il -t Namespace.Type Assembly.dll` | L'IL d'un type |
| `ilspycmd -p -o dir Assembly.dll` | Un projet complet, à lire dans un éditeur |
| `ilspycmd -usepdb Assembly.dll` | Utiliser les noms des variables locales du fichier de débogage placé à côté de l'assembly |

Les classes du programme d'exemple :

```bash
ilspycmd -l c build/Parcels.dll 2>/dev/null
```

```text
Class <Module>
Class Parcels.Parcel
Class Parcels.Routes
Class Parcels.Program
Class <PrivateImplementationDetails>
Class Parcels.Routes.<>c__DisplayClass1_0
```

Trois des six sont générées par le compilateur : `<Module>` contient des données au niveau du module, `<PrivateImplementationDetails>` contient des données constantes comme l'initialiseur de tableau, et la classe d'affichage contient la variable capturée de la lambda. La méthode `Cost` décompilée :

```bash
ilspycmd -t Parcels.Parcel build/Parcels.dll 2>/dev/null | sed -n '/public int Cost/,/^\t}/p'
```

```text
    public int Cost()
    {
        int num = (weightGrams + 499) / 500;
        int num2 = zone switch
        {
            Zone.Local => 3, 
            Zone.Regional => 5, 
            _ => 9, 
        };
        int num3 = 0;
        for (int i = 0; i < num; i++)
        {
            num3 += num2;
            if (i >= 4)
            {
                num3++;
            }
        }
        return num3;
    }
```

C'est la méthode du source avec trois différences : les variables locales s'appellent `num`, `num2`, `num3`, le commentaire a disparu, et `total += 1` est devenu `num3++`. La structure, y compris le choix par zone et la boucle, est la même. Le décompilateur a retrouvé la boucle et le choix à partir des branchements visibles dans l'IL ci-dessus.

### Ce qu'un décompilateur retrouve et ce qu'il perd

La comparaison du source et du programme décompilé est la meilleure façon d'apprendre à qui se fier. D'après l'exemple :

| Élément | Dans le source | Dans la sortie décompilée | Pourquoi |
|---|---|---|---|
| Noms de types, de méthodes et de champs | `Parcel`, `Cost`, `weightGrams` | Identiques | Ce sont des métadonnées |
| Signatures et types | `int Cost()` | Identiques | Ce sont des métadonnées |
| Flot de contrôle | `for`, `if`, une expression `switch` | Retrouvé sous les mêmes constructions | Les branchements s'y ramènent |
| Noms des variables locales | `units`, `rate`, `total` | `num`, `num2`, `num3` | Les noms des variables locales sont dans le fichier de débogage, pas dans l'assembly |
| Commentaires | Présents | Absents | Ils n'atteignent jamais le fichier compilé |
| `foreach` sur un tableau | `foreach (Parcel p in parcels)` | `for (int i = 0; i < array.Length; i++)` avec `array[i]` | Le compilateur en a déjà fait une boucle indexée |
| `total += 1` | Tel qu'écrit | `num3++` | Le compilateur émet la même IL ; le décompilateur choisit une forme |
| Switch sur chaîne | `switch (code) { case "N": ... }` | Une expression `switch` avec les mêmes cas | Le compilateur a testé la longueur puis les caractères ; le décompilateur reconnaît ce motif et restitue les chaînes |
| Lambda | Une lambda qui capture `limit` | Une lambda `(int w) => ...` | Le décompilateur masque la classe d'affichage ; l'IL la montre |

Quand le même programme est compilé avec un fichier de débogage, le décompilateur peut en tirer les noms des variables locales :

```bash
ilspycmd -usepdb -t Parcels.Parcel build/debug/Parcels.dll 2>/dev/null | sed -n '/public int Cost/,/^\t}/p'
```

```text
    public int Cost()
    {
        int units = (weightGrams + 499) / 500;
        int rate = zone switch
        {
            Zone.Local => 3, 
            Zone.Regional => 5, 
            _ => 9, 
        };
        int total = 0;
        for (int i = 0; i < units; i++)
        {
            total += rate;
            if (i >= 4)
            {
                total++;
            }
        }
        return total;
    }
```

Les noms `units`, `rate` et `total` reviennent. Les analystes n'ont presque jamais le fichier de débogage d'un échantillon ; attendez-vous donc aux noms génériques et nommez vous-même les variables à mesure que vous comprenez ce qu'elles font.

Un autre détail vient des chaînes utilisateur du premier listing. Le switch a sept cas, et les chaînes `N`, `S`, `E` et `W` n'apparaissent pas dans le tas `#US`, alors que `NE`, `NW` et `SE` y figurent. Le compilateur compare les chaînes d'un seul caractère par leur longueur et leur caractère au lieu de stocker un littéral, et le décompilateur retransforme la comparaison en littéral dans sa sortie. Le source décompilé contient donc des littéraux que le tas de chaînes ne contient pas. Une recherche de chaînes dans le fichier est une bonne première étape et non une liste complète.

### Lire du code managé obscurci

Les méthodes du module 20 s'appliquent avec des changements. Les obscurcisseurs managés renomment couramment les types, les méthodes et les champs avec des noms courts sans signification, chiffrent les littéraux de chaîne pour qu'une méthode les décode à l'exécution, aplatissent le flot de contrôle et ajoutent du bourrage. Comparez-les aux cas natifs :

- **Symboles renommés.** La sortie du décompilateur contient des noms comme `a`, `b`, `c` ou des caractères non imprimables. La structure et les types sont toujours là. Renommez à mesure que vous apprenez : ILSpy permet de chercher par signature, et les références de `MemberRef` vous disent toujours ce que le programme appelle dans le framework.
- **Chaînes chiffrées.** Le tas `#US` a peu de littéraux et le code contient beaucoup d'appels à une petite méthode qui prend un entier ou un tableau d'octets et renvoie une chaîne. C'est le décodeur du module 20, écrit en IL. Vous pouvez lire la méthode et la réimplémenter, ou l'exécuter : l'assembly est un programme, et un petit script ou un banc de test dans le laboratoire isolé peut appeler le décodeur avec les arguments que vous avez lus aux sites d'appel.
- **Flot de contrôle aplati.** La même structure de répartiteur et d'état que dans le module 20, lisible en IL comme un `switch` sur une variable locale dans une boucle.
- **Appels par réflexion.** Un programme peut appeler une méthode par un nom qu'il construit à l'exécution. La liste `MemberRef` ne le montrera pas ; cherchez les appels du framework qui trouvent un type ou une méthode par son nom.

Comme l'environnement d'exécution peut retrouver l'IL de n'importe quelle méthode, rien de la logique n'est caché à un lecteur déterminé ; le travail consiste à retirer le bruit. N'exécutez un assembly inconnu que dans le laboratoire isolé du module 19.

### Un ordre de travail

1. **Triage.** Confirmez que le fichier est managé. Listez les classes, les assemblies référencés et les lignes `MemberRef`. Lisez les chaînes des tas `#Strings` et `#US`.
2. **Décompiler.** Lisez d'abord le C# du point d'entrée (le jeton du point d'entrée de l'en-tête CLR nomme la méthode), puis suivez les appels.
3. **Vérifier les noms.** Si les noms sont sans signification, renommez au fur et à mesure et notez ce que fait chaque type.
4. **Vérifier la sortie avec l'IL** là où le code décompilé semble étrange, ou quand un détail décide de la réponse.
5. **Chercher le code généré.** Les noms avec `<` et `>` sont des aides du compilateur : lambdas, itérateurs, méthodes asynchrones. Lisez-les comme le code qu'ils implémentent.
6. **Traiter la protection.** Trouvez le décodeur de chaînes, réimplémentez-le ou appelez-le, et notez le résultat dans le rapport.

## À essayer

Exécutez les commandes du laboratoire depuis `modules/22-managed-code-dotnet/examples`.

1. Compilez avec `make` et exécutez `dotnet build/Parcels.dll`. Lancez `22_meta.py` dessus et comparez le nombre de lignes de `MethodDef` avec le nombre de méthodes du source (les constructeurs et la méthode générée par le compilateur comptent).
2. Lancez `strings -n 5` et `strings -el -n 4` sur l'assembly. Quels littéraux du programme apparaissent dans quelle sortie ? Pourquoi `N` n'apparaît-il dans aucune ?
3. Décompilez tout l'assembly. Notez trois différences avec le source de `22_parcels/Program.cs`.
4. Affichez l'IL de `Routes.Region`. Trouvez l'appel qui lit la longueur de la chaîne et les appels qui lisent des caractères isolés. Expliquez en une phrase comment le compilateur a transformé le switch sur chaîne en comparaisons.
5. Affichez l'IL de `Routes.Surcharges`. Trouvez où la classe auxiliaire est créée et où la variable capturée est rangée. Quelle instruction charge l'adresse de la méthode de la classe auxiliaire dans le délégué ?
6. Décompilez `build/debug/Parcels.dll` avec et sans `-usepdb` et comparez les noms des variables locales.
7. Modifiez `Cost` dans le source pour que le centime supplémentaire commence après la sixième unité, recompilez et trouvez la constante modifiée dans l'IL.

## Pièges courants

- **Chercher les littéraux avec une recherche ASCII.** Les littéraux d'un programme managé sont en UTF-16 dans un tas séparé. Utilisez `strings -el` ou lisez le tas.
- **Se fier aux noms locaux.** Sans fichier de débogage, les noms sont générés. Un nom comme `num2` ne dit rien du rôle de la variable.
- **Oublier que le compilateur écrit du code.** Une classe avec `<` dans son nom est générée. Une lecture qui la traite comme une conception de l'auteur sera déroutée.
- **Lire seulement le code décompilé.** Le décompilateur fait des choix. Quand un résultat semble faux ou qu'un détail compte, lisez l'IL de cette méthode.
- **Croire que les noms veulent dire ce qu'ils disent.** Les noms sont du texte libre. Une méthode appelée `Verify` peut faire n'importe quoi.
- **Exécuter un assembly inconnu hors du laboratoire.** Lire un assembly ne l'exécute jamais. L'exécuter, si, et un programme managé a le même accès qu'un programme natif.

## Résumé

- Le code managé est de l'IL plus des métadonnées, exécuté par un environnement d'exécution qui le compile quand il le faut ; le fichier conserve les noms, les types et la structure du programme.
- Un assembly managé est un fichier PE avec un en-tête CLR, un seul import de `mscoree.dll` et des tables et tas de métadonnées ; les littéraux sont en UTF-16 dans `#US` et les noms dans `#Strings`.
- L'IL est une machine à pile avec des variables locales et des champs typés ; elle se lit comme le bytecode du module 21, et les rôles des variables locales se déduisent de leur usage.
- Un décompilateur comme ILSpy restitue du C# qui conserve types, signatures et flot de contrôle, et perd les commentaires et, sans fichier de débogage, les noms des variables locales.
- Vérifiez la sortie décompilée avec l'IL quand un détail compte, et traitez les noms avec `<` et `>` comme du code généré par le compilateur.
- Le code managé obscurci se lit avec les méthodes du module 20 : renommer, trouver le décodeur de chaînes et défaire l'aplatissement du flot de contrôle.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 23, les décompilateurs et leurs limites.
