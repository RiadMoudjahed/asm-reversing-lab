<!-- source: README.md @ sha256:2cc754f9080c -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# 18. Auditer des binaires à la recherche de bogues

Durée : environ 2,5 heures. Prérequis : [17. Déchiffrer les formats de fichier](../17-deciphering-file-formats/README.fr.md)

## Pourquoi c'est important

La plupart des failles de sécurité graves dans le code natif sont des erreurs de sûreté mémoire : un programme copie, indexe ou libère la mémoire d'une façon que son auteur n'avait pas voulue. Celui qui sait lire du code compilé peut trouver un tel bogue dans un composant sans source, rédiger un rapport exploitable par le responsable et proposer un correctif qu'on peut tester. Ce module enseigne ce travail du côté du défenseur. Il s'arrête au diagnostic et à la réparation. Il n'enseigne pas comment transformer un bogue en attaque, et le cours ne vous le demande jamais. N'auditez que du code qui vous appartient ou que vous avez le droit d'examiner, et lisez [la déclaration d'éthique](../../ETHICS.md) avant de commencer.

## Concepts

### La méthode : des sources aux puits

Une erreur de sûreté mémoire exige deux choses : une valeur que le programme ne contrôle pas (lue dans un fichier, un paquet réseau, un argument ou un champ d'un enregistrement) et un endroit où cette valeur décide de la quantité de mémoire touchée. La première est une **source**, le second un **puits**. L'audit suit chaque source jusqu'à chaque puits et examine les vérifications entre les deux.

| Puits | Question à poser |
|---|---|
| Une copie (`memcpy`, `strcpy`, une boucle qui range des octets) | Qu'est-ce qui borne le nombre d'octets ? La borne est-elle comparée à la taille de la destination, et à la bonne taille ? |
| Un indice dans un tableau | Est-il d'abord comparé à la longueur, et la comparaison est-elle signée ou non signée ? |
| Une allocation (`malloc`, `calloc`, `realloc`) | Comment la taille est-elle calculée ? Le calcul peut-il déborder ? Le code suivant réutilise-t-il les mêmes nombres ? |
| Un appel à `free` | Le pointeur est-il utilisé ensuite sur un chemin quelconque ? Est-il libéré deux fois ? |
| Un appel à une fonction de bibliothèque avec un format ou une chaîne | Prend-elle une taille ? La taille est-elle correcte ? |

Dans un programme compilé, cherchez d'abord les puits : parcourez le désassemblage à la recherche des appels aux fonctions de copie, d'allocation et de libération, et des écritures à indice multiplié. Remontez ensuite depuis l'opérande de taille (`rdx` dans un appel à `memcpy`, `rdi` dans un appel à `malloc`) jusqu'à l'endroit d'où la valeur provient, et lisez chaque comparaison en chemin.

### Le code d'entraînement

Le fichier `examples/18_bugs.c` contient six petites fonctions, chacune avec un bogue d'une classe différente. `examples/18_fixed.c` contient les mêmes fonctions réparées. Les listings de cette leçon proviennent d'une compilation de `18_bugs.c` avec `-O1`, `-fno-builtin`, `-mstringop-strategy=libcall` et `-D_FORTIFY_SOURCE=0`, ce qui garde les appels de bibliothèque visibles comme dans beaucoup de binaires réels. Pour confirmer, `make` construit aussi les programmes de démonstration avec AddressSanitizer, une fonction du compilateur qui arrête un programme au premier accès mémoire incorrect et affiche un rapport. Vous pouvez l'utiliser quand vous avez le source ; pour un binaire que vous ne pouvez pas recompiler, utilisez le débogueur.

```bash
make
make run
```

### Bogue 1 : une longueur que personne ne vérifie

```text
0000000000000000 <set_name>:
   0:  push   rbp
   1:  push   rbx
   2:  sub    rsp,0x8
   6:  mov    rbx,rdi                      ; rbx = dst
   9:  movzx  ebp,BYTE PTR [rsi]           ; length = first byte of the packet, 0 to 255
   c:  add    rsi,0x1                      ; source = packet + 1
  10:  mov    rdx,rbp                      ; size for memcpy
  13:  call   memcpy
  18:  mov    BYTE PTR [rbx+rbp*1],0x0    ; terminator written at dst[length]
  1c:  add    rsp,0x8
  20:  pop    rbx
  21:  pop    rbp
  22:  ret
```

La taille passée à `memcpy` est un octet lu dans l'entrée et étendu par des zéros, et il n'y a aucune comparaison entre le chargement et l'appel. La fonction ne reçoit pas la taille de la destination, donc aucune vérification correcte n'est possible à l'intérieur : la fonction elle-même est le défaut, car son interface manque de l'information nécessaire. L'écriture du terminateur en `dst[length]` est une seconde écriture avec le même problème.

L'appelant de la démonstration a un tampon de 16 octets et une longueur de 40. Dans le débogueur, vous voyez la situation avant que la copie n'ait lieu :

```text
Breakpoint 1, set_name (dst=0x7ffdef7e5010 "", pkt=0x14a982a0 "(", 'A' <repeats 40 times>) at 18_bugs.c:13
13        unsigned len = pkt[0];
0x14a982a0:    40
#1  0x0000000000401244 in main (argc=2, argv=0x7ffdef7e5168) at 18_demo_bug.c:20
20        set_name(dst, pkt);
$1 = 16
$2 = 40 '('
```

Les commandes étaient `break set_name`, `run name`, `x/ub $rsi`, `up`, `print sizeof dst` et `print pkt[0]`, puis `kill`. Le programme a été arrêté avant la copie fautive, et les nombres suffisent pour le rapport : la destination contient 16 octets et la longueur est 40. Avec la compilation à sanitizer, le rapport désigne la même ligne :

```text
ERROR: AddressSanitizer: stack-buffer-overflow on address ADDR at pc ADDR bp ADDR sp ADDR
WRITE of size 40 at ADDR thread T0
    #1 ADDR in set_name 18_bugs.c:14
    #2 ADDR in main 18_demo_bug.c:20
```

Correctif : la fonction doit recevoir la capacité de la destination et la longueur du paquet, et refuser l'entrée qui ne tient pas. `18_fixed.c` a `set_name(dst, cap, pkt, pkt_len)`, qui refuse une longueur supérieure au reste du paquet ou non inférieure à la capacité (le dernier octet est pour le terminateur).

### Bogue 2 : erreur d'un

```text
0000000000000023 <copy_label>:
  ...
  30:  mov    rdi,rdx                      ; rdi = src
  33:  call   strlen                       ; rax = n
  38:  cmp    r12,rax                      ; cap against n
  3b:  jb     56 <copy_label+0x33>         ; cap < n: return -1
  3d:  lea    rdx,[rax+0x1]                ; size = n + 1
  41:  mov    rsi,rbx
  44:  mov    rdi,rbp
  47:  call   memcpy                       ; copies n + 1 bytes
```

Le code ne refuse la chaîne que si elle est plus longue que la capacité. Une chaîne d'exactement `cap` caractères passe, et la copie écrit alors `cap + 1` octets : les caractères et le terminateur. L'indice est la différence entre la quantité comparée (`n`) et la quantité copiée (`n + 1`, le `lea rdx,[rax+0x1]`). Chaque fois qu'une taille de copie comporte une constante ajoutée, la vérification qui la précède doit comprendre la même constante. Le sanitizer signale une écriture de taille 9 dans un tampon de 8 octets. Dans la version réparée, le branchement après `cmp rax,r12` est `jae` : refuser quand `n >= cap`.

### Bogue 3 : débordement d'entier avant une allocation

```text
000000000000005d <make_table>:
  65:  mov    r13d,edi                     ; count
  68:  mov    r12d,esi                     ; elem
  6b:  imul   edi,esi                      ; total = count * elem, in 32 bits
  6e:  call   malloc                       ; malloc(total), rdi zero-extended
  ...
  8b:  mov    rdx,r12                      ; the loop uses elem ...
  93:  call   memset
  98:  add    ebx,0x1
  9e:  cmp    r13d,ebx                     ; ... for count iterations
  a1:  jne    8b
```

La multiplication est un `imul` de 32 bits de deux valeurs venant de l'entrée, et le résultat va directement à `malloc`. La boucle qui remplit la table utilise `count` et `elem` séparément et ne regarde jamais la taille allouée. Si le produit boucle (un nombre de 0x10000001 avec une taille d'élément de 16 donne 16), l'allocation est minuscule et la boucle écrit bien au-delà. La signature à chercher est une instruction arithmétique sur des valeurs dérivées de l'entrée dont le résultat est une taille d'allocation, avec les mêmes entrées réutilisées plus loin pour une boucle ou un indice. Le rapport du sanitizer est une écriture de taille 16 après la fin d'une région de tas de 16 octets. La réparation calcule la taille sur 64 bits, ce qui ne peut pas déborder pour des entrées de 32 bits, et applique une limite de politique avant d'allouer. `__builtin_mul_overflow` rend le même service quand les opérandes sont déjà sur 64 bits.

