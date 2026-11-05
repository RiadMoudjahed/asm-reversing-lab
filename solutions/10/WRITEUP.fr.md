<!-- source: WRITEUP.md @ sha256:5cd10ad8d812 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 10. Chirurgien d'en-têtes

Le drapeau est `ARL{header_10000000_520_11_152_17_m}` : la base d'image est 0x10000000, le point d'entrée est au décalage 0x520 du fichier, il y a 11 en-têtes de programme, le segment inscriptible a 152 octets qui n'existent qu'en mémoire, la section personnalisée `.vault` fait 17 octets et la bibliothèque supplémentaire est `libm`.

## Partie 1 : Reverse

### L'en-tête ELF et les en-têtes de programme

```bash
readelf -h -l -W bin/surgeon
```

```text
  Type:                              EXEC (Executable file)
  Entry point address:               0x10000520
  Number of program headers:         11

Program Headers:
  Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
  LOAD           0x000000 0x0000000010000000 0x0000000010000000 0x000760 0x000760 R E 0x1000
  LOAD           0x000de8 0x0000000010001de8 0x0000000010001de8 0x000240 0x0002d8 RW  0x1000
```

Le listing est abrégé aux lignes qui répondent aux questions.

| Valeur | D'où elle vient |
|---|---|
| `B` | La première entrée `LOAD` a pour `VirtAddr` 0x10000000, la plus basse des deux. Écrivez `10000000` |
| `E` | Le point d'entrée est 0x10000520. La première entrée `LOAD` projette le décalage 0 du fichier à 0x10000000 ; le décalage dans le fichier est donc 0x10000520 - 0x10000000 = 0x520. Écrivez `520` |
| `P` | `Number of program headers: 11` |
| `M` | L'entrée `LOAD` en `RW` a `MemSiz` 0x2d8 et `FileSiz` 0x240 ; la différence est 0x98, soit 152 en décimal |

La valeur 0x10000000 est inhabituelle pour un programme non indépendant de la position, dont la valeur par défaut est 0x400000. On a demandé à l'éditeur de liens de placer le premier segment plus haut, et toutes les adresses du fichier découlent de cette base.

`M` est supérieur aux 0x80 octets de la section `.bss`, parce que le segment s'étend aussi sur l'espace d'alignement entre la fin des données du fichier et le début de `.bss`.

### Les sections

```bash
readelf -S -W bin/surgeon | grep -E "\.(rodata|vault|data|bss) "
```

```text
  [15] .rodata           PROGBITS        00000000100006a4 0006a4 00001c 00   A  0   0  4
  [16] .vault            PROGBITS        00000000100006c0 0006c0 000011 00   A  0   0 16
  [24] .data             PROGBITS        0000000010002010 001010 000018 00  WA  0   0  8
  [25] .bss              NOBITS          0000000010002040 001028 000080 00  WA  0   0 32
```

`.vault` est la seule section dont le nom n'est pas standard. Sa colonne `Size` vaut 0x11, soit 17 en décimal. La liste des sections est présentée de façon abrégée ; les autres sections sont `.interp`, `.dynsym`, `.text`, `.dynamic`, `.got` et d'autres noms standard.

### Les bibliothèques

```bash
readelf -d bin/surgeon | grep NEEDED
```

```text
 0x0000000000000001 (NEEDED)             Shared library: [libm.so.6]
 0x0000000000000001 (NEEDED)             Shared library: [libc.so.6]
```

La bibliothèque autre que la bibliothèque C est `libm.so.6` ; sans le préfixe `lib` et le suffixe `.so.6`, c'est `m`.

Le drapeau est `ARL{header_10000000_520_11_152_17_m}`.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. La disposition :

```text
file offset   0x000  ELF header                  64 bytes
              0x040  PT_LOAD, R + X              56 bytes
              0x078  PT_LOAD, R + W              56 bytes
              0x0b0  PT_GNU_STACK                56 bytes
              0x0e8  code                        the end is at 0x107
              0x200  message (18 bytes)
```

Les points qui comptent :

- **La base d'image.** `org 0x10000000` fait de chaque étiquette une adresse virtuelle absolue, et `$$` est le début du fichier, de sorte que `label - $$` est un décalage dans le fichier. La base est celle trouvée à la Partie 1.
- **Le premier segment.** `p_offset` vaut 0 et `p_vaddr` la base ; les en-têtes sont donc dans le segment, et le point d'entrée 0x10000000 + 0xe8 est une adresse à l'intérieur. `p_filesz` est la fin du code.
- **Le segment de données.** Il est au décalage 0x200 du fichier et à l'adresse virtuelle base + 0x1200. Les deux valeurs laissent le même reste, 0x200, modulo l'alignement 0x1000. C'est la condition qui permet au chargeur de projeter directement une page du fichier. En cas de désaccord, le chargeur projette la page au mauvais endroit et le programme plante au premier accès.
- **L'en-tête de pile.** `PT_GNU_STACK` avec les indicateurs 6 indique que le programme a besoin d'une pile lisible et inscriptible et n'a pas besoin d'y exécuter du code. Les compilateurs l'émettent toujours ; cet exercice l'exige.
- **Le code.** `write` reçoit l'adresse des données dans `esi`, soit 0x10001200 ; la taille du message est dans `edx`. `exit` suit avec le statut 0.

Le fichier fait 530 octets : 0x200 octets d'en-têtes, de code et de remplissage, et 18 octets de message.

## Erreurs qui coûtent du temps

- **Utiliser la base d'image par défaut 0x400000.** Tout le reste est correct, et le vérificateur signale que l'adresse `PT_LOAD` la plus basse doit correspondre à `bin/surgeon`.
- **Un segment avec les indicateurs 7.** Un segment en lecture, écriture et exécution est rejeté, et il n'est pas non plus trouvé comme segment en lecture-écriture.
- **Un décalage et une adresse qui ne concordent pas modulo 0x1000.** Le vérificateur signale la règle de congruence, et le programme meurt avec le signal 11 parce que le chargeur projette la page au mauvais endroit.
- **Pas de `PT_GNU_STACK`.** Le programme s'exécute, mais le vérificateur exige l'en-tête. Si `e_phnum` est inférieur au nombre d'en-têtes écrits, le dernier est ignoré.
- **Un point d'entrée hors du segment de code.** Le programme meurt avec le signal 11 dès son démarrage.
- **Un message avec un autre texte.** Le vérificateur signale que le texte est absent du fichier et que la sortie diffère.
- **Un octet `class` valant 1 dans `e_ident`.** Le fichier prétend être un ELF 32 bits. Le programme peut quand même s'exécuter, mais l'en-tête est faux et le vérificateur le signale.
