<!-- source: README.md @ sha256:306f7c5d94e2 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 11. Casse-tête portable

Level 3. Durée : environ 4 heures. Architecture : x86-64 Windows (sous Wine). Modules : [08. La pile, les procédures et les conventions d'appel](../../modules/08-stack-and-calls/README.fr.md), [10. Fondamentaux des systèmes d'exploitation pour le rétro-ingénieur](../../modules/10-os-for-reversers/README.fr.md), [12. Formats exécutables : ELF et PE](../../modules/12-executable-formats/README.fr.md)

## Scénario

Un outil console Windows, `puzzle.exe`, arrive sans documentation. Une équipe de packaging a besoin de quelques informations tirées de ses en-têtes, et elle veut aussi une petite bibliothèque auxiliaire, écrite en assembleur pour n'avoir aucune dépendance à l'exécution, qui détermine où se trouve dans le fichier sur disque une adresse donnée d'une image PE chargée. Vous lisez d'abord vous-même les en-têtes. Ensuite vous écrivez la bibliothèque auxiliaire.

## Objectifs

- Analyser les en-têtes, la table des sections et le répertoire d'importation d'un fichier PE32+.
- Convertir une adresse virtuelle relative (RVA) en décalage dans le fichier.
- Écrire des fonctions Windows x64 qui respectent la convention d'appel de Microsoft : registres d'arguments, espace d'ombre, alignement de la pile et registres non volatils.

## Partie 1 : Reverse

Le binaire est `bin/puzzle.exe`. C'est un programme console qui affiche une ligne ; vous pouvez l'exécuter sous Wine.

```bash
wine bin/puzzle.exe
x86_64-w64-mingw32-objdump -p bin/puzzle.exe
python3 ../../modules/12-executable-formats/examples/12_pe_dump.py bin/puzzle.exe
```

Trouvez ces six valeurs :

1. `S` : le nombre de sections, en décimal.
2. `I` : la base d'image, en hexadécimal minuscule sans `0x`.
3. `R` : la RVA du point d'entrée, en hexadécimal minuscule sans `0x`.
4. `O` : le décalage dans le fichier de l'octet situé à la RVA `0x12348`, en hexadécimal minuscule sans `0x`.
5. `D` : le nombre de bibliothèques du répertoire d'importation, en décimal.
6. `K` : le nombre de fonctions importées de `KERNEL32.dll`, en décimal.

```text
ARL{portable_<S>_<I>_<R>_<O>_<D>_<K>}
```

Par exemple, les valeurs 5, 140000000, 1410, 7a30, 2 et 9 donneraient `ARL{portable_5_140000000_1410_7a30_2_9}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour implémenter deux fonctions dans la convention d'appel Microsoft x64.

```c
#include <stdint.h>

uint32_t pe_rva_to_offset(const void *sections, uint32_t count, uint32_t rva);
uint32_t pe_entry_offset(const void *file);

/* Provided by the tests. */
void log_rva(uint32_t rva);
```

- `pe_rva_to_offset` reçoit un pointeur vers une table de sections de `count` entrées et une RVA. Elle renvoie le décalage dans le fichier de cette RVA, ou `0xFFFFFFFF` si aucune section ne contient la RVA dans ses données brutes. Une section contient les RVA depuis son adresse virtuelle jusqu'à, sans l'inclure, son adresse virtuelle plus sa taille de données brutes.
- `pe_entry_offset` reçoit un pointeur vers le début d'un fichier PE en mémoire. Si le fichier ne commence pas par `MZ`, ou si les en-têtes NT ne commencent pas par `PE\0\0`, elle renvoie `0xFFFFFFFF`. Sinon elle lit la RVA du point d'entrée dans l'en-tête facultatif et appelle `log_rva` une fois avec cette RVA. Elle renvoie ensuite le décalage dans le fichier du point d'entrée, calculé avec la table des sections qui suit l'en-tête facultatif. Si le point d'entrée n'est dans aucune section, elle renvoie `0xFFFFFFFF`.

Il vous faut la disposition des en-têtes et de la table des sections ; la Partie 1 vous la fait trouver.

## Règles

- Suivez la convention Microsoft x64 : arguments dans `rcx`, `rdx`, `r8` et `r9` ; 32 octets d'espace d'ombre à chaque appel ; `rsp` multiple de 16 à chaque `call`.
- Préservez `rbx`, `rbp`, `rdi`, `rsi` et `r12` à `r15`. Notez que `rdi` et `rsi` sont non volatils sous Windows.
- `log_rva` détruit tous les registres volatils et écrit dans son espace d'ombre. Conservez ce dont vous avez besoin après l'appel dans des registres non volatils ou dans votre propre cadre de pile.
- N'appelez aucune autre fonction.
- Les résultats font 32 bits et débordent comme des nombres non signés.

## Vérifier votre travail

```bash
make check EX=11 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur construit un programme de test avec le compilateur croisé MinGW, l'édite avec votre fichier objet et l'exécute sous Wine. Il rend compte séparément de la Partie 1 et de la Partie 2. Sans `--flag`, il ne vérifie que la Partie 2. Chaque test de la Partie 2 affiche un résultat suivi de `ok` ou d'une liste de ce qui s'est mal passé : `align`, `regs` ou `calls`.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
