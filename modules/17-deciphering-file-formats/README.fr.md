<!-- source: README.md @ sha256:312dbb9b2163 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 17. Déchiffrer les formats de fichier

Durée : environ 2,5 heures. Prérequis : [16. Interfaces non documentées](../16-undocumented-interfaces/README.fr.md)

## Pourquoi c'est important

Les programmes gardent leurs données dans des fichiers dont personne n'a écrit la structure : l'export d'un éditeur, un ancien format de sauvegarde, un journal écrit par un appareil. Pour lire un tel fichier, le convertir, le réparer ou vérifier qu'il est sûr de l'ouvrir, il vous faut la structure, et elle se cache à deux endroits : les octets de fichiers d'exemple et le code du programme qui les lit ou les écrit. Ce module enseigne à combiner les deux, à reconnaître les parties habituelles d'un format, à identifier une somme de contrôle et à transformer les constats en une spécification que vous testez ensuite avec un analyseur de votre cru.

## Concepts

### Le format d'entraînement

La leçon travaille sur un petit format de journaux de capteurs. La cible est le programme lecteur `17_reader`, construit sans symboles à partir de `examples/17_reader.c` ; la leçon n'utilise que le programme compilé et des fichiers d'exemple produits par `examples/17_make.py`.

```bash
make run
```

```text
python3 17_make.py build/sample.lg
build/17_reader build/sample.lg
channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
```

Le lecteur affiche un résumé et rien d'autre. Ce que le fichier contient vraiment, c'est à vous de le découvrir.

### Étape 1 : regarder les octets

Commencez par un vidage hexadécimal. `xxd` montre le décalage, les octets et leur forme imprimable. `strings -a -t x` liste les suites de caractères imprimables avec leurs décalages dans le fichier.

```bash
xxd build/sample.lg
strings -a -t x build/sample.lg
```

```text
00000000: 4c47 5231 0200 0200 401f 0000 4d45 5441  LGR1....@...META
00000010: 1300 0000 756e 6974 3d6d 560a 7369 7465  ....unit=mV.site
00000020: 3d6e 6f72 7468 0a44 4154 410c 0000 0064  =north.DATA....d
00000030: 00c8 002c 0190 01f4 0158 024e 4f54 4507  ...,.....X.NOTE.
00000040: 0000 0063 6865 636b 6564 454e 4420 0400  ...checkedEND ..
00000050: 0000 0898 7769                           ....wi
      0 LGR1
      c META
     14 unit=mV
     1c site=north
     27 DATA
     3b NOTE
     43 checkedEND 
```

Lisez-le comme un ensemble d'hypothèses.

| Observation | Hypothèse |
|---|---|
| Le fichier commence par `LGR1` | Une **valeur magique** : un identifiant fixe au décalage 0 qui dit à un lecteur de quel format il s'agit |
| Des mots de quatre lettres `META`, `DATA`, `NOTE`, `END` apparaissent, chacun suivi d'un petit nombre puis d'un contenu | Des blocs étiquetés : une étiquette, une longueur, une charge utile. Après `META` vient `13 00 00 00`, et le texte qui suit fait 19 octets (0x13) |
| `0c 00 00 00` après `DATA`, puis 12 octets | Le même motif : longueur 12, puis 12 octets de charge utile, soit 6 valeurs de 2 octets |
| Des nombres comme `13 00 00 00` ont l'octet de poids faible en premier | **Petit-boutiste** (little endian), l'ordre des octets de x86 (module 04) |
| Les quatre derniers octets semblent aléatoires | Une **somme de contrôle** : une valeur calculée à partir des autres octets qui détecte les dégâts |
| Les octets 4 à 11 sont `02 00 02 00 40 1f 00 00` | Un en-tête de champs aux tailles à déterminer : une valeur d'1 octet égale à 2, une valeur d'1 octet égale à 0, puis 2 et 8000 (0x1f40) sur 2 et 4 octets |

Un **champ** est une partie nommée du fichier, à position fixe ou donnée par des champs précédents. Son **décalage** compte les octets depuis le début du fichier. Sa **longueur** peut être fixe ou être stockée dans un autre champ.

### Étape 2 : ne changer qu'une chose à la fois

L'outil le plus puissant pour localiser des champs est la comparaison. Fabriquez deux échantillons qui ne diffèrent que par un paramètre et comparez les vidages. Ici le programme qui fabrique les échantillons est disponible ; avec un fichier simplement reçu, comparez deux fichiers que vous avez, ou changez vous-même un octet et regardez ce que dit le lecteur.

