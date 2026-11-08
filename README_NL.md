<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost Deluxe

[English](README.md) | [Français](README_FR.md) | [Deutsch](README_DE.md) | [Italiano](README_IT.md) | [Español](README_ES.md) | **Nederlands** | [Português](README_PT.md) | [中文](README_ZH.md)

**Een snelle, gratis en moderne Usenet-poster voor Windows, Linux en Docker.**<br/>
Comprimeer, voeg par2 toe, versluier en post je bestanden via een overzichtelijke interface (licht en donker thema) of via de opdrachtregel.

[![Latest release](https://img.shields.io/github/v/release/thenormal/ngPost-Deluxe)](https://github.com/thenormal/ngPost-Deluxe/releases/latest)
[![Build](https://github.com/thenormal/ngPost-Deluxe/actions/workflows/build-windows.yml/badge.svg)](https://github.com/thenormal/ngPost-Deluxe/actions)
[![Licence: GPL v3](https://img.shields.io/badge/licence-GPL%20v3-blue)](LICENSE)

[![Download](https://img.shields.io/badge/Download-latest%20release-2ea44f?style=for-the-badge)](https://github.com/thenormal/ngPost-Deluxe/releases/latest)

**ngPost Deluxe** is een herziene en opnieuw ontworpen versie van de bekende Usenet-uploader **ngPost**.<br/>
Het is een fork van [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, licentie GPL v3). Alle eer voor het oorspronkelijke programma gaat naar de auteur, Matthieu Bruel.<br/>

### Aan de slag
| Windows-installatieprogramma | Windows draagbaar | Docker / Unraid |
|---|---|---|
| Download `ngPostDeluxe-v…-win64-setup.exe` (of `win32`) van de pagina [Releases](https://github.com/thenormal/ngPost-Deluxe/releases/latest). 7-Zip is inbegrepen. | Pak `ngPostDeluxe-v…-win64.zip` (of `win32`) ergens uit en start `ngPostDeluxe.exe`. De instellingen staan in het `ngPost.conf` naast het programma. | Image `ghcr.io/thenormal/ngpost-deluxe` met een sjabloon voor Unraid. Zie [docker/README.md](docker/README.md). |

> Windows kan "Windows heeft uw pc beschermd" tonen omdat het installatieprogramma niet digitaal is ondertekend: klik op **Meer info** en daarna op **Toch uitvoeren**.

### Snelstart
1. Installeer ngPost Deluxe en start het.
2. Open **Servers**, voeg je Usenet-server toe en druk op **Test connection**.
3. Open **Quick Post** en sleep je bestanden of mappen in het venster.
4. Druk op **Post Files**. Het NZB-bestand wordt weggeschreven in de map die je in de instellingen koos, en elke upload staat in **History**.

Gebruik je al ngPost? Kopieer je `ngPost.conf` naar `%APPDATA%\ngPost Deluxe` en je servers en instellingen zijn er weer.

### Schermafbeeldingen
| Quick Post (licht thema) | Geschiedenis |
|---|---|
| ![Quick Post (licht thema)](docs/screenshots/quick-post.png) | ![Geschiedenis](docs/screenshots/history.png) |
| **Instellingen (donker thema)** | **Servers (donker thema)** |
| ![Instellingen (donker thema)](docs/screenshots/settings-dark.png) | ![Servers (donker thema)](docs/screenshots/servers-dark.png) |

### Wat is er anders in deze fork

**Interface**
-   **Nieuwe interface**: zijbalk (Quick Post, Auto Posting, Servers, Instellingen, Geschiedenis), pagina's met kaarten, geavanceerde opties verborgen tot ze nodig zijn, een nieuw pictogram en een leeg scherm dat uitlegt wat te doen. **Licht en donker thema**, of automatisch zoals Windows (sleutel `theme`). Zet de omgevingsvariabele `NGPOST_NO_THEME` om het native uiterlijk terug te krijgen
-   **Italiaanse interface**: de GUI/CMD is nu ook in het Italiaans vertaald (al beschikbaar in het Chinees, Nederlands, Engels, Frans, Duits, Portugees en Spaans)
-   **ngPost Deluxe v1.0**: nieuwe naam en versie (het Windows-programma heet nu `ngPostDeluxe.exe`; het configuratiebestand verandert niet), vernieuwd Over-venster, de oorspronkelijke donatieknoppen zijn vervangen door een **Ko-fi**-knop voor deze fork (in het Over-venster en in de Instellingen)
-   **Nieuwe sectie Servers**: lijst van de servers met een statuslampje en een aan/uit-vakje, detailformulier (wachtwoord op verzoek zichtbaar) en een knop **Verbinding testen** die de aanmelding controleert. Servers wijzigen tijdens het posten kan ngPost niet meer laten crashen
-   **Tabblad Auto Posting**: nieuwe knoppen **Voltooide verwijderen**, **Alles verwijderen** en **Voltooide tabbladen sluiten**
-   **Pagina Geschiedenis**: elke upload wordt bewaard (`ngPost_history.csv` naast de configuratie, of `POST_HISTORY`) met status, aantal bestanden en duur. De pagina toont ze met zoeken, periodefilter, totalen, verborgen wachtwoorden, kopiëren, exporteren en de map van de NZB. Let op: het bestand bevat de archiefwachtwoorden in leesbare tekst
-   **Quick Post**: een opmerking herinnert eraan dat alle items van de lijst als één release worden gepost (één NZB, één archief)

**Snelheid**
-   **ParPar** (een veel snellere par2-generator) wordt meegeleverd voor Windows 64bit (v0.4.6, publiek domein) en automatisch gebruikt als `parpar.exe` in de ngPost-map staat. De 64bit par2cmdline is uit het pakket verwijderd, de 32bit-versie blijft voor Windows 32bit
-   **Snellere compressie op Windows**: de uitvoer van 7-Zip / rar wordt nu gelezen zodra die binnenkomt (een post van 115 MB wachtte eerst 200 s, nu 1 s)

**Windows-pakketten en Docker**
-   **ngPost Deluxe v1.1**: **Docker-image** van de opdrachtregelversie (bewaakt een map en post alles wat daar binnenkomt, met een sjabloon voor **Unraid**), de knop Post staat gecentreerd in Quick Post en Auto Posting, tabpictogrammen en voltooide tabs zijn leesbaar in het donkere thema
-   **Windows-installatieprogramma** (64 bit en 32 bit, met **7-Zip inbegrepen**) en draagbare ZIP-pakketten, automatisch gebouwd op GitHub en gepubliceerd in de Releases (`ngPostDeluxe.exe`)
-   **Windows**: het consolevenster sluit vanzelf wanneer ngPost met een dubbelklik wordt gestart; het pakket bevat OpenSSL 1.1.1w (nieuwste versie die Qt 5 ondersteunt)

**Beveiliging en correcties**
-   **Beveiligingsfix**: mogelijke heap-overflow in de yEnc-codeerbuffer (bij bepaalde gegevens kon de uitvoer groter zijn dan de buffer)
-   **yEnc-fix**: een spatie of tab aan het einde van een regel wordt nu correct geëscaped (sommige servers verwijderden die, waardoor de gegevens beschadigd raakten)
-   **Veiligere wachtwoorden**: de gegenereerde archiefwachtwoorden en -namen gebruiken de cryptografische willekeurige generator van het besturingssysteem (Qt >= 5.10) in plaats van `std::rand()`
-   **Archiefwachtwoord niet meer op de opdrachtregel**: 7-Zip krijgt het via de standaardinvoer en rar via de omgevingsvariabele `RAR`, zodat het niet meer zichtbaar is in de proceslijst

**Instellingen en hulpmiddelen**
-   **NZB_POST_CMD**: de plaatshouders mogen spaties bevatten (een pad zoals `C:\Mijn mappen\bestand.nzb` blijft ÉÉN argument) en het commando wordt op de achtergrond gestart (ngPost wacht er niet meer op, dus de exitcode wordt niet meer gelogd)
-   **RAR_EXTRA en PAR2_ARGS**: worden verwerkt met respect voor dubbele aanhalingstekens. 7-Zip wordt herkend aan de programmanaam en niet aan het hele pad
-   **Nieuwe README**: herschreven voor deze fork en beschikbaar in 8 talen (Engels, Frans, Duits, Italiaans, Spaans, Nederlands, Portugees, Chinees)
-   **Automatisch opslaan**: elke wijziging van de instellingen wordt automatisch opgeslagen (het vorige bestand blijft bewaard als `ngPost.conf.bak`)
-   **Diagnose**: ngPost registreert de duur en snelheid van elke fase (compressie, par2, upload), de fouten en het vastlopen, en de knop **Diagnoserapport** slaat een rapport op om een probleem te analyseren. Wachtwoorden, gebruikersnamen en bestandsnamen worden gemaskeerd, er wordt niets verstuurd

De Windows-installatieprogramma's, de draagbare pakketten en de Docker-image staan bij [Releases](https://github.com/thenormal/ngPost-Deluxe/releases/latest). Op andere systemen bouw je het vanaf de broncode (zie hieronder).<br/>

### Belangrijkste functies
-   **Usenet-poster voor de opdrachtregel en met een mooie GUI** voor binaire bestanden, ontwikkeld in **C++11/Qt5**
-   hij is ontworpen om **zo snel mogelijk** te zijn en ALLE belangrijke functies te bieden **om gegevens eenvoudig en veilig te posten**.
-   hij kan **comprimeren** (met je externe rar-programma) en vóór het posten **de par2-bestanden genereren**!
-   hij heeft een **wachtrij** om meerdere posts voor te bereiden (vooral in de GUI met tabbladen).
-   hij **pakt de volgende post in terwijl de huidige wordt geüpload** (compressie + par2-generatie)
-   hij kan posts **automatiseren** door **mappen te scannen** en elk bestand/elke map afzonderlijk te posten nadat ze zijn gecomprimeerd (eventueel met willekeurige archiefnaam en wachtwoord) en de par2 is gegenereerd!
-   hij kan **mappen bewaken en elk nieuw bestand/elke nieuwe map** afzonderlijk posten nadat ze zijn gecomprimeerd.
-   hij kan **bestanden/mappen automatisch verwijderen na het posten** (alleen bij bewaking in de GUI en met --auto en --monitor samen in de opdrachtregel)
-   hij kan na elke post **een COMMANDO of script uitvoeren** (zie NZB_POST_CMD in het configuratiebestand)
-   hij kan **de computer uitschakelen** als alle posts klaar zijn
-   hij is **vertaald in het Chinees, Nederlands, Engels, Frans, Duits, Italiaans, Portugees en Spaans**.

### In deze fork gewijzigde configuratiesleutelwoorden
Het configuratiebestand werkt zoals in het oorspronkelijke project (standaard: **~/.ngPost** op Linux en Mac, **ngPost.conf** in de installatiemap op Windows). Hier staan alleen de sleutelwoorden waarvan het gedrag is veranderd:

- **NZB_POST_CMD**: commando dat na elke post wordt uitgevoerd. De plaatshouders mogen spaties bevatten en het commando draait op de achtergrond (de exitcode wordt niet gelogd)
- **RAR_EXTRA**: om het rar-commando aan te passen (je hoeft de 'a', '-idp' of '-r' niet op te geven). Niet nodig voor 7-zip, behalve als je het compressieniveau wilt wijzigen.
- **PAR2_PATH**: om de par2-generator te wijzigen en [Parpar](https://github.com/animetosho/ParPar) of [Multipar](http://hp.vector.co.jp/authors/VA021385/) te kunnen gebruiken. (par2cmdline is de standaard meegeleverde generator, ParPar wordt op Windows 64bit automatisch gebruikt als `parpar.exe` in de ngPost-map staat)
- **PAR2_ARGS**: om het par2-commando aan te passen, vooral als je een ander programma kiest dan de standaard par2cmdline
- **theme**: thema van de interface, `AUTO` (zoals Windows), `LIGHT` of `DARK`
- **history**: `ON` (standaard) of `OFF`, geschiedenis van de uploads op de pagina Geschiedenis. Zonder **POST_HISTORY** is het bestand `ngPost_history.csv` naast de configuratie. Aan het einde van elke regel komen nieuwe kolommen: status, aantal bestanden, duur en NZB-bestand

### Compileren
#### Linux
Afhankelijkheden (Ubuntu / Debian):
```
sudo apt install build-essential qtbase5-dev qt5-qmake libssl-dev
```
Bouw de grafische versie, of alleen de opdrachtregelversie (de grafische Qt-bibliotheken zijn dan niet nodig):
```
# GUI
cd src && qmake ngPost.pro && make -j$(nproc)

# command line only, in its own folder
mkdir build-cmd && cd build-cmd && qmake ../src/ngPost_cmd.pro && make -j$(nproc)
```
Het uitvoerbare bestand **ngPost** staat daarna in de map waar je hebt gebouwd: kopieer het naar een map in je PATH.

#### Windows
Installeer [Qt 5.15](https://www.qt.io/download) met MinGW en open `src/ngPost.pro` in Qt Creator, of vanaf een Qt-opdrachtprompt: `mkdir build && cd build && qmake ../src/ngPost.pro CONFIG+=release && mingw32-make`. Het programma heet `ngPostDeluxe.exe`. Het installatieprogramma wordt gemaakt met [Inno Setup](https://jrsoftware.org/isinfo.php) (`installer/ngPostDeluxe.iss`); alle stappen van de officiële pakketten staan in `.github/workflows/build-windows.yml`.

#### Docker
Opdrachtregelversie in een container (zie [docker/README.md](docker/README.md)):
```
docker build -t ngpost-deluxe .
```

#### Andere systemen
Omdat het in C++/Qt is geschreven, kun je het op elk systeem bouwen en draaien (macOS, Android...): het makkelijkst is [Qt installeren](https://www.qt.io/download) en `src/ngPost.pro` in Qt Creator laden.

### Volledige documentatie
De opdrachtregelopties, de volledige lijst van configuratiesleutelwoorden en de installatiehandleidingen zijn die van het oorspronkelijke project: zie de [README van ngPost](https://github.com/mbruel/ngPost#readme). De installatiehandleidingen daar verwijzen naar de originele binaire bestanden, niet naar deze fork.

### FAQ

**Windows meldt "Windows heeft uw pc beschermd". Is het veilig?**<br/>
Het installatieprogramma is niet digitaal ondertekend (een handtekening kost geld), daarom waarschuwt SmartScreen voor elk onbekend programma. Klik op **Meer informatie** en dan op **Toch uitvoeren**. Het programma is open source en de pakketten worden automatisch uit deze repository gebouwd.

**Waar staan mijn instellingen en mijn geschiedenis?**<br/>
Met het installatieprogramma: in `%APPDATA%\ngPost Deluxe` (`ngPost.conf` en `ngPost_history.csv`). Met de draagbare versie: `ngPost.conf` naast het programma. Staat er een `ngPost.conf` naast `ngPostDeluxe.exe`, dan wordt die gebruikt in plaats van die in `%APPDATA%`. De geschiedenis kun je verplaatsen met `POST_HISTORY`.

**Hoe behoud ik de instellingen van het originele ngPost?**<br/>
Sluit ngPost Deluxe, kopieer je oude `ngPost.conf` naar `%APPDATA%\ngPost Deluxe` en verwijder, als er een regel `PAR2_PATH` in staat die naar een oude map wijst, die regel (het programma gebruikt dan zijn eigen ParPar). Je oude `ngPost_history.csv` wordt ook gelezen: kopieer het naast de configuratie, of behoud het pad in `POST_HISTORY`.

**Iets werkt niet (compressie, par2, upload…). Wat kan ik doen?**<br/>
Druk in het logboek op **Diagnostic report** en voeg het bestand toe aan een [issue](https://github.com/thenormal/ngPost-Deluxe/issues). Wachtwoorden, gebruikersnamen en bestandsnamen worden gemaskeerd en er wordt niets verzonden.

**Moet ik 7-Zip of rar installeren?**<br/>
Nee: 7-Zip zit in het installatieprogramma en in het draagbare pakket. In de instellingen kun je nog steeds een andere 7-Zip of WinRAR kiezen.

**Is de geschiedenis privé?**<br/>
Ze bevat de wachtwoorden van de archieven in leesbare tekst: houd `ngPost_history.csv` en `ngPost.conf` (met je serverlogin) privé en voeg ze niet toe aan een openbaar issue.

### Licentie
[**ngPost**](https://github.com/mbruel/ngPost) wordt uitgebracht onder de **GPL v3**-licentie (zie [LICENSE](LICENSE)). Copyright (C) 2020 Matthieu Bruel. ngPost Deluxe is een fork die door [thenormal](https://github.com/thenormal) wordt onderhouden.

### Vragen / Problemen / Verzoeken
- voor vragen over deze fork kun je contact opnemen met de beheerder op GitHub: [thenormal](https://github.com/thenormal)
- voor het originele programma kun je terecht bij het [oorspronkelijke project](https://github.com/mbruel/ngPost) en de auteur Matthieu Bruel

### Ondersteunde talen
Op dit moment is ngPost vertaald in het Chinees, Nederlands, Engels, Frans, Duits, Italiaans, Portugees en Spaans (de Italiaanse vertaling is in deze fork toegevoegd).<br/>
Als je ngPost in jouw taal wilt vertalen, is dat eenvoudig (daarvoor is er een handige GUI: QtLinguist). De vertaalbestanden staan in de map **src/lang**.

### Dankwoord
- Matthieu Bruel (mbruel) voor het maken van ngPost en het publiceren onder GPL v3
- Uukrull voor zijn intensieve tests en feedback en voor het bouwen van alle MacOS-pakketten.
- awsms voor zijn tests op een echte server met een verbinding van 10 Gb/s, die hebben geholpen om de uploadsnelheid van ngPost en de multithreading-ondersteuning te verbeteren
- animetosho voor de ontwikkeling van ParPar, de snelste par2-generator ooit!
- demanuel voor de ontwikkeling van NewsUP, dat mijn eerste poster was
- noobcoder1983, tensai en daarna yuppie voor de Duitse vertaling
- tiriclote voor de Spaanse vertaling
- hunesco voor de Portugese vertaling
- Peng voor de Chinese vertaling
- alle gebruikers van ngPost ;)

### Steun deze fork
ngPost Deluxe is gratis. Als je er iets aan hebt, kun je de ontwikkeling steunen op Ko-fi:<br/>
[![Ko-fi](https://img.shields.io/badge/Ko--fi-support%20ngPost%20Deluxe-FF5E5B?logo=ko-fi&logoColor=white)](https://ko-fi.com/thenormal)

### Steun de oorspronkelijke auteur
ngPost is gemaakt door Matthieu Bruel. Als je de app gebruikt en wilt bijdragen aan de inspanning, doe dat dan via het [oorspronkelijke project](https://github.com/mbruel/ngPost).
