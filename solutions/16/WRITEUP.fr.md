<!-- source: WRITEUP.md @ sha256:c04d588def8e -->
[English](WRITEUP.md) | [العربية](WRITEUP.ar.md) | [Deutsch](WRITEUP.de.md) | [Español](WRITEUP.es.md) | [Français](WRITEUP.fr.md)

# Corrigé : exercice 16. Bibliothèque silencieuse

Le drapeau est `ARL{silent_16_8_5_13_7}` : le bloc d'état fait 16 octets, le compteur d'octets de 64 bits est au décalage 8, la fonction d'injection fait tourner de 5 bits, la fonction qui lit le condensé fait tourner de 13 bits, et la fonction d'empaquetage écrit 7 octets pour les quatre entiers.

## Partie 1 : Reverse

### Les exportations

```text
$ nm -D --defined-only bin/libsilent.so
0000000000001141 T sl_feed
0000000000001173 T sl_final
000000000000117f T sl_free
000000000000119a T sl_live
0000000000001119 T sl_new
00000000000011a1 T sl_pack
```

Les noms suggèrent un ensemble (`sl_new`, `sl_feed`, `sl_final`, `sl_free`) et deux fonctions en dehors. L'ensemble se vérifie par son code, on ne se fie pas à ses noms.

### L'état : `sl_new`

```text
1119:  push   rbx
111a:  mov    ebx,edi                       ; seed, 32 bits
111c:  mov    esi,0x10
111f:  mov    edi,0x1                       ; calloc(1, 16)
1126:  call   calloc@plt
112b:  test   rax,rax
112e:  je     113f                          ; null stays null
1130:  xor    ebx,0x811c9dc5
1136:  mov    DWORD PTR [rax],ebx           ; first field = seed xor constant
1138:  add    DWORD PTR [rip+0x2edd],0x1    ; a global counter goes up
113f:  pop    rbx
1140:  ret
```

La fonction prend une valeur de 32 bits, alloue un bloc de 16 octets mis à zéro, range la graine xor 0x811c9dc5 dans les quatre premiers octets et incrémente une variable globale. `S` vaut donc 16. La globale est lue par `sl_live` (`mov eax, DWORD PTR [rip+...]`, `ret`) et décrémentée par `sl_free`, qui passe aussi son argument à `free` après un test de nullité. Les deux ensemble comptent les états ouverts.

### La fonction d'injection : `sl_feed`

```text
1141:  test   rdx,rdx
1144:  je     116e                          ; no bytes: skip the loop
1146:  mov    r8,rsi
1149:  add    rsi,rdx                       ; end of the buffer
114c:  movzx  eax,BYTE PTR [r8]             ; next byte
1150:  xor    eax,DWORD PTR [rdi]           ; ... xor first field
1152:  imul   eax,eax,0x1000193
1158:  mov    DWORD PTR [rdi],eax           ; first field = (first xor byte) * 16777619
115a:  mov    ecx,DWORD PTR [rdi+0x4]
115d:  rol    ecx,0x5                       ; second field rotated left by 5
1160:  add    eax,ecx
1162:  mov    DWORD PTR [rdi+0x4],eax       ; second field = rol(second, 5) + first
1165:  add    r8,0x1
1169:  cmp    r8,rsi
116c:  jne    114c
116e:  add    QWORD PTR [rdi+0x8],rdx       ; the 64-bit count grows by n
1172:  ret
```

La signature a trois arguments : l'état dans `rdi`, le tampon dans `rsi` (lu avec `movzx ... BYTE PTR`) et la longueur dans `rdx`. L'état a deux champs de 32 bits aux décalages 0 et 4 qui changent à chaque octet, et un champ de 64 bits au décalage 8 qui grandit une seule fois de la longueur totale, après la boucle. `C` vaut donc 8 et `F` vaut 5. L'état fait 4 + 4 + 8 = 16 octets, ce qui concorde avec l'allocation.

### Le condensé : `sl_final`

```text
1173:  mov    eax,DWORD PTR [rdi+0x4]
1176:  rol    eax,0xd                       ; second field rotated left by 13
1179:  xor    eax,DWORD PTR [rdi+0x8]       ; xor the low 32 bits of the count
117c:  xor    eax,DWORD PTR [rdi]           ; xor the first field
117e:  ret
```

La fonction lit l'état sans le modifier et renvoie une valeur de 32 bits : `unsigned sl_final(const struct ctx *)`. `G` vaut donc 13. Les deux valeurs de rotation sont dans des fonctions différentes, ce qui est le piège de la question.

