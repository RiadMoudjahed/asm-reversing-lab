<!-- source: elf.md @ sha256:1621c3a969bd -->
[English](elf.md) | [العربية](elf.ar.md) | [Deutsch](elf.de.md) | [Español](elf.es.md) | [Français](elf.fr.md)

# ELF

Le format exécutable de Linux. Leçon : [12. Formats exécutables : ELF et PE](../modules/12-executable-formats/README.fr.md).

## Structure d'un fichier

| Partie | Où | Utilisé par |
|---|---|---|
| En-tête ELF | Décalage 0, 64 octets dans un fichier 64 bits | Tout |
| Table des en-têtes de programme | À `e_phoff`, en général 64 | Le chargeur |
| Données des sections | Entre les tables | Les deux |
| Table des en-têtes de section | À `e_shoff`, en général à la fin | Éditeur de liens, débogueur, rétro-ingénieur (inutile pour l'exécution) |

Le chargeur ne lit que les en-têtes de programme. Un fichier sans en-têtes de section s'exécute quand même.

## Champs de l'en-tête ELF (64 bits)

| Décalage | Taille | Champ | Signification |
|---|---|---|---|
| 0 | 4 | Nombre magique de `e_ident` | `7f 45 4c 46` (`\x7fELF`) |
| 4 | 1 | Classe | 1 = 32 bits, 2 = 64 bits |
| 5 | 1 | Données | 1 = petit-boutiste, 2 = gros-boutiste |
| 16 | 2 | `e_type` | 2 = EXEC, 3 = DYN (objet partagé ou exécutable indépendant de la position) |
| 18 | 2 | `e_machine` | 3 = x86, 62 = x86-64 |
| 24 | 8 | `e_entry` | Adresse virtuelle de la première instruction |
| 32 | 8 | `e_phoff` | Décalage dans le fichier de la table des en-têtes de programme |
| 40 | 8 | `e_shoff` | Décalage dans le fichier de la table des en-têtes de section |
| 54 | 2 | `e_phentsize` | Taille d'un en-tête de programme (56) |
| 56 | 2 | `e_phnum` | Nombre d'en-têtes de programme |
| 58 | 2 | `e_shentsize` | Taille d'un en-tête de section (64) |
| 60 | 2 | `e_shnum` | Nombre d'en-têtes de section |
| 62 | 2 | `e_shstrndx` | Indice de la section qui contient les noms de sections |

## Types d'en-têtes de programme

| Type | Signification |
|---|---|
| `PT_LOAD` | Une plage du fichier à projeter : décalage dans le fichier, adresse virtuelle, taille dans le fichier, taille en mémoire, permissions |
| `PT_INTERP` | Chemin du chargeur dynamique |
| `PT_DYNAMIC` | La section dynamique : bibliothèques nécessaires, tables de relocalisation |
| `PT_NOTE` | Notes, comme l'identifiant de compilation |
| `PT_GNU_STACK` | Permissions de la pile |
| `PT_GNU_RELRO` | Plage rendue en lecture seule après la relocalisation |

Un segment dont la taille en mémoire dépasse la taille dans le fichier est rempli de zéros au-delà des données du fichier. C'est ainsi que `.bss` ne prend aucune place dans le fichier.

## Sections courantes

| Section | Contenu |
|---|---|
| `.text` | Code |
| `.rodata` | Données en lecture seule, littéraux de chaîne |
| `.data` | Données initialisées et modifiables |
| `.bss` | Données initialisées à zéro, aucun octet dans le fichier |
| `.plt`, `.plt.sec` | Fragments qui sautent par la GOT |
| `.got`, `.got.plt` | Adresses remplies par le chargeur dynamique |
| `.dynsym`, `.dynstr` | Symboles et noms nécessaires au chargement |
| `.symtab`, `.strtab` | Table des symboles complète ; supprimée par `strip` |
| `.rela.dyn`, `.rela.plt` | Entrées de relocalisation |
| `.init_array`, `.fini_array` | Fonctions exécutées avant `main` et à la sortie |
| `.eh_frame` | Informations de déroulement de la pile |

## Appels dynamiques

Un appel à une fonction de bibliothèque va vers un fragment de la PLT. Le fragment saute par une entrée de la GOT. Avec la liaison paresseuse, le premier appel arrive dans le chargeur dynamique, qui résout le nom et écrit l'adresse réelle dans l'entrée de la GOT ; les appels suivants y vont directement. Un binaire lié avec `-z now` résout tout au démarrage.

## Commandes

```bash
file prog                    # kind, bits, stripped or not
readelf -h prog              # ELF header
readelf -lW prog             # program headers and the section-to-segment map
readelf -SW prog             # section headers
readelf -sW prog             # symbol tables
readelf -dW prog             # dynamic section (needed libraries)
readelf -rW prog             # relocations
objdump -d -M intel prog     # disassemble code sections
objdump -s -j .rodata prog   # hex dump of one section
nm -D prog                   # dynamic symbols
ldd prog                     # libraries the loader would bring in
strings -a -t x prog         # printable strings with file offsets
```

## Conversions

- Décalage dans le fichier d'une adresse située dans un segment `PT_LOAD` : `offset = address - p_vaddr + p_offset`.
- Vérifiez le segment, pas la section, quand les deux divergent. Le chargeur utilise les segments.
- Un exécutable indépendant de la position (`e_type` 3) est chargé à une base aléatoire. Les adresses du fichier sont relatives à cette base.
