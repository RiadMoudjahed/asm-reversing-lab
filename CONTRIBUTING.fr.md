<!-- source: CONTRIBUTING.md @ sha256:1e4e4a084d48 -->
[English](CONTRIBUTING.md) | [العربية](CONTRIBUTING.ar.md) | [Deutsch](CONTRIBUTING.de.md) | [Español](CONTRIBUTING.es.md) | [Français](CONTRIBUTING.fr.md)

# Contribuer

Merci d'aider d'autres personnes à apprendre. Cette page explique comment bien contribuer.

## Façons de contribuer

- Signaler une erreur technique, une explication peu claire ou un exemple défaillant.
- Améliorer ou relire une traduction. La relecture par des locuteurs natifs est particulièrement précieuse.
- Proposer une amélioration pour un indice ou un corrigé commenté.
- Corriger les fautes de frappe et la mise en forme.

Pour de nouveaux modules ou exercices, ouvrez d'abord une issue afin de valider le périmètre avant de commencer.

## Règles de base

1. **Travail original uniquement.** Rédigez vos propres explications, exemples et défis. Ne copiez ni ne paraphrasez de près aucun livre, cours, blog ou vidéo, et ne citez pas d'œuvres commerciales comme sources. Les spécifications officielles peuvent être citées.
2. **Rester dans le périmètre.** Pas de développement d'exploits, de shellcode ni de logiciels malveillants, et aucun contenu sur le contournement de la protection contre la copie ou des licences. Les programmes des défis sont compilés depuis leur code source dans ce dépôt ; aucun binaire tiers.
3. **Pas de divulgâcheurs.** Les drapeaux et les solutions complètes ne vont que dans `solutions/`. Ne les publiez jamais dans des issues ou des discussions.
4. **Présentation professionnelle.** Pas d'émojis ni de symboles décoratifs. Un langage clair et direct.
5. **Tout fonctionne.** Les blocs de code marqués `nasm`, `c` ou `bash` doivent fonctionner dans l'image du laboratoire.

## Déroulement

1. Faites un fork du dépôt et créez une branche thématique.
2. Effectuez votre modification en suivant les règles de base ci-dessus et le style des pages existantes.
3. Exécutez `make all-checks` dans le laboratoire. Toutes les vérifications doivent réussir.
4. Ouvrez une pull request et remplissez la liste de contrôle du modèle.

## Traductions

Les fichiers en anglais sont la source. Une traduction se place à côté du fichier anglais avec un suffixe de langue : `README.fr.md`, `README.de.md`, `README.es.md`, `README.ar.md`.

- Commencez le fichier par `<!-- source: FILE.md @ sha256:<12 hex> -->` (les 12 premiers caractères de `sha256sum FILE.md`).
- Ne traduisez pas le code, les mnémoniques, les noms de registres, les commandes, les noms de fichiers ni les drapeaux.
- Utilisez les termes de `i18n/terminology.csv`. Ajoutez une ligne lorsque vous avez besoin d'un nouveau terme.
- Suivez `i18n/STYLE.md` pour le ton et la typographie de chaque langue.
- Exécutez `make i18n` pour voir quelles pages manquent ou sont obsolètes.

## Signaler un problème de sécurité

Si vous pensez qu'une page ou un programme de ce dépôt pourrait causer des dommages, contactez les mainteneurs en privé plutôt que d'ouvrir une issue publique.