```bash
python3 17_make.py /tmp/r.lg --rate 16000
diff <(xxd build/sample.lg) <(xxd /tmp/r.lg)
```

```text
1c1
< 00000000: 4c47 5231 0200 0200 401f 0000 4d45 5441  LGR1....@...META
---
> 00000000: 4c47 5231 0200 0200 803e 0000 4d45 5441  LGR1.....>..META
6c6
< 00000050: 0000 0898 7769                           ....wi
---
> 00000050: 0000 5ef2 5aa3                           ..^.Z.
```

Passer le taux de 8000 à 16000 change les octets 8 et 9 (0x1f40 est devenu 0x3e80), donc ces quatre octets au décalage 8 contiennent le taux. Les quatre derniers octets ont aussi changé : ils dépendent du contenu. La même expérience avec `--channels 3` change l'octet 6 et les quatre derniers octets, ce qui place le nombre de canaux au décalage 6. Avec `--samples 7`, la longueur après `DATA` passe de 0x0c à 0x0e, une nouvelle valeur apparaît dans la charge utile, les décalages de tout ce qui suit bougent de 2 et la somme de contrôle change encore. Cela montre que la longueur compte des octets (7 échantillons de 2 octets font 14) et que les blocs se suivent sans aucun index.

### Étape 3 : trouver l'analyseur dans le programme

Les octets donnent des hypothèses ; le programme qui lit le fichier les tranche. Cherchez le code qui fait la lecture, en partant de ce qui est le plus facile à trouver.

- **Messages d'erreur.** `strings -a -t x build/17_reader` montre `bad magic`, `unsupported version`, `truncated chunk header`, `checksum mismatch`, `chunk too large`. Les messages d'un lecteur nomment ses vérifications. Trouvez le code qui référence chaque chaîne ; la condition qui y saute est la vérification.
- **Importations.** Le programme importe `fopen`, `fread` et `memcmp`. Les appels à `fread` montrent combien d'octets sont lus à la fois ; ses arguments sont la taille du champ ou de la structure suivante.
- **Constantes.** Une comparaison avec une étiquette ou une valeur magique, ou une boucle sur une table.

Dans Rizin, `axt` liste le code qui utilise une adresse, et dans Ghidra on fait de même avec les références croisées (module 11). La première partie du listing ci-dessous est `main` après l'ouverture du fichier et la construction de la table. Chaque appel à `fread` est annoté.

```text
401248:  mov    rcx,r15
40124b:  mov    edx,0xc                     ; size = 12
401250:  mov    esi,0x1                     ; one-byte items
401255:  lea    rdi,[rsp+0x34]              ; buffer
40125a:  call   fread@plt
40125f:  cmp    rax,0xc                     ; fewer than 12 bytes read: "truncated header"
401263:  jne    4012f5
401269:  mov    edx,0x4
40126e:  mov    esi,0x402047                ; a 4-byte constant in .rodata
401273:  lea    rdi,[rsp+0x34]
401278:  call   memcmp@plt                  ; compare the first 4 bytes with it
40127d:  test   eax,eax
40127f:  jne    401307                      ; different: "bad magic"
401285:  cmp    BYTE PTR [rsp+0x38],0x2     ; byte at offset 4 must be 2: "unsupported version"
40128a:  jne    401319
401290:  movzx  r14d,BYTE PTR [rsp+0x39]    ; byte at offset 5: kept for the output (flags)
```

Le programme lit un en-tête de 12 octets, comme le vidage le suggérait. La constante à l'adresse 0x402047 est dans `.rodata`. Dans ce fichier, le décalage vaut l'adresse moins l'adresse de chargement 0x400000, donc `xxd -s 0x2047 -l 4 build/17_reader` montre `LGR1`. L'octet de décalage 4 est comparé à 2 et l'octet de décalage 5 est conservé. Viennent ensuite les octets de décalages 6 et 7 et de décalages 8 à 11 ; le code les assemble par des décalages de bits en un nombre de 16 bits et un nombre de 32 bits, ce qui est l'allure du petit-boutisme dans le code :

```text
4013b4:  movzx  ecx,BYTE PTR [rsp+0x1d]     ; byte 1 of the rate
4013b9:  shl    ecx,0x8
4013bc:  movzx  eax,BYTE PTR [rsp+0x1e]     ; byte 2
4013c1:  shl    eax,0x10
4013c4:  or     ecx,eax
4013c6:  movzx  eax,BYTE PTR [rsp+0x1c]     ; byte 0
4013cb:  or     ecx,eax
...
```

La boucle sur les blocs suit :

