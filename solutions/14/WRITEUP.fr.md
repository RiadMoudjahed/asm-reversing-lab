<!-- source: WRITEUP.md @ sha256:0381fee8b79f -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 14. Métamorphe

Le drapeau est `ARL{shape_4_3_24_16_3_9}` : il y a 4 classes, chaque vtable a 3 fonctions, les plus grands objets font 24 octets avec leur dernier champ au décalage 16, trois paires de classes partagent une fonction dans le même emplacement, et les quatre vtables contiennent ensemble 9 adresses de fonction différentes.

## Partie 1 : Reverse

### La fabrique et les objets

La fonction qui traite `kind` appelle `operator new` une fois par classe et range le pointeur de vtable et les champs. Les quatre chemins de code, avec l'argument de `new` et les rangements qui suivent :

```text
mov    edi,0x10                       ; new(16)
call   _Znwm@plt
mov    QWORD PTR [rax],0x402030       ; vptr
mov    DWORD PTR [rax+0x8],ebx        ; first field

mov    edi,0x10                       ; new(16)
call   _Znwm@plt
mov    DWORD PTR [rax+0x8],ebx
mov    QWORD PTR [rax],0x402058       ; vptr
mov    DWORD PTR [rax+0xc],ebp        ; second field

mov    edi,0x18                       ; new(24)
call   _Znwm@plt
mov    DWORD PTR [rax+0x8],ebx
mov    DWORD PTR [rax+0xc],ebp
mov    QWORD PTR [rax],0x402080       ; vptr
mov    DWORD PTR [rax+0x10],r12d      ; third field

mov    edi,0x10                       ; new(16)
call   _Znwm@plt
mov    DWORD PTR [rax+0x8],ebx
mov    QWORD PTR [rax],0x4020a8       ; vptr
mov    DWORD PTR [rax+0xc],ebp        ; second field
```

Quatre pointeurs de vtable différents sont rangés, donc `N` vaut 4. Trois classes ont des objets de 16 octets et une a un objet de 24 octets, donc `Z` vaut 24. L'objet de 24 octets a des champs aux décalages 8, 12 et 16, et le dernier est en 16, donc `P` vaut 16. Tout objet commence par le pointeur de vtable au décalage 0, et le premier champ est au décalage 8, ce qui laisse 4 octets de remplissage dans les objets de 16 octets après le décalage 12.

Les classes peuvent être nommées d'après ce qu'elles partagent. La classe aux trois champs est la plus dérivée ; la classe dont la vtable partage la première fonction avec elle est son parent (elle a deux champs) ; la racine est la classe qui n'a qu'un champ ; et la classe restante a deux champs et dérive de la racine.

### Les vtables

Chaque pointeur rangé pointe 16 octets à l'intérieur d'une table de `.rodata`. Les deux mots qui le précèdent sont le décalage vers le début de l'objet et l'information de type à l'exécution manquante, tous deux nuls. Les trois mots à partir du pointeur sont les fonctions virtuelles :

```text
vtable pointer   slot 0 (offset 0)   slot 1 (offset 8)   slot 2 (offset 16)
0x402030         0x401272            0x40127a            0x401280
0x402058         0x401286            0x40127a            0x401290
0x402080         0x401286            0x40129a            0x4012ac
0x4020a8         0x4012ba            0x4012c2            0x401280
```

Dans la boucle principale, trois appels virtuels utilisent `[rax+0x10]`, `[rax+0x8]` et `[rax]` à partir du pointeur de vtable ; il y a donc trois emplacements et `S` vaut 3.

En comparant les lignes emplacement par emplacement :

| Emplacement | Adresses égales | Entre quelles tables |
|---|---|---|
| 0 | 0x401286 | la deuxième et la troisième |
| 1 | 0x40127a | la première et la deuxième |
| 2 | 0x401280 | la première et la quatrième |

Cela fait trois paires de classes avec une adresse égale dans le même emplacement, donc `E` vaut 3. Les neuf adresses différentes sont 0x401272, 0x40127a, 0x401280, 0x401286, 0x401290, 0x40129a, 0x4012ac, 0x4012ba et 0x4012c2, donc `F` vaut 9.

Le motif de partage donne la hiérarchie. La première table est la classe de base : son deuxième emplacement (0x40127a) et son troisième emplacement (0x401280) sont hérités par d'autres. La deuxième table partage l'emplacement 1 avec la base et redéfinit les emplacements 0 et 2. La troisième table partage l'emplacement 0 avec la deuxième et redéfinit les deux autres, c'est donc une classe dérivée de la deuxième. La quatrième table partage l'emplacement 2 avec la base et redéfinit les emplacements 0 et 1. Les classes auxquelles appartiennent les objets sont donc : la racine (16 octets), une dérivée avec un champ ajouté (16 octets), une dérivée à nouveau avec un autre champ (24 octets) et une seconde classe dérivée de la racine (16 octets).

Le drapeau est `ARL{shape_4_3_24_16_3_9}`.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Les points qui comptent :

- **La disposition.** Le décalage 0 contient le pointeur de vtable. Le décalage 8 contient le premier champ de toutes les classes. Le décalage 12 contient le deuxième champ, qui existe pour trois classes. Le décalage 16 contient le troisième champ, que seule la plus grande classe possède.
- **Le pointeur de vtable.** `gadget_init` appelle `gadget_vptr(kind)` et range le résultat dans le premier mot. Les valeurs de kind sont celles sur lesquelles la fonction fabrique de `bin/gadgets` fait son switch, et `gadget_vptr` renvoie le pointeur de la classe correspondante, soit l'adresse située 16 octets à l'intérieur de sa table.
- **Champs.** Les arguments `a`, `b` et `c` vont aux décalages 8, 12 et 16, et une classe ne reçoit que les champs qu'elle a. Comme l'appel à `gadget_vptr` détruit les registres d'arguments, `gadget_init` les garde dans des registres à sauvegarder par l'appelé et empile d'abord ces registres.
- **L'appel virtuel.** `gadget_call` charge le pointeur de vtable avec `mov rax, [rdi]`, multiplie le numéro d'emplacement par 8, déplace `arg` dans `esi` et appelle `[rax + rcx*8]`. Le pointeur d'objet est toujours dans `rdi`, qui est `this` pour l'appelé. La pile est alignée avec un seul `sub rsp, 8` parce qu'à l'entrée `rsp` vaut 8 modulo 16.
- **Registres.** Seuls des registres à sauvegarder par l'appelé qui ont été sauvegardés sont modifiés, et ils sont restaurés dans l'ordre inverse.

## Erreurs qui coûtent du temps

- **Ranger un champ au mauvais décalage.** Les appels virtuels de la bibliothèque lisent d'autres valeurs que celles que calcule le binaire, et les résultats des tests diffèrent.
- **Le pointeur de vtable au mauvais endroit.** Le vérificateur signale `vptr` avant de faire le moindre appel, car un appel par un mauvais pointeur planterait.
- **Multiplier l'emplacement par 4.** L'appel va à une adresse à l'intérieur d'une fonction ou dans le vide, et le programme se termine avec le signal 11.
- **Ne pas déplacer `arg` dans `esi`.** L'appelé reçoit ce que contenait `esi`, et le vérificateur signale `call`.
- **Détruire un registre à sauvegarder par l'appelé.** Les résultats sont justes et le vérificateur signale `regs`.
- **Écrire au-delà de l'objet.** Un rangement au décalage 40, hors de tous les objets, est signalé comme `canary`.
