<!-- source: WRITEUP.md @ sha256:abe06d06e270 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 13. Nombres magiques

Le drapeau est `ARL{magic_12_9_60_11_5_3}` : le score vaut `(u / 12) * 5 + (u % 9) * 3 - (x / 60) + (x % 11) * 4`, où `u` est `x` lu comme non signé ; les deux premières opérations sont non signées et les deux dernières signées.

## Partie 1 : Reverse

### Le code

```bash
objdump -d -M intel --no-show-raw-insn bin/magic
```

```text
0000000000401190 <mix>:
  401190:    mov    edx,edi
  401192:    mov    ecx,0xaaaaaaab
  401197:    mov    rax,rdx
  40119a:    imul   rcx,rdx
  40119e:    imul   rdx,rdx,0x38e38e39
  4011a5:    mov    edi,eax
  4011a7:    sar    edi,0x1f
  4011aa:    shr    rcx,0x23
  4011ae:    shr    rdx,0x21
  4011b2:    lea    esi,[rcx+rcx*4]
  4011b5:    lea    ecx,[rdx+rdx*8]
  4011b8:    mov    edx,eax
  4011ba:    sub    edx,ecx
  4011bc:    lea    edx,[rdx+rdx*2]
  4011bf:    add    esi,edx
  4011c1:    movsxd rdx,eax
  4011c4:    imul   rcx,rdx,0xffffffff88888889
  4011cb:    imul   rdx,rdx,0x2e8ba2e9
  4011d2:    shr    rcx,0x20
  4011d6:    add    ecx,eax
  4011d8:    sar    rdx,0x21
  4011dc:    sar    ecx,0x5
  4011df:    sub    edx,edi
  4011e1:    sub    ecx,edi
  4011e3:    sub    esi,ecx
  4011e5:    lea    ecx,[rdx+rdx*4]
  4011e8:    lea    edx,[rdx+rcx*2]
  4011eb:    sub    eax,edx
  4011ed:    lea    eax,[rsi+rax*4]
  4011f0:    ret
```

Le compilateur a entremêlé les quatre calculs ; lisez-les donc en suivant chaque constante.

### Les quatre divisions

Chaque division est un `imul` avec une grande constante suivi d'un ou plusieurs décalages à droite. Pour chacune, le diviseur est `2^shift / constant`, arrondi. Le script du module 14 fait le calcul et le vérifie :

```bash
python3 14_magic.py 0xaaaaaaab 35
python3 14_magic.py 0x38e38e39 33
python3 14_magic.py 0x88888889 37 signed
python3 14_magic.py 0x2e8ba2e9 33 signed
```

```text
2**35 / 2863311531 = 11.9999999986  ->  divisor 12
checked 200007 inputs, 0 differences
2**33 / 954437177 = 8.9999999990  ->  divisor 9
checked 200007 inputs, 0 differences
2**37 / 2290649225 = 59.9999999878  ->  divisor 60
checked 200007 inputs, 0 differences
2**33 / 780903145 = 10.9999999962  ->  divisor 11
checked 200007 inputs, 0 differences
```

| Division | Indice | Diviseur |
|---|---|---|
| Quotient non signé | `0xaaaaaaab` (une valeur 32 bits dans `ecx`, donc la multiplication est non signée) et `shr rcx, 0x23`, soit 35 | 12 |
| Reste non signé | `0x38e38e39` et `shr rdx, 0x21`, soit 33 ; le quotient est ensuite multiplié par 9 avec `lea ecx,[rdx+rdx*8]` et retranché de l'entrée | 9 |
| Quotient signé | `0xffffffff88888889` (la constante 32 bits 0x88888889 dont le bit de poids fort est à 1, donc affichée étendue avec signe), `shr rcx, 0x20`, le `add ecx, eax` qui rajoute `x`, et `sar ecx, 5` ; le décalage total est 32 + 5 = 37 | 60 |
| Reste signé | `movsxd`, `0x2e8ba2e9` et `sar rdx, 0x21`, soit 33 ; le quotient est multiplié par 11 avec deux instructions `lea` et retranché de `x` | 11 |

