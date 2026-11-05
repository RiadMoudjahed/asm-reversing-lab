<!-- source: WRITEUP.md @ sha256:61ad56447536 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 06. Labyrinthe de branchements

Le drapeau est `ARL{maze_101_m31_m56}`. Les entrées acceptées sont A dans {101, 109, 117}, B de -31 à -21 et C égal à -56, soit 33 triplets au total. Le plus petit A est 101, le plus petit B est -31 et C n'a qu'une seule valeur possible.

## Partie 1 : Reverse

### Exécuter le programme n'apprend rien

```text
$ bin/maze 1 2 3
wall
```

Le programme ne dit jamais quel contrôle a échoué : deviner ne passe pas à l'échelle. Les règles se trouvent dans le désassemblage.

### Lire les contrôles

Après les trois appels qui convertissent les arguments en nombres, `main` garde A dans `ebp`, B dans `r12d` et C dans `eax`. Les contrôles, issus du vrai désassemblage :

```text
  lea    edx,[rbp-0x64]
  cmp    edx,0x14
  ja     <wall>
  and    ebp,0x7
  cmp    ebp,0x5
  jne    <wall>
  add    r12d,0x1f
  cmp    r12d,0xa
  ja     <wall>
  test   eax,eax
  jns    <wall>
  cmp    al,0xc8
  jne    <wall>
  cmp    eax,0xfffffed4
  jl     <wall>
```

Lecture de chaque paire :

| Instructions | Règle | Valeurs autorisées |
|---|---|---|
| `lea edx,[rbp-0x64]`, `cmp edx,0x14`, `ja` | A moins 100 vaut au plus 20 en non signé | A de 100 à 120 |
| `and ebp,7`, `cmp ebp,5`, `jne` | Les trois bits de poids faible de A valent 101 | A vaut 5 de plus qu'un multiple de 8 |
| `add r12d,0x1f`, `cmp r12d,0xa`, `ja` | B plus 31 vaut au plus 10 en non signé | B de -31 à -21 |
| `test eax,eax`, `jns` | C est négatif (saut si l'indicateur de signe est à zéro) | C inférieur à 0 |
| `cmp al,0xc8`, `jne` | L'octet de poids faible de C est `0xC8` (200) | C vaut 200 modulo 256 |
| `cmp eax,0xfffffed4`, `jl` | C vaut au moins -300 en signé (`0xfffffed4` est -300) | C à partir de -300 |

Les deux sauts non signés (`ja`) précédés d'une soustraction sont des contrôles de plage. Le `jl` final est signé parce que la borne est un nombre négatif. Si c'avait été un `jb`, la borne aurait été le grand nombre non signé `0xfffffed4`.

### Combiner les règles

- A est compris entre 100 et 120 et vaut 5 modulo 8 : 101, 109 ou 117.
- B est compris entre -31 et -21.
- C est négatif, vaut 200 modulo 256 et est au moins égal à -300. Les nombres négatifs qui valent 200 modulo 256 sont -56, -312, -568 et ainsi de suite, et seul -56 est au moins égal à -300.

Le plus petit triplet est 101, -31, -56 :

```text
$ bin/maze 101 -31 -56
path open
$ bin/maze 100 -31 -56
wall
$ bin/maze 101 -32 -56
wall
$ bin/maze 101 -31 -312
wall
```

Le drapeau est `ARL{maze_101_m31_m56}`.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Elle reprend les mêmes contrôles dans le même ordre, chaque saut d'échec menant à une seule étiquette :

```nasm-fragment
maze_ok:
    lea     eax, [rdi - 100]            ; a - 100 as unsigned: one test for both bounds
    cmp     eax, 20
    ja      .no
    ; ... the AND 7 check on a, the range check on b ...
    test    edx, edx
    jns     .no                         ; c must be negative (signed)
    cmp     dl, 200                     ; the low byte of c
    jne     .no
    cmp     edx, -300
    jl      .no                         ; c >= -300 (signed)
    mov     eax, 1
    ret
.no:
    xor     eax, eax
    ret
```

## Erreurs qui font perdre du temps

- **Utiliser un saut signé là où le contrôle est non signé, ou l'inverse.** Remplacer le test de signe sur `c` par un saut non signé rejette toutes les entrées : les 10 cas de test acceptés échouent.
- **Erreur d'une unité dans une plage.** Utiliser `b + 30` et 9 au lieu de `b + 31` et 10 rejette `b = -31` ; les tests comprennent les deux valeurs limites.
- **Oublier une règle.** Sans le contrôle `and 7` sur `a`, 11 des 90 cas de test échouent, car des valeurs comme 116 et 120 sont acceptées par erreur.
- **Utiliser `setcc` ou `cmov`.** Elles donnent les bonnes réponses, et le vérificateur les rejette par règle.
- **Se fier aux essais et erreurs dans la partie 1.** Le programme ne donne aucun indice sur le contrôle qui a échoué ; la seule voie est de lire les comparaisons.
