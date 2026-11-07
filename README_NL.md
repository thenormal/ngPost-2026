<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost-2026

[English](README.md) | [Français](README_FR.md) | [Deutsch](README_DE.md) | [Italiano](README_IT.md) | [Español](README_ES.md) | **Nederlands** | [Português](README_PT.md) | [中文](README_ZH.md)

**ngPost-2026** is een herziene en opnieuw ontworpen versie van de bekende Usenet-uploader **ngPost**.<br/>
Het is een fork van [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, licentie GPL v3). Alle eer voor het oorspronkelijke programma gaat naar de auteur, Matthieu Bruel.<br/>

### Wat is er anders in deze fork
-   **Beveiligingsfix**: mogelijke heap-overflow in de yEnc-codeerbuffer (bij bepaalde gegevens kon de uitvoer groter zijn dan de buffer)
-   **yEnc-fix**: een spatie of tab aan het einde van een regel wordt nu correct geëscaped (sommige servers verwijderden die, waardoor de gegevens beschadigd raakten)
-   **Veiligere wachtwoorden**: de gegenereerde archiefwachtwoorden en -namen gebruiken de cryptografische willekeurige generator van het besturingssysteem (Qt >= 5.10) in plaats van `std::rand()`
-   **NZB_POST_CMD**: de plaatshouders mogen spaties bevatten (een pad zoals `C:\Mijn mappen\bestand.nzb` blijft ÉÉN argument) en het commando wordt op de achtergrond gestart (ngPost wacht er niet meer op, dus de exitcode wordt niet meer gelogd)
-   **RAR_EXTRA en PAR2_ARGS**: worden verwerkt met respect voor dubbele aanhalingstekens. 7-Zip wordt herkend aan de programmanaam en niet aan het hele pad
-   **ParPar** (een veel snellere par2-generator) wordt meegeleverd voor Windows 64bit (v0.4.6, publiek domein) en automatisch gebruikt als `parpar.exe` in de ngPost-map staat. De 64bit par2cmdline is uit het pakket verwijderd, de 32bit-versie blijft voor Windows 32bit
-   **Modern licht thema**: de GUI heeft een schoon uiterlijk geïnspireerd op Windows 11 (Fusion-stijl + stylesheet). Stel de omgevingsvariabele `NGPOST_NO_THEME` in om het native uiterlijk terug te krijgen
-   **Italiaanse interface**: de GUI/CMD is nu ook in het Italiaans vertaald (al beschikbaar in het Chinees, Nederlands, Engels, Frans, Duits, Portugees en Spaans)
-   **Nieuwe README**: herschreven voor deze fork en beschikbaar in 8 talen (Engels, Frans, Duits, Italiaans, Spaans, Nederlands, Portugees, Chinees)
-   **ngPost 2026 v5.0 bèta**: nieuwe naam en versie, vernieuwd Over-venster, de donatieknoppen zijn verwijderd (om de oorspronkelijke auteur te steunen, zie het einde van deze pagina)
-   **Nieuwe sectie Servers**: lijst van de servers met een statuslampje en een aan/uit-vakje, detailformulier (wachtwoord op verzoek zichtbaar) en een knop **Verbinding testen** die de aanmelding controleert. Servers wijzigen tijdens het posten kan ngPost niet meer laten crashen
-   **Automatisch opslaan**: elke wijziging van de instellingen wordt automatisch opgeslagen (het vorige bestand blijft bewaard als `ngPost.conf.bak`)
-   **Tabblad Auto Posting**: nieuwe knoppen **Voltooide verwijderen**, **Alles verwijderen** en **Voltooide tabbladen sluiten**
-   **Archiefwachtwoord niet meer op de opdrachtregel**: 7-Zip krijgt het via de standaardinvoer en rar via de omgevingsvariabele `RAR`, zodat het niet meer zichtbaar is in de proceslijst
-   **Diagnose**: ngPost registreert de duur en snelheid van elke fase (compressie, par2, upload), de fouten en het vastlopen, en de knop **Diagnoserapport** slaat een rapport op om een probleem te analyseren. Wachtwoorden, gebruikersnamen en bestandsnamen worden gemaskeerd, er wordt niets verstuurd
-   **Windows**: het consolevenster sluit vanzelf wanneer ngPost met een dubbelklik wordt gestart; het pakket bevat OpenSSL 1.1.1w (nieuwste versie die Qt 5 ondersteunt)

Er zijn nog geen officiële releases van deze fork. Een kant-en-klaar **Windows 64bit**-pakket wordt bij elke wijziging automatisch gebouwd door GitHub Actions (tabblad *Actions* > run *Build Windows (x64)* > *Artifacts*, een GitHub-account is nodig). Op andere systemen bouw je het vanaf de broncode (zie hieronder).<br/>

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
- **PAR2_CMD**: om de par2-generator te wijzigen en [Parpar](https://github.com/animetosho/ParPar) of [Multipar](http://hp.vector.co.jp/authors/VA021385/) te kunnen gebruiken. (par2cmdline is de standaard meegeleverde generator, ParPar wordt op Windows 64bit automatisch gebruikt als `parpar.exe` in de ngPost-map staat)
- **PAR2_ARGS**: om het par2-commando aan te passen, vooral als je een ander programma kiest dan de standaard par2cmdline

### Compileren
#### Afhankelijkheden:
- build-essential (C++-compiler, libstdc++, make, ...)
- qt5-default (Qt5-bibliotheken en headers)
- qt5-qmake (om de moc-bestanden te genereren en de Makefile te maken)
- libssl (v1.0.2 of v1.1), maar die zou al op je systeem moeten staan

#### Build:
- ga naar de map src
- qmake
- make

Eenvoudig! Het zou het uitvoerbare bestand **ngPost** moeten hebben gegenereerd</br>
je kunt het naar een map in je PATH kopiëren, zodat het overal bereikbaar is


Omdat het in C++/Qt is gemaakt, kun je het op elk besturingssysteem (Linux / Windows / MacOS / Android) bouwen en draaien. <br/>
Om op een ander besturingssysteem te bouwen, is het het eenvoudigst om [Qt te installeren](https://www.qt.io/download) en het project in QtCreator te laden<br/>

### Volledige documentatie
De opdrachtregelopties, de volledige lijst van configuratiesleutelwoorden en de installatiehandleidingen zijn die van het oorspronkelijke project: zie de [README van ngPost](https://github.com/mbruel/ngPost#readme). De installatiehandleidingen daar verwijzen naar de originele binaire bestanden, niet naar deze fork.

### Licentie
**ngPost** wordt gepubliceerd onder de licentie **GPL v3**
<pre>
//========================================================================
//
// Copyright (C) 2020 Matthieu Bruel <Matthieu.Bruel@gmail.com>
// This file is a part of ngPost : https://github.com/mbruel/ngPost
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3..
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>
//
//========================================================================
</pre>

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

### Steun de oorspronkelijke auteur
ngPost is gemaakt door Matthieu Bruel. Als je de app gebruikt en wilt bijdragen aan de inspanning, doe dat dan via het [oorspronkelijke project](https://github.com/mbruel/ngPost).
