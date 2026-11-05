<!-- source: WRITEUP.md @ sha256:312a383646e6 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 01. Fusée de détresse

Le drapeau est `ARL{flare_42_23_7}`. Le programme écrit 42 octets sur la sortie standard, 23 octets sur la sortie d'erreur standard et se termine avec le code 7. Ce corrigé montre comment y parvenir et comment le reconstruire.

## Partie 1 : Reverse

### Identifier le fichier

```text
$ file bin/signal_flare
bin/signal_flare: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, not stripped
```

« Statically linked » signifie qu'aucune bibliothèque n'est chargée. « Not stripped » signifie que les noms de symboles subsistent, ce qui explique que `_start` soit visible dans le désassemblage.

### Observer les appels système

```text
$ strace ./bin/signal_flare
execve("./bin/signal_flare", ["./bin/signal_flare"], ...) = 0
write(1, "FLARE LAUNCHED\nCOLOR: RED\nALTITU"..., 42) = 42
write(2, "[flare] burn time 30 s\n", 23[flare] burn time 30 s
) = 23
exit(7)                                 = ?
+++ exited with 7 +++
```

`strace` affiche les 32 premiers octets de chaque tampon, si bien que le premier message est tronqué, mais le nombre d'octets (le dernier argument) et le résultat sont complets. Le second message apparaît au milieu de la ligne de strace parce que la sortie d'erreur standard et la sortie propre de strace partagent le même terminal. C'est normal et ce n'est pas un défaut du programme.

Les trois réponses sont déjà visibles : 42 octets vers le descripteur 1, 23 octets vers le descripteur 2, code de sortie 7.

### Confirmer dans le désassemblage

`strace` montre ce qui s'est passé lors d'une exécution. Le désassemblage montre ce que fait le code, ce qui compte lorsqu'un programme se comporte autrement avec d'autres entrées.

```text
$ objdump -d -M intel bin/signal_flare
0000000000401000 <_start>:
  401000:  b8 01 00 00 00        mov    eax,0x1
  401005:  bf 01 00 00 00        mov    edi,0x1
  40100a:  48 8d 35 ef 0f 00 00  lea    rsi,[rip+0xfef]        # 402000 <report>
  401011:  ba 2a 00 00 00        mov    edx,0x2a
  401016:  0f 05                 syscall
  401018:  b8 01 00 00 00        mov    eax,0x1
  40101d:  bf 02 00 00 00        mov    edi,0x2
  401022:  48 8d 35 01 10 00 00  lea    rsi,[rip+0x1001]       # 40202a <notice>
  401029:  ba 17 00 00 00        mov    edx,0x17
  40102e:  0f 05                 syscall
  401030:  b8 3c 00 00 00        mov    eax,0x3c
  401035:  bf 07 00 00 00        mov    edi,0x7
  40103a:  0f 05                 syscall
```

Lu comme trois appels :

| Appel | `eax` | `edi` | `rsi` | `edx` | Signification |
|---|---|---|---|---|---|
| 1 | 0x1 | 0x1 | adresse de `report` | 0x2a = 42 | `write` de 42 octets sur la sortie standard |
| 2 | 0x1 | 0x2 | adresse de `notice` | 0x17 = 23 | `write` de 23 octets sur la sortie d'erreur standard |
| 3 | 0x3c = 60 | 0x7 | non utilisé | non utilisé | `exit` avec le code 7 |

Le drapeau est `ARL{flare_42_23_7}`.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Sa structure suit le tableau ci-dessus : un bloc par appel.

```nasm-fragment
_start:
    mov     eax, SYS_WRITE      ; write(STDOUT, banner, banner_len)
    mov     edi, STDOUT
    lea     rsi, [banner]
    mov     edx, banner_len
    syscall
    ...
    mov     eax, SYS_EXIT       ; exit(7)
    mov     edi, 7
    syscall
```

Deux détails permettent d'obtenir des longueurs justes sans compter à la main :

- `banner_len equ $ - banner` est placé immédiatement après la dernière ligne `db` du texte. `$` est l'adresse courante, la différence est donc le nombre d'octets entre l'étiquette et ce point.
- Le `10` à la fin de chaque ligne est l'octet de saut de ligne et il compte dans la longueur. Le premier texte fait 14 + 1, 10 + 1 et 15 + 1 octets, soit 42.

## Erreurs qui font perdre du temps

- **Compter le texte à l'œil.** Les octets de saut de ligne s'oublient facilement. Utilisez `$ - label`.
- **Oublier `exit`.** Le processeur continue d'exécuter ce qui suit la dernière instruction et le programme meurt sur une erreur de segmentation. Le vérificateur indique « terminated by signal 11 ».
- **Intervertir les descripteurs.** Le descripteur 1 est la sortie standard et le 2 la sortie d'erreur standard. Exécutez `./solution 2>/dev/null` pour ne voir que la sortie standard et le confirmer.
- **Lire `edx` comme du décimal.** `0x2a` vaut 42, pas 2 ni 20.