### Bogue 4 : signé et non signé

```text
00000000000000af <copy_block>:
  af:  cmp    edx,0x40                     ; len against 64
  b2:  jg     c3 <copy_block+0x14>         ; signed: greater than 64 is refused
  b4:  push   rbx
  b5:  mov    ebx,edx
  b7:  movsxd rdx,edx                      ; sign-extend len to 64 bits
  ba:  call   memcpy                       ; size = rdx
```

Deux instructions ensemble forment la signature : une comparaison signée (`jg`) qui ne protège que l'extrémité haute, et une extension de signe (`movsxd`) quand la valeur est élargie à la taille de 64 bits. Un `len` négatif passe la comparaison, est étendu en signe jusqu'à un énorme nombre non signé et arrive à `memcpy` comme taille. Le sanitizer appelle cela `negative-size-param`. Dans la version réparée, le compilateur émet une seule comparaison non signée (`cmp edx,0x40` puis `ja`), parce que le source refuse aussi `len < 0` et qu'une valeur négative se lit comme un grand nombre non signé. Retenez la règle : une taille est non signée, et la vérification de la borne haute d'une valeur signée est incomplète sans celle de la borne basse.

### Bogue 5 : appels de bibliothèque dangereux

```text
00000000000000c9 <greet>:
  c9:  push   rbx
  ca:  sub    rsp,0x20                     ; a 32-byte frame; the buffer is 24 bytes of it
  ce:  mov    rdx,rdi                      ; the user string
  d1:  mov    rbx,rsi
  d4:  mov    esi,0x0                      ; the format "hello, %s" (a relocation)
  d9:  mov    rdi,rsp                      ; destination: a buffer on the stack
  dc:  mov    eax,0x0
  e1:  call   sprintf                      ; no size anywhere
  e6:  mov    rsi,rsp
  e9:  mov    rdi,rbx
  ec:  call   strcpy                       ; and the copy to the caller's buffer is unbounded too
```

