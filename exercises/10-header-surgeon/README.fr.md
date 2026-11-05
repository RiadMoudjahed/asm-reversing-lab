<!-- source: README.md @ sha256:b4dcee2d886b -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 10. Chirurgien d'en-têtes

Level 3. Durée : environ 3 heures. Architecture : x86-64 Linux. Modules : [12. Formats exécutables : ELF et PE](../../modules/12-executable-formats/README.fr.md)

## Scénario

Un fournisseur a livré un programme ELF, `surgeon`, dont personne n'a noté les réglages de compilation. L'équipe de déploiement a besoin de sa base d'image, de son point d'entrée et de quelques tailles avant de pouvoir le placer dans un lanceur restreint. Elle a aussi besoin d'un exécutable de test minimal qui suive la même base d'image mais sans bibliothèques ni compilateur derrière, afin de vérifier le lanceur sans aucune dépendance. Cet exécutable de test doit être écrit à la main, en-tête par en-tête.

## Objectifs

- Lire avec des outils en ligne de commande les en-têtes, les en-têtes de programme, les sections, les entrées dynamiques et les relocalisations d'un fichier ELF.
- Convertir entre adresses virtuelles et décalages dans le fichier.
- Écrire un exécutable valide avec `nasm -f bin`, y compris la règle de congruence entre décalages dans le fichier et adresses virtuelles.

## Partie 1 : Reverse

Le binaire est `bin/surgeon`. Il s'exécute et affiche une ligne.

```bash
bin/surgeon
readelf -h -l -S -d bin/surgeon
```

Trouvez ces six valeurs :

1. `B` : la base d'image, la plus basse adresse virtuelle d'un segment `PT_LOAD`. Écrivez-la en hexadécimal minuscule sans `0x`.
2. `E` : le décalage dans le fichier du point d'entrée, en hexadécimal minuscule sans `0x`.
3. `P` : le nombre d'en-têtes de programme, en décimal.
4. `M` : combien d'octets du segment `PT_LOAD` en lecture-écriture n'existent qu'en mémoire, c'est-à-dire `MemSiz` moins `FileSiz`, en décimal.
5. `V` : la taille en octets, en décimal, de l'unique section dont le nom n'est pas un nom standard de compilateur ou d'éditeur de liens.
6. `L` : le nom de la bibliothèque partagée autre que la bibliothèque C, sans le préfixe `lib` ni le suffixe `.so` et la version (pour `libz.so.1`, écrivez `z`).

```text
ARL{header_<B>_<E>_<P>_<M>_<V>_<L>}
```

Par exemple, les valeurs 400000, 1050, 13, 8, 21 et z donneraient `ARL{header_400000_1050_13_8_21_z}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour que `nasm -f bin` écrive un fichier exécutable complet. Rien n'est ajouté : vous écrivez vous-même chaque octet des en-têtes. Le programme affiche `header surgery ok` suivi d'un saut de ligne sur la sortie standard et se termine avec le statut 0.

Le fichier doit satisfaire toutes les conditions suivantes :

- C'est un exécutable ELF 64 bits, little-endian, pour x86-64.
- La plus basse adresse virtuelle d'un `PT_LOAD` est la base d'image `B` de `bin/surgeon`.
- Il a un segment `PT_LOAD` avec les indicateurs lecture + exécution qui contient les en-têtes et le code, et un autre segment `PT_LOAD` avec les indicateurs lecture + écriture qui contient le message. Aucun segment ne peut être à la fois inscriptible et exécutable.
- Pour chaque segment `PT_LOAD`, `p_offset` et `p_vaddr` ont le même reste modulo `p_align`.
- Le point d'entrée se trouve à l'intérieur du segment lecture + exécution.
- Un en-tête de programme `PT_GNU_STACK` avec les indicateurs lecture + écriture marque la pile comme non exécutable.
- Le texte du message est stocké dans la plage de fichier du segment en lecture-écriture, et le code le lit à cet endroit.
- Le fichier fait au plus 1024 octets.

Le vérificateur lit `B` dans `bin/surgeon` lui-même, puis inspecte les en-têtes de votre fichier et l'exécute.

## Règles

- Utilisez uniquement `nasm -f bin` : le vérificateur assemble votre fichier ainsi et exécute directement le résultat.
- N'utilisez ni éditeur de liens, ni bibliothèque C, ni chargement dynamique.
- Les en-têtes de section ne sont pas exigés.

## Vérifier votre travail

```bash
make check EX=10 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte séparément de la Partie 1 et de la Partie 2. Sans `--flag`, il ne vérifie que la Partie 2. Pour la Partie 2, il liste chaque exigence que votre fichier enfreint.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
