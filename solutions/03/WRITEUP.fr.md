<!-- source: WRITEUP.md @ sha256:6e65700b9119 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 03. Registre comptable

Le drapeau est `ARL{ledger_37000_1010_1100_0011}`. À `mark1`, le registre `rax` contient 37000 ; aux trois marques suivantes, les indicateurs sont CF et SF, puis CF et ZF, puis SF et OF. La solution de référence capture les indicateurs avec `setc`, `setz`, `sets` et `seto` et les combine par des décalages et `or`.

## Partie 1 : Reverse

### Lire d'abord le programme

```text
$ objdump -d -M intel --no-show-raw-insn bin/ledger
  mov    eax,0x3e8
  imul   rax,rax,0x25
<mark1>:
  sub    rax,0x9c40
<mark2>:
  mov    ecx,0xffffffff
  add    ecx,0x1
<mark3>:
  movabs rdx,0x7fffffffffffffff
  add    rdx,0x1
<mark4>:
```

Quatre petits calculs, chacun suivi d'une marque. Prédire les indicateurs avant d'exécuter le programme est tout l'intérêt de l'exercice : faites-le à chaque marque, puis vérifiez avec le débogueur. L'assembleur a écrit `mov rax, 1000` sous la forme `mov eax, 0x3e8`, qui met aussi à zéro la moitié haute de `rax` ; `movabs` est la forme de `mov` qui porte une constante de 64 bits.

### Confirmer dans le débogueur

```text
$ gdb -q bin/ledger
(gdb) break mark1
(gdb) break mark2
(gdb) break mark3
(gdb) break mark4
(gdb) run
(gdb) p/d $rax
(gdb) continue
(gdb) p/x $rax
(gdb) p $eflags
...
```

Les valeurs à chaque marque, issues d'une exécution réelle :

| Marque | Instruction juste au-dessus | Résultat | Indicateurs listés par GDB |
|---|---|---|---|
| `mark1` | `imul rax, rax, 37` | `rax` = 37000 (`0x9088`) | non relevés (`imul` laisse SF et ZF indéfinis) |
| `mark2` | `sub rax, 40000` | `rax` = `0xfffffffffffff448` = -3000 | CF PF SF |
| `mark3` | `add ecx, 1` | `ecx` = 0, `rcx` = 0 | CF PF AF ZF |
| `mark4` | `add rdx, 1` | `rdx` = `0x8000000000000000` | PF AF SF OF |

Lire les indicateurs :

- **`mark2` :** 37000 moins 40000 nécessite un emprunt, car 37000 est inférieur à 40000 en non signé (CF). Le résultat est négatif (SF). Pas de débordement signé (OF est à zéro). PF est la parité de l'octet de poids faible et n'est pas demandé.
- **`mark3` :** `0xFFFFFFFF + 1` repasse à 0 sur 32 bits avec une retenue sortante (CF) et un résultat nul (ZF). Comme l'écriture dans le registre était de 32 bits, la moitié haute de `rcx` est aussi mise à zéro : `rcx` vaut donc 0.
- **`mark4` :** `0x7FFFFFFFFFFFFFFF + 1` est le plus grand nombre signé positif plus un. En résultat signé, il a basculé sur le nombre le plus négatif (OF, SF). En addition non signée, rien n'est sorti (CF est à zéro).

Dans l'ordre CF, ZF, SF, OF : `mark2` vaut `1010`, `mark3` vaut `1100` et `mark4` vaut `0011`. Le drapeau est `ARL{ledger_37000_1010_1100_0011}`.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Chaque fonction effectue son opération, range le résultat avec `mov` (qui ne touche pas aux indicateurs) et saute vers du code partagé par les deux :

```nasm-fragment
ledger_add:
    add     rdi, rsi
    mov     [rdx], rdi
    jmp     collect_flags

collect_flags:
    setc    al                          ; all four setcc first,
    setz    cl                          ; while the flags are still those of the operation
    sets    r8b
    seto    r9b
    movzx   eax, al
    movzx   ecx, cl
    shl     ecx, 1
    or      eax, ecx
    ; ... SF at bit 2 and OF at bit 3 the same way
    ret
```

Pour la soustraction de la partie 1, `ledger_sub(37000, 40000)` renvoie `5` : CF dans le bit 0 et SF dans le bit 2.

## Erreurs qui font perdre du temps

- **Lire les indicateurs après une autre opération arithmétique.** `or`, `shl` et `add` réécrivent les indicateurs. Exécutez d'abord toutes les instructions `set`, puis combinez les octets. Avec un `add rdx, 0` égaré avant la collecte, 6 des 17 cas de test échouent.
- **Utiliser `cmp` ou `test` pour reconstruire les indicateurs.** Cela paraît équivalent, mais le vérificateur l'interdit, et `cmp` écrase en plus les indicateurs que l'opération vient de positionner.
- **Utiliser `lea` pour additionner.** `lea rax, [rdi + rsi]` calcule la somme mais ne positionne aucun indicateur.
- **Oublier que `ledger_sub` emprunte.** CF après `sub` signifie que le premier opérande était inférieur au second en non signé.
- **Ne pas ranger le résultat.** Les tests remplissent d'abord `*out` avec une valeur sentinelle ; omettre l'écriture fait échouer tous les cas.
