<!-- source: pe.md @ sha256:83be44b0ddeb -->
[English](pe.md) | [العربية](pe.ar.md) | [Deutsch](pe.de.md) | [Español](pe.es.md) | [Français](pe.fr.md)

# PE

Le format exécutable de Windows. Leçon : [12. Formats exécutables : ELF et PE](../modules/12-executable-formats/README.fr.md). Les décalages ci-dessous ont été vérifiés sur un exécutable MinGW 64 bits.

## Structure d'un fichier

| Partie | Où | Taille |
|---|---|---|
| En-tête DOS | Décalage 0, commence par `MZ` | 64 octets ; le champ à 0x3c est `e_lfanew` |
| Fragment DOS | Après l'en-tête DOS | Variable |
| Signature NT | À `e_lfanew` | 4 octets : `PE\0\0` |
| En-tête de fichier | Juste après la signature | 20 octets |
| En-tête optionnel | Juste après l'en-tête de fichier | 240 octets pour PE32+, 224 pour PE32 |
| Table des sections | Juste après l'en-tête optionnel | 40 octets par section |
| Données des sections | Aux décalages bruts | Alignées sur l'alignement du fichier |

## En-tête de fichier

| Décalage | Taille | Champ | Remarques |
|---|---|---|---|
| 0 | 2 | Machine | 0x8664 x86-64, 0x14c x86 |
| 2 | 2 | Nombre de sections | |
| 16 | 2 | Taille de l'en-tête optionnel | Où commence la table des sections |
| 18 | 2 | Caractéristiques | 0x2000 marque une DLL |

## En-tête optionnel (décalages depuis son début)

| Décalage | Taille | Champ | Remarques |
|---|---|---|---|
| 0 | 2 | Nombre magique | 0x10b PE32, 0x20b PE32+ |
| 16 | 4 | Point d'entrée | Une RVA |
| 24 | 8 | Base de l'image | PE32+. En PE32, la base de l'image est un champ de 4 octets au décalage 28 |
| 32 | 4 | Alignement des sections | En mémoire, en général 0x1000 |
| 36 | 4 | Alignement du fichier | Dans le fichier, en général 0x200 |
| 56 | 4 | Taille de l'image | Taille totale en mémoire, multiple de l'alignement des sections |
| 60 | 4 | Taille des en-têtes | Arrondie à l'alignement du fichier |
| 108 | 4 | Nombre de répertoires de données | En général 16 (PE32+) |
| 112 | 8 chacun | Répertoires de données | Paires `(RVA, size)` (PE32+). En PE32 elles commencent au décalage 96 |

## Répertoires de données

| Indice | Répertoire |
|---|---|
| 0 | Exportation |
| 1 | Importation |
| 2 | Ressource |
| 3 | Exception (`.pdata`, 64 bits) |
| 4 | Certificat (un décalage dans le fichier, pas une RVA) |
| 5 | Relocalisation de base |
| 6 | Débogage |
| 9 | Stockage local au thread |
| 12 | Table d'adresses d'importation |
| 13 | Importation différée |

## En-tête de section (40 octets)

| Décalage | Taille | Champ |
|---|---|---|
| 0 | 8 | Nom, complété par des zéros |
| 8 | 4 | Taille virtuelle |
| 12 | 4 | RVA |
| 16 | 4 | Taille brute |
| 20 | 4 | Décalage brut |
| 36 | 4 | Caractéristiques |

Bits des caractéristiques : 0x20 code, 0x40 données initialisées, 0x80 données non initialisées, 0x20000000 exécutable, 0x40000000 lisible, 0x80000000 modifiable.

## Sections courantes

| Section | Contenu |
|---|---|
| `.text` | Code |
| `.rdata` | Données en lecture seule, noms d'importation et souvent le répertoire d'importation |
| `.data`, `.bss` | Données modifiables initialisées et initialisées à zéro |
| `.idata` | Répertoire d'importation et tables (MinGW) |
| `.edata` | Répertoire d'exportation |
| `.pdata`, `.xdata` | Données de déroulement et d'exceptions (64 bits) |
| `.reloc` | Relocalisations de base |
| `.rsrc` | Ressources |
| `.tls` | Modèle de stockage local au thread |

## Conversions

- De la RVA au décalage dans le fichier : trouvez la section avec `section RVA <= RVA < section RVA + virtual size`, puis calculez `offset = RVA - section RVA + section raw offset`.
- Adresse virtuelle : `image base + RVA`. Avec l'ASLR, le chargeur peut choisir une autre base, et la table de relocalisation corrige les adresses absolues.
- La taille virtuelle d'une section peut être plus grande ou plus petite que sa taille brute. La partie au-delà des données brutes est remplie de zéros.

## Importations

Un tableau de descripteurs, un par DLL, se termine par un descripteur entièrement à zéro. Chacun désigne le nom de la DLL, la table des noms d'importation (ce qui est demandé) et la table des adresses d'importation (où le chargeur écrit les adresses). Les appels dans le code passent par l'IAT, comme `call [rip + disp]`.

## Commandes

```bash
file prog.exe                                  # kind, bits
x86_64-w64-mingw32-objdump -x prog.exe         # headers, sections, imports
x86_64-w64-mingw32-objdump -d -M intel prog.exe
x86_64-w64-mingw32-objdump -p prog.exe         # import tables and data directories
strings -a -t x prog.exe
python3 modules/12-executable-formats/examples/12_pe_dump.py prog.exe
```
