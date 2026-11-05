<!-- source: WRITEUP.md @ sha256:30156d030acd -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 12. Standard

Le drapeau est `ARL{switch_11_3_8_2_4011c0_10}` : la table de sauts a 11 entrées, la plus petite commande est 3, la table atteint 8 adresses de code différentes en plus du cas par défaut, 2 entrées pointent vers le cas par défaut, la commande 7 est en 0x4011c0 et la commande 10 partage son code avec la commande 6.

## Partie 1 : Reverse

### Le répartiteur

```bash
objdump -d -M intel --no-show-raw-insn bin/switchboard
```

```text
00000000004011a0 <dispatch>:
  4011a0:    sub    edi,0x3
  4011a3:    cmp    edi,0xa
  4011a6:    ja     401208 <dispatch+0x68>
  4011a8:    jmp    QWORD PTR [rdi*8+0x402008]
  4011b0:    add    esi,0x5
  4011b3:    lea    eax,[rsi+rsi*2+0x1]
  4011b7:    ret
  4011c0:    shl    esi,0x2
  4011c3:    lea    eax,[rsi+0x65]
  4011c6:    ret
  4011d0:    xor    esi,0x55
  4011d3:    lea    eax,[rsi+0x1]
  4011d6:    ret
  4011e0:    mov    eax,0x1
  4011e5:    sub    eax,esi
  4011e7:    ret
  4011f0:    or     esi,0x1
  4011f3:    lea    eax,[rsi+0x1]
  4011f6:    ret
  401200:    lea    eax,[rsi-0x2]
  401203:    ret
  401208:    xor    eax,eax
  40120a:    ret
```

Les instructions `nop` de remplissage entre les blocs sont omises. Les trois premières instructions forment le motif habituel : soustraire la plus petite valeur de cas, comparer à l'indice le plus grand, sauter au cas par défaut si l'indice est supérieur (non signé). On soustrait 3, donc `L` vaut 3. La comparaison est avec 10 ; l'indice le plus grand est donc 10 et la table a 11 entrées, donc `N` vaut 11. Le `jmp` lit à partir de 0x402008.

### La table

```bash
objdump -s -j .rodata bin/switchboard
```

```text
 402000 01000200 00000000 b0114000 00000000
 402010 b3114000 00000000 00124000 00000000
 402020 d0114000 00000000 c0114000 00000000
 402030 c3114000 00000000 08124000 00000000
 402040 d0114000 00000000 08124000 00000000
 402050 e0114000 00000000 f0114000 00000000
```

Les 8 premiers octets à 0x402000 appartiennent aux données d'exécution du programme (le marqueur de version de la bibliothèque standard) ; la table commence en 0x402008, comme l'indique le `jmp`. Chaque entrée fait 8 octets en ordre petit-boutiste ; `b0 11 40 00 00 00 00 00` vaut donc 0x4011b0. En lisant 11 entrées et en leur associant le numéro de commande (indice + 3) :

| Commande | Adresse | Ce que fait le code à l'adresse |
|---|---|---|
| 3 | 0x4011b0 | `acc + 5`, puis continue dans le code de la commande 4 |
| 4 | 0x4011b3 | `acc * 3 + 1` ; cette adresse est à l'intérieur du bloc qui commence en 0x4011b0 |
| 5 | 0x401200 | `acc - 2` |
| 6 | 0x4011d0 | `(acc ^ 0x55) + 1` |
| 7 | 0x4011c0 | `acc << 2`, puis continue dans le code de la commande 8 |
| 8 | 0x4011c3 | `acc + 100 + 1` ; cette adresse est à l'intérieur du bloc qui commence en 0x4011c0 |
| 9 | 0x401208 | le code par défaut |
| 10 | 0x4011d0 | la même adresse que la commande 6 |
| 11 | 0x401208 | le code par défaut |
| 12 | 0x4011e0 | `1 - acc`, c'est-à-dire `-acc + 1` |
| 13 | 0x4011f0 | `acc` avec le bit 0 positionné, plus 1 |

Le code par défaut en 0x401208 est `xor eax, eax`, qui renvoie 0. Dans le source, c'est le résultat de la mise de `acc` à 0xFFFFFFFF puis de l'ajout de 1.

Les réponses :

| Valeur | Raisonnement |
|---|---|
| `N` | 11 entrées |
| `L` | 3, d'après `sub edi, 3` |
| `B` | Les adresses autres que celle par défaut sont 0x4011b0, 0x4011b3, 0x401200, 0x4011d0, 0x4011c0, 0x4011c3, 0x4011e0 et 0x4011f0, soit 8. Les commandes 6 et 10 partagent 0x4011d0 et comptent une fois |
| `G` | Les commandes 9 et 11 pointent vers 0x401208, soit 2 |
| `A` | L'entrée de la commande 7 est `c0 11 40 ...`, donc `4011c0` |
| `S` | L'entrée de la commande 10 est égale à celle de la commande 6, donc 10 |

Le drapeau est `ARL{switch_11_3_8_2_4011c0_10}`.

Deux détails à noter. Le compilateur a transformé une continuation (fall-through) en un bloc à deux points d'entrée : l'entrée de la commande 4 est au milieu du code de la commande 3, de sorte qu'un appel avec la commande 3 exécute l'addition puis la multiplication, tandis qu'un appel avec la commande 4 entre après l'addition. Il en va de même pour les commandes 7 et 8. Et les commandes 9 et 11 n'ont rien de particulier dans la table ; ce sont des trous dans la plage qui vont au même cas par défaut que toute commande hors plage.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Les points qui comptent :

- **L'indice et la vérification de bornes.** `eax = cmd - 3` et `cmp eax, 10` suivi de `ja`. La comparaison est non signée, de sorte que toute commande inférieure à 3 (la soustraction déborde vers un nombre énorme) ou supérieure à 13 va au cas par défaut avec un seul test.
- **La table.** Elle est dans `.rodata`, avec une adresse `dq` par commande de 3 à 13. En NASM, une étiquette locale comme `.c3` appartient à la dernière étiquette non locale ; la table fait donc référence à `run_cmd.c3`.
- **Code partagé.** Les commandes 6 et 10 ont la même adresse. Les commandes 9 et 11 pointent vers le cas par défaut.
- **Continuation.** Le code de la commande 3 fait `add esi, 5` puis passe sans saut à l'étiquette de la commande 4. Il en va de même pour la commande 7 et la commande 8.
- **Les résultats.** Chaque bloc se termine par un `lea eax, [...]` qui calcule la valeur finale et un `ret`. Le `+ 1` final de `dispatch` est intégré dans chaque `lea`, et la commande 5 l'omet.

## Erreurs qui coûtent du temps

- **Une borne trop petite.** `cmp eax, 9` envoie la commande 13 au cas par défaut ; le vérificateur liste chaque cas de la commande 13.
- **Une comparaison signée.** `jg` au lieu de `ja` laisse passer les indices négatifs, et le saut indirect sort de la table ; le programme se termine avec le signal 11.
- **Une mauvaise entrée pour un bloc partagé.** Si la commande 10 pointe vers un autre bloc, seul le résultat de la commande 10 est faux.
- **Retourner à la fin du bloc de la commande 3.** Au résultat de la commande 3 il manque la multiplication.
- **Un cas par défaut qui renvoie -1.** Toute commande hors de 3 à 13 et les commandes 9 et 11 donnent un résultat faux.
- **Une chaîne de comparaisons.** Elle peut donner des résultats justes pour certaines commandes, mais le vérificateur la rejette pour plus de deux instructions `cmp` et pour l'absence de saut indirect.
