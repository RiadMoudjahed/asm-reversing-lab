<!-- source: WRITEUP.md @ sha256:1cccb2fc3c39 -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 11. Casse-tête portable

Le drapeau est `ARL{portable_11_150000000_2410_8d48_4_13}` : le fichier a 11 sections, la base d'image est 0x150000000, la RVA du point d'entrée est 0x2410, la RVA 0x12348 se trouve au décalage 0x8d48 du fichier, quatre bibliothèques sont importées et 13 fonctions viennent de `KERNEL32.dll`.

## Partie 1 : Reverse

### Les en-têtes

```bash
x86_64-w64-mingw32-objdump -p bin/puzzle.exe | grep -E "^(ImageBase|AddressOfEntryPoint|SectionAlignment|FileAlignment)|DLL Name"
```

```text
AddressOfEntryPoint    0000000000002410
ImageBase              0000000150000000
SectionAlignment       00002000
FileAlignment          00000200
    DLL Name: ADVAPI32.dll
    DLL Name: KERNEL32.dll
    DLL Name: msvcrt.dll
    DLL Name: USER32.dll
```

Les colonnes d'`objdump -p` sont séparées par des tabulations ; elles sont représentées ici par des espaces. `AddressOfEntryPoint` est une RVA, donc `R` vaut `2410` et non la somme avec la base d'image. `ImageBase` donne `I` = `150000000`. Il y a quatre lignes `DLL Name`, donc `D` vaut 4.

### Les sections

```bash
python3 ../../modules/12-executable-formats/examples/12_pe_dump.py bin/puzzle.exe
```

```text
name      virtual size  RVA       raw size  raw offset  flags
.text     0x00006cc8    0x002000  0x006e00  0x000400    0x60000060
.data     0x000000d0    0x00a000  0x000200  0x007200    0xc0000040
.rdata    0x00000dc0    0x00c000  0x000e00  0x007400    0x40000040
.ledger   0x00000020    0x00e000  0x000200  0x008200    0x40000040
.pdata    0x00000480    0x010000  0x000600  0x008400    0x40000040
.xdata    0x00000528    0x012000  0x000600  0x008a00    0x40000040
.bss      0x00000da0    0x014000  0x000000  0x000000    0xc0000080
.idata    0x00000780    0x016000  0x000800  0x009000    0xc0000040
.CRT      0x00000060    0x018000  0x000200  0x009800    0xc0000040
.tls      0x00000010    0x01a000  0x000200  0x009a00    0xc0000040
.reloc    0x00000084    0x01c000  0x000200  0x009c00    0x42000040
```

Il y a 11 lignes, donc `S` vaut 11. La section `.ledger` n'a pas de nom standard ; elle ne fait pas partie des questions.

Pour `O`, trouvez la section qui contient la RVA 0x12348. `.xdata` commence à la RVA 0x12000 et sa taille brute est 0x600 : elle couvre donc 0x12000 à 0x125ff. Appliquez la formule du module 12 :

```text
file offset = RVA - section RVA + section raw offset
            = 0x12348 - 0x12000 + 0x8a00
            = 0x348 + 0x8a00
            = 0x8d48
```

### Les importations

Le répertoire d'importation se trouve dans `.idata`. En comptant les noms de chaque bibliothèque dans la table des noms d'importation, on obtient pour `KERNEL32.dll` :

```text
DeleteCriticalSection, EnterCriticalSection, GetLastError, InitializeCriticalSection,
IsDBCSLeadByteEx, LeaveCriticalSection, MultiByteToWideChar, SetUnhandledExceptionFilter,
Sleep, TlsGetValue, VirtualProtect, VirtualQuery, WideCharToMultiByte
```

Cela fait 13 fonctions, donc `K` vaut 13. Le script du module 12 n'affiche que les six premiers noms de chaque bibliothèque ; il ne suffit donc pas pour cette question, alors que compter les entrées de la table ou lire `objdump -p` suffit.

Le drapeau est `ARL{portable_11_150000000_2410_8d48_4_13}`.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Les points qui comptent :

- **Lire la table des sections.** Chaque entrée fait 40 octets. L'adresse virtuelle est au décalage 12, la taille des données brutes au décalage 16 et le pointeur vers les données brutes au décalage 20. Le script du module 12 les décompose dans l'ordre nom, taille virtuelle, adresse virtuelle, taille brute, décalage brut.
- **Le test d'appartenance.** `rva - VirtualAddress` est calculé sur 32 bits et comparé à la taille des données brutes par un saut non signé (`jae`). Si la RVA est en dessous de la section, la soustraction déborde vers un grand nombre, de sorte qu'une seule comparaison remplace les deux vérifications « pas en dessous » et « en dessous de la fin ».
- **La taille virtuelle n'est pas utilisée.** Une section peut avoir une taille virtuelle supérieure à sa taille brute (les octets sont des zéros en mémoire). Ces RVA n'ont pas de décalage dans le fichier, et la fonction renvoie `0xFFFFFFFF` pour elles.
- **`pe_entry_offset`.** Elle lit `e_lfanew` au décalage 0x3c, l'ajoute au pointeur de fichier et vérifie `PE\0\0`. La RVA du point d'entrée est à en-têtes NT + 24 + 16 ; le nombre de sections à en-têtes NT + 6 ; la taille de l'en-tête facultatif à en-têtes NT + 20, et la table des sections commence à en-têtes NT + 24 + cette taille.
- **Le cadre.** La fonction fait un appel ; elle sauvegarde donc `rbx` avec `push`, ce qui laisse `rsp` aligné. Elle réserve ensuite 48 octets : 32 pour l'espace d'ombre et 16 pour une variable locale qui contient la RVA du point d'entrée. La variable locale est au-dessus de l'espace d'ombre parce que `log_rva` écrit dans les 32 octets situés juste au-dessus de son adresse de retour.
- **Les appels.** `log_rva` détruit les registres volatils ; la RVA du point d'entrée est donc rechargée depuis la pile pour l'appel à `pe_rva_to_offset`, et le pointeur vers la table des sections est recalculé à partir de `rbx`.

## Erreurs qui coûtent du temps

- **Un cadre de 40 octets.** La pile vaut 8 modulo 16 à l'appel ; le vérificateur signale `align` dans chaque test qui atteint `log_rva`.
- **Utiliser `rbx` sans le sauvegarder.** Les résultats sont justes, et le vérificateur signale `regs`.
- **Utiliser `esi` comme registre de travail.** Dans la convention System V, `rsi` est volatil, mais pas dans la convention Microsoft. Le vérificateur signale `regs` dans tous les cas.
- **Ne pas appeler `log_rva`, ou l'appeler avec une autre valeur.** Le vérificateur signale `calls`.
- **Une comparaison signée.** `jge` au lieu de `jae` fait paraître les RVA situées sous une section comme si elles étaient dedans ; les cas « below the first section », « rva zero » et « largest rva » échouent.
- **Comparer avec la taille virtuelle.** Le cas où la RVA est dans la queue remplie de zéros d'une section renvoie un décalage au lieu de `0xFFFFFFFF`.
- **Garder une variable locale dans l'espace d'ombre.** `log_rva` l'écrase, et la RVA du point d'entrée est fausse ensuite.
