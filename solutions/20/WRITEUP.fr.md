<!-- source: WRITEUP.md @ sha256:de2d238ddb27 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé détaillé : exercice 20. Projet de synthèse : Tiny VM

Le flag est `ARL{tinyvm_20_41_24}` : la machine a 20 codes opération valides, `call` est l'octet de code opération 0x29 (41 en décimal) et le programme d'exemple a 24 instructions.

## Partie 1 : Reverse

### Orientation

`tinyvm` est sans symboles et petit. `strings` montre le message d'usage et les deux diagnostics (`halt after %zu steps`, `fault %u at offset %u`), et rien d'autre de lisible. Le programme lit son argument comme un fichier, vérifie les quatre octets `TVM1` au début, puis exécute une boucle. La boucle est l'interpréteur.

### La répartition

La boucle lit un octet du code, puis effectue deux consultations de tables avant de sauter :

```text
  401305: movzx  eax,BYTE PTR [rcx]            ; the opcode byte
  401308: movzx  eax,BYTE PTR [rax+0x402160]   ; first table: 256 bytes
  40130f: test   al,al
  401311: je     401403                        ; 0: not an instruction
  401317: movzx  edx,al
  40131a: movzx  edx,BYTE PTR [rdx+0x402130]   ; second table: one byte per instruction
  401321: add    edx,esi                       ; position + length
  401325: cmp    rbx,rsi                       ; against the length of the code
  401328: jb     4013fc                        ; the instruction does not fit
  40132e: mov    DWORD PTR [rsp+0x20],edx      ; new program counter
  401332: cmp    al,0x14
  401334: ja     4012d9
  401336: movzx  eax,al
  401339: jmp    QWORD PTR [rax*8+0x402080]    ; third table: 21 code addresses
```

Trois tables, donc :

