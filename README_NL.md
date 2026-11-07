<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost-2026

[English](README.md) | [Français](README_FR.md) | [Deutsch](README_DE.md) | [Italiano](README_IT.md) | [Español](README_ES.md) | **Nederlands** | [Português](README_PT.md) | [中文](README_ZH.md)

**ngPost-2026** is een herziene en opnieuw ontworpen versie van de bekende Usenet-uploader **ngPost**.<br/>
Het is een fork van [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, licentie GPL v3). Alle eer voor het oorspronkelijke programma gaat naar de auteur, Matthieu Bruel.<br/>

### Wat is er anders in deze fork
-   **Beveiligingsfix**: mogelijke heap-overflow in de yEnc-codeerbuffer (bij bepaalde gegevens kon de uitvoer groter zijn dan de buffer)
-   **Veiligere wachtwoorden**: de gegenereerde archiefwachtwoorden en -namen gebruiken de cryptografische willekeurige generator van het besturingssysteem (Qt >= 5.10) in plaats van `std::rand()`
-   **NZB_POST_CMD**: de plaatshouders mogen spaties bevatten (een pad zoals `C:\Mijn mappen\bestand.nzb` blijft ÉÉN argument) en het commando wordt op de achtergrond gestart (ngPost wacht er niet meer op, dus de exitcode wordt niet meer gelogd)
-   **RAR_EXTRA en PAR2_ARGS**: worden verwerkt met respect voor dubbele aanhalingstekens. 7-Zip wordt herkend aan de programmanaam en niet aan het hele pad
-   **ParPar** (een veel snellere par2-generator) wordt meegeleverd voor Windows 64bit (v0.4.6, publiek domein) en automatisch gebruikt als `parpar.exe` in de ngPost-map staat. De 64bit par2cmdline is uit het pakket verwijderd, de 32bit-versie blijft voor Windows 32bit

Deze fork publiceert nog geen binaire bestanden: compileer hem vanuit de broncode (zie hieronder). De downloadlinks in de installatiesecties zijn die van het originele ngPost v4.16 en bevatten de bovenstaande wijzigingen niet.<br/>
<br/>

**Usenet-poster voor de opdrachtregel en met een mooie GUI** voor binaire bestanden, ontwikkeld in **C++11/Qt5**<br/>
hij is ontworpen om **zo snel mogelijk** te zijn en ALLE belangrijke functies te bieden **om gegevens eenvoudig en veilig te posten**.<br/>
hij kan **comprimeren** (met je externe rar-programma) en vóór het posten **de par2-bestanden genereren**!<br/>
hij heeft een **wachtrij** om meerdere posts voor te bereiden (vooral in de GUI met tabbladen).<br/>
hij **pakt de volgende post in terwijl de huidige wordt geüpload** (compressie + par2-generatie)<br/>
hij kan posts **automatiseren** door **mappen te scannen** en elk bestand/elke map afzonderlijk te posten nadat ze zijn gecomprimeerd (eventueel met willekeurige archiefnaam en wachtwoord) en de par2 is gegenereerd!<br/>
hij kan **mappen bewaken en elk nieuw bestand/elke nieuwe map** afzonderlijk posten nadat ze zijn gecomprimeerd.<br/>
hij kan **bestanden/mappen automatisch verwijderen na het posten** (alleen bij bewaking in de GUI en met --auto en --monitor samen in de opdrachtregel)<br/>
hij kan na elke post **een COMMANDO of script uitvoeren** (zie NZB_POST_CMD in het configuratiebestand)<br/>
hij kan **de computer uitschakelen** als alle posts klaar zijn<br/>
hij is **vertaald in het Chinees, Nederlands, Engels, Frans, Duits, Italiaans, Portugees en Spaans**.<br/>


![ngPost_v4.3](pics/ngPost_v4.3.png)

