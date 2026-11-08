<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost Deluxe

[English](README.md) | [Français](README_FR.md) | **Deutsch** | [Italiano](README_IT.md) | [Español](README_ES.md) | [Nederlands](README_NL.md) | [Português](README_PT.md) | [中文](README_ZH.md)

**Ein schneller, kostenloser und moderner Usenet-Poster für Windows, Linux und Docker.**<br/>
Komprimieren, par2 hinzufügen, verschleiern und Dateien posten: über eine übersichtliche Oberfläche (helles und dunkles Design) oder über die Kommandozeile.

[![Latest release](https://img.shields.io/github/v/release/thenormal/ngPost-Deluxe)](https://github.com/thenormal/ngPost-Deluxe/releases/latest)
[![Build](https://github.com/thenormal/ngPost-Deluxe/actions/workflows/build-windows.yml/badge.svg)](https://github.com/thenormal/ngPost-Deluxe/actions)
[![Licence: GPL v3](https://img.shields.io/badge/licence-GPL%20v3-blue)](LICENSE)

[![Download](https://img.shields.io/badge/Download-latest%20release-2ea44f?style=for-the-badge)](https://github.com/thenormal/ngPost-Deluxe/releases/latest)

**ngPost Deluxe** ist eine überarbeitete und neu gestaltete Version des bekannten Usenet-Uploaders **ngPost**.<br/>
Es ist ein Fork von [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, Lizenz GPL v3). Das Verdienst für das ursprüngliche Programm gebührt seinem Autor, Matthieu Bruel.<br/>

### Erste Schritte
| Windows-Installer | Windows portabel | Docker / Unraid |
|---|---|---|
| Lade `ngPostDeluxe-v…-win64-setup.exe` (oder `win32`) von der Seite [Releases](https://github.com/thenormal/ngPost-Deluxe/releases/latest) herunter. 7-Zip ist enthalten. | Entpacke `ngPostDeluxe-v…-win64.zip` (oder `win32`) an einen beliebigen Ort und starte `ngPostDeluxe.exe`. Die Einstellungen stehen in der `ngPost.conf` neben dem Programm. | Image `ghcr.io/thenormal/ngpost-deluxe` mit einer Vorlage für Unraid. Siehe [docker/README.md](docker/README.md). |

> Windows zeigt eventuell „Der Computer wurde durch Windows geschützt“, weil der Installer nicht digital signiert ist: Klicke auf **Weitere Informationen**, dann auf **Trotzdem ausführen**.

### Schnellstart
1. Installiere ngPost Deluxe und starte es.
2. Öffne **Servers**, füge deinen Usenet-Server hinzu und drücke **Test connection**.
3. Öffne **Quick Post** und ziehe deine Dateien oder Ordner ins Fenster.
4. Drücke **Post Files**. Die NZB-Datei wird in den in den Einstellungen gewählten Ordner geschrieben, und jeder Upload erscheint in **History**.

Du benutzt schon ngPost? Kopiere deine `ngPost.conf` nach `%APPDATA%\ngPost Deluxe`, dann sind deine Server und Einstellungen da.

### Screenshots
| Quick Post (helles Design) | Verlauf |
|---|---|
| ![Quick Post (helles Design)](docs/screenshots/quick-post.png) | ![Verlauf](docs/screenshots/history.png) |
| **Einstellungen (dunkles Design)** | **Server (dunkles Design)** |
| ![Einstellungen (dunkles Design)](docs/screenshots/settings-dark.png) | ![Server (dunkles Design)](docs/screenshots/servers-dark.png) |

### Was in diesem Fork anders ist

**Oberfläche**
-   **Neue Oberfläche**: Seitenleiste (Quick Post, Auto Posting, Server, Einstellungen, Verlauf), Seiten mit Karten, erweiterte Optionen ausgeblendet, bis sie gebraucht werden, ein neues Symbol und ein leerer Bildschirm, der erklärt, was zu tun ist. **Helles und dunkles Design** oder automatisch wie Windows (Schlüssel `theme`). Setze die Umgebungsvariable `NGPOST_NO_THEME`, um das native Aussehen zurückzubekommen
-   **Italienische Oberfläche**: GUI/CMD sind jetzt auch auf Italienisch übersetzt (bereits verfügbar auf Chinesisch, Niederländisch, Englisch, Französisch, Deutsch, Portugiesisch und Spanisch)
-   **ngPost Deluxe v1.0**: neuer Name und neue Version (das Windows-Programm heißt jetzt `ngPostDeluxe.exe`; die Konfigurationsdatei bleibt gleich), neu gestaltetes Über-Fenster, die ursprünglichen Spenden-Schaltflächen wurden durch eine **Ko-fi**-Schaltfläche für diesen Fork ersetzt (im Über-Fenster und in den Einstellungen)
-   **Neuer Server-Bereich**: Liste der Server mit Statuspunkt und Ein/Aus-Kästchen, Detailformular (Passwort auf Wunsch sichtbar) und eine Schaltfläche **Verbindung testen**, die die Anmeldung prüft. Das Ändern der Server während eines Uploads kann ngPost nicht mehr abstürzen lassen
-   **Tab Auto Posting**: neue Schaltflächen **Abgeschlossene entfernen**, **Alle entfernen** und **Beendete Tabs schließen**
-   **Seite Verlauf**: jeder Upload wird gespeichert (`ngPost_history.csv` neben der Konfiguration oder `POST_HISTORY`) mit Status, Anzahl der Dateien und Dauer. Die Seite listet sie mit Suche, Zeitraumfilter, Summen, ausgeblendeten Passwörtern, Kopieren, Export und dem Ordner der NZB auf. Achtung: die Datei enthält die Archivpasswörter im Klartext
-   **Quick Post**: ein Hinweis erinnert daran, dass alle Einträge der Liste als ein einziges Release gepostet werden (eine NZB, ein Archiv)

**Geschwindigkeit**
-   **ParPar** (ein viel schnellerer par2-Generator) wird für Windows 64bit mitgeliefert (v0.4.6, gemeinfrei) und automatisch verwendet, wenn sich `parpar.exe` im ngPost-Ordner befindet. Das 64bit-par2cmdline wurde aus dem Paket entfernt, das 32bit-Programm bleibt für Windows 32bit erhalten
-   **Schnellere Komprimierung unter Windows**: die Ausgabe von 7-Zip / rar wird jetzt laufend gelesen (ein Post mit 115 MB wartete vorher 200 s, jetzt 1 s)

**Windows-Pakete und Docker**
-   **ngPost Deluxe v1.1**: **Docker-Image** der Kommandozeilenversion (überwacht einen Ordner und postet alles, was dort ankommt, mit einer Vorlage für **Unraid**), die Schaltfläche Post ist in Quick Post und Auto Posting zentriert, Tab-Symbole und beendete Tabs sind im dunklen Design lesbar
-   **Windows-Installer** (64 Bit und 32 Bit, **7-Zip inklusive**) und portable ZIP-Pakete, automatisch auf GitHub erstellt und in den Releases veröffentlicht (`ngPostDeluxe.exe`)
-   **Windows**: das Konsolenfenster schließt sich von selbst, wenn ngPost per Doppelklick gestartet wird; das Paket enthält OpenSSL 1.1.1w (neueste von Qt 5 unterstützte Version)

**Sicherheit und Korrekturen**
-   **Sicherheitskorrektur**: möglicher Heap-Überlauf im yEnc-Kodierungspuffer (die Ausgabe konnte bei bestimmten Daten größer als der Puffer sein)
-   **yEnc-Fix**: Ein Leerzeichen oder Tabulator am Zeilenende wird jetzt korrekt maskiert (manche Server entfernen ihn, wodurch die Daten beschädigt wurden)
-   **Sicherere Passwörter**: die erzeugten Archiv-Passwörter und -Namen verwenden den kryptografischen Zufallsgenerator des Betriebssystems (Qt >= 5.10) statt `std::rand()`
-   **Archiv-Passwort nicht mehr in der Kommandozeile**: 7-Zip erhält es über die Standardeingabe und rar über die Umgebungsvariable `RAR`, es ist also nicht mehr in der Prozessliste sichtbar

**Einstellungen und Werkzeuge**
-   **NZB_POST_CMD**: die Platzhalter dürfen Leerzeichen enthalten (ein Pfad wie `C:\Meine Ordner\datei.nzb` bleibt EIN Argument) und der Befehl wird im Hintergrund gestartet (ngPost wartet nicht mehr darauf, daher wird sein Exit-Code nicht mehr protokolliert)
-   **RAR_EXTRA und PAR2_ARGS**: werden unter Beachtung der Anführungszeichen ausgewertet. 7-Zip wird am Programmnamen erkannt und nicht am gesamten Pfad
-   **Neue README**: für diesen Fork neu geschrieben und in 8 Sprachen verfügbar (Englisch, Französisch, Deutsch, Italienisch, Spanisch, Niederländisch, Portugiesisch, Chinesisch)
-   **Automatisches Speichern**: jede Änderung der Einstellungen wird automatisch gespeichert (die vorherige Datei bleibt als `ngPost.conf.bak` erhalten)
-   **Diagnose**: ngPost zeichnet Dauer und Geschwindigkeit jeder Phase (Kompression, par2, Upload), Fehler und Hänger auf, und die Schaltfläche **Diagnosebericht** speichert einen Bericht zur Fehleranalyse. Passwörter, Benutzernamen und Dateinamen werden maskiert, nichts wird versendet

Die Windows-Installer, die portablen Pakete und das Docker-Image findest du unter [Releases](https://github.com/thenormal/ngPost-Deluxe/releases/latest). Auf anderen Systemen aus den Quellen bauen (siehe unten).<br/>

### Hauptfunktionen
-   **Usenet-Poster für die Kommandozeile und mit schicker GUI** für Binärdateien, entwickelt in **C++11/Qt5**
-   er ist darauf ausgelegt, **so schnell wie möglich** zu sein und ALLE wichtigen Funktionen zu bieten, **um Daten einfach und sicher zu posten**.
-   er kann **komprimieren** (mit deinem externen rar-Programm) und vor dem Posten **die par2-Dateien erzeugen**!
-   er hat eine **Warteschlange**, um mehrere Posts vorzubereiten (besonders in der GUI mit Tabs).
-   er **packt den nächsten Post, während der aktuelle hochgeladen wird** (Komprimierung + par2-Erzeugung)
-   er kann Posts **automatisieren**, indem er **Ordner scannt** und jede Datei/jeden Ordner einzeln postet, nachdem sie komprimiert (mit eventuell zufälligem Archivnamen und Passwort) und die par2 erzeugt wurden!
-   er kann **Ordner überwachen und jede neue Datei/jeden neuen Ordner** einzeln posten, nachdem sie komprimiert wurden.
-   er kann **Dateien/Ordner nach dem Posten automatisch löschen** (nur beim Überwachen in der GUI und mit --auto und --monitor in der Kommandozeile)
-   er kann nach jedem Post **einen BEFEHL oder ein Skript ausführen** (siehe NZB_POST_CMD in der Konfigurationsdatei)
-   er kann **den Computer ausschalten**, wenn alle Posts fertig sind
-   er ist **übersetzt in Chinesisch, Niederländisch, Englisch, Französisch, Deutsch, Italienisch, Portugiesisch und Spanisch**.

### In diesem Fork geänderte Konfigurations-Schlüsselwörter
Die Konfigurationsdatei funktioniert wie im Originalprojekt (Standard: **~/.ngPost** unter Linux und Mac, **ngPost.conf** im Installationsordner unter Windows). Hier sind nur die Schlüsselwörter aufgeführt, deren Verhalten sich geändert hat:

- **NZB_POST_CMD**: Befehl, der nach jedem Post ausgeführt wird. Die Platzhalter dürfen Leerzeichen enthalten und der Befehl läuft im Hintergrund (sein Exitcode wird nicht protokolliert)
- **RAR_EXTRA**: um den rar-Befehl anzupassen (das 'a', '-idp' oder '-r' musst du nicht angeben). Für 7-zip nicht nötig, außer du willst die Kompressionsstufe ändern.
- **PAR2_PATH**: um den par2-Generator zu ändern und [Parpar](https://github.com/animetosho/ParPar) oder [Multipar](http://hp.vector.co.jp/authors/VA021385/) zu verwenden. (par2cmdline ist der mitgelieferte Standardgenerator, ParPar wird unter Windows 64bit automatisch verwendet, wenn `parpar.exe` im ngPost-Ordner liegt)
- **PAR2_ARGS**: um den par2-Befehl anzupassen, besonders wenn du ein anderes Programm als das Standard-par2cmdline wählst
- **theme**: Design der Oberfläche, `AUTO` (wie Windows), `LIGHT` oder `DARK`
- **history**: `ON` (Standard) oder `OFF`, Verlauf der Uploads auf der Seite Verlauf. Ohne **POST_HISTORY** ist die Datei `ngPost_history.csv` neben der Konfiguration. Am Ende jeder Zeile kommen neue Spalten hinzu: Status, Anzahl der Dateien, Dauer und NZB-Datei

### Kompilieren
#### Linux
Abhängigkeiten (Ubuntu / Debian):
```
sudo apt install build-essential qtbase5-dev qt5-qmake libssl-dev
```
Die grafische Version bauen, oder nur die Kommandozeilenversion (die Qt-GUI-Bibliotheken werden nicht benötigt):
```
# GUI
cd src && qmake ngPost.pro && make -j$(nproc)

# command line only, in its own folder
mkdir build-cmd && cd build-cmd && qmake ../src/ngPost_cmd.pro && make -j$(nproc)
```
Die ausführbare Datei **ngPost** liegt danach im Ordner, in dem du gebaut hast: kopiere sie in einen Ordner deines PATH.

#### Windows
Installiere [Qt 5.15](https://www.qt.io/download) mit MinGW und öffne `src/ngPost.pro` in Qt Creator, oder in einer Qt-Eingabeaufforderung: `mkdir build && cd build && qmake ../src/ngPost.pro CONFIG+=release && mingw32-make`. Das Programm heißt `ngPostDeluxe.exe`. Der Installer wird mit [Inno Setup](https://jrsoftware.org/isinfo.php) erstellt (`installer/ngPostDeluxe.iss`); alle Schritte der offiziellen Pakete stehen in `.github/workflows/build-windows.yml`.

#### Docker
Kommandozeilenversion in einem Container (siehe [docker/README.md](docker/README.md)):
```
docker build -t ngpost-deluxe .
```

#### Andere Systeme
Da es in C++/Qt geschrieben ist, kann es auf jedem System (macOS, Android...) gebaut und ausgeführt werden: am einfachsten [installierst du Qt](https://www.qt.io/download) und lädst `src/ngPost.pro` in Qt Creator.

### Vollständige Dokumentation
Die Kommandozeilenoptionen, die vollständige Liste der Konfigurations-Schlüsselwörter und die Installationsanleitungen entsprechen dem Originalprojekt: siehe die [README von ngPost](https://github.com/mbruel/ngPost#readme). Die dortigen Installationsanleitungen beziehen sich auf die Original-Binärdateien, nicht auf diesen Fork.

### Lizenz
**ngPost** wird unter der Lizenz **GPL v3** veröffentlicht
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

### Fragen / Probleme / Wünsche
- bei Fragen zu diesem Fork kannst du dich auf GitHub an den Betreuer wenden: [thenormal](https://github.com/thenormal)
- für das originale Programm wende dich bitte an das [Originalprojekt](https://github.com/mbruel/ngPost) und seinen Autor Matthieu Bruel

### Unterstützte Sprachen
Derzeit ist ngPost in Chinesisch, Niederländisch, Englisch, Französisch, Deutsch, Italienisch, Portugiesisch und Spanisch übersetzt (die italienische Übersetzung stammt aus diesem Fork).<br/>
Wenn du ngPost in deine Sprache übersetzen möchtest, ist das einfach (es gibt dafür eine praktische GUI: QtLinguist). Die Übersetzungsdateien liegen im Ordner **src/lang**.

### Danksagung
- Matthieu Bruel (mbruel), der ngPost geschaffen und unter GPL v3 veröffentlicht hat
- Uukrull für seine intensiven Tests und Rückmeldungen und dafür, dass er alle MacOS-Pakete gebaut hat.
- awsms für seine Tests auf einem richtigen Server mit 10-Gb/s-Anbindung, die geholfen haben, ngPosts Upload-Geschwindigkeit und die Multi-Threading-Unterstützung zu verbessern
- animetosho für die Entwicklung von ParPar, dem schnellsten par2-Generator überhaupt!
- demanuel für die Entwicklung von NewsUP, das mein erster Poster war
- noobcoder1983, tensai und dann yuppie für die deutsche Übersetzung
- tiriclote für die spanische Übersetzung
- hunesco für die portugiesische Übersetzung
- Peng für die chinesische Übersetzung
- alle ngPost-Nutzer ;)

### Diesen Fork unterstützen
ngPost Deluxe ist kostenlos. Wenn es dir nützt, kannst du die Entwicklung auf Ko-fi unterstützen:<br/>
[![Ko-fi](https://img.shields.io/badge/Ko--fi-support%20ngPost%20Deluxe-FF5E5B?logo=ko-fi&logoColor=white)](https://ko-fi.com/thenormal)

### Den ursprünglichen Autor unterstützen
ngPost wurde von Matthieu Bruel geschaffen. Wenn du die App nutzt und zur Weiterentwicklung beitragen möchtest, tu das bitte beim [Originalprojekt](https://github.com/mbruel/ngPost).
