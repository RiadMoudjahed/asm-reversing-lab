<!-- source: rizin.md @ sha256:ae965ed6a7c5 -->
[English](rizin.md) | [العربية](rizin.ar.md) | [Deutsch](rizin.de.md) | [Español](rizin.es.md) | [Français](rizin.fr.md)

# Rizin

Un désassembleur et cadre d'analyse en ligne de commande. Leçon : [11. La boîte à outils du rétro-ingénieur](../modules/11-toolkit/README.fr.md). Les commandes ci-dessous ont été exécutées dans l'image du laboratoire.

## Démarrage

| Ligne de commande | Effet |
|---|---|
| `rizin prog` | Ouvrir en mode interactif |
| `rizin -A prog` | Ouvrir et tout analyser |
| `rizin -q -e scr.color=0 -A -c "CMDS" prog` | Exécuter des commandes séparées par `;`, afficher du texte brut, quitter |
| `rizin -d prog` | Ouvrir sous le débogueur |

## Forme des commandes

Les commandes sont de courts mots formés d'une lettre de famille et d'un verbe. Ajoutez `?` pour voir l'aide d'un préfixe : `a?`, `p?`, `i?`. Ajoutez `@ ADDR` ou `@ sym.name` pour exécuter une commande ailleurs, et `~TEXT` pour ne garder que les lignes qui contiennent `TEXT`.

## Analyse

| Commande | Signification |
|---|---|
| `aaa` | Tout analyser (l'option `-A` le fait à l'ouverture) |
| `afl` | Lister les fonctions avec adresse, nombre de blocs et taille |
| `afvl @ sym.check` | Lister les arguments et les variables locales d'une fonction |
| `axt @ sym.check` | Qui référence cette adresse (appelants) |
| `afn NAME @ ADDR` | Renommer une fonction |

## Affichage

| Commande | Signification |
|---|---|
| `pdf @ sym.check` | Désassembler une fonction entière |
| `pd 3 @ ADDR` | Désassembler 3 instructions |
| `px 16 @ 0x402038` | Vidage hexadécimal de 16 octets |
| `ps ascii @ ADDR` | Afficher une chaîne ASCII terminée par zéro (le simple `ps` devine l'encodage et peut afficher n'importe quoi) |
| `s ADDR` | Déplacer la position courante |

## Informations

| Commande | Signification |
|---|---|
| `iI` | Résumé du binaire : format, bits, protections |
| `iS` | Sections avec décalages, adresses et permissions |
| `is` | Symboles (`is~check` filtre) |
| `ii` | Importations |
| `iz` | Chaînes des sections de données |
| `izz` | Chaînes de tout le fichier |
| `ie` | Points d'entrée (programme, init, fini) |

## Conseils sur la sortie

- `scr.color=0` supprime les codes de couleur quand vous copiez la sortie dans un fichier.
- `-A` prend du temps sur les gros fichiers. Pour un coup d'œil rapide, ouvrez sans elle et lancez `af` à une adresse.
- Rizin nomme les choses avec des préfixes : `sym.` fonctions, `obj.` données, `sym.imp.` importations, `str.` chaînes.
