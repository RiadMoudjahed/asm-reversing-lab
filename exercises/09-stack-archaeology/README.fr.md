<!-- source: README.md @ sha256:5695d04dc8a5 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 09. Archéologie de la pile

Level 3. Durée : environ 3 heures. Architecture : x86-64 Linux. Modules : [08. La pile, les procédures et les conventions d'appel](../../modules/08-stack-and-calls/README.fr.md), [13. Reconnaître les structures de code](../../modules/13-code-structures/README.fr.md)

## Scénario

Une version d'un outil interne, `stackdig`, plante par débordement de pile sur des entrées profondes, et le source de sa fonction récursive `chain` est perdu. Seul le binaire optimisé subsiste. Pour décider si la fonction peut être réécrite à moindre coût, quelqu'un doit mesurer exactement ce qu'un niveau de récursion place sur la pile. Ensuite, un remplacement écrit à la main doit se comporter à l'identique en n'utilisant pas plus de pile par niveau.

## Objectifs

- Lire le prologue et l'épilogue d'une fonction optimisée sans pointeur de cadre.
- Reconstituer un cadre de pile : registres sauvegardés, données locales, espace inutilisé et adresse de retour.
- Écrire une fonction récursive qui garde la pile alignée sur 16 octets à chaque appel et préserve les registres à sauvegarder par l'appelé.

## Partie 1 : Reverse

Le binaire est `bin/stackdig`. Il exécute un calcul et affiche `stackdig: ok`. La fonction à étudier est le symbole global `chain`. Elle a été compilée avec optimisation et sans pointeur de cadre, et elle appelle une autre fonction, `note`, une fois par niveau, avec un pointeur vers une table de trois valeurs de 8 octets qu'elle garde sur sa pile.

```bash
bin/stackdig
objdump -d -M intel bin/stackdig
```

Trouvez ces cinq nombres, tous en décimal :

1. `P` : combien de registres à sauvegarder par l'appelé `chain` empile avec push.
2. `S` : combien d'octets le prologue retranche de `rsp`.
3. `L` : combien d'octets de pile occupe un niveau de récursion, en comptant l'adresse de retour, les registres sauvegardés et l'espace retranché.
4. `O` : la distance en octets entre la première entrée de la table, telle que `note` la voit, et l'adresse de retour sauvegardée de ce même niveau.
5. `U` : combien d'octets de l'espace retranché ne sont pas utilisés par la table.

```text
ARL{stack_<P>_<S>_<L>_<O>_<U>}
```

Par exemple, les valeurs 1, 8, 24, 16 et 0 donneraient `ARL{stack_1_8_24_16_0}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour écrire `chain_asm`, une version de `chain` écrite à la main dans la convention d'appel System V :

```c
long chain_asm(long depth, long seed);

/* Provided by the tests. Clobbers every caller-saved register. */
void note(long depth, const long *entries);
```

`chain_asm` doit faire ce que fait `chain` : construire la même table, appeler `note(depth, table)` une fois par niveau, s'appeler récursivement et renvoyer la même valeur. Lisez les formules dans le code de `chain`. Les entrées de la table et l'arithmétique font partie de ce que la Partie 1 vous permet de voir.

## Règles

- `rsp` doit être un multiple de 16 à chaque `call note`. Le test le vérifie dans `note`.
- `rbx`, `rbp` et `r12` à `r15` doivent avoir leur valeur d'origine quand `chain_asm` revient.
- `note` détruit `rax`, `rcx`, `rdx`, `rsi`, `rdi` et `r8` à `r11`. Conservez ailleurs tout ce dont vous avez besoin après l'appel.
- Un niveau de votre fonction ne peut pas utiliser plus de pile qu'un niveau de `chain`. Le test compare le pointeur de pile lors d'appels consécutifs à `note` avec le nombre `L` de la Partie 1.
- La récursion est la seule façon de descendre ; les tests vont jusqu'à 2000 niveaux.
- Les résultats débordent sur 64 bits.

## Vérifier votre travail

```bash
make check EX=09 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte séparément de la Partie 1 et de la Partie 2. Sans `--flag`, il ne vérifie que la Partie 2. Pour la Partie 2, chaque test affiche le résultat suivi de `ok` ou d'une liste de ce qui s'est mal passé : `result`, `align`, `table`, `calls`, `regs` ou `stack`.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
