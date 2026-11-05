<!-- source: gdb.md @ sha256:5d1d9c061ac4 -->
[English](gdb.md) | [العربية](gdb.ar.md) | [Deutsch](gdb.de.md) | [Español](gdb.es.md) | [Français](gdb.fr.md)

# GDB

Le débogueur GNU. Leçon : [11. La boîte à outils du rétro-ingénieur](../modules/11-toolkit/README.fr.md). Chaque commande ci-dessous a été exécutée dans l'image du laboratoire.

## Démarrage

| Ligne de commande | Effet |
|---|---|
| `gdb prog` | Charger un programme |
| `gdb --args prog a b` | Le charger avec des arguments |
| `gdb -nx -q -batch -ex "CMD" prog` | Exécuter des commandes sans extensions et afficher du texte brut (pour les notes et les scripts) |
| `gdb -p PID` | S'attacher à un processus en cours |

Dans GDB, `run ARGS` lance le programme et `start` s'arrête au début de `main` quand les symboles existent. `quit` quitte.

## Points d'arrêt et points de surveillance

| Commande | Signification |
|---|---|
| `break check`, `b *0x401184` | S'arrêter à une fonction ou à une adresse exacte |
| `tbreak check` | S'arrêter une fois, puis supprimer le point d'arrêt |
| `info breakpoints` | Lister les points d'arrêt et leurs déclenchements |
| `delete N`, `disable N`, `enable N` | Supprimer, désactiver ou activer le point d'arrêt `N` |
| `condition N EXPR` | S'arrêter au point d'arrêt `N` seulement quand `EXPR` est vraie |
| `watch EXPR`, `watch *(int *)$rsp` | S'arrêter quand une valeur change |
| `rwatch EXPR`, `awatch EXPR` | S'arrêter sur une lecture, ou sur une lecture ou une écriture |

Un programme sans symboles n'a pas de noms de fonctions. Utilisez l'adresse (`b *0x401184`) ou le point d'entrée (`info files`, puis `b *ADDR`).

## Exécution

| Commande | Signification |
|---|---|
| `continue` | Reprendre |
| `stepi`, `nexti` | Une instruction, en entrant dans les appels ou en les enjambant |
| `step`, `next` | Une ligne de source (nécessite des informations de débogage) |
| `finish` | Exécuter jusqu'au retour de la fonction courante |
| `until ADDR` | Exécuter jusqu'à atteindre une adresse |
| `call EXPR` | Appeler une fonction du programme |
| `set var X = V`, `set $rax = 0` | Modifier une variable ou un registre |
| `jump *ADDR` | Continuer à une autre adresse |

## Examiner l'état

| Commande | Signification |
|---|---|
| `info registers`, `info registers rdi` | Tous les registres, ou ceux indiqués |
| `print EXPR`, `p/x $rsp` | Évaluer ; `/x` hexadécimal, `/d` décimal, `/t` binaire, `/c` caractère |
| `x/NFU ADDR` | Examiner la mémoire : `N` unités, format `F`, taille d'unité `U` |
| `bt` | Pile d'appels |
| `info frame` | Cadre à l'arrêt courant |
| `info proc mappings` | Carte mémoire du processus |
| `info functions REGEX` | Fonctions correspondant à un motif |
| `disassemble check` | Désassembler une fonction |
| `display/i $pc` | Afficher l'instruction suivante à chaque arrêt |

### Formats et unités de `x`

| Lettre | Signification |
|---|---|
| `x`, `d`, `u`, `o`, `t` | Hexadécimal, signé, non signé, octal, binaire |
| `c`, `s` | Caractères, chaîne |
| `i` | Instructions |
| `b`, `h`, `w`, `g` | Taille d'unité : 1, 2, 4, 8 octets |

Exemples : `x/4gx $rsp` quatre mots de 8 octets en hexadécimal, `x/s $rdi` la chaîne pointée par `rdi`, `x/5i $pc` cinq instructions à partir de l'instruction courante.

## Réglages utiles à connaître

| Commande | Signification |
|---|---|
| `set disassembly-flavor intel` | Syntaxe Intel (l'image du laboratoire l'active) |
| `set args A B` | Arguments du prochain `run` |
| `set disable-randomization on` | Adresses fixes (le défaut hors conteneur) |
| `set logging enabled on` | Copier la sortie dans `gdb.txt` |
| `set confirm off` | Aucune question de confirmation |

## GEF

L'image du laboratoire charge GEF, qui affiche les registres, la pile et le code après chaque arrêt. Commandes utiles :

| Commande | Signification |
|---|---|
| `context` | Redessiner la vue des registres, de la pile et du code |
| `vmmap` | Carte mémoire avec permissions |
| `checksec` | Protections du fichier chargé |
| `telescope $rsp 8` | Mots de la pile avec les valeurs qu'ils désignent |
| `search-pattern TEXT` | Chercher des octets en mémoire |

Utilisez `gdb -nx` quand vous avez besoin d'une sortie brute.

## Rappels

- L'adresse de pile dans `rdi` ou `rsp` change d'une exécution à l'autre. Ne la recopiez pas dans vos notes comme un fait.
- Après `finish`, le résultat est dans `rax`.
- Dans un conteneur, l'avertissement "Error disabling address space randomization" est normal.