Le quotient signé montre une variante que la leçon ne détaille pas. Le nombre magique pour 60 ne tient pas dans une valeur 32 bits positive ; le compilateur multiplie donc par la valeur 32 bits lue comme négative, et rajoute `x` après avoir pris la moitié haute : `(x * (M - 2^32)) >> 32` plus `x` égale `(x * M) >> 32`. Le script utilise `M` lu comme positif, qui est la valeur dont la formule du diviseur a besoin.

Le `sar edi, 0x1f` près du début calcule une fois le signe de `x` (0 ou -1). Les deux quotients signés le retranchent (`sub edx, edi` et `sub ecx, edi`), ce qui est la correction pour les entrées négatives. Les termes non signés partent de `mov edx, edi` et `mov rax, rdx`, qui étendent `x` avec des zéros ; les termes signés utilisent `movsxd`.

### Les facteurs

Une fois le quotient non signé calculé dans `rcx`, `lea esi, [rcx+rcx*4]` le multiplie par 5, donc `K` vaut 5. Après le reste `u - q * 9`, `lea edx, [rdx+rdx*2]` le multiplie par 3, donc `M` vaut 3. Les deux résultats sont additionnés (`add esi, edx`). Le quotient signé est ensuite retranché (`sub esi, ecx`), et le reste signé est multiplié par 4 et ajouté dans le `lea eax, [rsi+rax*4]` final.

La formule est `(u / 12) * 5 + (u % 9) * 3 - (x / 60) + (x % 11) * 4`, et le drapeau est `ARL{magic_12_9_60_11_5_3}`.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Les points qui comptent :

- **Quotient non signé.** `mov eax, edi` étend `x` avec des zéros dans `rax` ; un `imul` 64 bits par `0xAAAAAAAB` donne le produit complet, et `shr rax, 35` est le quotient. `lea r8d, [rax+rax*4]` le multiplie par 5.
- **Reste non signé.** Le quotient par 9 vient de `0x38E38E39` et d'un décalage de 33. Le reste est `u` moins quotient fois 9 (`lea eax, [rax+rax*8]`), et `lea edx, [rdx+rdx*2]` le multiplie par 3.
- **Quotient signé.** `movsxd rax, edi` étend avec signe. La constante `0x88888889` est chargée avec `mov edx, 0x88888889`, qui étend avec des zéros et la garde donc positive, et le produit 64 bits est décalé de 37 vers la droite. Comme `sar` arrondit vers moins l'infini, on ajoute 1 pour les entrées négatives, ce que fait `sub edx, ecx` avec `ecx = x >> 31`.
- **Reste signé.** La même correction de signe s'applique au quotient `(x * 0x2E8BA2E9) >> 33`, et `q * 11` vaut `q + q*5*2`, calculé avec deux instructions `lea`.
- **Combinaison.** Le dernier `lea` ajoute `d * 4` à la somme courante, qui contient déjà les trois premiers termes.

## Erreurs qui coûtent du temps

- **Pas de correction de signe.** Sans la soustraction de `x >> 31`, un quotient signé est trop petit d'une unité en valeur absolue pour les entrées négatives : le score est décalé de 1 quand la correction manque pour `x / 60`, et de 44 (quatre fois le diviseur 11) quand elle manque pour `x % 11`.
- **Un mauvais décalage.** `shr rax, 34` au lieu de 35 change le quotient pour toute entrée à partir de 6.
- **Étendre avec des zéros les termes signés.** `mov eax, edi` au lieu de `movsxd` donne des résultats faux pour toute entrée négative.
- **Lire la constante comme négative.** Le cas 60 demande la constante sous la forme du positif 2290649225 quand on ne rajoute pas `x`.
- **Utiliser `div`.** Le vérificateur la rejette même si les résultats sont justes.
