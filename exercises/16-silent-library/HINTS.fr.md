<!-- source: HINTS.md @ sha256:fe7fcc651f83 -->
[English](HINTS.md) | [العربية](HINTS.ar.md) | [Deutsch](HINTS.de.md) | [Español](HINTS.es.md) | [Français](HINTS.fr.md)

# Indices : exercice 16

Ouvrez un niveau à la fois. Réessayez entre les niveaux.

## Partie 1 : Reverse

<details>
<summary>Niveau 1 : coup de pouce</summary>

Commencez par la liste des exportations et regroupez les fonctions selon ce qu'elles partagent : celles qui prennent le même premier argument travaillent sur le même objet. La fonction de création se termine par un appel à l'allocateur, et ses arguments donnent la taille. Lisez ensuite la fonction d'injection ; c'est une courte boucle.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Dans la boucle d'injection, suivez ce qui est lu dans l'état et ce qui y est réécrit, et notez le décalage de chaque élément. Deux champs de 32 bits changent à chaque octet et un champ de 64 bits change une fois à la fin ; l'instruction qui ajoute un registre à un opérande mémoire après la boucle est le compteur. La rotation est une seule instruction avec une constante. La fonction qui lit le condensé combine trois valeurs de l'état ; l'une d'elles est tournée. Pour la fonction d'empaquetage, n'essayez pas de compter en lisant : écrivez un court programme C avec votre prototype retrouvé et interrogez la bibliothèque.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

Ne confondez pas les deux valeurs de rotation : une instruction est dans la boucle de la fonction d'injection et l'autre dans la courte fonction qui lit le condensé. Les décalages se mesurent depuis le début du bloc que renvoie la fonction de création. Pour la taille empaquetée, rappelez-vous que le codage est de longueur variable et que les petites valeurs prennent peu d'octets ; la bibliothèque vous donnera le compte exact si vous lui laissez assez de place, passez donc une capacité généreuse.

</details>

## Partie 2 : Assemble

<details>
<summary>Niveau 1 : coup de pouce</summary>

Écrivez d'abord `digest_buffer`. Ce sont quatre appels : créer, injecter, lire, libérer. Le descripteur renvoyé par le premier appel doit survivre aux trois suivants, et le condensé doit survivre à la libération ; décidez donc quelles valeurs vont dans des registres à sauvegarder par l'appelé.

</details>

<details>
<summary>Niveau 2 : direction</summary>

Empilez autant de registres à sauvegarder par l'appelé que nécessaire et vérifiez l'alignement : à l'entrée, `rsp` vaut 8 modulo 16, et un nombre impair d'empilements l'amène à un multiple de 16. Déclarez les fonctions de la bibliothèque avec `extern` et appelez-les par la PLT. Pour `pack_values`, déduisez le codage des résultats de votre propre programme de test : les valeurs négatives et positives sont d'abord transformées en nombres non signés, puis écrites par groupes de sept bits, et le bit de poids fort d'un octet indique que d'autres octets suivent. Trouvez laquelle des deux directions de décalage un nombre négatif exige, et ce qui arrive exactement à la sortie quand une valeur ne tient pas.

</details>

<details>
<summary>Niveau 3 : presque la réponse</summary>

La transformation double la valeur et, pour une valeur négative, inverse tous les bits du résultat ; faites-le avec un décalage arithmétique d'une copie de l'original, qui donne zéro ou moins un, et un ou exclusif. Calculez combien d'octets la valeur transformée demande avant d'en écrire un seul, et comparez le total écrit plus ce nombre à la capacité. Si elle ne tient pas, renvoyez -1 sans toucher à la sortie pour cette valeur ; les valeurs précédentes sont déjà écrites et restent. Une valeur de 0x80 ou plus demande une continuation : écrivez les sept bits bas avec le bit de poids fort positionné, décalez de sept vers la droite et recommencez. Si le vérificateur affiche `handle`, un état n'a pas été libéré ou l'a été deux fois ; `regs` signifie qu'un registre à sauvegarder par l'appelé a été modifié.

</details>
