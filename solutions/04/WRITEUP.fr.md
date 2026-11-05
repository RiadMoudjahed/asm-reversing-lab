<!-- source: WRITEUP.md @ sha256:8af0bc97c35b -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 04. Ordre des octets

Le drapeau est `ARL{v3_7_p8443_t45000}`. L'enregistrement a un champ gros-boutiste de 32 bits au décalage 0, un champ petit-boutiste de 16 bits au décalage 4, un champ gros-boutiste de 16 bits au décalage 6 et un champ petit-boutiste de 32 bits au décalage 8. La solution de référence le décode avec des chargements d'octets, des décalages et `or`.

## Partie 1 : Reverse

### Lire le code

```text
$ objdump -d -M intel --no-show-raw-insn bin/config_probe
0000000000401000 <_start>:
  mov    eax,DWORD PTR [rip+0xffa]        # 402000 <record>
  bswap  eax
  movzx  ecx,WORD PTR [rip+0xff5]         # 402004 <record+0x4>
  movzx  edx,WORD PTR [rip+0xff0]         # 402006 <record+0x6>
  rol    dx,0x8
  mov    ebx,DWORD PTR [rip+0xfe8]        # 402008 <record+0x8>
  add    eax,ecx
  add    eax,edx
  add    eax,ebx
  cmp    eax,0x450cb
```

Quatre chargements depuis l'enregistrement, dont deux suivis d'un échange :

| Champ | Instruction | Décalage | Largeur | Boutisme |
|---|---|---|---|---|
| A | `mov eax, [record]` puis `bswap eax` | 0 | 4 octets | gros-boutiste |
| B | `movzx ecx, word [record+4]` | 4 | 2 octets | petit-boutiste |
| C | `movzx edx, word [record+6]` puis `rol dx, 8` | 6 | 2 octets | gros-boutiste |
| D | `mov ebx, [record+8]` | 8 | 4 octets | petit-boutiste |

`bswap` et `rol dx, 8` inversent tous deux les octets de ce sur quoi elles agissent, ce qui permet à un programme de transformer des données gros-boutistes en valeur native petit-boutiste. Un chargement sans échange conserve l'ordre de la mémoire, qui est petit-boutiste sur x86.

### Décoder les octets

```text
$ objdump -s -j .rodata bin/config_probe
 402000 00030007 fb208001 c8af0000 ...
```

L'enregistrement est `00 03 00 07 | fb 20 | 80 01 | c8 af 00 00`.

| Champ | Octets | Inversés si nécessaire | Valeur |
|---|---|---|---|
| A | `00 03 00 07` | déjà dans l'ordre de lecture (gros-boutiste) | `0x00030007` : moitié haute 3, moitié basse 7 |
| B | `fb 20` | `20 fb` | `0x20fb` = 8443 |
| C | `80 01` | déjà dans l'ordre de lecture (gros-boutiste) | `0x8001` |
| D | `c8 af 00 00` | `00 00 af c8` | `0xafc8` = 45000 |

### Vérifier avec la somme de contrôle

Le programme additionne les quatre valeurs décodées et compare la somme à `0x450cb` :

```text
0x00030007 + 0x20fb + 0x8001 + 0xafc8 = 0x450cb
196615 + 8443 + 32769 + 45000 = 282827
```

Si vous aviez lu B en gros-boutiste (`0xfb20` = 64288), la somme ne correspondrait pas. La constante du programme est un corrigé intégré pour votre décodage.

Le drapeau est `ARL{v3_7_p8443_t45000}`.

## Partie 2 : Assemble

La solution de référence est `solution.asm`.

`load_le16` tient en une seule instruction, parce que le chargement natif du processeur est petit-boutiste :

```nasm-fragment
load_le16:
    movzx   eax, word [rdi]
    ret
```

`load_be32` construit la valeur octet par octet, en commençant par l'octet de poids fort. Chaque étape décale de 8 bits ce qui est déjà là et ajoute l'octet suivant :

```nasm-fragment
    movzx   eax, byte [rdi]             ; first byte
    shl     eax, 8
    movzx   edx, byte [rdi + 1]
    or      eax, edx                    ; now two bytes
    ; ... repeat for bytes 2 and 3
```

`decode_record` fait de même pour `version` et `flags`, et copie `port` et `timeout_ms` avec de simples chargements et écritures de même largeur.

## Erreurs qui font perdre du temps

- **Charger huit octets dans `load_be32`.** `mov rax, [rdi]` est un début valable, mais il lit quatre octets au-delà de l'entrée. Les tests placent l'entrée juste avant une page inaccessible : le programme plante donc avec le signal 11.
- **Écrire trop large dans `out`.** `mov [rsi + 4], eax` range quatre octets dans un champ de deux et écrase `flags`. Les tests remplissent d'abord `out` avec un motif connu : les dégâts se voient.
- **Ne pas nettoyer le registre.** Charger un octet avec `mov al, [rdi]` laisse en place les anciens bits hauts de `eax`. Utilisez `movzx` pour que les bits hauts soient à zéro.
- **Utiliser `bswap` ou `movbe`.** Elles produisent les bonnes valeurs, et le vérificateur les rejette par règle.
- **Lire B ou D en gros-boutiste.** Un chargement sans échange dans le programme d'origine est petit-boutiste. La somme de contrôle le révèle aussitôt.
