<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost-2026

[English](README.md) | [Français](README_FR.md) | **Deutsch** | [Italiano](README_IT.md) | [Español](README_ES.md) | [Nederlands](README_NL.md) | [Português](README_PT.md) | [中文](README_ZH.md)

**ngPost-2026** ist eine überarbeitete und neu gestaltete Version des bekannten Usenet-Uploaders **ngPost**.<br/>
Es ist ein Fork von [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, Lizenz GPL v3). Das Verdienst für das ursprüngliche Programm gebührt seinem Autor, Matthieu Bruel.<br/>

### Was in diesem Fork anders ist
-   **Sicherheitskorrektur**: möglicher Heap-Überlauf im yEnc-Kodierungspuffer (die Ausgabe konnte bei bestimmten Daten größer als der Puffer sein)
-   **yEnc-Fix**: Ein Leerzeichen oder Tabulator am Zeilenende wird jetzt korrekt maskiert (manche Server entfernen ihn, wodurch die Daten beschädigt wurden)
-   **Sicherere Passwörter**: die erzeugten Archiv-Passwörter und -Namen verwenden den kryptografischen Zufallsgenerator des Betriebssystems (Qt >= 5.10) statt `std::rand()`
-   **NZB_POST_CMD**: die Platzhalter dürfen Leerzeichen enthalten (ein Pfad wie `C:\Meine Ordner\datei.nzb` bleibt EIN Argument) und der Befehl wird im Hintergrund gestartet (ngPost wartet nicht mehr darauf, daher wird sein Exit-Code nicht mehr protokolliert)
-   **RAR_EXTRA und PAR2_ARGS**: werden unter Beachtung der Anführungszeichen ausgewertet. 7-Zip wird am Programmnamen erkannt und nicht am gesamten Pfad
-   **ParPar** (ein viel schnellerer par2-Generator) wird für Windows 64bit mitgeliefert (v0.4.6, gemeinfrei) und automatisch verwendet, wenn sich `parpar.exe` im ngPost-Ordner befindet. Das 64bit-par2cmdline wurde aus dem Paket entfernt, das 32bit-Programm bleibt für Windows 32bit erhalten
-   **Neue Oberfläche**: Seitenleiste (Quick Post, Auto Posting, Server, Einstellungen, Verlauf), Seiten mit Karten, erweiterte Optionen ausgeblendet, bis sie gebraucht werden, ein neues Symbol und ein leerer Bildschirm, der erklärt, was zu tun ist. **Helles und dunkles Design** oder automatisch wie Windows (Schlüssel `theme`). Setze die Umgebungsvariable `NGPOST_NO_THEME`, um das native Aussehen zurückzubekommen
-   **Italienische Oberfläche**: GUI/CMD sind jetzt auch auf Italienisch übersetzt (bereits verfügbar auf Chinesisch, Niederländisch, Englisch, Französisch, Deutsch, Portugiesisch und Spanisch)
-   **Neue README**: für diesen Fork neu geschrieben und in 8 Sprachen verfügbar (Englisch, Französisch, Deutsch, Italienisch, Spanisch, Niederländisch, Portugiesisch, Chinesisch)
-   **ngPost 2026 v5.0 Beta**: neuer Name und neue Version, neu gestaltetes Über-Fenster, die Spenden-Schaltflächen wurden entfernt (um den ursprünglichen Autor zu unterstützen, siehe Ende dieser Seite)
-   **Neuer Server-Bereich**: Liste der Server mit Statuspunkt und Ein/Aus-Kästchen, Detailformular (Passwort auf Wunsch sichtbar) und eine Schaltfläche **Verbindung testen**, die die Anmeldung prüft. Das Ändern der Server während eines Uploads kann ngPost nicht mehr abstürzen lassen
-   **Automatisches Speichern**: jede Änderung der Einstellungen wird automatisch gespeichert (die vorherige Datei bleibt als `ngPost.conf.bak` erhalten)
-   **Tab Auto Posting**: neue Schaltflächen **Abgeschlossene entfernen**, **Alle entfernen** und **Beendete Tabs schließen**
-   **Archiv-Passwort nicht mehr in der Kommandozeile**: 7-Zip erhält es über die Standardeingabe und rar über die Umgebungsvariable `RAR`, es ist also nicht mehr in der Prozessliste sichtbar
-   **Diagnose**: ngPost zeichnet Dauer und Geschwindigkeit jeder Phase (Kompression, par2, Upload), Fehler und Hänger auf, und die Schaltfläche **Diagnosebericht** speichert einen Bericht zur Fehleranalyse. Passwörter, Benutzernamen und Dateinamen werden maskiert, nichts wird versendet
-   **Windows**: das Konsolenfenster schließt sich von selbst, wenn ngPost per Doppelklick gestartet wird; das Paket enthält OpenSSL 1.1.1w (neueste von Qt 5 unterstützte Version)
-   **Seite Verlauf**: jeder Upload wird gespeichert (`ngPost_history.csv` neben der Konfiguration oder `POST_HISTORY`) mit Status, Anzahl der Dateien und Dauer. Die Seite listet sie mit Suche, Zeitraumfilter, Summen, ausgeblendeten Passwörtern, Kopieren, Export und dem Ordner der NZB auf. Achtung: die Datei enthält die Archivpasswörter im Klartext
-   **Quick Post**: ein Hinweis erinnert daran, dass alle Einträge der Liste als ein einziges Release gepostet werden (eine NZB, ein Archiv)

