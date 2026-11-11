<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost Deluxe

[English](README.md) | **Français** | [Deutsch](README_DE.md) | [Italiano](README_IT.md) | [Español](README_ES.md) | [Nederlands](README_NL.md) | [Português](README_PT.md) | [中文](README_ZH.md)

**Un poster Usenet rapide, gratuit et moderne pour Windows, Linux et Docker.**<br/>
Compressez, ajoutez les par2, obfusquez et postez vos fichiers depuis une interface claire (thèmes clair et sombre) ou en ligne de commande.

[![Latest release](https://img.shields.io/github/v/release/thenormal/ngPost-Deluxe)](https://github.com/thenormal/ngPost-Deluxe/releases/latest)
[![Build](https://github.com/thenormal/ngPost-Deluxe/actions/workflows/build-windows.yml/badge.svg)](https://github.com/thenormal/ngPost-Deluxe/actions)
[![Licence: GPL v3](https://img.shields.io/badge/licence-GPL%20v3-blue)](LICENSE)

[![Download](https://img.shields.io/badge/Download-latest%20release-2ea44f?style=for-the-badge)](https://github.com/thenormal/ngPost-Deluxe/releases/latest)

**ngPost Deluxe** est une version revisitée et redessinée du célèbre uploader Usenet **ngPost**.<br/>
C'est un fork de [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, licence GPL v3). Tout le mérite du programme original revient à son auteur, Matthieu Bruel.<br/>

### Pour commencer
| Installateur Windows | Windows portable | Docker / Unraid |
|---|---|---|
| Téléchargez `ngPostDeluxe-v…-win64-setup.exe` (ou `win32`) depuis la page [Releases](https://github.com/thenormal/ngPost-Deluxe/releases/latest). 7-Zip est inclus. | Décompressez `ngPostDeluxe-v…-win64.zip` (ou `win32`) où vous voulez et lancez `ngPostDeluxe.exe`. Les réglages sont dans le `ngPost.conf` à côté du programme. | Image `ghcr.io/thenormal/ngpost-deluxe` avec un modèle pour Unraid. Voir [docker/README.md](docker/README.md). |

> Windows peut afficher « Windows a protégé votre ordinateur » car l'installateur n'est pas signé numériquement : cliquez sur **Informations complémentaires**, puis **Exécuter quand même**.

### Démarrage rapide
1. Installez et lancez ngPost Deluxe.
2. Ouvrez **Servers**, ajoutez votre serveur Usenet et appuyez sur **Test connection**.
3. Ouvrez **Quick Post** et glissez vos fichiers ou dossiers dans la fenêtre.
4. Appuyez sur **Post Files**. Le fichier NZB est écrit dans le dossier choisi dans les réglages, et chaque envoi apparaît dans **History**.

Vous utilisez déjà ngPost ? Copiez votre `ngPost.conf` dans `%APPDATA%\ngPost Deluxe` et vos serveurs et réglages sont repris.

### Captures d'écran
| Quick Post (thème clair) | Historique |
|---|---|
| ![Quick Post (thème clair)](docs/screenshots/quick-post.png) | ![Historique](docs/screenshots/history.png) |
| **Paramètres (thème sombre)** | **Serveurs (thème sombre)** |
| ![Paramètres (thème sombre)](docs/screenshots/settings-dark.png) | ![Serveurs (thème sombre)](docs/screenshots/servers-dark.png) |

### Ce qui change dans ce fork

**Interface**
-   **Nouvelle interface** : barre latérale (Quick Post, Post Auto, Serveurs, Paramètres, Historique), pages en cartes, options avancées masquées tant qu'elles ne servent pas, une nouvelle icône et un écran vide qui explique quoi faire. **Thèmes clair et sombre**, ou automatique comme Windows (clé `theme`). Définissez la variable d'environnement `NGPOST_NO_THEME` pour retrouver l'apparence native
-   **Interface en italien**: l'interface graphique et la ligne de commande sont maintenant aussi traduites en italien (déjà disponibles en allemand, anglais, chinois, espagnol, français, néerlandais et portugais)
-   **ngPost Deluxe v1.0** : nouveau nom et nouvelle version (le programme Windows s'appelle maintenant `ngPostDeluxe.exe` ; le fichier de configuration ne change pas), fenêtre À propos redessinée, les boutons de don d'origine sont remplacés par un bouton **Ko-fi** pour ce fork (dans la fenêtre À propos et dans les Paramètres)
-   **Nouvelle section Serveurs** : liste des serveurs avec un voyant d'état et une case activé/désactivé, fiche de détail (mot de passe affiché à la demande) et un bouton **Tester la connexion** qui vérifie l'identification. Modifier les serveurs pendant un envoi ne peut plus faire planter ngPost
-   **Onglet Auto Posting** : nouveaux boutons **Retirer les terminés**, **Tout retirer** et **Fermer les onglets terminés**
-   **Page Historique** : chaque upload est enregistré (`ngPost_history.csv` à côté de la configuration, ou `POST_HISTORY`) avec son statut, le nombre de fichiers et la durée. La page les liste avec recherche, filtre par période, totaux, mots de passe masqués jusqu'à la demande, copie, export et dossier du NZB. Attention : le fichier contient les mots de passe des archives en clair
-   **Quick Post** : une note rappelle que tous les éléments de la liste sont postés comme une seule release (un NZB, une archive)

**Vitesse**
-   **ParPar** (un générateur de par2 bien plus rapide) est fourni pour Windows 64bit (v0.4.6, domaine public) et utilisé automatiquement quand `parpar.exe` est dans le dossier de ngPost. par2cmdline 64bit a été retiré du package, la version 32bit est conservée pour Windows 32bit
-   **Compression plus rapide sous Windows** : la sortie de 7-Zip / rar est maintenant lue au fur et à mesure (un post de 115 Mo attendait 200 s avant, 1 s maintenant)

**Paquets Windows et Docker**
-   **ngPost Deluxe v1.1** : **image Docker** de la version en ligne de commande (elle surveille un dossier et poste tout ce qui y arrive, avec un modèle pour **Unraid**), le bouton Post est centré dans Quick Post et Auto Posting, les icônes des onglets et les onglets terminés sont lisibles dans le thème sombre
-   **Installateur Windows** (64 bits et 32 bits, avec **7-Zip inclus**) et paquets ZIP portables, construits automatiquement sur GitHub et publiés dans les Releases (`ngPostDeluxe.exe`)
-   **Windows** : la fenêtre de console se ferme d'elle-même quand ngPost est lancé par un double clic ; le paquet contient OpenSSL 1.1.1w (dernière version supportée par Qt 5)

**Fiabilité et confort (v1.2)**
-   **Articles en échec** : un Article qui échoue est retenté sur un autre serveur et lors d'un second tour ; le bouton **Repost missing Articles** reposte uniquement les Articles manquants et corrige le NZB ; les fichiers d'origine sont conservés quand des Articles ont échoué
-   **Mauvais mot de passe** : l'envoi s'arrête, avec un message clair, quand le serveur refuse la connexion sur toutes les connexions
-   **File d'attente** : les envois non terminés sont enregistrés et proposés à nouveau au prochain démarrage (`--resume_queue` en ligne de commande)
-   **Progression et journal** : barre de progression avec la phase, le temps restant et des couleurs ; journal en couleurs avec la durée de chaque phase ; mesures de latence dans le rapport de diagnostic
-   **Newsgroups** : une zone avec des « chips », des suggestions et des vérifications, et un menu pour choisir où vont les envois quand il y a plusieurs groupes
-   Page d'**Aide** dans le programme (en anglais), guide pour Docker derrière un VPN (gluetun) et tests automatiques de l'envoi dans la CI

**Sécurité et correctifs**
-   **Correctif de sécurité**: dépassement de mémoire possible dans le tampon d'encodage yEnc (la sortie pouvait être plus grande que le tampon avec certaines données)
-   **Correctif yEnc**: un espace ou une tabulation en fin de ligne est maintenant correctement échappé (certains serveurs le supprimaient, ce qui corrompait les données)
-   **Mots de passe plus sûrs**: les mots de passe et noms d'archives générés utilisent le générateur aléatoire cryptographique du système (Qt >= 5.10) à la place de `std::rand()`
-   **Mot de passe de l'archive hors de la ligne de commande** : 7-Zip le reçoit sur son entrée standard et rar par la variable d'environnement `RAR`, il n'apparaît donc plus dans la liste des processus

**Réglages et outils**
-   **NZB_POST_CMD**: les variables peuvent contenir des espaces (un chemin comme `C:\Mes dossiers\fichier.nzb` reste UN SEUL argument) et la commande est lancée en arrière-plan (ngPost ne l'attend plus, son code de retour n'est donc plus affiché)
-   **RAR_EXTRA et PAR2_ARGS**: analysés en respectant les guillemets. 7-Zip est détecté d'après le nom du programme et non d'après tout le chemin
-   **Nouveau README**: réécrit pour ce fork et disponible en 8 langues (anglais, français, allemand, italien, espagnol, néerlandais, portugais, chinois)
-   **Enregistrement automatique** : chaque modification des réglages est enregistrée automatiquement (le fichier précédent est conservé en `ngPost.conf.bak`)
-   **Diagnostic** : ngPost enregistre la durée et la vitesse de chaque phase (compression, par2, envoi), les erreurs et les blocages, et le bouton **Rapport de diagnostic** enregistre un rapport pour analyser un problème. Mots de passe, identifiants et noms de fichiers sont masqués, rien n'est envoyé

Les installateurs et paquets portables pour Windows et l'image Docker sont dans les [Releases](https://github.com/thenormal/ngPost-Deluxe/releases/latest). Sur les autres systèmes, compilez-le depuis les sources (voir ci-dessous).<br/>

### Fonctionnalités principales
-   **Posteur Usenet en ligne de commande et avec interface graphique** pour les binaires, développé en **C++11/Qt5** et conçu pour être le plus rapide possible
-   **compresse** (avec votre rar/7zip externe) et **génère les par2** avant de poster
-   dispose d'une **file d'attente** et **prépare le post suivant pendant l'upload du courant**
-   **automatise les posts**: scanne ou surveille des dossiers et poste chaque nouveau fichier/dossier individuellement (nom d'archive et mot de passe aléatoires possibles)
-   **obfusque les en-têtes des Articles**: poster aléatoire et sujet en UUID, un post ne se retrouve qu'avec le fichier nzb
-   **génère le fichier nzb** et écrit un **historique des posts** au format csv (nom d'archive et mot de passe inclus)
-   peut **supprimer les fichiers une fois postés**, **exécuter une commande ou un script** après chaque post (NZB_POST_CMD) et **éteindre l'ordinateur**
-   traduit en **allemand, anglais, chinois, espagnol, français, italien, néerlandais et portugais**

### Mots-clés de configuration modifiés dans ce fork
Le fichier de configuration fonctionne comme dans le projet original (par défaut: **~/.ngPost** sous Linux et Mac, **ngPost.conf** dans le dossier d'installation sous Windows). Seuls les mots-clés dont le comportement a changé sont listés ici:

- **NZB_POST_CMD**: commande exécutée après chaque post. Les variables peuvent contenir des espaces et la commande est lancée en arrière-plan (son code de retour n'est pas enregistré)
- **RAR_EXTRA**: pour personnaliser la commande rar (inutile de préciser 'a', '-idp' ou '-r'). Inutile pour 7-zip, sauf pour changer le niveau de compression. Respecte maintenant les guillemets.
- **PAR2_PATH**: pour changer le générateur de par2 et pouvoir utiliser [Parpar](https://github.com/animetosho/ParPar) ou [Multipar](http://hp.vector.co.jp/authors/VA021385/). (ParPar est utilisé automatiquement sous Windows 64bit si `parpar.exe` est dans le dossier de ngPost; par2cmdline reste le choix par défaut ailleurs)
- **PAR2_ARGS**: pour personnaliser la commande par2, surtout si vous choisissez un programme différent de celui par défaut. Respecte maintenant les guillemets.
- **theme** : thème de l'interface, `AUTO` (comme Windows), `LIGHT` ou `DARK`
- **history** : `ON` (par défaut) ou `OFF`, historique des uploads affiché dans la page Historique. Sans **POST_HISTORY** le fichier est `ngPost_history.csv` à côté de la configuration. De nouvelles colonnes sont ajoutées à la fin de chaque ligne : statut, nombre de fichiers, durée et fichier NZB

### Comment compiler ngPost
#### Linux
Dépendances (Ubuntu / Debian) :
```
sudo apt install build-essential qtbase5-dev qt5-qmake libssl-dev
```
Compiler la version graphique, ou seulement la version en ligne de commande (les bibliothèques graphiques de Qt ne sont pas nécessaires) :
```
# GUI
cd src && qmake ngPost.pro && make -j$(nproc)

# command line only, in its own folder
mkdir build-cmd && cd build-cmd && qmake ../src/ngPost_cmd.pro && make -j$(nproc)
```
L'exécutable **ngPost** est créé dans le dossier où vous avez compilé : copiez-le dans un dossier de votre PATH.

#### Windows
Installez [Qt 5.15](https://www.qt.io/download) avec MinGW, puis ouvrez `src/ngPost.pro` dans Qt Creator, ou depuis une invite de commandes Qt : `mkdir build && cd build && qmake ../src/ngPost.pro CONFIG+=release && mingw32-make`. Le programme s'appelle `ngPostDeluxe.exe`. L'installateur est créé avec [Inno Setup](https://jrsoftware.org/isinfo.php) (`installer/ngPostDeluxe.iss`) ; toutes les étapes des paquets officiels sont dans `.github/workflows/build-windows.yml`.

#### Docker
Version en ligne de commande dans un conteneur (voir [docker/README.md](docker/README.md)) :
```
docker build -t ngpost-deluxe .
```

#### Autres systèmes
Écrit en C++/Qt, il peut être compilé et exécuté sur n'importe quel système (macOS, Android...) : le plus simple est d'[installer Qt](https://www.qt.io/download) et d'ouvrir `src/ngPost.pro` dans Qt Creator.

### Documentation complète
Les options de la ligne de commande, la liste complète des mots-clés de configuration et les guides d'installation sont ceux du projet original: voir le [README de ngPost](https://github.com/mbruel/ngPost#readme). Ses guides d'installation concernent les binaires originaux, pas ce fork.

### FAQ

**Windows affiche « Windows a protégé votre ordinateur ». Est-ce sûr ?**<br/>
L'installateur n'est pas signé numériquement (une signature coûte de l'argent), donc SmartScreen avertit pour tout programme inconnu. Cliquez sur **Informations complémentaires**, puis sur **Exécuter quand même**. Le programme est open source et les paquets sont construits automatiquement à partir de ce dépôt.

**Où sont mes réglages et mon historique ?**<br/>
Avec l'installateur : dans `%APPDATA%\ngPost Deluxe` (`ngPost.conf` et `ngPost_history.csv`). Avec la version portable : `ngPost.conf` à côté du programme. S'il y a un `ngPost.conf` à côté de `ngPostDeluxe.exe`, il est utilisé à la place de celui de `%APPDATA%`. L'historique peut être déplacé avec `POST_HISTORY`.

**Comment garder les réglages de l'ngPost d'origine ?**<br/>
Fermez ngPost Deluxe, copiez votre ancien `ngPost.conf` dans `%APPDATA%\ngPost Deluxe` et, s'il contient une ligne `PAR2_PATH` qui pointe vers un ancien dossier, supprimez-la (le programme utilisera son propre ParPar). Votre ancien `ngPost_history.csv` est lu aussi : copiez-le à côté de la configuration, ou gardez son chemin dans `POST_HISTORY`.

**Quelque chose ne fonctionne pas (compression, par2, envoi…). Que faire ?**<br/>
Appuyez sur **Diagnostic report** dans le journal et joignez le fichier à une [issue](https://github.com/thenormal/ngPost-Deluxe/issues). Les mots de passe, noms d'utilisateur et noms de fichiers sont masqués, rien n'est envoyé.

**Dois-je installer 7-Zip ou rar ?**<br/>
Non : 7-Zip est inclus dans l'installateur et dans le paquet portable. Vous pouvez toujours choisir un autre 7-Zip ou WinRAR dans les réglages.

**L'historique est-il privé ?**<br/>
Il contient les mots de passe des archives en clair : gardez privés `ngPost_history.csv` et `ngPost.conf` (qui contient l'identifiant de votre serveur) et ne les joignez pas à une issue publique.

### Licence
[**ngPost**](https://github.com/mbruel/ngPost) est publié sous licence **GPL v3** (voir [LICENSE](LICENSE)). Copyright (C) 2020 Matthieu Bruel. ngPost Deluxe est un fork maintenu par [thenormal](https://github.com/thenormal).

### Questions / Problèmes / Demandes
- pour toute question sur ce fork, vous pouvez contacter son mainteneur sur GitHub: [thenormal](https://github.com/thenormal)
- pour le programme original, référez-vous au [projet original](https://github.com/mbruel/ngPost) et à son auteur Matthieu Bruel

### Langues supportées
Pour l'instant ngPost est traduit en allemand, anglais, chinois, espagnol, français, italien, néerlandais et portugais (la traduction italienne a été ajoutée dans ce fork).<br/>
Si vous souhaitez le traduire dans une autre langue, c'est très simple (Qt fournit un GUI pour cela: QtLinguist), les fichiers de traduction sont dans le dossier **src/lang**.

### Remerciements
- Matthieu Bruel (mbruel) pour avoir créé ngPost et l'avoir publié sous licence GPL v3
- Uukrull pour ses tests intensifs, ses retours et pour avoir construit tous les packages MacOS.
- awsms pour ses tests sur un vrai serveur avec une connexion de 10Gb/s qui ont permis d'améliorer la vitesse d'upload de ngPost et le support du multi-threading
- animetosho pour avoir développé ParPar, le générateur de par2 le plus rapide qui soit!
- demanuel pour le développement de NewsUP qui fut mon premier posteur
- noobcoder1983 pour la traduction allemande
- tiriclote pour la traduction espagnole
- hunesco pour la traduction portugaise
- Peng pour la traduction chinoise
- tous les utilisateurs de ngPost ;)

### Soutenir ce fork
ngPost Deluxe est gratuit. S'il vous est utile, vous pouvez soutenir son développement sur Ko-fi :<br/>
[![Ko-fi](https://img.shields.io/badge/Ko--fi-support%20ngPost%20Deluxe-FF5E5B?logo=ko-fi&logoColor=white)](https://ko-fi.com/thenormal)

### Soutenir l'auteur original
ngPost a été créé par Matthieu Bruel. Si vous utilisez l'application et souhaitez contribuer à l'effort, merci de le faire sur le [projet original](https://github.com/mbruel/ngPost).
