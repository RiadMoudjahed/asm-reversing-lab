<!-- source: WRITEUP.md @ sha256:936cb9e94e24 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 18. Rapport de chasse aux bogues

Le drapeau est `ARL{bughunt_16_128_8_64_5}` : le gestionnaire de noms accepte une longueur allant jusqu'à 16 dans un tampon de 16 octets, le gestionnaire de nombres laisse passer par sa vérification signée tous les octets de longueur à partir de 128, le gestionnaire d'étiquettes accepte une valeur d'étiquette allant jusqu'à 8 pour une table de 8 entrées, la taille du bloc de données déborde sur 8 bits à partir d'un nombre de 64, et le type d'enregistrement 5 libère de la mémoire qui est réutilisée.

## Partie 1 : Reverse

### Orientation

Le binaire est dépouillé de ses symboles et le code d'analyse a été intégré dans la fonction principale. La première étape est de trouver la structure du programme plutôt que de le lire depuis le haut :

- Le fichier est lu par un seul `fread` d'au plus 4096 octets dans un tampon statique en 0x404080. Après l'appel, la longueur renvoyée n'est plus jamais utilisée : rien ne compare une position à la taille des données. C'est le premier indice sur le parcours des enregistrements.
- Les enregistrements sont aiguillés par une table de sauts :

```text
401465:  mov    rbx,rdi
401468:  cmp    al,0x5                       ; type above 5: unknown, error -6
40146a:  ja     40153f
401470:  movzx  eax,al
401473:  jmp    QWORD PTR [rax*8+0x402088]   ; table for the types 1 to 5
```

La table en 0x402088 contient, pour les types 1 à 5, les adresses 0x401316, 0x401362, 0x4012e4, 0x4013bc et 0x401438. Chaque adresse est le code d'un type d'enregistrement, et chacune est un puits à auditer. Essayer de petits fichiers d'entrée confirme ce qu'ils font : le type 1 fixe le nom, le type 2 ajoute des nombres, le type 3 compte des étiquettes, le type 4 conserve un bloc de données, le type 5 l'abandonne.

### Le parcours des enregistrements (tous les types)

```text
401440:  add    rbp,0x1                      ; one more record
401444:  movzx  eax,BYTE PTR [rbx]           ; type
401447:  movzx  edx,BYTE PTR [rbx+0x1]       ; length
40144b:  movzx  r8d,dl
40144f:  lea    rsi,[rbx+0x2]                ; payload
401453:  movzx  ecx,dl
401456:  lea    rdi,[rsi+rcx*1]              ; the next record = payload + length
40145a:  mov    r14,rbx
40145d:  test   al,al
40145f:  je     4012b6                       ; type 0 ends the loop
401465:  mov    rbx,rdi
```

La boucle lit deux octets d'en-tête et calcule la position suivante, et à aucun moment la position n'est comparée à la fin des données. Un fichier dont le dernier enregistrement déclare une longueur qui ne tient pas fait lire le code au-delà de la fin du tampon, et un fichier sans enregistrement de type 0 le fait lire jusqu'à en rencontrer un, où qu'il soit. La classe est une lecture hors limites (CWE-125).

### Noms (type 1, 0x401316)

```text
401316:  pxor   xmm0,xmm0
40131a:  movaps XMMWORD PTR [rsp+0x80],xmm0  ; the 16-byte local name, cleared
401322:  cmp    r8d,0x10
401326:  ja     401356                       ; length above 16: error -2
401328:  lea    rax,[rsp+0x80]
...
401335:  rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]    ; copy length bytes
40133a:  mov    BYTE PTR [rsp+r8*1+0x80],0x0            ; terminator at name[length]
```

Le tampon fait 16 octets, la limite est 16, et le terminateur est écrit à l'indice `length`. À la longueur 16, le terminateur est le dix-septième octet, un de trop : une erreur d'un qui écrit un octet nul en dehors du tampon (CWE-193, une écriture hors limites). Donc `N` vaut 16, et la bonne limite est 15.

### Nombres (type 2, 0x401362)

```text
401362:  cmp    dl,0x20
401365:  jg     4013b0                       ; signed: above 32 is error -3
401367:  lea    rax,[rsp+0x20]               ; a local array of 8 integers (32 bytes)
40136c:  movsx  rcx,dl                       ; the size, sign-extended to 64 bits
401370:  mov    rdi,rax
401373:  rep movs BYTE PTR es:[rdi],BYTE PTR ds:[rsi]
```

La comparaison utilise le registre de 8 bits `dl` et un saut signé. Les octets de longueur 0 à 32 passent, ainsi que tous les octets de 128 à 255, qui sont négatifs lus comme des nombres signés de 8 bits. Le `movsx` qui suit étend une valeur négative en un nombre gigantesque de 64 bits pour `rep movs`. Donc `V` vaut 128. La vérification oublie aussi de tester que la longueur est un multiple de 4. C'est une erreur de conversion de signé en non signé (CWE-195) qui mène à une écriture hors limites. Le correctif compare la longueur non signée et vérifie le multiple de 4.