### La fonction d'empaquetage : `sl_pack`

```text
11f0:  lea    edx,[rcx+rcx*1]               ; v + v
11f3:  sar    ecx,0x1f                      ; 0 or -1 from the sign of v
11f6:  xor    edx,ecx                       ; zigzag value
11f8:  cmp    edx,0x7f
11fb:  jbe    11d0                          ; fits in one byte
...
1218:  add    rax,0x1
121c:  mov    ecx,edx
121e:  or     ecx,0xffffff80                ; low 7 bits with bit 7 set
1221:  mov    BYTE PTR [rdi+rax*1-0x1],cl
1225:  shr    edx,0x7
1228:  cmp    edx,0x7f
122b:  ja     1218
```

Quatre arguments sont lus : les entiers dans `rdi` (lus avec un facteur de 4 octets), le nombre dans `rsi`, le tampon de sortie dans `rdx` (déplacé vers `rdi`) et la capacité dans `rcx` (déplacée vers `r11`). Les chemins d'échec fixent `mov eax,0xffffffff` : un moins un de 32 bits, le type de retour est donc `int`. Chaque valeur est d'abord transformée par `(v + v) xor (v >> 31)` avec un décalage arithmétique, de sorte que de petites grandeurs de l'un ou l'autre signe donnent de petits nombres. Le nombre transformé est ensuite écrit par groupes de sept bits, bits bas d'abord, avec le bit 7 positionné sur chaque octet sauf le dernier. Avant d'écrire une valeur, le code ajoute sa longueur aux octets déjà écrits et compare le total à la capacité (`cmp r11,rsi` / `jb` vers le chemin d'échec) ; le test se fait par valeur, donc une valeur tient entièrement ou n'est pas écrite, et les valeurs précédentes restent dans le tampon.

Pour `B` : 0 devient 0 (un octet), -1 devient 1 (un octet), 300 devient 600 (deux octets, car 600 dépasse 127 et reste inférieur à 16384) et -70000 devient 139999 (trois octets, car il dépasse 16383 et reste inférieur à 2097152). Le total est 1 + 1 + 2 + 3 = 7. Un court programme C qui appelle la bibliothèque le confirme : la zone de sortie contient `00 01 d8 04 df c5 08`.

## Partie 2 : Assemble

La solution de référence est `solution.asm`. Les points qui comptent :

- **`digest_buffer`.** Quatre appels : `sl_new(seed)`, `sl_feed(handle, p, n)`, `sl_final(handle)`, `sl_free(handle)`. Le descripteur et les arguments du tampon vivent dans des registres à sauvegarder par l'appelé parce qu'ils doivent survivre aux appels, et le condensé est déplacé dans un tel registre avant `sl_free` pour que le résultat survive aussi à cet appel. Trois push et l'adresse de retour gardent `rsp` aligné.
- **`pack_values`.** Elle n'a besoin ni d'appels ni de registres sauvegardés. Pour chaque valeur, elle calcule le nombre en zigzag, compte les octets nécessaires, compare le total à la capacité, puis renvoie -1 ou écrit les octets. Le test de capacité utilise une comparaison non signée ; la longueur est calculée avant toute écriture, de sorte qu'une valeur qui ne tient pas laisse la sortie intacte.
- **L'étape de zigzag.** Le décalage arithmétique `sar r10d, 31` transforme le signe en 0 ou -1, et le ou exclusif inverse tous les bits d'un double négatif.

## Erreurs qui font perdre du temps

- **Oublier `sl_free`.** Les résultats sont justes et le vérificateur signale `handle`.
- **Libérer le descripteur deux fois.** Le compteur de la bibliothèque devient négatif ou le programme s'interrompt dans `free`.
- **Un décalage logique dans l'étape de zigzag.** Les valeurs positives fonctionnent. Les négatives sont mal codées.
- **Vérifier la capacité octet par octet.** La valeur qui échoue est écrite en partie, et la zone de sortie diffère de celle de la bibliothèque.
- **Une erreur d'un dans le test de capacité.** Une valeur qui tient juste est refusée, ou un octet de trop est écrit.
- **Déclarer la fonction d'empaquetage comme `long`.** Le chemin d'échec fixe `eax` à un moins un de 32 bits, le résultat est donc un `int` ; un mauvais prototype dans votre programme de test le cache.
- **Perdre le condensé pendant `sl_free`.** `rax` est écrasé par l'appel ; le condensé doit être sauvegardé avant.
