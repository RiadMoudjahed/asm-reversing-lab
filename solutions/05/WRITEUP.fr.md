<!-- source: WRITEUP.md @ sha256:58eca3846a74 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 05. Compteur de billets

Le drapeau est `ARL{ticket_12_1_2}`. Le programme accepte au plus 12 chiffres par nombre, renvoie le code de sortie 1 pour une entrée invalide et le code de sortie 2 lorsque la somme comporte 13 chiffres. Il additionne les nombres sous forme de texte, chiffre par chiffre, de droite à gauche. La solution de référence fait de même.

## Partie 1 : Reverse

### Expériences

```text
$ printf '12\n34\n' | bin/ticket_counter ; echo $?
46
0
$ printf '000\n0000\n' | bin/ticket_counter ; echo $?
0
0
$ printf '12a\n5\n' | bin/ticket_counter ; echo $?
ERR
1
$ printf '999999999999\n1\n' | bin/ticket_counter ; echo $?
OVERFLOW
2
$ printf '1234567890123\n1\n' | bin/ticket_counter ; echo $?
ERR
1
```

Les zéros en tête sont supprimés (`000` plus `0000` affiche `0`). Une entrée de 13 chiffres est rejetée comme invalide, ce qui suggère une limite de longueur pour chaque nombre. Une somme de deux nombres de 12 chiffres qui exige 13 chiffres affiche un autre message et un autre code. Les expériences montrent le comportement, et le désassemblage montre les limites exactes.

### Lire la routine de vérification

```text
parse_line:
  mov    edi,esi
  xor    ecx,ecx
.scan:
  movzx  eax,BYTE PTR [esi]
  cmp    al,0xa                 ; newline: the line ends
  je     .end
  sub    al,0x30                ; '0' is 0x30
  cmp    al,0x9
  ja     fail                   ; not '0'..'9'
  inc    esi
  inc    ecx                    ; count the digits
  jmp    .scan
.end:
  inc    esi
  test   ecx,ecx
  je     fail                   ; empty line
  cmp    ecx,0xc                ; 12
  ja     fail                   ; too long
  ret
```

- **Le test de chiffre.** `sub al, 0x30` transforme `0` à `9` en 0 à 9, et tout le reste en un nombre supérieur à 9 (car la soustraction repasse de l'autre côté pour les caractères inférieurs à `0`). `cmp al, 9` avec `ja` envoie tout cela vers `fail`.
- **La limite de longueur.** `cmp ecx, 0xc` avec `ja fail` : 12 chiffres est la plus longue entrée acceptée.
- **Un saut de ligne manquant.** À la fin de l'entrée, le tampon contient un octet nul. Ce n'est ni un saut de ligne ni un chiffre : le balayage se termine donc dans `fail`.

### Les codes de sortie

Les deux chemins de fin écrivent leur texte, puis chargent `ebx` avant l'appel de sortie (`eax` = 1, `int 0x80`) :

```text
overflow:  ...  mov ebx,0x2   -> exit status 2
fail:      ...  mov ebx,0x1   -> exit status 1
```

Le code d'une exécution réussie est 0, avec `ebx` mis à zéro. Les réponses sont 12, 1 et 2, et le drapeau est `ARL{ticket_12_1_2}`.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Elle fonctionne en quatre étapes :

1. **Lire** jusqu'à 63 octets sur l'entrée standard avec `read` et placer un octet nul après eux.
2. **Vérifier** chacune des deux lignes avec une routine comme `parse_line`, en notant le début et la longueur de chaque nombre.
3. **Additionner** de droite à gauche sur 13 positions. À chaque position, prendre le chiffre de chaque nombre (0 quand l'un est épuisé), les additionner avec la retenue, soustraire 10 et positionner la retenue si la somme vaut 10 ou plus, puis ranger le caractère dans le tampon.
4. **Afficher** les chiffres sans zéros en tête, ou le texte de débordement si la première des 13 positions n'est pas `0`.

Le cœur de l'addition est l'arithmétique des chiffres :

```nasm-fragment
    add     eax, ebx                    ; digit of a + digit of b
    add     eax, edx                    ; plus the carry from the previous position
    xor     edx, edx
    cmp     eax, 10
    jb      .store
    sub     eax, 10                     ; 10 or more: keep the low digit
    mov     edx, 1                      ; and carry 1 into the next position
```

## Erreurs qui font perdre du temps

- **Convertir les nombres en entiers.** Une valeur de 12 chiffres dépasse 32 bits. Additionnez le texte chiffre par chiffre.
- **Une limite trop haute ou trop basse d'une unité.** Accepter 13 chiffres fait échouer 18 des 80 cas ; les tests utilisent des entrées d'exactement 12 et d'exactement 13 chiffres.
- **Ne pas supprimer les zéros en tête.** Le programme affiche alors des résultats comme `0004294967296` ; 53 des 80 cas échouent.
- **Le mauvais code de sortie.** Une erreur qui se termine par 3 au lieu de 1 fait échouer 21 cas alors que le texte est exact.
- **Accepter une entrée sans saut de ligne final.** L'original la traite comme une erreur ; les entrées de test l'incluent.
- **Un programme vide.** L'éditeur de liens produit un fichier sans code et le vérificateur signale qu'il n'a pas pu le démarrer.
