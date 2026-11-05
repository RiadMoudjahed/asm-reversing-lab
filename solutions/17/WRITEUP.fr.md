<!-- source: WRITEUP.md @ sha256:9ade8d8d380b -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 17. Archive des inconnues

Le drapeau est `ARL{ark_16_4_7_122_48}` : une entrée de table fait 16 octets, les blocs de données sont alignés sur 4 octets, la somme de contrôle est tournée de 7 bits par octet, la clé pour 3 fichiers est 122, et l'outil accepte au plus 48 fichiers.

## Partie 1 : Reverse

### D'abord des échantillons

Une archive de trois petits fichiers donne ce vidage (`a.txt` contient `AAA`, `hello.txt` contient `hello world` et un saut de ligne, `empty.bin` est vide) :

```text
00000000: 4152 4b32 0300 7a00 2000 0000 5000 0000  ARK2..z. ...P...
00000010: 3b3a 3d00 121e 1011 115f f7ee f0ef e08f  ;:=......_......
00000020: 0000 0000 1000 0000 0300 0000 b7cb f27b  ...............{
00000030: 0600 0000 1400 0000 0c00 0000 b504 3829  ..............8)
00000040: 1000 0000 2000 0000 0000 0000 df9b 5713  .... .........W.
00000050: 612e 7478 7400 6865 6c6c 6f2e 7478 7400  a.txt.hello.txt.
00000060: 656d 7074 792e 6269 6e00                 empty.bin.
```

Les quatre premiers octets sont la valeur magique `ARK2`. Vient ensuite `03 00`, le nombre de fichiers. L'octet suivant est `7a`, soit 122, et celui d'après est nul. Viennent ensuite deux nombres de 32 bits, 0x20 et 0x50, qui sont des décalages : à 0x20 se trouve une table de trois groupes de 16 octets, et à 0x50 les noms `a.txt`, `hello.txt` et `empty.bin`, chacun terminé par un octet nul. Les blocs de données sont entre l'en-tête et la table : le fichier `AAA` a été stocké sous la forme `3b 3a 3d`, et `hello world` sous la forme de 12 octets qui ne ressemblent pas à du texte. Les trois octets `41` sont devenus trois valeurs différentes, les données sont donc transformées par quelque chose qui change le long du bloc. Les valeurs 0x3b, 0x3a et 0x3d sont 0x41 xor 0x7a, 0x41 xor 0x7b et 0x41 xor 0x7c : la clé 0x7a plus la position de l'octet, en ne gardant que l'octet de poids faible. Ainsi l'octet i d'un bloc est stocké xor l'octet de poids faible de key + i.

