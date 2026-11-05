<!-- source: quiz.md @ sha256:28af4ab2262e -->
[English](quiz.md) | [العربية](quiz.ar.md) | [Deutsch](quiz.de.md) | [Español](quiz.es.md) | [Français](quiz.fr.md)

# Quiz : 01. Représentation des données

**1. Quelle est la plage d'une valeur signée sur 8 bits ?**

<details>
<summary>Réponse</summary>

De -128 à 127.

</details>

**2. Quelle est la représentation en complément à deux de -1 sur un octet ?**

<details>
<summary>Réponse</summary>

`0xFF`. Inversez `0000 0001` pour obtenir `1111 1110`, puis ajoutez un pour obtenir `1111 1111`.

</details>

**3. L'octet `0x80` est étendu en signe sur 32 bits. Quel est le résultat ?**

<details>
<summary>Réponse</summary>

`0xFFFFFF80`. Le bit de poids fort de `0x80` vaut 1 : les nouveaux bits sont donc remplis avec des 1.

</details>

**4. Un vidage hexadécimal montre `78 56 34 12` à une adresse sur x86. Quelle valeur 32 bits y est stockée ?**

<details>
<summary>Réponse</summary>

`0x12345678`. Le premier octet est celui de poids faible : inversez les octets.

</details>

**5. Combien d'octets l'encodage UTF-8 de la lettre `A` utilise-t-il ?**

<details>
<summary>Réponse</summary>

Un octet, `0x41`. Les caractères inférieurs à 128 sont identiques en ASCII et en UTF-8.

</details>

**6. Quel est le nombre 47 en BCD compacté, sous forme d'octet hexadécimal ?**

<details>
<summary>Réponse</summary>

`0x47`. Chaque chiffre décimal occupe un demi-octet.

</details>

**7. Un mot contient `0xFFFE`. Quelles sont ses valeurs non signée et signée ?**

<details>
<summary>Réponse</summary>

Non signée 65534 ; signée -2.

</details>
