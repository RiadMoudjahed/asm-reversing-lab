<!-- source: README.md @ sha256:c29540243c82 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 05. Compteur de billets

Level 2. Durée : environ 2 heures. Architecture : x86 (32 bits) Linux. Modules : [01. Représentation des données](../../modules/01-data-representation/README.fr.md), [03. Votre premier programme](../../modules/03-first-program/README.fr.md), [05. Arithmétique et logique](../../modules/05-arithmetic-and-logic/README.fr.md), [06. Conditions, branchements et boucles](../../modules/06-conditions-and-loops/README.fr.md)

## Scénario

Un guichet de billetterie additionne les ventes de deux caisses avec un ancien outil 32 bits appelé `ticket_counter`. Les nombres sont saisis sous forme de texte et peuvent être plus longs que ce qu'un registre de 32 bits peut contenir : un nombre de 12 chiffres est supérieur à 2^32. Le guichet passe à un nouveau système et a besoin que le comportement de l'outil soit documenté avec exactitude, y compris ce qu'il fait des entrées erronées, avant de pouvoir le remplacer.

## Objectifs

- Déterminer les règles d'un programme à partir de son désassemblage et d'expériences.
- Lire le test de chiffre ASCII et le code de sortie dans du code 32 bits.
- Additionner des nombres donnés sous forme de texte, chiffre par chiffre, avec retenue, sans les convertir en entiers.

## Partie 1 : Reverse

Le binaire est `bin/ticket_counter`. Il lit deux lignes sur l'entrée standard et affiche un résultat.

```bash
printf '12\n34\n' | bin/ticket_counter ; echo "exit status: $?"
objdump -d -M intel bin/ticket_counter
strace bin/ticket_counter
```

Expérimentez avec les entrées et lisez le code pour répondre à ces questions :

1. Combien de chiffres comporte la plus longue entrée acceptée par le programme ?
2. Quel code de sortie renvoie-t-il lorsqu'une entrée est invalide ?
3. Quel code de sortie renvoie-t-il lorsque la somme est trop grande pour être affichée ?
4. Qu'affiche-t-il pour les entrées `000` et `0000` ? (Vérifiez votre réponse en l'exécutant.)

Construisez le drapeau à partir de vos réponses :

```text
ARL{ticket_<longest input>_<status for invalid input>_<status for a sum that is too large>}
```

Par exemple, si la plus longue entrée était de 8 chiffres, le premier code 3 et le second 4, le drapeau serait `ARL{ticket_8_3_4}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour écrire un programme 32 bits complet qui se comporte exactement comme `bin/ticket_counter` : le même texte sur la sortie standard et le même code de sortie pour toute entrée. Construisez-le et essayez-le depuis le dossier `starter` :

```bash
make run
```

Le vérificateur exécute votre programme sur 80 entrées : des sommes ordinaires, des retenues qui traversent tous les chiffres, des zéros en tête, des entrées exactement à chaque limite et juste au-delà, et des entrées mal formées de plusieurs sortes.

## Règles

- Écrivez du code 32 bits qui utilise l'interface `int 0x80` et aucune bibliothèque C.
- Ne convertissez pas les nombres dans un seul registre. Un nombre de 12 chiffres ne tient pas sur 32 bits : additionnez donc chiffre par chiffre.
- Votre programme ne doit pas exécuter `ticket_counter`.

## Vérifier votre travail

```bash
make check EX=05 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte de la partie 1 et de la partie 2 séparément. Sans `--flag`, il ne vérifie que la partie 2.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
