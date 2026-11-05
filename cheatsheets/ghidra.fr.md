<!-- source: ghidra.md @ sha256:2b16cc1b1f48 -->
[English](ghidra.md) | [العربية](ghidra.ar.md) | [Deutsch](ghidra.de.md) | [Español](ghidra.es.md) | [Français](ghidra.fr.md)

# Ghidra

Une suite graphique de rétro-ingénierie avec désassembleur et décompilateur. Elle s'exécute sur votre hôte, pas dans l'image du laboratoire. Leçon : [11. La boîte à outils du rétro-ingénieur](../modules/11-toolkit/README.fr.md). Les raccourcis sont ceux par défaut ; ils peuvent différer selon les versions et les réglages, et les menus indiquent les raccourcis actuels.

## Déroulement

1. Créez un projet et importez le binaire depuis `exercises/NN-name/bin/`. Acceptez le format et le langage détectés.
2. Ouvrez le fichier dans le navigateur de code et lancez l'analyse par défaut.
3. Trouvez le point d'entrée, `main`, les chaînes et les importations.
4. Lisez le listing et le décompilateur côte à côte.
5. Renommez, changez les types et commentez à mesure que vous apprenez. Chaque modification rend la fonction suivante plus facile à lire.
6. Enregistrez le projet avant de le fermer.

## Fenêtres du navigateur de code

| Fenêtre | Usage |
|---|---|
| Listing | Désassemblage avec adresses, octets, étiquettes et commentaires |
| Décompilateur | C approximatif de la fonction sous le curseur |
| Arbre des symboles | Fonctions, étiquettes, importations, exportations |
| Chaînes définies | Chaînes trouvées dans le fichier ; double-clic pour y aller |
| Graphe de fonction | Blocs de base et branchements d'une fonction |
| Gestionnaire de types de données | Structures, énumérations et typedefs intégrés et personnels |

## Touches par défaut

| Touche | Action |
|---|---|
| `G` | Aller à une adresse ou à une étiquette |
| `L` | Renommer l'étiquette, la fonction ou la variable sous le curseur |
| `Ctrl+L` | Changer le type d'une variable dans le décompilateur |
| `;` | Ajouter un commentaire |
| `X` | Afficher les références à l'élément sous le curseur |
| `D` | Désassembler à partir du curseur |
| `F` | Créer une fonction au curseur |
| `T` | Fixer le type de données de l'élément dans le listing |

## Retrouver des structures

1. Dans le décompilateur, cherchez des accès répétés comme `*(int *)(p + 0x10)` sur un même pointeur. Ce sont les champs d'une structure.
2. Dans le gestionnaire de types de données, créez une structure avec un champ à chaque décalage trouvé et une taille qui correspond à l'allocation.
3. Changez le type de la variable pointeur en pointeur vers cette structure (`Ctrl+L`).
4. Le décompilateur écrit maintenant `p->key` au lieu du décalage. Renommez les champs à mesure que leur rôle s'éclaircit.

## Quand le décompilateur semble faux

- Comparez-le avec les instructions. Les types, les opérations signées ou non signées et la largeur d'un champ peuvent être mal devinés.
- Vérifiez la convention d'appel de la fonction. Une convention erronée cache ou invente des arguments.
- Vérifiez que les limites des fonctions sont justes. Un `F` oublié ou un octet de données traité comme du code produit n'importe quoi.
- Considérez le listing comme la référence.

## Hôte et laboratoire

Ghidra lit les fichiers par les chemins de votre hôte, par exemple `~/asm-reversing-lab/exercises/01-signal-flare/bin/`, et non `/lab/...`. Utilisez l'image du laboratoire pour `objdump`, `rizin` et `gdb`.
