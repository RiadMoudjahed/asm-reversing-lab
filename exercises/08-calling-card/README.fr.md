<!-- source: README.md @ sha256:d6d12a704b76 -->
[English](README.md) | [العربية](README.ar.md) | [Deutsch](README.de.md) | [Español](README.es.md) | [Français](README.fr.md)

# Exercice 08. Carte de visite

Level 3. Durée : environ 3 heures. Architecture : x86 et x86-64 Linux. Modules : [08. La pile, les procédures et les conventions d'appel](../../modules/08-stack-and-calls/README.fr.md)

## Scénario

Un fournisseur a livré deux petits programmes de test, `callcard32` et `callcard64`, avec un fichier d'en-tête qui a été perdu. L'en-tête indiquait la convention d'appel de chaque fonction, et l'équipe d'intégration doit maintenant appeler ces fonctions depuis d'autre code. Sans l'en-tête, la convention doit être lue dans le code machine. Il faut ensuite écrire un remplacement pour trois des fonctions, avec les mêmes conventions, pour que les appelants existants continuent de fonctionner.

## Objectifs

- Distinguer cdecl, stdcall et fastcall dans du code 32 bits d'après la façon dont les arguments sont lus et dont la pile est nettoyée après l'appel.
- Distinguer System V AMD64 de Microsoft x64 dans du code 64 bits d'après les registres d'arguments, l'espace d'ombre (shadow space) et l'ajustement de pile de l'appelant.
- Écrire des fonctions dont l'effet sur la pile correspond à leur convention, de sorte qu'un appelant écrit pour cette convention reste équilibré.

## Partie 1 : Reverse

Les binaires sont `bin/callcard32` (32 bits) et `bin/callcard64` (64 bits). Chacun exécute un autotest de ses fonctions et affiche `callcard32: ok` ou `callcard64: ok`. Les fonctions sont les symboles globaux `f1` à `f4` dans `callcard32` et `g1` et `g2` dans `callcard64`.

```bash
bin/callcard32
bin/callcard64
objdump -d -M intel bin/callcard32
objdump -d -M intel bin/callcard64
```

Chaque fonction utilise l'une de cinq conventions. Attribuez une lettre à chaque fonction :

| Lettre | Convention |
|---|---|
| `c` | cdecl : arguments sur la pile, l'appelant les retire |
| `s` | stdcall : arguments sur la pile, l'appelé les retire |
| `f` | fastcall (32 bits) : les deux premiers arguments dans `ecx` et `edx`, le reste sur la pile, l'appelé retire ceux-ci |
| `v` | System V AMD64 : premiers arguments dans `rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9` |
| `m` | Microsoft x64 : premiers arguments dans `rcx`, `rdx`, `r8`, `r9`, avec 32 octets d'espace d'ombre |

Construisez le drapeau à partir des six lettres, dans l'ordre `f1`, `f2`, `f3`, `f4`, `g1`, `g2` :

```text
ARL{calling_<f1><f2><f3><f4><g1><g2>}
```

Par exemple, si les lettres étaient `c`, `c`, `s`, `f`, `v` et `m`, le drapeau serait `ARL{calling_ccsfvm}`.

## Partie 2 : Assemble

Modifiez `starter/solution.asm` pour implémenter trois fonctions en code 32 bits. Chacune doit calculer le même résultat que la fonction de même position dans la Partie 1 et utiliser la même convention d'appel que cette fonction :

- `cc_f1` correspond à `f1`.
- `cc_f2` correspond à `f2`.
- `cc_f3` correspond à `f3`.

Les trois prennent trois arguments `int` et renvoient un `int`. Les tests les appellent par des prototypes qui déclarent chaque convention :

```c
int __attribute__((stdcall))  cc_f1(int a, int b, int c);
int                           cc_f2(int a, int b, int c);   /* cdecl */
int __attribute__((fastcall)) cc_f3(int a, int b, int c);
```

Lisez la formule de chaque fonction dans son code de `callcard32` et notez quel argument se trouve où.

## Règles

- Le harnais compare le pointeur de pile avant et après chaque appel. Une fonction qui retire trop peu ou trop d'octets échoue, même si son résultat est correct.
- Préservez `ebx`, `esi`, `edi` et `ebp`.
- N'utilisez pas `call`.
- Les résultats débordent sur 32 bits ; les tests comprennent des valeurs très grandes et très petites.

## Vérifier votre travail

```bash
make check EX=08 ARGS='--flag ARL{your_answer_here}'
```

Le vérificateur rend compte séparément de la Partie 1 et de la Partie 2. Sans `--flag`, il ne vérifie que la Partie 2.

## Indices

Ouvrez [HINTS.md](HINTS.fr.md) un niveau à la fois.
