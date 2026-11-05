<!-- source: WRITEUP.md @ sha256:33109aef56b5 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 02. Deux mondes

Le drapeau est `ARL{worlds_36_162_1_60}`. Les deux binaires effectuent les trois mêmes appels (`write`, `sync`, `exit`), mais avec des instructions, des numéros et des registres différents. Le portage de la partie 2 change les trois.

## Partie 1 : Reverse

### Identifier les fichiers

```text
$ file bin/two_worlds32 bin/two_worlds64
bin/two_worlds32: ELF 32-bit LSB executable, Intel 80386, version 1 (SYSV), statically linked, not stripped
bin/two_worlds64: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, not stripped
```

### Le comportement est identique

```text
$ strace ./bin/two_worlds32
[ Process PID=... runs in 32 bit mode. ]
write(1, "two worlds\n", 11)            = 11
sync()                                  = 0
exit(0)                                 = ?

$ strace ./bin/two_worlds64
write(1, "two worlds\n", 11)            = 11
sync()                                  = 0
exit(0)                                 = ?
```

`strace` signale que le premier processus s'exécute en mode 32 bits et montre les trois mêmes appels pour les deux. Les numéros ne sont pas affichés : lisez-les dans le désassemblage.

### Le code est différent

```text
$ objdump -d -M intel bin/two_worlds32
08049000 <_start>:
 8049000:  mov  eax,0x4
 8049005:  mov  ebx,0x1
 804900a:  mov  ecx,0x804a000
 804900f:  mov  edx,0xb
 8049014:  int  0x80
 8049016:  mov  eax,0x24
 804901b:  int  0x80
 804901d:  mov  eax,0x1
 8049022:  xor  ebx,ebx
 8049024:  int  0x80

$ objdump -d -M intel bin/two_worlds64
0000000000401000 <_start>:
  401000:  mov  eax,0x1
  401005:  mov  edi,0x1
  40100a:  lea  rsi,[rip+0xfef]        # 402000 <text>
  401011:  mov  edx,0xb
  401016:  syscall
  401018:  mov  eax,0xa2
  40101d:  syscall
  40101f:  mov  eax,0x3c
  401024:  xor  edi,edi
  401026:  syscall
```

| | 32 bits | 64 bits |
|---|---|---|
| Instruction | `int 0x80` | `syscall` |
| Numéro de `write` | 0x4 = 4 | 0x1 = 1 |
| Numéro de `sync` | 0x24 = 36 | 0xa2 = 162 |
| Numéro de `exit` | 0x1 = 1 | 0x3c = 60 |
| Registre du descripteur | `ebx` | `rdi` |
| Registre de l'adresse | `ecx` | `rsi` |
| Registre de la longueur | `edx` | `rdx` |
| Adresse du texte | absolue : `mov ecx, 0x804a000` | relative : `lea rsi, [rip+0xfef]` |

La dernière ligne est une différence facile à manquer. Le code 32 bits à adresse fixe charge l'adresse comme une constante ; le code 64 bits la calcule relativement à `rip`.

Le drapeau est `ARL{worlds_36_162_1_60}`.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Par rapport au programme de départ, elle change :

- `bits 32` en `bits 64`, et ajoute `default rel` pour l'adresse du texte.
- `int 0x80` en `syscall` aux trois endroits.
- Les numéros : de 4 à 1 pour `write`, de 36 à 162 pour `sync`, de 1 à 60 pour `exit`.
- Les registres d'arguments : `ebx`, `ecx`, `edx` en `rdi`, `rsi`, `rdx`.
- `mov ecx, message` en `lea rsi, [message]`.

## Erreurs qui font perdre du temps

- **Ne changer que l'en-tête.** Un programme 32 bits assemblé en 64 bits fonctionne encore, car Linux accepte `int 0x80` dans les processus 64 bits et lit la table d'appels 32 bits. C'est pourquoi le vérificateur cherche l'instruction elle-même et pas seulement la sortie. En code 64 bits, les adresses au-delà de 4 Gio ne peuvent pas être transmises ainsi : un programme qui marche aujourd'hui par hasard peut échouer plus tard.
- **Garder les anciens numéros.** Avec `syscall`, le numéro d'appel 4 est `stat`, pas `write`. La sortie disparaît ou l'appel échoue.
- **Lire les numéros comme du décimal.** `0xa2` vaut 162, pas 2.
- **Omettre `sync`.** Il n'a pas d'effet visible, ce qui le rend facile à oublier. Le vérificateur compare la suite des appels.