Chaque entrée de la table est faite de quatre nombres de 32 bits. Pour la première, ce sont 0, 0x10, 3 et 0x7bf2cbb7 : le décalage du nom dans la zone de noms, le décalage des données (0x10, juste après l'en-tête), la taille et une valeur qui dépend du contenu. La deuxième entrée a le décalage de nom 6 (après `a.txt` et son octet nul) et le décalage de données 0x14, qui est la fin du premier bloc (0x10 + 3) arrondie à un multiple de 4. La troisième entrée a la taille 0 et la valeur 0x13579bdf : la valeur de départ de la somme de contrôle, inchangée par un bloc vide.

### Le programme

Le code confirme les échantillons et ajoute les limites. L'en-tête est construit avec ces instructions :

```text
40134d:  mov    DWORD PTR [out],0x324b5241   ; "ARK2"
401357:  mov    eax,DWORD PTR [rsp+0x14]     ; number of files
40135b:  mov    BYTE PTR [out+4],al          ; count, low byte
401361:  mov    BYTE PTR [out+5],0x0         ; count, high byte
401368:  imul   eax,eax,0x25                 ; count * 37
40136b:  add    eax,0xb                      ; + 11
40136e:  mov    BYTE PTR [out+6],al          ; the key
401374:  mov    BYTE PTR [out+7],0x0
```

La clé est `count * 37 + 11` tronqué à un octet. Pour 3 fichiers, cela donne 3 * 37 + 11 = 122, donc `K` vaut 122. Les limites se trouvent avant la boucle :

```text
4011fb:  cmp    DWORD PTR [rsp+0x14],0x30    ; more than 48 files: "too many files"
```

La valeur 0x30 est 48, donc `N` vaut 48. Une comparaison `cmp rax,0x1f` protège la longueur d'un nom (31 caractères au plus), et `cmp rbx,0x10000` la taille d'un fichier (65536 octets au plus). Les décalages de données sont arrondis avec `add rbp,0x3` et `and rbp,0xfffffffffffffffc` : on ajoute 3 et on efface les deux bits bas, donc l'alignement `A` est 4. Les entrées sont adressées avec `shl rax,0x4`, un facteur 16, donc `E` vaut 16. La boucle de la somme de contrôle part de 0x13579bdf :

```text
40147c:  mov    edx,0x13579bdf
401481:  rol    edx,0x7                      ; rotate left by 7
...
401497:  jne    401481                       ; for every byte
```

donc `R` vaut 7. Après la boucle, la valeur est combinée avec la taille par un ou exclusif. L'entrée vide ne peut pas le montrer, car la taille est 0, mais l'entrée de `AAA` le peut : la somme cumulée simple des trois octets est 0x7bf2cbb4, et la valeur stockée 0x7bf2cbb7 est cette somme xor 3.

Le format complet, tous les nombres en petit-boutiste :

| Partie | Décalage | Taille | Contenu |
|---|---|---|---|
| En-tête | 0 | 4 | Valeur magique `ARK2` |
| | 4 | 2 | Nombre de fichiers |
| | 6 | 1 | Clé : `count * 37 + 11`, octet de poids faible |
| | 7 | 1 | Zéro |
| | 8 | 4 | Décalage de la table des entrées |
| | 12 | 4 | Décalage de la zone de noms |
| Blocs de données | multiples de 4 à partir de 16 | taille | L'octet i est stocké xor l'octet de poids faible de `key + i` ; remplissage par des octets nuls |
| Table des entrées | décalage de la table | 16 par fichier | Décalage du nom (relatif à la zone de noms), décalage des données, taille, somme de contrôle |
| Zone de noms | décalage de la zone de noms | | Noms terminés par zéro, l'un après l'autre |

La somme de contrôle d'une entrée est calculée sur les octets d'origine : partir de 0x13579bdf, pour chaque octet tourner de 7 bits vers la gauche et ajouter l'octet, et à la fin faire un xor avec la taille.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Les points qui comptent :

- **`ark_count` fait la validation.** Elle vérifie la longueur, la valeur magique (une seule comparaison de 32 bits avec 0x324b5241), la table (son décalage d'au moins 16 et sa fin, calculée sur 64 bits comme décalage plus 16 fois le nombre, pas au-delà de `n`) et le décalage de la zone de noms (pas au-delà de `n`). Les deux autres fonctions l'appellent, l'en-tête est donc vérifié en un seul endroit. Comme elle ne modifie que `rax`, `rcx` et `rdx`, les appelants gardent leurs arguments dans les autres registres.
- **L'indice.** Une comparaison non signée avec le nombre rejette d'un coup les indices négatifs et ceux qui demandent plus de 32 bits. Une comparaison sur 32 bits laisserait passer l'indice 4294967296 comme 0.
- **Noms.** Le décalage du nom est ajouté sur 64 bits à celui de la zone de noms, doit se trouver dans l'archive, et l'octet nul est cherché avec une limite à `n`. C'est seulement alors que la capacité est comparée à la longueur plus un, et que le nom est copié avec son octet nul.
- **Extraction.** Le décalage des données et la taille sont des valeurs de 32 bits étendues par des zéros. La somme est calculée sur 64 bits, de sorte qu'un décalage 0xfffffff0 avec une taille 0x20 est rejeté et ne repasse pas par un petit nombre. La capacité est vérifiée ensuite, puis chaque octet est décodé avec l'octet de poids faible de `key + i`, écrit et ajouté à la somme de contrôle après une rotation de 7. La somme est terminée avec la taille et comparée à la valeur stockée.
- **Registres.** Rien en dehors des registres que l'appelé peut modifier n'est utilisé, il n'y a donc ni sauvegarde ni restauration. Le `call` interne est précédé de `sub rsp, 8` pour garder la pile alignée.

## Erreurs qui font perdre du temps

- **Additionner le décalage et la taille sur 32 bits.** Une archive endommagée avec un énorme décalage passe la vérification des bornes et le code lit loin en dehors de l'archive.
- **Une comparaison signée de l'indice.** Un indice négatif est accepté et l'entrée de la table est calculée à partir d'un nombre négatif.
- **Oublier la position dans la clé.** Seul le premier octet d'un bloc est bien décodé.
- **Oublier la taille dans la somme de contrôle.** Toute archive avec un bloc non vide est déclarée endommagée, mais un bloc vide passe, ce qui cache l'erreur à un test hâtif.
- **Tourner de 8.** Le premier octet est bien décodé et la somme de contrôle ne correspond jamais.
- **Chercher l'octet nul sans limite.** Une archive dont le dernier nom n'a pas de terminateur fait lire le code au-delà de la fin.
- **Écrire la sortie avant de vérifier la capacité.** La référence admet n'importe quel contenu dans `out` après une erreur, mais jamais au-delà de `cap`.
