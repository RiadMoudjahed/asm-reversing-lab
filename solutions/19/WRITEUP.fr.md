<!-- source: WRITEUP.md @ sha256:daf1bb341543 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé détaillé : exercice 19. Bureau de triage

Le flag est `ARL{triage_6_34_1543003_1574002_1071004}` : la configuration compte 6 entrées encodées, l'entrée décodée la plus longue fait 34 octets, et les trois comportements correspondent à T1543.003 (service Windows), T1574.002 (chargement latéral de DLL) et T1071.004 (DNS).

## Partie 1 : Reverse

### Identification et chaînes

Les deux fichiers sont petits et sans symboles. `file` indique un ELF x86-64 pour `dispatch` et un exécutable console PE32+ pour `dispatch.exe`. Exécuter la version Linux est sans danger, car le programme n'appelle rien qui ait un effet :

```text
[sim] simulator: no action is performed, the steps below are only printed
[sim] would query the list of running processes
[sim] would create the service <cfg 0> that runs <cfg 1>
[sim] would place the library <cfg 2> next to the program <cfg 3>
[sim] would resolve names under <cfg 4> and carry data in the labels
[sim] would clear the event log <cfg 5>
```

La sortie répond à la question du comportement au niveau des verbes, et les chaînes du fichier confirment que ces modèles sont le seul texte lisible : `strings -n 6` affiche les six mêmes lignes avec `%d` là où la sortie contient les nombres. Les données intéressantes manquent : les noms du service et des fichiers, le domaine et le journal. Il n'y a pas d'autres chaînes lisibles ni d'import lié aux services, aux fichiers ou au réseau : la version Linux n'importe que `puts` et `__printf_chk`, et la version Windows n'importe que les fonctions de démarrage et de mémoire dont a besoin l'environnement d'exécution du compilateur depuis `KERNEL32.dll` et `msvcrt.dll`, ainsi que `puts`. Les données doivent donc se trouver dans le fichier sous une forme que `strings` ne sait pas lire.

### La table et le décodeur

Le marqueur `<cfg %d>` est rempli à partir d'un tableau de six emplacements de 64 octets chacun en mémoire inscriptible. Le code qui remplit le tableau est la première chose que fait la fonction principale après la première ligne :

```text
401178:  mov    ebx,0x404040            ; destination: six slots of 0x40 bytes
40117d:  mov    r12d,0x4041c0           ; end of the destination
401183:  mov    ebp,0x4021a0            ; source: the table in .rodata
401188:  mov    rsi,rbx
40118b:  mov    rdi,rbp
40118e:  call   401136                  ; decode one entry, returns its length
401193:  lea    rbp,[rbp+rax*1+0x2]     ; next entry = this one + length + 2
401198:  add    rbx,0x40
40119c:  cmp    rbx,r12
40119f:  jne    401188
```

La boucle est la clé de la structure. La destination va de 0x404040 à 0x4041c0, soit 0x180 octets ou six emplacements de 0x40. Le pointeur source avance de la valeur que renvoie la fonction plus 2 ; chaque entrée de la table est donc une longueur suivie d'un octet de plus, puis de ce nombre d'octets. Le nombre `N` est 6.

La fonction à 0x401136 est le décodeur :

```text
401136:  movzx  r8d,BYTE PTR [rdi]        ; length
40113b:  movzx  edx,BYTE PTR [rdi+0x1]    ; seed: the initial state
40113f:  test   r8,r8
401141:  je     401161                    ; length 0: nothing to decode
401143:  mov    eax,0x0                   ; position i = 0
401148:  rol    dl,0x3                    ; state = rotate left by 3
40114b:  lea    edx,[rdx+rax*1+0x5d]      ; state = state + i + 0x5d
40114f:  mov    ecx,edx
401151:  xor    cl,BYTE PTR [rdi+rax*1+0x2]  ; plain = encoded xor state
401155:  mov    BYTE PTR [rsi+rax*1],cl
401158:  add    rax,0x1
40115c:  cmp    r8,rax
40115f:  jne    401148
401161:  mov    BYTE PTR [rsi+r8*1],0x0   ; terminating zero byte
401166:  mov    rax,r8
401169:  ret
```

En C, la même fonction se lit ainsi. Le `lea` à trois termes est l'addition de la position et de la constante, et seul l'octet de poids faible du résultat compte, car l'état est utilisé par `dl` et `cl` :