Certaines fonctions de bibliothèque ne peuvent pas être utilisées sans risque parce qu'elles ne prennent pas la taille de la destination : `strcpy`, `strcat`, `sprintf`, `vsprintf`, `gets` et `scanf` avec une conversion `%s` sans largeur. D'autres prennent une taille et sont pourtant mal utilisées : `strncpy` n'ajoute pas de terminateur quand la source est aussi longue que la limite, et `memcpy` prend une taille que l'appelant doit avoir vérifiée. Un appel est dangereux quand la destination est un tampon de taille fixe (une zone du cadre de pile ou une globale, comme `rdi = rsp` ici), que la source dépend de l'entrée et qu'aucune comparaison avec la taille de la destination ne s'interpose. Beaucoup de distributions compilent avec le renforcement (fortification), qui remplace de tels appels par `__sprintf_chk` et des fonctions semblables qui reçoivent la taille de la destination en argument supplémentaire (la compilation `-O1` avec les options par défaut montre `__sprintf_chk` avec la taille 0x18). Cela transforme l'écrasement silencieux en arrêt, l'effet est donc moins grave, mais le bogue est toujours là et mérite toujours un rapport. Voir une fonction `_chk` dans un binaire indique que le code a été compilé avec la protection ; voir la fonction simple indique qu'il ne l'a pas été. La réparation utilise `snprintf` avec la capacité et vérifie le résultat.

### Bogue 6 : utilisation après libération

