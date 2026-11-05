<!-- source: WRITEUP.md @ sha256:7c0f74c0b75b -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 09. Archéologie de la pile

Le drapeau est `ARL{stack_2_40_64_56_16}` : `chain` empile 2 registres à sauvegarder par l'appelé, retranche 40 octets, occupe 64 octets par niveau, a l'adresse de retour sauvegardée 56 octets au-dessus du début de sa table et laisse 16 octets de sa zone locale inutilisés.

## Partie 1 : Reverse

### Le prologue et l'épilogue

```text
chain:
    push   rbp
    ...
    push   rbx
    ...
    sub    rsp,0x28
    ...
    mov    rsi,rsp              ; second argument of note: the table
    mov    QWORD PTR [rsp+0x8],rax
    mov    QWORD PTR [rsp],rbx
    mov    QWORD PTR [rsp+0x10],rbp
    call   note
    ...
    call   chain
    ...
    add    rsp,0x28
    pop    rbx
    pop    rbp
    ret
```

Il n'y a pas de `push rbp; mov rbp,rsp` : `rbp` n'est sauvegardé que parce que la fonction l'utilise comme registre ordinaire, ici pour conserver une valeur entre les appels. Le compilateur utilise `rbx` de la même façon.

| Nombre | Valeur | Indice |
|---|---|---|
| `P` | 2 | Deux instructions `push`, `rbp` et `rbx`, compensées par deux instructions `pop` |
| `S` | 40 | `sub rsp,0x28` dans le prologue, `add rsp,0x28` dans l'épilogue |
| `L` | 64 | 8 (adresse de retour) + 16 (deux push) + 40 (soustraction) |
| `O` | 56 | La table commence en `rsp` ; les registres empilés sont en `rsp + 0x28` et `rsp + 0x30` ; l'adresse de retour est en `rsp + 0x38` |
| `U` | 16 | La table utilise de `[rsp]` à `[rsp + 0x17]`, soit 24 des 40 octets |

### Le cadre

Les adresses les plus hautes sont en haut. Les décalages sont relatifs à `rsp` pendant l'exécution de `chain`.

```text
higher addresses
  [rsp + 0x38]  return address into the caller           8 bytes
  [rsp + 0x30]  saved rbp                                8 bytes
  [rsp + 0x28]  saved rbx                                8 bytes
  [rsp + 0x20]  unused                                   8 bytes
  [rsp + 0x18]  unused                                   8 bytes
  [rsp + 0x10]  table[2]                                 8 bytes
  [rsp + 0x08]  table[1]                                 8 bytes
  [rsp]         table[0]    <- rsp during the calls      8 bytes
lower addresses
```

À l'entrée, `rsp` vaut 8 modulo 16, parce que le `call` de l'appelant a empilé l'adresse de retour sur une frontière de 16 octets. Les deux push le maintiennent à 8 modulo 16, donc la soustraction doit aussi valoir 8 modulo 16 pour laisser `rsp` à un multiple de 16 avant chaque `call`. Des valeurs plus petites qui conviennent sont 24, 40, et ainsi de suite. La table demande 24 octets ; le compilateur en a réservé 40 et en a laissé 16 inutilisés. Le listing ne dit pas pourquoi, et ce corrigé ne le devine pas.

### Vérification avec un débogueur

En s'arrêtant trois fois sur `note` et en affichant `rsp`, on obtient ces valeurs, qui diffèrent de 0x40 (64) à chaque fois :

```text
$1 = 0x7ffe23da3e08
$3 = 0x7ffe23da3dc8
$4 = 0x7ffe23da3d88
```

Au troisième arrêt, la table est en `rsp + 8`, la valeur de `rsi`, et l'adresse de retour sauvegardée (`0x4011e3`, dans `chain` après l'appel récursif) se trouve 0x38 (56) octets au-dessus de la table. Les adresses varient d'une exécution à l'autre ; les différences non.

Le drapeau est `ARL{stack_2_40_64_56_16}`.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Les points qui comptent :

- **Valeurs qui doivent survivre à `note`.** `note` détruit `rdi` et `rsi`, donc `depth` va dans `rbx` et `seed` dans `rbp`. Ces registres doivent être sauvegardés par l'appelé : ils sont empilés à l'entrée et dépilés dans l'ordre inverse à la sortie.
- **La table.** `table[0] = seed ^ (depth * 7)`, `table[1] = seed + depth`, `table[2] = table[0] - table[1]`. Elle reste sur la pile pendant les deux appels, et le code relit `table[0]` et `table[2]` après l'appel récursif.
- **Alignement.** Deux push plus une soustraction de 24 octets laissent `rsp` à un multiple de 16 aux deux appels.
- **Empreinte.** 8 + 16 + 24 = 48 octets par niveau, 16 de moins que la fonction compilée, car il n'y a pas d'espace inutilisé.
- **Le résultat.** À la profondeur 0, la fonction renvoie `seed & 0xff`. Sinon elle renvoie `((below ^ table[0]) * 3) + (table[2] >> 1)`, où `below` est le résultat récursif, et le décalage est arithmétique.

## Erreurs qui coûtent du temps

- **Un cadre de 32 octets.** L'empreinte semble inférieure à 64, mais `rsp` vaut 8 modulo 16 aux appels ; le vérificateur signale `align` dans chaque test.
- **Un cadre de 56 octets.** L'alignement est correct, mais chaque niveau utilise 80 octets ; le vérificateur signale `stack` pour toute profondeur supérieure à 0.
- **Utiliser `rbx` sans le sauvegarder.** Les résultats sont corrects, mais le `rbx` de l'appelant est détruit ; le vérificateur signale `regs`.
- **Lire `rdi` après `note`.** `note` l'écrase avec un motif fixe ; les résultats sont faux dès le premier test.
- **Un décalage logique pour `table[2] >> 1`.** L'entrée de la table est souvent négative ; 7 des 18 vecteurs échouent.
- **Stocker `table[1]` dans `table[2]`.** Le vérificateur signale `table`.