```c-fragment
size_t decode_record(const unsigned char *rec, char *out)
{
    size_t len = rec[0];
    unsigned char k = rec[1];
    for (size_t i = 0; i < len; i++) {
        k = (unsigned char)(rol8(k, 3) + i + 0x5d);
        out[i] = rec[2 + i] ^ k;
    }
    out[len] = 0;
    return len;
}
```

Un schéma à clé fixe, comme celui du module 20, ne décoderait pas cette table : la clé change avec la position, et chaque entrée part de sa propre graine. Une première tentative qui applique un xor avec une constante à chaque octet donne du bruit pour toutes les entrées, et l'essai des 256 constantes ne trouve rien qui se lise comme du texte. C'est la reconnaissance de la rotation qui résout le problème.

### Décodage

Avec la règle dans un script, ou dans l'émulateur du module 20, les six entrées se décodent ainsi :

| Emplacement | Longueur | Graine | Texte |
|---|---|---|---|
| 0 | 13 | 0x13 | WinUpdSvcHost |
| 1 | 34 | 0xA7 | C:\ProgramData\Intel\svchost32.exe |
| 2 | 11 | 0x5C | version.dll |
| 3 | 19 | 0x2E | OneDriveUpdater.exe |
| 4 | 22 | 0xD1 | ns1.cdn-telemetry.test |
| 5 | 8 | 0x66 | Security |

Le texte le plus long est le chemin de l'emplacement 1, avec 34 octets : `L` vaut 34. Le texte décodé n'existe en mémoire qu'après l'exécution de la boucle, ce qui explique que ni `strings` ni une recherche dans le fichier ne le trouvent. Un débogueur qui s'arrête après la boucle montre les mêmes valeurs à 0x404040.

### Comportements et techniques

Chaque modèle, rempli avec les valeurs décodées, décrit un comportement :

| Modèle | Avec les valeurs décodées | ATT&CK |
|---|---|---|
| query the list of running processes | énumération du système | T1057 Process Discovery |
| create the service `<cfg 0>` that runs `<cfg 1>` | un service nommé WinUpdSvcHost exécute un programme dans un dossier de l'éditeur | T1543.003 Create or Modify System Process: Windows Service |
| place the library `<cfg 2>` next to the program `<cfg 3>` | version.dll à côté de OneDriveUpdater.exe, que le programme charge d'abord depuis son propre dossier | T1574.002 Hijack Execution Flow: DLL Side-Loading |
| resolve names under `<cfg 4>` and carry data in the labels | noms sous un domaine de l'opérateur, avec des données dans les étiquettes du sous-domaine | T1071.004 Application Layer Protocol: DNS |
| clear the event log `<cfg 5>` | le journal Security | T1070.001 Indicator Removal: Clear Windows Event Logs |

Le flag demande les trois techniques du milieu. Deux précautions s'appliquent au rapport. Le programme est un simulateur : il affiche ce qu'il ferait, et l'association décrit le comportement qu'il modélise, non un comportement observé sur un système. Et un identifiant de technique découle du comportement et non d'un nom dans une chaîne : le nom de service WinUpdSvcHost est choisi pour ressembler à un composant du système, ce qui est un point à noter dans le rapport, mais T1543.003 s'appuie sur la création du service elle-même.

On obtient `ARL{triage_6_34_1543003_1574002_1071004}` : `N` vaut 6, `L` vaut 34, et les identifiants sans le `T` ni le point sont 1543003, 1574002 et 1071004.

### La règle de détection

Le fichier ne contient aucun texte décodé, la règle ne peut donc pas l'utiliser. Ce que le fichier contient, ce sont les six modèles de lignes de journal en clair et les premiers octets de la première entrée encodée. La règle de référence se trouve dans `solutions/19/rule.yar` :

```text
rule triage_desk_dispatch
{
    strings:
        $service  = "would create the service <cfg" ascii
        $sideload = "would place the library <cfg" ascii
        $dns      = "would resolve names under <cfg" ascii
        $clear    = "would clear the event log <cfg" ascii
        $discover = "would query the list of running processes" ascii
        $record0  = { 0D 13 A2 64 A9 CB 25 68 90 F4 1A 79 9F 9C 9C }
    condition:
        (uint32(0) == 0x464c457f or uint16(0) == 0x5a4d) and
        3 of ($service, $sideload, $dns, $clear, $discover) and $record0
}
```