```text
00000000000000f7 <finish>:
  f7:  push   rbx
  f8:  mov    rbx,rdi                      ; rbx = the item
  fb:  test   esi,esi
  fd:  je     103 <finish+0xc>             ; ok == 0: go to the free
  ff:  mov    eax,DWORD PTR [rbx]          ; read it->id
 101:  pop    rbx
 102:  ret
 103:  call   free                          ; free(rbx)
 108:  jmp    ff <finish+0x8>              ; and then continue at the read through rbx
```

La signature d'une utilisation après libération (use after free) dans le désassemblage est un chemin qui passe par un `call free` sur un pointeur puis atteint une instruction qui déréférence de nouveau le même pointeur, ici par le saut en arrière vers `ff`. Suivez le registre ou l'emplacement de pile qui contenait l'argument de `free` : s'il est lu après l'appel, sur un chemin quelconque, le code utilise de la mémoire libérée. Un second `call free` sur la même valeur est une double libération. Le sanitizer signale `heap-use-after-free` avec une lecture de taille 4. Dans la version réparée, `18_fixed.c` lit le champ avant la libération. Une autre bonne habitude est de ranger un pointeur nul après la libération, pour qu'une utilisation ultérieure échoue aussitôt.

### Confirmer une découverte sans causer de dégâts

Une découverte est plus utile avec des preuves. Les formes sûres de preuve sont :

- Exécutez votre propre compilation du code sous un sanitizer, avec une entrée qui déclenche le chemin. C'est la meilleure preuve quand vous avez le source.
- Dans un débogueur, arrêtez-vous au puits avant son exécution et lisez les nombres : la taille et la capacité. Cela ne demande ni plantage ni écriture excessive.
- Pour le binaire seul, montrez le chemin dans le désassemblage : la source, la comparaison absente ou fausse, et le puits.

Ne fabriquez pas d'entrées dont le but est de prendre le contrôle du programme. Pour l'audit, peu importe ce qu'un attaquant pourrait faire ensuite ; ce qui compte, c'est que le programme touche de la mémoire qu'il ne devrait pas toucher, et que le correctif en supprime la cause.

### Rédiger le rapport

Un rapport est lu par la personne qui doit corriger le bogue. Il doit être exact et bref. Utilisez les mêmes champs à chaque fois.

```text
Title:      Unchecked packet length in set_name
Class:      CWE-787 out-of-bounds write (CWE-120, copy without checking the size)
Location:   set_name, call to memcpy at offset 0x13 (source line 14)
Severity:   High. The length and the bytes both come from the input, and the
            destination is a fixed buffer.
Cause:      set_name copies the number of bytes given by the first byte of the
            packet (0 to 255) into a destination whose size it does not know.
            No comparison separates the load of the length from the call.
Evidence:   Debugger, stopped before the copy: destination 16 bytes, length 40.
            Sanitizer build: stack-buffer-overflow, WRITE of size 40 at
            18_bugs.c:14.
Impact:     Memory outside the destination is overwritten with bytes from the
            packet. This corrupts adjacent data and can crash the program. What
            is next to the buffer depends on the caller; that was not assessed.
Fix:        Pass the capacity and the packet length. Reject a length larger than
            the rest of the packet or not smaller than the capacity. See
            set_name in 18_fixed.c.
Test:       Packets with lengths 0, 15, 16, 17 and 255 and a one-byte packet.
            The first two succeed, the rest are rejected without a write. Run
            the tests under the sanitizer.
```

Choisissez dans la liste publique des faiblesses (CWE) la classe qui correspond à la cause, et non au symptôme : 787 pour une écriture hors limites, 125 pour une lecture, 193 pour une erreur d'un, 190 pour un débordement ou bouclage d'entier, 195 pour une conversion de signé en non signé, 416 pour l'utilisation après libération, 415 pour une double libération et 676 pour l'emploi d'une fonction dangereuse. Indiquez la gravité en la justifiant : qui contrôle l'entrée, ce qui est écrit et où. Dites ce que vous n'avez pas évalué. Si vous avez trouvé le bogue dans un logiciel qui appartient à quelqu'un d'autre, signalez-le en privé à son responsable et laissez-lui le temps de le corriger avant d'en parler en public.