```text
40143b:  mov    rcx,r15
40143e:  mov    edx,0x8                     ; read 8 bytes: a chunk header
401443:  mov    esi,0x1
401448:  lea    rdi,[rsp+0x2c]
40144d:  call   fread@plt
401452:  cmp    rax,0x8
401456:  jne    40132b                      ; "truncated chunk header"
...                                         ; bytes 4 to 7 combined into a 32-bit length in ebp
40147f:  mov    edx,0x4
401484:  mov    esi,0x402081                ; the tag "END "
401489:  lea    rdi,[rsp+0x2c]
40148e:  call   memcmp@plt                  ; first 4 bytes against "END "
401493:  mov    r13d,eax
401496:  test   eax,eax
401498:  je     40133d                      ; equal: handle the end chunk
40149e:  cmp    ebp,0x10000
4014a4:  ja     401413                      ; length above 65536: "chunk too large"
4014aa:  mov    r14d,ebp
4014ad:  mov    rcx,r15
4014b0:  mov    rdx,r14                     ; read length bytes
4014b3:  mov    esi,0x1
4014b8:  mov    edi,0x404060                ; into a static buffer
4014bd:  call   fread@plt
4014c5:  cmp    r14,rax
4014c8:  jne    401425                      ; fewer than length bytes: "truncated chunk"
```

Chaque en-tête de bloc fait 8 octets : une étiquette de 4 octets suivie d'une longueur de 4 octets en petit-boutiste. La charge utile est lue selon la longueur, avec une limite supérieure de 0x10000 octets. Rien dans la boucle n'aligne ni ne remplit, ce qui concorde avec le vidage. Après la charge utile, le code appelle deux fois une fonction (en 0x4014db et 0x4014ea) : une fois avec les 8 octets d'en-tête et une fois avec la charge utile, et les résultats s'enchaînent par `r12d`. C'est une valeur cumulée sur tout ce qui a été lu jusque-là. C'est la somme de contrôle.

Après la mise à jour de la somme de contrôle, trois autres appels à `memcmp` comparent l'étiquette à `DATA`, `META` et `NOTE`. Seules ces trois étiquettes sont comptées ; toute autre étiquette est lue et incluse dans la somme de contrôle, mais ignorée pour le reste. C'est un fait sur le format qu'aucun échantillon ne pourrait montrer.

### Étape 4 : identifier la somme de contrôle

Une somme de contrôle est une fonction sur des octets. Les courantes se reconnaissent à leurs constantes et à leur structure.

| Type | Comment la reconnaître |
|---|---|
| Somme simple ou xor des octets | Une boucle avec `add` ou `xor` de chaque octet dans un accumulateur |
| Adler-32 | Deux sommes cumulées, modulo 65521 (0xfff1) |
| CRC-32 | La constante 0xedb88320 (ou 0x04c11db7), ou une table de 256 valeurs qui commence par 0x00000000, 0x77073096, 0xee0e612c, 0x990951ba |

Dans `17_reader`, la fonction en 0x401166 est une boucle qui traite un octet par tour :

```text
401173:  mov    edx,eax                     ; eax holds the running value
401175:  xor    dl,BYTE PTR [rcx]           ; low byte xor the next input byte
401177:  movzx  edx,dl                      ; ... as an index from 0 to 255
40117a:  shr    eax,0x8                     ; running value shifted right by one byte
40117d:  xor    eax,DWORD PTR [rdx*4+0x414060]   ; xor the table entry for that index
401184:  add    rcx,0x1
401188:  cmp    rcx,rsi
40118b:  jne    401173
```

Une boucle antérieure remplit la table, et elle contient l'instruction `and eax,0xedb88320` :

```text
401231:  mov    eax,edx
401233:  and    eax,0x1
401236:  neg    eax                         ; 0 or 0xffffffff
401238:  and    eax,0xedb88320              ; the polynomial, reflected
40123d:  shr    edx,1
40123f:  xor    edx,eax
401241:  sub    ebx,0x1
401244:  jne    401231                      ; eight rounds for each table entry
```

C'est la façon habituelle de construire la table du CRC-32, et les premières valeurs sont celles données plus haut. Trois détails d'une somme de contrôle doivent être fixés parce qu'ils changent le résultat : la valeur initiale (ici `0xffffffff`, l'argument `edi` du premier appel), l'étape finale (ici `not r12d` avant la comparaison avec la valeur stockée, en 0x401377) et la plage d'octets couverte. La plage vient de la boucle : l'en-tête (12 octets) et chaque bloc avant le bloc de fin, chacun avec son en-tête de 8 octets. Le bloc de fin lui-même n'est pas inclus. La valeur stockée est lue comme quatre octets, octet de poids faible d'abord. Le `zlib.crc32` de Python implémente le même CRC-32 avec cette valeur initiale et cette étape finale, de sorte que la règle peut être testée directement :

