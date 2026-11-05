<!-- source: WRITEUP.md @ sha256:16bd2713f12d -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 15. Inventaire chaîné

Le drapeau est `ARL{inventory_32_16_20_24_0_4}` : un nœud fait 32 octets, la clé est au décalage 16, le compteur de passage au décalage 20, le compteur de doublons au décalage 24, le pointeur des clés plus grandes au décalage 0, et le compteur de doublons de 16 bits affiche 4 après 65540 insertions de la même clé.

## Partie 1 : Reverse

### La boucle de recherche

La plus courte fonction qui parcourt la structure est la recherche. Le binaire est dépouillé, elle n'a donc pas de nom ; c'est la fonction qui prend un pointeur et une clé et se termine par une boucle sans appel :

```text
401176:  mov    rax,rdi
401179:  test   rdi,rdi
40117c:  jne    401188
40117e:  ret
40117f:  mov    rax,QWORD PTR [rax+0x8]      ; lesser link
401183:  test   rax,rax
401186:  je     401196
401188:  mov    edx,DWORD PTR [rax+0x10]     ; key
40118b:  cmp    esi,edx
40118d:  je     401196                       ; found
40118f:  jle    40117f                       ; key <= node key: follow offset 8
401191:  mov    rax,QWORD PTR [rax]          ; key > node key: follow offset 0
401194:  jmp    401183
401196:  ret
```

La boucle lit la clé au décalage 16 comme une valeur de 32 bits et la compare à `esi` avec `jle`, une comparaison signée. Des clés égales terminent la boucle. Une clé plus petite ou égale suit le pointeur du décalage 8, une clé plus grande suit le pointeur du décalage 0. `G` vaut donc 0 et `K` vaut 16. La structure est un arbre binaire de recherche, et les deux pointeurs sont les liens « plus grand » et « plus petit ».

### L'insertion

La fonction d'insertion alloue et compte :

```text
4011ac:  add    DWORD PTR [rdx+0x14],0x1     ; pass counter
4011b0:  mov    ecx,DWORD PTR [rdx+0x10]
4011b3:  cmp    ebp,ecx
4011b5:  je     4011f5                       ; same key
4011b7:  lea    rbx,[rdx+0x8]                ; link = &node->lesser
4011bb:  cmovg  rbx,rdx                      ; if key > node key: link = &node->greater (offset 0)
4011bf:  mov    rdx,QWORD PTR [rbx]          ; node behind the link
4011c2:  test   rdx,rdx
4011c5:  jne    4011ac
4011c7:  mov    esi,0x20                     ; calloc(1, 32)
4011cc:  mov    edi,0x1
4011d1:  call   calloc
4011d6:  mov    DWORD PTR [rax+0x10],ebp     ; key
4011d9:  mov    DWORD PTR [rax+0x14],0x1     ; pass counter starts at 1
4011e0:  mov    WORD PTR [rax+0x18],0x1      ; duplicate counter starts at 1
4011e6:  mov    QWORD PTR [rbx],rax          ; store into the link
...
4011f5:  add    WORD PTR [rdx+0x18],0x1      ; same key: duplicate counter
```

Plusieurs faits découlent de ce listing :

- **Taille.** L'allocation demande un bloc de 32 octets, donc `S` vaut 32. Le dernier champ se termine au décalage 26, le bloc a donc 6 octets de remplissage. La taille vient de l'appel, pas des champs.
- **Compteur de passage.** L'instruction à `4011ac` s'exécute, avant la comparaison, pour chaque nœud que visite le parcours, y compris le nœud de clé égale. C'est un champ de 32 bits au décalage 20, donc `T` vaut 20.
- **Compteur de doublons.** L'instruction à `4011f5` ne s'exécute que lorsque la clé a été trouvée. Elle additionne sur un `WORD PTR` : le champ fait 16 bits et se trouve au décalage 24, donc `C` vaut 24.
- **Le lien.** Le code garde dans `rbx` l'adresse du lien à suivre, et non le nœud. Le premier lien est l'emplacement de pile qui contient la racine (`lea rbx,[rsp+0x8]` à `4011ff`). Lorsque le lien contient 0, le nouveau nœud y est rangé. Un seul morceau de code traite une structure vide et une nouvelle feuille à n'importe quelle profondeur.
- **Nouveaux nœuds.** Les deux compteurs commencent à 1 et les deux liens vers les enfants restent à 0, parce que `calloc` met le bloc à zéro.