### Vérifier le correctif

Un correctif se vérifie comme le bogue. Relancez la même entrée et constatez que le rapport a disparu. Lancez les valeurs limites : la plus grande valeur acceptée, la plus petite refusée, zéro, négatif, et la valeur qui déborderait. Le programme `demo_fix` le fait pour les six fonctions réparées :

```text
name:   -1
name:   0 (alice)
label:  -1
label:  0
table:  refused
block:  -1
greet:  -1
greet:  0 (hello, bob)
finish: 7
```

Chaque entrée dangereuse est refusée sans aucun accès mémoire, et l'entrée ordinaire fonctionne toujours. Un correctif qui ne fait que déplacer le problème (par exemple un tampon plus grand) n'est pas un correctif.

## Essayez

Lancez les commandes du laboratoire depuis `modules/18-auditing-binaries/examples`.

1. Construisez avec `make` et lancez `make run`. Pour chacun des six cas, notez la classe du bogue et la ligne du rapport avant de lire l'explication de la leçon.
2. Désassemblez `build/18_bugs.o` avec `objdump -dr -M intel`. Trouvez l'appel à `memcpy` dans `copy_block` et remontez à l'origine de `rdx`. Quelle instruction l'a produit à partir de `edx` ?
3. Comparez `build/18_bugs.o` et `build/18_fixed.o` avec la même commande. Pour `copy_label` et `copy_block`, listez les instructions qui ont changé.
4. Lancez `gdb -nx -q build/demo_plain` et utilisez les commandes de la leçon pour arrêter `set_name` avant sa copie et lire les nombres. Ne continuez pas au-delà du point d'arrêt avec l'entrée excessive.
5. Rédigez un rapport au format ci-dessus pour le bogue de `finish`. Incluez la classe, l'emplacement, la cause, la preuve et le correctif.
6. Cherchez dans un programme de votre cru (un petit programme que vous avez écrit) les appels à `strcpy`, `sprintf` et `memcpy` et décidez pour chacun si la taille de la destination est vérifiée.

## Pièges courants

- **Auditer le symptôme.** Un plantage à une ligne vient souvent d'une erreur antérieure, comme une taille d'allocation mal calculée. Suivez la valeur jusqu'à sa source.
- **Comparer la mauvaise quantité.** La vérification doit utiliser la quantité qui est copiée, terminateurs et octets d'en-tête compris.
- **Se fier à un seul type.** Une taille lue comme `int` et passée ensuite comme `size_t` change de sens quand elle est négative. Trouvez où le type change.
- **Croire sûr l'appel renforcé.** Une fonction `_chk` transforme la corruption en arrêt, mais un programme qui s'arrête sur une entrée d'apparence valide a toujours un bogue.
- **Ne corriger qu'un endroit.** Le même motif apparaît en général ailleurs. Cherchez-le après avoir corrigé le premier cas.
- **En démontrer trop.** Le rapport a besoin de la cause et de la preuve, pas d'une démonstration de ce qu'un attaquant pourrait faire.

## Résumé

- Suivez les valeurs d'entrée des sources aux puits et lisez chaque comparaison entre les deux.
- Six motifs couvrent la plupart des découvertes : une longueur non vérifiée, une comparaison erronée d'un, un débordement arithmétique avant une allocation, une valeur signée utilisée comme taille, un appel de bibliothèque dangereux et une utilisation après libération.
- Chaque motif a une signature dans le désassemblage : un opérande de taille sans comparaison, une quantité comparée différente de la quantité copiée, un `imul` sur des valeurs d'entrée avant `malloc`, `jg` avec `movsxd`, un appel sans taille et un déréférencement après `free`.
- Confirmez les découvertes avec une compilation à sanitizer ou un débogueur arrêté avant le puits, jamais avec une entrée nuisible.
- Un rapport indique la classe, l'emplacement, la cause, la preuve, l'impact, le correctif et le test ; un correctif se vérifie avec les valeurs limites.

## Étapes suivantes

- Faites le [quiz](quiz.fr.md).
- Poursuivez avec le module 19, l'analyse de programmes suspects.
- Exercices qui utilisent ce module : [Exercice 18](../../exercises/18-bug-hunt-report/README.fr.md).
