<!-- source: README.md @ sha256:c943aa4625a0 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 04. Ordre des octets

Level 2. Durée : environ 1,5 heure. Architecture : x86-64 Linux. Modules : [01. Représentation des données](../../modules/01-data-representation/README.fr.md), [04. Déplacer des données et modes d'adressage](../../modules/04-moving-data/README.fr.md)

## Scénario

Un petit service stocke sa configuration dans un enregistrement fixe de 12 octets. L'enregistrement mélange des largeurs de champs et des boutismes parce qu'il a été assemblé à partir de plusieurs anciens formats. La seule documentation est un programme appelé `config_probe` qui lit l'enregistrement et le vérifie. Vous devez déduire la structure à partir de ce programme, puis écrire les routines de décodage d'un nouveau composant.

## Objectifs

- Retrouver les décalages, les largeurs et les boutismes des champs à partir du désassemblage.
- Décoder à la main des valeurs de plusieurs octets à partir d'un vidage hexadécimal.
- Implémenter des chargements gros-boutistes et petit-boutistes sans instruction d'échange d'octets.

## Partie 1 : Reverse

Le binaire est `bin/config_probe`. Il décode quatre champs (appelons-les A, B, C et D) d'un enregistrement de sa section `.rodata` et vérifie que les valeurs décodées s'additionnent en une somme de contrôle attendue.

```bash
bin/config_probe
objdump -d -M intel bin/config_probe
objdump -s -j .rodata bin/config_probe
```

Répondez à ces questions :

1. Pour chaque champ de A à D : quel est son décalage dans l'enregistrement, sa largeur et son boutisme, tels que le code les traite ?
2. Quelle est la valeur décodée de chaque champ ? Donnez A sous forme de deux nombres (ses 16 bits de poids fort et ses 16 bits de poids faible), B et D en décimal, et C en hexadécimal.
3. Les quatre valeurs décodées s'additionnent-elles en la constante de l'instruction `cmp` ? Sinon, un boutisme est faux.

Construisez le drapeau à partir de vos réponses :

```text
ARL{v<A high>_<A low>_p<B>_t<D>}
```

Par exemple, si A valait `0x00020005`, B 100 et D 2000, le drapeau serait `ARL{v2_5_p100_t2000}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour implémenter trois fonctions qui décodent l'enregistrement que vous avez analysé. Elles utilisent la convention d'appel System V : le premier argument arrive dans `rdi`, le deuxième dans `rsi`, et un résultat repart dans `rax`.

```c
#include <stdint.h>

struct config { uint32_t version; uint16_t port; uint16_t flags; uint32_t timeout_ms; };

uint32_t load_be32(const uint8_t *p);   // 4 bytes at p, most significant byte first
uint16_t load_le16(const uint8_t *p);   // 2 bytes at p, least significant byte first
void decode_record(const uint8_t *rec, struct config *out);
```

`decode_record` lit l'enregistrement de 12 octets à l'adresse `rec` et remplit `out` : `version` est le champ A, `port` est le champ B, `flags` est le champ C et `timeout_ms` est le champ D. La structure de l'enregistrement est celle que vous avez retrouvée en partie 1 ; elle n'est pas répétée ici.

## Règles

- N'utilisez ni `bswap` ni `movbe`. Construisez les valeurs à partir d'octets, de décalages et de `or`, ou utilisez de simples chargements là où le boutisme le permet.
- Ne lisez que les octets qui appartiennent à chaque champ. Les tests placent chaque entrée juste avant une page inaccessible : un chargement qui dépasse la fin provoque un plantage.
- N'écrivez dans `out` que les octets de chaque champ. Les tests vérifient que les champs voisins ne sont pas touchés.

## Vérifier votre travail

```bash
make check EX=04 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte de la partie 1 et de la partie 2 séparément. Sans `--flag`, il ne vérifie que la partie 2.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