### Étiquettes (type 3, 0x4012e4)

```text
4012f6:  movzx  eax,BYTE PTR [rdx]           ; the tag byte
4012f9:  cmp    al,0x8
4012fb:  ja     40154b                       ; above 8: error -4
401301:  movzx  eax,al
401304:  add    DWORD PTR [rsp+rax*4],0x1    ; histogram[tag]++
```

L'histogramme est fait de huit compteurs de 32 bits au bas du cadre (mis à zéro par un `rep stos` antérieur de 8 mots doubles). La vérification laisse passer la valeur 8, et l'incrémentation à l'indice 8 touche les quatre octets qui suivent la table : une erreur d'un avec écriture (CWE-193). Donc `T` vaut 8 ; la bonne limite est 7. Dans cette compilation, le voisin est le tableau du gestionnaire de nombres, le dégât est donc silencieux, ce qui rend ce genre de bogue dangereux.

### Blocs de données (type 4, 0x4013bc)

```text
4013bc:  movzx  r13d,BYTE PTR [r14+0x2]      ; count = first payload byte
4013c1:  mov    rdi,r12
4013c4:  call   free                         ; the previous block
4013c9:  lea    edi,[r13*4+0x0]
4013d1:  movzx  edi,dil                      ; the size is truncated to 8 bits
4013d5:  call   malloc
4013da:  mov    r12,rax
4013dd:  movzx  eax,r13b
4013e1:  shl    eax,0x2                      ; the copy size is count * 4 in 32 bits
...                                          ; a copy of that many bytes into the block
```

La taille d'allocation est `count * 4` tronqué à 8 bits, et la taille de copie est `count * 4` sur 32 bits. Elles concordent tant que le nombre est inférieur à 64. À partir d'un nombre de 64, le produit atteint 256 et la taille d'allocation boucle (64 donne `malloc(0)`, 65 donne `malloc(4)`), tandis que la copie écrit toujours 4 fois le nombre : un débordement de tas (CWE-190 menant à CWE-122). Donc `B` vaut 64. Le code ne compare en outre jamais la longueur de l'enregistrement au nombre, de sorte que la copie peut lire au-delà de la charge utile (CWE-125). Le correctif calcule la taille dans un type large et exige que la longueur soit égale à 1 plus 4 fois le nombre.

### Libération (type 5, 0x401438)

```text
401438:  mov    rdi,r12
40143b:  call   free                         ; free the block; r12 keeps its value
401440:  ...                                 ; the loop continues
...
40147a:  movzx  ecx,BYTE PTR [r12+rax*1]     ; at the end: sum the block bytes (r13 of them)
...
401494:  mov    rdi,r12
401497:  call   free                         ; and free it again
```

L'enregistrement de type 5 libère le bloc et laisse `r12` et la longueur `r13` inchangés. Le code après la boucle additionne les octets du bloc et le libère une seconde fois : une lecture après libération (CWE-416) et une double libération (CWE-415). Donc `U` vaut 5. (L'enregistrement de type 4 libère aussi l'ancien bloc avant d'en allouer un nouveau, ce qui n'est correct que parce qu'il écrase `r12`.) Le correctif met le pointeur à nul et la longueur à 0 après la libération.

### Un rapport modèle