```bash
python3 -c "import zlib;d=open('build/sample.lg','rb').read();print(hex(zlib.crc32(d[:-12])), d[-4:].hex())"
```

```text
0x69779808 08987769
```

La valeur calculée 0x69779808 égale les octets stockés `08 98 77 69` lus en petit-boutiste. Quand vous ne pouvez pas déduire la plage du code, essayez les candidates (le fichier entier sans les quatre derniers octets, sans l'en-tête, sans le bloc de fin) et voyez laquelle correspond à l'échantillon.

### Étape 5 : écrire la spécification

La spécification est le produit du travail. Elle doit être assez précise pour que quelqu'un d'autre puisse en tirer un lecteur ou un écrivain, et elle doit dire ce qui n'est pas connu.

| Décalage | Taille | Champ | Remarques |
|---|---|---|---|
| 0 | 4 | Magique | Les octets `LGR1` |
| 4 | 1 | Version | Doit valoir 2 ; les autres valeurs sont refusées |
| 5 | 1 | Indicateurs | Conservés et affichés ; le lecteur ne les interprète pas |
| 6 | 2 | Canaux | Petit-boutiste ; affiché |
| 8 | 4 | Taux en Hz | Petit-boutiste ; affiché |
| 12 | | Blocs | L'un après l'autre, sans remplissage, jusqu'au bloc de fin |

Structure d'un bloc : une étiquette de 4 octets, une longueur de 4 octets en petit-boutiste d'au plus 65536, puis la charge utile. Étiquettes : `DATA` (comptée comme longueur / 2 échantillons), `META` et `NOTE` (comptées), toute autre étiquette (ignorée, mais couverte par la somme de contrôle). Le bloc `END` (son étiquette est formée des trois lettres suivies d'un espace) a une longueur de 4 et une charge utile contenant le CRC-32 (polynôme 0xedb88320, valeur initiale 0xffffffff, complément final) de tous les octets depuis le décalage 0 jusqu'au début du bloc `END`, stocké en petit-boutiste. Les octets après le bloc de fin ne sont pas lus. Non déterminé : ce que signifient les indicateurs et comment la charge utile de `DATA` est organisée entre les canaux, parce que le lecteur n'y touche jamais.

Indiquez les vérifications dans l'ordre où le lecteur les fait, car l'ordre décide quelle erreur produit un fichier endommagé : taille de l'en-tête, valeur magique, version, puis pour chaque bloc la taille de son en-tête, le bloc de fin, la limite de longueur, la taille de la charge utile et, à la fin, la somme de contrôle.

### Étape 6 : valider avec votre propre analyseur

Une spécification se teste en l'implémentant indépendamment et en la comparant à l'original sur des fichiers que l'original accepte et sur des fichiers qu'il refuse. Le fichier `examples/17_parse.py` est écrit uniquement d'après le tableau ci-dessus. Il peut afficher ce qu'il décode :

```bash
python3 17_parse.py build/sample.lg --dump
```

```text
0x0000  header: version=2 flags=0x00 channels=2 rate=8000
0x000c  META  length=19
0x0027  DATA  length=12
0x003b  NOTE  length=7
0x004a  END   length=4 crc stored=0x69779808 computed=0x69779808
channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
```

Le script `examples/17_compare.py` fabrique des variantes endommagées de l'échantillon (une valeur magique, une version, un taux ou un octet d'indicateurs modifié ; un octet de charge utile modifié ; une troncature à plusieurs endroits ; des octets en plus ; une longueur impossible) et compare le verdict du lecteur original avec celui de l'analyseur :

```bash
python3 17_compare.py build/17_reader build/sample.lg
```

```text
same  unchanged                    channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
same  empty file                   error: truncated header
same  magic changed                error: bad magic
same  version 1                    error: unsupported version
same  version 3                    error: unsupported version
same  flags changed                error: checksum mismatch
same  rate changed                 error: checksum mismatch
same  one payload byte changed     error: checksum mismatch
same  stored checksum changed      error: checksum mismatch
same  tag changed to unknown       error: checksum mismatch
same  cut inside header            error: truncated header
same  cut inside a chunk header    error: truncated chunk header
same  cut inside a payload         error: truncated chunk
same  cut before end chunk         error: truncated chunk header
same  cut inside end chunk         error: bad end chunk
same  bytes appended               channels=2 rate=8000 flags=0 samples=6 meta=1 notes=1
same  huge length                  error: chunk too large
same  end length 5                 error: bad end chunk
18 of 18 verdicts agree
```

Chaque ligne dit `same`, donc la spécification couvre ces cas. Plusieurs lignes montrent des points qu'une spécification doit consigner : un indicateur ou un taux modifié n'est détecté que par la somme de contrôle (le lecteur ne vérifie pas les valeurs), et des octets ajoutés après le bloc de fin sont acceptés. Quand une ligne dit `DIFF`, soit l'analyseur soit la spécification est faux : trouvez le premier octet où le lecteur s'arrête et comparez-le avec la règle. Au moindre doute, exécutez l'original ; c'est la référence finale.

## Essayez

Lancez les commandes du laboratoire depuis `modules/17-deciphering-file-formats/examples`.

1. Construisez avec `make`. Affichez `build/sample.lg` avec `xxd` et marquez sur papier l'en-tête, chaque en-tête de bloc et chaque charge utile avant de lire le tableau de la leçon.
2. Fabriquez des échantillons avec `17_make.py --flags 1`, `--samples 0` et `--samples 1`. Prédisez comment le vidage change, puis comparez avec `diff <(xxd a) <(xxd b)`.
3. Dans Rizin, lancez `axt` sur l'adresse de la chaîne `checksum mismatch` (`rizin -A build/17_reader`, puis `iz~checksum` pour l'adresse) et lisez le code qui y saute.
4. Ouvrez `build/17_reader` dans Ghidra. Trouvez l'appel à `memcmp` qui compare quatre octets avec la valeur magique, renommez la fonction qui met à jour la somme de contrôle et le tampon qui contient la table, et écrivez le tableau des champs d'après le code décompilé.
5. Retirez le dernier bloc de `sample.lg` avec un script et corrigez la somme de contrôle en recalculant le CRC-32. Quelle erreur le lecteur donne-t-il si vous calculez la somme de contrôle sur la mauvaise plage d'octets ?
6. Étendez `17_parse.py` pour qu'il affiche le contenu de `META` sous forme de paires clé et valeur. Vérifiez-le sur `sample.lg`.