### Le débordement

Comme le compteur de doublons fait 16 bits, il déborde après 65535. La commande `f 7` affiche d'abord le compteur de doublons, puis le compteur de passage. Après 65540 insertions de la clé 7, le premier nombre est `65540 mod 65536`, soit 4, et le second est 65540 :

```text
$ for i in $(seq 65540); do echo 'i 7'; done | { cat; echo 'f 7'; } | bin/inventory | tail -1
4 65540
```

Donc `W` vaut 4. Le compteur de passage fait 32 bits et ne déborde pas ici.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Les points qui comptent :

- **La disposition.** Le décalage 0 est le lien plus grand, le décalage 8 le lien plus petit, le décalage 16 la clé, le décalage 20 le compteur de passage et le décalage 24 le compteur de doublons. Le nœud fait 32 octets, et les octets 26 à 31 restent à zéro.
- **`inv_find`.** Elle n'a besoin ni de pile ni de registres sauvegardés. À chaque tour, elle teste si le lien est vide, compare la clé avec `cmp esi, [rdi + 16]` et s'arrête en cas d'égalité. Les indicateurs de ce `cmp` sont encore valables quand `cmovle` choisit le lien plus petit plutôt que le plus grand, parce que les chargements intermédiaires ne modifient pas les indicateurs.
- **`inv_insert`.** Elle garde l'adresse du lien dans `r13`. Le premier lien est un emplacement de pile qui contient la racine, la fonction réserve donc 16 octets. Trois push et les 16 octets gardent `rsp` aligné pour l'appel à `inv_alloc`. À chaque nœud, elle incrémente le compteur de passage avec un `inc` de 32 bits, compare la clé, puis soit incrémente le compteur de doublons avec un `inc` de 16 bits, soit choisit le lien suivant avec `lea` et `cmovg`.
- **Un nouveau nœud.** `inv_alloc` renvoie de la mémoire mise à zéro. Le code range la clé, met les deux compteurs à 1 et range le pointeur du nœud dans le lien. La fonction renvoie la valeur de l'emplacement de la racine, qui n'a changé que si la structure était vide.
- **Registres.** `rbx`, `r12` et `r13` sont sauvegardés et restaurés dans l'ordre inverse.

## Erreurs qui font perdre du temps

- **Utiliser un incrément de 32 bits pour le compteur de doublons.** La valeur affichée par la bibliothèque est la même jusqu'au débordement du compteur, mais l'incrément écrit aussi dans le remplissage qui suit le champ. Le vérificateur signale `overrun`.
- **Oublier le compteur de passage au nœud qui correspond.** Les totaux de tous les nœuds sur le chemin d'une clé répétée sont trop petits d'une unité.
- **Comparer les clés comme des valeurs non signées.** Les clés négatives et les valeurs extrêmes vont du mauvais côté. Le texte de l'arbre montre les clés aux mauvais endroits.
- **Appeler `inv_alloc` pour une clé qui existe déjà.** L'arbre paraît correct, mais le vérificateur signale `alloc`.
- **Garder la racine dans un registre.** Un lien doit avoir une adresse ; `r13` peut pointer vers un emplacement de pile ou vers un champ d'un nœud, mais pas vers un registre.
- **Une pile mal alignée à l'appel.** L'appel peut fonctionner par chance ; le vérificateur signale `align`.