[Releases van het originele ngPost zijn beschikbaar](https://github.com/mbruel/ngPost/releases/tag/v4.16) voor: Linux 64bit, Windows (32bit en 64bit), MacOS en Raspbian (RPI 4). Binnenkort ook voor Android en daarna iOS...

Dit zijn de belangrijkste functies en voordelen van ngPost:

-   **volledige verhulling van de Article-header**: het onderwerp is een UUID (zoals de msg-id) en er wordt een willekeurige afzender gebruikt. **Let op**: hierdoor vind je je post niet meer terug op Usenet (of in indexers) als je het NZB-bestand kwijtraakt. Maar deze methode is **volledig veilig**, **je hoeft je bestanden niet te verhullen of een wachtwoord te gebruiken**.
-   **Wachtrij**: je kunt meerdere posts voorbereiden terwijl je iets post. Wachtende posts kun je uiteraard annuleren ;)
-   **Post-automatisering**: scan een map en post elk bestand/elke map na compressie. (zie de optie **--auto in de opdrachtregel** en het tabblad "Auto Posting" in de GUI)
-   **comprimeren met RAR of 7zip** (extern commando) met willekeurige **naamverhulling** en wachtwoord, en **par2 genereren** vóór het posten
-   **geschiedenis van posts in een csv-bestand**: zo heb je de datum, **bestandsnaam**, grootte, uploadsnelheid en vooral **archiefnaam en wachtwoord** in een eenvoudige Excel-sheet ;)
-   **ParPar** (veel sneller dan par2cmdline) wordt op Windows 64bit automatisch gebruikt als `parpar.exe` in de ngPost-map staat (de Windows 64bit-versie staat in `release/libs`). **par2cmdline** wordt nog steeds meegeleverd voor Windows 32bit, en je kunt ook een ander programma (zoals Multipar) gebruiken met de sleutelwoorden PAR2_CMD en PAR2_ARGS in het configuratiebestand
-   ondersteunt **meerdere bestanden** en **meerdere mappen**
-   ondersteunt **meerdere servers** (via het configuratiebestand of de GUI), elk met **meerdere verbindingen** (met SSL)
-   verdeelt die verbindingen over **meerdere threads**. Standaard het aantal kernen van de computer, maar je kunt het aantal instellen.
-   **bereidt de Articles voor in de hoofdthread** (yEnc-codering) zodat de uploadthreads **altijd klaar zijn om te schrijven**. (elke verbinding heeft 2 Articles vooraf klaar)
-   gebruikt **asynchrone sockets** in de uploadthreads
-   **beperkt de schijftoegang** tot het minimum: de te posten bestanden worden maar één keer geopend in de hoofdthread en één voor één verwerkt (je hoeft niet meerdere bestanden tegelijk te openen, de Articles worden over alle verbindingen verdeeld)
-   genereert een **willekeurige afzender** (from), maar je kunt ook een vaste gebruiken
-   **genereert het nzb-bestand**
-   **verwerkt onderbrekingen**: als een post wordt gestopt, **wordt de nzb gemaakt met de bestanden die al zijn gepost** en worden **de niet-geposte bestanden vermeld**, zodat je alleen die opnieuw kunt posten en de nzb-bestanden daarna handmatig kunt samenvoegen. (in de opdrachtregel zou de applicatie netjes afsluiten)
-   **meta-gegevens toevoegen in de header van de nzb** (meestal een wachtwoord)
-   Articles bij een fout opnieuw posten **met een andere UUID**
-   opnieuw verbinden bij een fout op een socket (zelfde Retry-parameter als voor de Articles)
-   ...


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
Er zijn tot nu toe alleen releases gemaakt voor Linux x64, Windows x64 (vanaf Windows 7) en MacOS</br>
Om op een ander besturingssysteem te bouwen, is het het eenvoudigst om [Qt te installeren](https://www.qt.io/download) en het project in QtCreator te laden<br/>

### Gebruik in de opdrachtregel (de optiebeschrijvingen zijn de Engelse uitvoer van het programma)
<pre>
Syntax: ngPost (options)* (-i <file or folder> | --auto <folder> | --monitor <folder>)+
	--help             : Help: display syntax
	-v or --version    : app version
	-c or --conf       : use configuration file (if not provided, we try to load $HOME/.ngPost)
	--disp_progress    : display cmd progressbar: NONE (default), BAR or FILES
	-d or --debug      : display extra information
	--fulldebug        : display full debug information
	-l or --lang       : application language
	--check            : check nzb file (if articles are available on Usenet) cf https://github.com/mbruel/nzbCheck
	-q or --quiet      : quiet mode (no output on stdout)

// automated posting (scanning and/or monitoring)
	--auto             : parse directory and post every file/folder separately. You must use --compress, should add --gen_par2, --gen_name and --gen_pass
	--monitor          : monitor directory and post every new file/folder. You must use --compress, should add --gen_par2, --gen_name and --gen_pass
	--rm_posted        : delete file/folder once posted. You must use --auto or --monitor with this option.

// quick posting (several files/folders)
	-i or --input      : input file to upload (single file or directory), you can use it multiple times
	-o or --output     : output file path (nzb)
	-x or --obfuscate  : obfuscate the subjects of the articles (CAREFUL you won't find your post if you lose the nzb file)
	-g or --groups     : newsgroups where to post the files (coma separated without space)
	-m or --meta       : extra meta data in header (typically "password=qwerty42")
	-f or --from       : poster email (random one if not provided)
	-a or --article_size: article size (default one: 716800)
	-z or --msg_id     : msg id signature, after the @ (default one: ngPost)
	-r or --retry      : number of time we retry to an Article that failed (default: 5)
	-t or --thread     : number of Threads (the connections will be distributed amongs them)
	--gen_from         : generate a new random email for each Post (--auto or --monitor)

// for compression and par2 support
	--tmp_dir          : temporary folder where the compressed files and par2 will be stored
	--rar_path         : RAR absolute file path (external application)
	--rar_size         : size in MB of the RAR volumes (0 by default meaning NO split)
	--rar_max          : maximum number of archive volumes
	--par2_pct         : par2 redundancy percentage (0 by default meaning NO par2 generation)
	--par2_path        : par2 absolute file path (in case of self compilation of ngPost)
	--auto_compress    : compress inputs with random name and password and generate par2 (equivalent of --compress --gen_name --gen_pass --gen_par2)
	--compress         : compress inputs using RAR or 7z
	--gen_par2         : generate par2 (to be used with --compress)
	--rar_name         : provide the RAR file name (to be used with --compress)
	--rar_pass         : provide the RAR password (to be used with --compress)
	--gen_name         : generate random RAR name (to be used with --compress)
	--gen_pass         : generate random RAR password (to be used with --compress)
	--length_name      : length of the random RAR name (to be used with --gen_name), default: 17
	--length_pass      : length of the random RAR password (to be used with --gen_pass), default: 13
	--rar_no_root_folder: Remove root (parent) folder when compressing Folders using RAR

// you can provide servers in one string using -S and/or split the parameters for ONE SINGLE server (this will overwrite the configuration file)
	-S or --server     : NNTP server following the format (&lt;user&gt;:&lt;pass&gt;@@@)?&lt;host&gt;:&lt;port&gt;:&lt;nbCons&gt;:(no)?ssl
	-h or --host       : NNTP server hostname (or IP)
	-P or --port       : NNTP server port
	-s or --ssl        : use SSL
	-u or --user       : NNTP server username
	-p or --pass       : NNTP server password
	-n or --connection : number of NNTP connections

Examples:
  - with monitoring: ngPost_v4.16_cmd-x86_64.AppImage --monitor /data/folder1 --monitor /data/folder2 --auto_compress --rm_posted --disp_progress files
  - with auto post: ngPost_v4.16_cmd-x86_64.AppImage --auto /data/folder1 --auto /data/folder2 --compress --gen_par2 --gen_name --gen_pass --rar_size 42 --disp_progress files
  - with compression, filename obfuscation, random password and par2: ngPost_v4.16_cmd-x86_64.AppImage -i /tmp/file1 -i /tmp/folder1 -o /nzb/myPost.nzb --compress --gen_name --gen_pass --gen_par2
  - with config file: ngPost_v4.16_cmd-x86_64.AppImage -c ~/.ngPost -m "password=qwerty42" -f ngPost@nowhere.com -i /tmp/file1 -i /tmp/file2 -i /tmp/folderToPost1 -i /tmp/folderToPost2
  - with all params:  ngPost_v4.16_cmd-x86_64.AppImage -t 1 -m "password=qwerty42" -m "metaKey=someValue" -h news.newshosting.com -P 443 -s -u user -p pass -n 30 -f ngPost@nowhere.com  -g "alt.binaries.test,alt.binaries.test2" -a 64000 -i /tmp/folderToPost -o /tmp/folderToPost.nzb

If you don't provide the output file (nzb file), we will create it in the nzbPath with the name of the first file or folder given in the command line.
so in the second example above, the nzb would be: /tmp/file1.nzb
</pre>

### Configuratiebestand en sleutelwoorden die alleen daar voorkomen
Het standaard configuratiebestand voor Linux en Mac is: **~/.ngPost** (zonder extensie .conf)<br/>
Als je wilt, kun je in de opdrachtregel met de optie -c een ander bestand gebruiken.<br/>
[Hier het voorbeeld om te volgen](https://github.com/mbruel/ngPost/blob/master/ngPost.conf).<br/>
<br/>
De meeste sleutelwoorden van de configuratie kunnen in de opdrachtregel worden gebruikt, maar enkele staan alleen in het configuratiebestand:
- **MONITOR_NZB_FOLDERS**: elke bewakingspost komt in een eigen map die in nzbPath wordt aangemaakt (Req/Issue #15)
- **inputDir**: standaardmap die wordt geopend om bestanden te kiezen in de GUI (kan veranderen als je de Auto-map in de GUI wijzigt en OPSLAAT)
- **POST_HISTORY**: csv-bestand waar elke geslaagde post de datum, bestandsnaam, grootte, uploadsnelheid, archiefnaam en wachtwoord aan toevoegt
- **GROUP_POLICY**: bepaalt hoe er wordt gepost als er meerdere groepen zijn opgegeven (op alle, één per post of één per bestand)
- **NZB_RM_ACCENTS**: verwijdert accenten en speciale tekens uit de namen van nzb-bestanden
- **AUTO_CLOSE_TABS**: sluit de Quick Post-tabbladen na een geslaagde post (voor de GUI)
- **RESUME_WAIT**: wachttijd (seconden) voordat bij netwerkverlies automatisch wordt geprobeerd een Post te hervatten (min: 30)
- **NO_RESUME_AUTO**: stop een post als het netwerk wegvalt
- **PREPARE_PACKING**: als er meerdere posts in de wachtrij staan, bereid het inpakken van de volgende voor terwijl de huidige wordt geüpload

De volgende zijn voor ervaren posters:
- **RAR_EXTRA**: om het rar-commando aan te passen (je hoeft de 'a', '-idp' of '-r' niet op te geven). Niet nodig voor 7-zip, behalve als je het compressieniveau wilt wijzigen.
- **PAR2_CMD**: om de par2-generator te wijzigen en [Parpar](https://github.com/animetosho/ParPar) of [Multipar](http://hp.vector.co.jp/authors/VA021385/) te kunnen gebruiken. (par2cmdline is de standaard meegeleverde generator, ParPar wordt op Windows 64bit automatisch gebruikt als `parpar.exe` in de ngPost-map staat)
- **PAR2_ARGS**: om het par2-commando aan te passen, vooral als je een ander programma kiest dan de standaard par2cmdline
- **TMP_RAM**: gebruik een tijdelijke map met groottebeperking, meestal een tmpfs-partitie, voor posts die erin passen (anders wordt TMP_DIR gebruikt)
- **TMP_RAM_RATIO**: factor op de grootte van de bronbestanden om de par2-generatie te compenseren

### Draagbare Linux 64bit-release (gecompileerd met Qt v5.12.6)
Als je niet wilt compileren en de afhankelijkheden niet wilt installeren, kun je ook de draagbare release gebruiken, die alles bevat.<br/>
- download [ngPost_v4.16-x86_64.AppImage](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16-x86_64.AppImage) of [ngPost_v4.16_cmd-x86_64.AppImage](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16_cmd-x86_64.AppImage) voor alleen opdrachtregel (de eerste versie kan beide: CMD en GUI)
- chmod 755 ngPost_v4.16-x86_64.AppImage
- start hem met dezelfde syntaxis als in de sectie hierboven
- als je het configuratiebestand wilt bewaren, bewerk dan het bestand **~/.ngPost** volgens [dit model](https://raw.githubusercontent.com/mbruel/ngPost/master/ngPost.conf) (zonder de extensie .conf)

PS: voor oudere systemen met GLIBC < 2.24 is hier een versie gecompileerd op Debian8 met GLIBC 2.19 en Qt v5.8.0: [ngPost_v4.16-x86_64_debian8.AppImage](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16-x86_64_debian8.AppImage)


### Raspbian-release (armhf voor Raspberry PI)
- download [ngPost_v4.16-armhf.AppImage](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16-armhf.AppImage)
- chmod 755 ngPost_v4.16-armhf.AppImage
- start hem met dezelfde syntaxis als in de sectie hierboven
- als je het configuratiebestand wilt bewaren, bewerk dan het bestand **~/.ngPost** volgens [dit model](https://raw.githubusercontent.com/mbruel/ngPost/master/ngPost.conf) (zonder de extensie .conf)

**Omdat rar niet beschikbaar is op de RPI, is ondersteuning voor 7zip toegevoegd.** Gebruik RAR_PATH om naar 7z te wijzen en RAR_EXTRA voor de 7zip-opties. Je zou zoiets moeten hebben:
<pre>
## RAR or 7zip absolute file path (external application)
## /!\ The file MUST EXIST and BE EXECUTABLE /!\
## this is set for Linux environment, Windows users MUST change it
#RAR_PATH = /usr/bin/rar
RAR_PATH = /usr/bin/7z

## RAR EXTRA options (the first 'a' and '-idp' will be added automatically)
## -hp will be added if you use a password with --gen_pass, --rar_pass or using the HMI
## -v42m will be added with --rar_size or using the HMI
## you could change the compression level, lock the archive, add redundancy...
#RAR_EXTRA = -ep1 -m0 -k -rr5p
RAR_EXTRA = -mx0 -mhe=on
</pre>


### Windows-installatieprogramma
- gebruik gewoon het pakket [ngPost_v4.16_x64_setup.exe](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16_x64_setup.exe) of [ngPost_v4.16_x86_setup.exe](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16_x86_setup.exe) voor de 32bit-versie
- bewerk **ngPost.conf** (in de installatiemap) om je serverinstellingen toe te voegen (je kunt er meerdere opgeven).
- start **ngPost.exe** (GUI-versie)
- of gebruik het in de opdrachtregel: **ngPost.exe** -i "je bestand of map"

als je wilt, kun je alle serverparameters in de opdrachtregel opgeven (zie de sectie hierboven)<br/>
Standaard:
- laadt ngPost het configuratiebestand 'ngPost.conf' dat in de map staat
- schrijft het ook het nzb-bestand in deze map. (de naam is die van het laatste invoerbestand in de opdrachtregel)

**Bekend probleem zonder SSL-ondersteuning:** mogelijk moet je de [msvc2015 redistributables](https://www.microsoft.com/en-us/download/details.aspx?id=48145) installeren, omdat libssl afhankelijk is van hun API's<br/>


### MacOS-release gebouwd op High Sierra (v10.13)
- download [ngPost_v4.16.1.dmg](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16.1.dmg)
- start hem met dezelfde syntaxis als in de sectie hierboven
- als je het configuratiebestand wilt bewaren, bewerk dan het bestand **~/.ngPost** volgens [dit model](https://raw.githubusercontent.com/mbruel/ngPost/master/ngPost.conf) (zonder de extensie .conf)


### Alternatieven

Een lijst van Usenet-posters van de Nyuu-GitHub [vind je hier](https://github.com/animetosho/Nyuu/wiki/Usenet-Uploaders).


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