## Pièges courants

- **Supposer le gros-boutisme.** Lisez le champ comme le code l'assemble. Un décalage du premier octet de 8 ou plus signifie que cet octet n'est pas le moins significatif.
- **Deviner à partir d'un seul échantillon.** Deux échantillons qui diffèrent par un paramètre localisent un champ ; un seul échantillon ne le peut pas.
- **Oublier de recalculer la somme de contrôle.** Après chaque modification d'un champ, recalculez la somme de contrôle, sinon le lecteur refuse le fichier avant d'atteindre le champ que vous avez changé.
- **Manquer une dépendance entre champs.** Une longueur qui compte des octets de charge utile, un nombre qui compte des éléments et un décalage depuis le début d'une section se comportent différemment quand vous changez une taille.
- **Ne documenter que ce que montrent les échantillons.** L'étiquette que le lecteur ignore, la limite de longueur et les octets après le bloc de fin n'apparaissent que dans le code.
- **Sauter les cas d'échec.** Un analyseur qui accepte tout ce que le lecteur accepte, mais aussi ce qu'il refuse, n'est pas équivalent. Testez des fichiers endommagés.

## Résumé

- Un format se déchiffre à partir de deux sources : les octets des échantillons et le code qui les lit ou les écrit. Utilisez les deux.
- Les parties habituelles sont une valeur magique, un en-tête de champs, des longueurs, des nombres, des étiquettes, des décalages et une somme de contrôle ; l'ordre des octets doit être fixé pour chaque nombre.
- Comparez des échantillons qui diffèrent par un paramètre pour localiser les champs, et lisez l'analyseur pour connaître les limites, l'ordre des vérifications et les cas ignorés.
- Une somme de contrôle s'identifie par ses constantes ; sa valeur initiale, son étape finale et sa plage couverte se déterminent ensuite d'après le code et se testent avec une fonction de bibliothèque.
- La spécification liste les champs avec décalages, tailles et sens, les règles et l'ordre des vérifications, et dit ce qui est inconnu. Un analyseur écrit d'après elle et comparé à l'original sur des fichiers endommagés la valide.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 18, l'audit de binaires à la recherche de bogues.
- Exercices qui utilisent ce module : [Exercice 17](../../exercises/17-archive-of-unknowns/README.fr.md).