Le raisonnement suit les leurres, qui sont la partie de l'exercice qui met la règle à l'épreuve :

- `notes` affiche un texte d'aide qui mentionne une clé Run, un service et `version.dll`. Une règle fondée sur ces mots se déclenche sur lui. L'échantillon ne les contient pas en clair.
- `dryrun` affiche des lignes de la forme `[dry-run] would ...`. Une règle fondée sur le mot `would`, ou sur un court fragment comme `would create`, se déclenche sur lui. Les modèles de l'échantillon portent le marqueur `<cfg`, que l'exécution à blanc n'a pas.
- `updater` contient la même routine de décodage et une table encodée avec des textes inoffensifs. Une règle fondée sur les octets du décodeur se déclenche sur lui. La première entrée encodée de l'échantillon n'apparaît pas dans le programme de mise à jour.
- `/bin/ls` représente les programmes sur lesquels rien ne doit jamais se déclencher.

Le test du type de fichier fait que la règle ne s'exécute que sur des exécutables. Il n'est pas nécessaire pour séparer les leurres, et le vérificateur confirme qu'une règle sans lui est tout de même acceptée, mais c'est l'habitude qui garde une règle rapide sur de grandes collections. Les premiers octets de la première entrée sont la seule partie de la règle qui dépend de la compilation exacte ; une variante de l'échantillon avec une autre table demande une nouvelle règle, ce qui est l'état normal de la détection et non un défaut de celle-ci. Une règle qui n'a besoin que des modèles, par exemple `all of them` sur les quatre plus longs, est correcte aussi.

## Partie 2 : Assemble

Le décodeur de l'échantillon, complété par les deux contrôles de la spécification, constitue toute la fonction. La solution de référence se trouve dans `solutions/19/solution.asm` :

```nasm-fragment
decode_record:
    cmp     rsi, 2
    jb      .truncated                  ; no room for the length and the seed
    movzx   r8d, byte [rdi]             ; length
    lea     r9, [r8 + 2]
    cmp     r9, rsi
    ja      .truncated                  ; the record is cut off
    lea     r9, [r8 + 1]
    cmp     r9, rcx
    ja      .small                      ; text and zero byte do not fit
    movzx   r10d, byte [rdi + 1]        ; k = seed
    xor     r11d, r11d                  ; i = 0
.next:
    cmp     r11, r8
    jae     .done
    rol     r10b, 3
    add     r10b, r11b                  ; plus i (low byte)
    add     r10b, 0x5d
    mov     al, [rdi + r11 + 2]
    xor     al, r10b
    mov     [rdx + r11], al
    inc     r11
    jmp     .next
```

L'ordre des contrôles compte. Une entrée à la fois coupée et trop longue pour `cap` renvoie -1, car le premier contrôle s'applique d'abord, et les tests comprennent de telles entrées. Le test de `avail` inférieur à 2 précède la lecture de l'octet de longueur, puisque la longueur fait elle-même partie de l'entrée. Faire la comparaison avec `avail` sous la forme `length + 2 > avail` dans un registre de 64 bits évite tout débordement, car la longueur vaut au plus 255.

## Erreurs qui font perdre du temps

- Décoder avec une clé fixe. La rotation se laisse facilement lire comme une « étape de mélange » ; c'est l'état qui change à chaque position qui fait échouer une clé constante.
- Oublier que la position est ajoutée à l'état à chaque étape, y compris la première. Commencer la position à 1, ou l'ajouter après le xor, donne un texte faux dès le premier octet.
- Utiliser le registre de 64 bits pour l'état. L'état est un octet : la rotation et les additions doivent boucler sur 8 bits. Les tests utilisent de nombreuses graines et longueurs différentes, qui révèlent un état qui ne boucle pas.
- Une règle fondée sur un mot courant ou sur le seul décodeur. Les leurres existent pour cela. Le vérificateur nomme les programmes sur lesquels la règle donne la mauvaise réponse.
- Contrôler `cap` avant `avail`. La spécification fixe l'ordre, et les entrées coupées avec un petit `cap` le montrent.
- Se fier au nom du service. Le nom WinUpdSvcHost est censé ressembler à une partie du système ; la technique découle de ce que fait le programme.