Es gibt noch keine offiziellen Releases dieses Forks. Ein sofort lauffähiges **Windows 64bit**-Paket wird bei jeder Änderung automatisch von GitHub Actions erstellt (Tab *Actions* > Lauf *Build Windows (x64)* > *Artifacts*, ein GitHub-Konto ist nötig). Auf anderen Systemen aus den Quellen bauen (siehe unten).<br/>

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
- **PAR2_CMD**: um den par2-Generator zu ändern und [Parpar](https://github.com/animetosho/ParPar) oder [Multipar](http://hp.vector.co.jp/authors/VA021385/) zu verwenden. (par2cmdline ist der mitgelieferte Standardgenerator, ParPar wird unter Windows 64bit automatisch verwendet, wenn `parpar.exe` im ngPost-Ordner liegt)
- **PAR2_ARGS**: um den par2-Befehl anzupassen, besonders wenn du ein anderes Programm als das Standard-par2cmdline wählst
- **theme**: Design der Oberfläche, `AUTO` (wie Windows), `LIGHT` oder `DARK`
- **history**: `ON` (Standard) oder `OFF`, Verlauf der Uploads auf der Seite Verlauf. Ohne **POST_HISTORY** ist die Datei `ngPost_history.csv` neben der Konfiguration. Am Ende jeder Zeile kommen neue Spalten hinzu: Status, Anzahl der Dateien, Dauer und NZB-Datei

### Kompilieren
#### Abhängigkeiten:
- build-essential (C++-Compiler, libstdc++, make, ...)
- qt5-default (Qt5-Bibliotheken und Header)
- qt5-qmake (um die moc-Dateien und das Makefile zu erzeugen)
- libssl (v1.0.2 oder v1.1), sollte aber bereits auf deinem System installiert sein

#### Build:
- in den Ordner src wechseln
- qmake
- make

Einfach! Es sollte die ausführbare Datei **ngPost** erzeugt haben</br>
du kannst sie in einen Ordner deines PATH kopieren, dann ist sie überall verfügbar


Da es in C++/Qt geschrieben ist, kannst du es auf jedem Betriebssystem (Linux / Windows / MacOS / Android) kompilieren und ausführen. <br/>
Um auf einem anderen Betriebssystem zu kompilieren, ist es am einfachsten, [Qt zu installieren](https://www.qt.io/download) und das Projekt in QtCreator zu laden<br/>

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

### Den ursprünglichen Autor unterstützen
ngPost wurde von Matthieu Bruel geschaffen. Wenn du die App nutzt und zur Weiterentwicklung beitragen möchtest, tu das bitte beim [Originalprojekt](https://github.com/mbruel/ngPost).