```text
Title:      The record walk never compares its position with the end of the data
Class:      CWE-125 out-of-bounds read
Location:   main (the parser is inlined), loop at 0x401440 to 0x401465
Severity:   Medium. The length bytes come from the file; only reads are affected.
Cause:      The next record is computed as payload + length and the position is
            never compared with the size of the data returned by fread.
Evidence:   A file with a last record of length 255 and 10 bytes of data makes
            the loop read the stale content of the static buffer.
Impact:     Bytes outside the file are interpreted as records; the output depends
            on memory that is not part of the input and the program can fault.
Fix:        Track the bytes that are left, and refuse a record when fewer than 2
            bytes remain or the payload does not fit.
Test:       Truncations at every position of a valid file; a last record of length
            255 with no data.

Title:      Names: off-by-one in the terminator
Class:      CWE-193 off-by-one (out-of-bounds write of one byte)
Location:   0x401322 (limit) and 0x40133a (terminator)
Severity:   Medium. One byte with a fixed value, written next to a stack buffer.
Cause:      The limit is 16 for a buffer of 16 bytes, and the terminator is written
            at index length.
Evidence:   A name of 16 bytes passes the check; the store at [rsp+r8+0x80] with
            r8 = 16 is outside the 16 bytes at [rsp+0x80].
Impact:     A zero byte is written to whatever follows the buffer.
Fix:        Accept lengths up to 15.
Test:       Names of 0, 15, 16 and 255 bytes.

Title:      Numbers: signed length check
Class:      CWE-195 signed to unsigned conversion (leading to an out-of-bounds write)
Location:   0x401362 to 0x401373
Severity:   High. A length byte of 128 to 255 gives a copy size of about 2^64.
Cause:      The length is tested as a signed 8-bit value and then sign-extended to
            the copy size.
Evidence:   A record with length 128 crashes the program; a debugger stopped at
            0x401370 shows rcx = 0xffffffffffffff80.
Impact:     The copy runs far beyond its 32-byte destination and ends with a fault.
Fix:        Treat the length as unsigned; require a multiple of 4 and at most 32.
Test:       Lengths 0, 4, 32, 33, 36, 127, 128, 255.

Title:      Tags: the table has 8 entries and the check accepts 8
Class:      CWE-193 off-by-one (out-of-bounds write of a counter)
Location:   0x4012f9 and 0x401304
Severity:   Medium. A counter increment on the four bytes after the table.
Cause:      The limit is "above 8" for a table whose last index is 7.
Evidence:   A tag value of 8 passes the check and the increment is at [rsp+0x20].
Impact:     The data that follows the table is changed.
Fix:        Reject tag values above 7.
Test:       Tag values 0, 7, 8, 9, 255.

Title:      Data blocks: the allocation size wraps
Class:      CWE-190 integer overflow (leading to CWE-122 heap overflow)
Location:   0x4013c9 to 0x4013e1
Severity:   High. The count and the data come from the file.
Cause:      malloc receives count * 4 truncated to 8 bits; the copy uses count * 4
            in 32 bits; the record length is not compared with the count.
Evidence:   A count of 64 gives malloc(0) and a copy of 256 bytes.
Impact:     A heap overflow of up to 768 bytes beyond a small allocation.
Fix:        Compute the size in 64 bits, require length = 1 + 4 * count, and
            allocate that size.
Test:       Counts 0, 1, 63, 64, 65, 255, each with a matching and a mismatching
            length.

Title:      Release: the freed block is used and freed again
Class:      CWE-416 use after free (and CWE-415 double free)
Location:   0x401438 and 0x40147a to 0x401497
Severity:   High. The block is read after it was freed, then freed again.
Cause:      Type 5 frees the block and keeps the pointer and the length.
Evidence:   A file with a data block followed by a record of type 5 is enough;
            in the debugger, the second call to free at 0x401497 receives the
            pointer that the first call at 0x40143b has already released.
Impact:     Reads of freed memory, and corruption of the allocator state.
Fix:        Set the pointer to null and the length to 0 after the free.
Test:       A block then a reset; a reset with no block; a reset twice; a block
            after a reset.
```

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Les points qui comptent :

- **La boucle.** Les octets restants sont calculés comme `n - pos` avant chaque comparaison. On pourrait aussi comparer position plus longueur à `n`, mais la soustraction ne peut pas déborder car `pos` ne dépasse jamais `n`. Les deux octets d'en-tête sont vérifiés d'abord, puis la charge utile, et le type n'est examiné qu'ensuite, de sorte que l'enregistrement de type 0 suit la même règle.
- **Noms.** Une écriture de 16 octets efface le champ, puis les octets de la charge utile sont copiés. La limite est 15.
- **Nombres.** `test r11b, 3` refuse les longueurs qui ne sont pas des multiples de 4, et une comparaison non signée refuse celles qui dépassent 32. Les valeurs sont chargées avec `movsxd` pour qu'un nombre négatif soit correctement ajouté à la somme de 64 bits.
- **Étiquettes.** L'octet est comparé à 7 avant d'indexer la table.
- **Blocs de données.** `lea r10d, [r10*4 + 1]` calcule la longueur attendue dans un registre de 32 bits ; un nombre qui tient dans un octet ne peut pas le faire déborder. La comparaison avec la longueur décide de tout, donc aucune lecture ne dépasse la charge utile. La somme repart de 0 pour chaque bloc.
- **Registres.** Seuls les registres que l'appelé peut modifier sont utilisés, il n'y a donc rien à sauvegarder, et il n'y a aucun appel.

## Erreurs qui font perdre du temps

- **Comparer position plus longueur à `n` sur 32 bits.** Pour un tampon proche du haut de l'espace d'adressage, ou avec une grande longueur, la somme boucle. Calculez plutôt ce qui reste.
- **Tester le type d'enregistrement avant la troncature.** Un enregistrement de type 0 dont la charge utile est coupée doit quand même être refusé.
- **Utiliser un chargement non signé pour les nombres.** Les valeurs négatives ajoutent le mauvais montant ; la somme diffère.
- **Oublier d'effacer le nom.** Un nom plus court qui en remplace un plus long laisse d'anciens octets.
- **Ne pas remettre à zéro la somme du bloc.** Un second bloc s'ajoute au premier.
- **Lire le nombre avant de vérifier que la longueur vaut au moins 1.** Un enregistrement de type 4 sans charge utile lit un octet au-delà.
- **Vérifier l'étiquette contre 8.** L'erreur d'un de l'original est recopiée dans le correctif.