- La table à 0x402160 fait 256 octets et a une entrée non nulle à exactement 20 positions. Ces positions sont les codes opération valides, donc `N` vaut 20. L'entrée est un petit indice de 1 à 20.
- La table à 0x402130 est indexée par cet indice et donne la longueur de l'instruction : 4 pour une instruction, 3 pour quatre, 1 pour deux et 2 pour les treize autres.
- La table à 0x402080 contient 21 adresses de code, une par indice (l'indice 0 revient en haut de la boucle). La comparaison avec 0x14 est le contrôle de borne.

Le compteur ordinal est stocké à `[rsp+0x20]` et mis à jour avant l'exécution du gestionnaire : c'est la convention derrière les gestionnaires de la partie 2. L'état est une structure sur la pile : les huit registres à partir de `[rsp]`, puis le compteur ordinal (0x20), l'indicateur de zéro (0x24), l'indicateur de retenue (0x28), le code de faute (0x2c), l'indicateur d'arrêt (0x30), la profondeur de la pile d'appels (0x34), les seize adresses de retour (0x38) et les 256 octets de mémoire (0x58).

### Les gestionnaires

Chacune des vingt adresses est le code d'une instruction. Les lire donne le comportement ; trois exemples :

```text
; index 3
  401448: movzx  edx,BYTE PTR [rcx+0x1]        ; the byte after the opcode
  40144c: mov    ecx,edx
  40144e: shr    cl,0x4
  401451: and    ecx,0x7                       ; first register: high nibble, modulo 8
  401454: mov    eax,DWORD PTR [rsp+rcx*4]
  401457: and    edx,0x7                       ; second register: low nibble, modulo 8
  40145a: mov    edx,DWORD PTR [rsp+rdx*4]
  40145d: lea    esi,[rax+rdx*1]
  401460: mov    DWORD PTR [rsp+rcx*4],esi     ; r[a] = r[a] + r[b]
  401463: mov    eax,eax
  401465: mov    edx,edx
  401467: add    rax,rdx                       ; the sum again, in 64 bits
  40146a: shr    rax,0x20
  40146e: mov    DWORD PTR [rsp+0x28],eax      ; carry flag = carry out
  401472: test   esi,esi
  401474: sete   al
  401477: movzx  eax,al
  40147a: mov    DWORD PTR [rsp+0x24],eax      ; zero flag = result is 0
  40147e: jmp    4012d9
```

C'est `add` : il positionne les deux indicateurs. Les gestionnaires de `sub`, `xor` et des autres se lisent de la même façon. Les détails se trouvent dans le gestionnaire de décalage :

```text
; index 8
  401550: movzx  eax,BYTE PTR [rcx+0x1]
  401554: mov    esi,eax
  401556: and    esi,0xf                       ; the count: the whole low nibble
  401559: mov    edi,eax
  40155b: and    edi,0xf
  40155e: je     4012d9                        ; count 0: nothing changes, flags included
  401564: shr    al,0x4
  401567: and    eax,0x7                       ; the register
  40156a: mov    edx,DWORD PTR [rsp+rax*4]
  40156d: mov    ecx,0x20
  401572: sub    ecx,edi                       ; 32 - count
  401574: mov    edi,edx
  401576: shr    edi,cl                        ; value >> (32 - count)
  401578: mov    ecx,edi
  40157a: and    ecx,0x1
  40157d: mov    DWORD PTR [rsp+0x28],ecx      ; carry flag = last bit shifted out at the top
  401581: mov    ecx,esi
  401583: shl    edx,cl
  401585: mov    DWORD PTR [rsp+rax*4],edx
  401588: test   edx,edx
  40158a: sete   al
  40158d: movzx  eax,al
  401590: mov    DWORD PTR [rsp+0x24],eax      ; zero flag
  401594: jmp    4012d9
```

Le compteur est le quartet de poids faible entier (0 à 15), alors que le registre utilise trois bits du quartet de poids fort. Un compteur de zéro retourne avant de toucher le moindre indicateur. `shr` est l'image inverse, avec l'indicateur de retenue pris sur le bit `count - 1`.

```text
; index 14 (call)
  401666: mov    eax,DWORD PTR [rsp+0x34]      ; depth
  40166a: cmp    eax,0x10
  40166d: je     401693                        ; full: fault 3
  40166f: lea    esi,[rax+0x1]
  401672: mov    DWORD PTR [rsp+0x34],esi      ; depth + 1
  401676: mov    eax,eax
  401678: mov    WORD PTR [rsp+rax*2+0x38],dx  ; push the program counter (already advanced)
  40167d: movzx  eax,BYTE PTR [rcx+0x2]
  401681: shl    eax,0x8
  401684: movzx  edx,BYTE PTR [rcx+0x1]
  401688: or     eax,edx                       ; target: byte 1 + 256 * byte 2 (low byte first)
  40168a: mov    DWORD PTR [rsp+0x20],eax
  40168e: jmp    4012d9
  401693: mov    DWORD PTR [rsp+0x2c],0x3      ; fault code 3
  40169b: jmp    4016c4                        ; then halt
```

### Le jeu d'instructions

En réunissant les vingt gestionnaires, on obtient la spécification (les noms sont ceux de l'exercice) :

| Code | Nom | Longueur | Effet |
|---|---|---|---|
| 0x5b | `movi` | 4 | `r[a] = number` (le nombre de 16 bits est dans les octets 2 et 3, octet de poids faible d'abord) ; indicateurs inchangés |
| 0xc2 | `mov` | 2 | `r[a] = r[b]` ; indicateurs inchangés |
| 0x17 | `add` | 2 | `r[a] += r[b]` ; `cf` = retenue sortante, `zf` = le résultat est 0 |
| 0x8e | `sub` | 2 | `r[a] -= r[b]` ; `cf` = a < b non signé, `zf` = le résultat est 0 |
| 0x34 | `mul` | 2 | `r[a] *= r[b]` (32 bits de poids faible) ; `cf` = 0, `zf` = le résultat est 0 |
| 0xe9 | `xor` | 2 | `r[a] ^= r[b]` ; `cf` = 0, `zf` = le résultat est 0 |
| 0x61 | `and` | 2 | `r[a] &= r[b]` ; `cf` = 0, `zf` = le résultat est 0 |
| 0xa5 | `shl` | 2 | décalage à gauche du quartet de poids faible ; compteur 0 ne change rien ; `cf` = dernier bit sorti par le haut, `zf` = le résultat est 0 |
| 0x0d | `shr` | 2 | décalage à droite du quartet de poids faible ; compteur 0 ne change rien ; `cf` = dernier bit sorti par le bas, `zf` = le résultat est 0 |
| 0x7a | `cmp` | 2 | indicateurs comme pour `sub`, rien n'est rangé |
| 0xf3 | `jmp` | 3 | `pc` = la cible de 16 bits des octets 1 et 2 |
| 0x42 | `jz` | 3 | saute si `zf` vaut 1 |
| 0xb8 | `jnz` | 3 | saute si `zf` vaut 0 |
| 0x29 | `call` | 3 | empile `pc` (déjà avancé) puis saute ; faute 3 et arrêt si 16 adresses sont sur la pile |
| 0xd6 | `ret` | 1 | dépile `pc` ; faute 2 et arrêt si la pile est vide |
| 0x93 | `ldb` | 2 | `r[a] = mem[r[b] & 255]` |
| 0x6c | `stb` | 2 | `mem[r[a] & 255] = r[b] & 255` |
| 0xfc | `out` | 2 | affiche `r[a]` comme nombre décimal non signé et un saut de ligne |
| 0x05 | `outc` | 2 | affiche le caractère `r[a] & 255` |
| 0xa0 | `halt` | 1 | arrête |

Dans toute instruction avec un octet de registre, le premier registre est le quartet de poids fort et le second le quartet de poids faible, chacun pris modulo 8 ; les nombres de 16 bits sont stockés octet de poids faible d'abord. `isa.json` dans `solutions/20/` contient ce tableau au format de l'exercice. Les fautes 1 (code opération invalide), 5 (sortie par la fin ou trop de pas) et 6 (une instruction coupée par la fin du code) viennent de la boucle de l'interpréteur et ne font pas partie des gestionnaires.

### Le programme d'exemple

Avec le tableau, un désassembleur est un petit script (`solutions/20/tvdis.py`). Le listing de `bin/sample.tvm` :

```text
      0000  5b 00 05 15   movi r0, 5381
      0004  5b 10 00 00   movi r1, 0
      0008  5b 20 06 00   movi r2, 6
      000c  5b 30 01 00   movi r3, 1
L010: 0010  93 41         ldb r4, [r1]
      0012  29 34 00      call L034
      0015  17 13         add r1, r3
      0017  7a 12         cmp r1, r2
      0019  b8 10 00      jnz L010
      001c  fc 00         out r0
      001e  5b 10 00 00   movi r1, 0
L022: 0022  93 41         ldb r4, [r1]
      0024  05 40         outc r4
      0026  17 13         add r1, r3
      0028  7a 12         cmp r1, r2
      002a  b8 22 00      jnz L022
      002d  5b 50 0a 00   movi r5, 10
      0031  05 50         outc r5
      0033  a0            halt
L034: 0034  c2 60         mov r6, r0
      0036  a5 65         shl r6, 5
      0038  17 06         add r0, r6
      003a  e9 04         xor r0, r4
      003c  d6            ret
24 instructions; data segment: b'vmcore'
```

Il calcule un hachage des six octets de données avec `h = h * 33 xor c` (le sous-programme à 0x34 calcule `h + (h << 5)`, c'est-à-dire `h * 33`), affiche le hachage puis affiche les octets comme du texte. Le listing compte 24 instructions, donc `I` vaut 24, et `call` est 0x29, soit 41. Le flag en découle.

## Partie 2 : Assemble

### Les gestionnaires

Les gestionnaires de référence sont dans `solutions/20/solution.asm`. Tous commencent par décomposer l'octet de registre. Les contrôles qui comptent et les erreurs qui font perdre du temps :

- **Les masques.** Les champs de registre sont des quartets, de 0 à 15, pris modulo 8 (`and 7`). Le compteur de décalage est le quartet de poids faible entier (`and 15`) et n'est pas réduit.
- **Quels indicateurs.** `mul` met l'indicateur de retenue à zéro ; `shl` et `shr` positionnent l'indicateur de retenue d'après le dernier bit sorti et ne font rien du tout pour un compteur de zéro. Un gestionnaire qui efface ou recalcule les indicateurs pour un compteur de zéro échoue aux tests qui partent de valeurs d'indicateurs aléatoires.
- **La retenue de `shl`.** C'est le bit `32 - count` de l'ancienne valeur, qui n'est pas le même bit que pour `shr` (bit `count - 1`).
- **`cmp`.** L'indicateur de retenue est une comparaison non signée ; une comparaison signée échoue quand le bit de poids fort de l'une des valeurs est positionné.
- **`call` et `ret`.** L'adresse empilée est le compteur ordinal déjà avancé. Les cas de faute ne modifient que le code de faute et l'indicateur d'arrêt.
- **`ldb` et `stb`.** Seul l'octet de poids faible du registre d'adresse est utilisé, et `stb` ne range que l'octet de poids faible de la valeur.
- **Préserver les registres.** Les gestionnaires n'utilisent que des registres qu'une fonction peut modifier ; il n'y a donc rien à sauvegarder.

### Le programme

Le programme de référence est `solutions/20/program.tvs`. Une boucle range le nombre courant à l'adresse donnée par un compteur et fait avancer la paire ; un sous-programme compte à rebours depuis 12, charge chaque octet et l'affiche. Le registre qui n'est jamais positionné sert de zéro pour le test de fin de la boucle. Il s'exécute en 165 pas.

```text
fill:   stb  [r2], r0
        mov  r4, r0
        add  r4, r1
        mov  r0, r1
        mov  r1, r4
        add  r2, r3
        cmp  r2, r5
        jnz  fill
```

## Erreurs qui font perdre du temps

- Prendre les champs de registre dans le mauvais ordre ou dans le mauvais quartet. Un `register_a` faux est détecté par la vérification de `isa.json` et par le programme, parce que l'assembleur place les registres d'après vos notes.
- Lire `sub` avec les opérandes inversés. Le premier registre est la destination et l'opérande de gauche.
- Rater que certaines instructions laissent les indicateurs intacts (`movi`, `mov`, `ldb`, `stb`).
- Compter le segment de données ou l'en-tête dans le nombre d'instructions.
- Écrire un gestionnaire qui fonctionne pour les registres 0 à 7 et échoue pour les quartets 8 à 15, qui bouclent.
