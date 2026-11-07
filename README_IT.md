<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost-2026

[English](README.md) | [Français](README_FR.md) | [Deutsch](README_DE.md) | **Italiano** | [Español](README_ES.md) | [Nederlands](README_NL.md) | [Português](README_PT.md) | [中文](README_ZH.md)

**ngPost-2026** è una versione rivista e ridisegnata del famoso uploader Usenet **ngPost**.<br/>
È un fork di [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, licenza GPL v3). Il merito del programma originale va al suo autore, Matthieu Bruel.<br/>

### Cosa cambia in questo fork
-   **Correzione di sicurezza**: possibile overflow dell'heap nel buffer di codifica yEnc (con certi dati l'output poteva essere più grande del buffer)
-   **Password più sicure**: le password e i nomi degli archivi generati usano il generatore casuale crittografico del sistema operativo (Qt >= 5.10) invece di `std::rand()`
-   **NZB_POST_CMD**: i segnaposto possono contenere spazi (un percorso come `C:\Le mie cartelle\file.nzb` resta UN SOLO argomento) e il comando viene avviato in background (ngPost non lo aspetta più, quindi il suo codice di uscita non viene più registrato)
-   **RAR_EXTRA e PAR2_ARGS**: analizzati rispettando le virgolette. 7-Zip viene riconosciuto dal nome del programma e non dall'intero percorso
-   **ParPar** (un generatore di par2 molto più veloce) è incluso per Windows a 64 bit (v0.4.6, pubblico dominio) e usato automaticamente quando `parpar.exe` si trova nella cartella di ngPost. par2cmdline a 64 bit è stato tolto dal pacchetto, quello a 32 bit resta per Windows a 32 bit
-   **Interfaccia in italiano**: la GUI/CMD è ora tradotta anche in italiano (era già disponibile in cinese, olandese, inglese, francese, tedesco, portoghese e spagnolo)
-   **Nuovo README**: riscritto per questo fork e disponibile in 8 lingue (inglese, francese, tedesco, italiano, spagnolo, olandese, portoghese, cinese)

Questo fork non pubblica ancora binari: compilalo dal codice sorgente (vedi sotto).<br/>
**Utenti Windows**: per il momento non esiste un installer pronto all'uso per questo fork, il programma va compilato (ad esempio con Qt Creator). Gli installer del progetto originale non contengono le modifiche qui sopra e usano ancora il vecchio par2cmdline.<br/>

### Funzioni principali
-   **Poster Usenet da riga di comando e con una bella interfaccia grafica** per file binari, sviluppato in **C++11/Qt5**
-   è progettato per essere **il più veloce possibile** e offrire TUTTE le funzioni principali **per postare dati in modo semplice e sicuro**.
-   può **comprimere** (con il tuo programma rar esterno) e **generare i par2** prima di postare!
-   ha una **coda di invio** per preparare più post (soprattutto nell'interfaccia grafica con le schede).
-   **prepara il post successivo mentre carica quello attuale** (compressione + generazione dei par2)
-   può **automatizzare i post** **scansionando cartelle** e postando ogni file/cartella singolarmente dopo averli compressi (con un eventuale nome d'archivio e password casuali) e aver generato i par2!
-   può **monitorare cartelle e postare ogni nuovo file/cartella** singolarmente dopo averli compressi.
-   può **eliminare automaticamente file/cartelle dopo il post** (solo con il monitoraggio nell'interfaccia grafica e con --auto e --monitor insieme da riga di comando)
-   può **eseguire un COMANDO o uno script dopo ogni post** (vedi NZB_POST_CMD nel file di configurazione)
-   può **spegnere il computer** quando tutti i post sono finiti
-   è **tradotto in cinese, olandese, inglese, francese, tedesco, italiano, portoghese e spagnolo**.

### Parole chiave di configurazione modificate in questo fork
Il file di configurazione funziona come nel progetto originale (predefinito: **~/.ngPost** su Linux e Mac, **ngPost.conf** nella cartella di installazione su Windows). Qui sono elencate solo le parole chiave il cui comportamento è cambiato:

- **NZB_POST_CMD**: comando eseguito dopo ogni post. I segnaposto possono contenere spazi e il comando viene avviato in background (il suo codice di uscita non viene registrato)
- **RAR_EXTRA**: per personalizzare il comando rar (non serve indicare 'a', '-idp' o '-r'). Non serve per 7-zip, tranne se vuoi cambiare il livello di compressione.
- **PAR2_CMD**: per cambiare il generatore di par2 e poter usare [Parpar](https://github.com/animetosho/ParPar) o [Multipar](http://hp.vector.co.jp/authors/VA021385/). (par2cmdline è il generatore incluso di default, ParPar viene usato automaticamente su Windows a 64 bit se `parpar.exe` è nella cartella di ngPost)
- **PAR2_ARGS**: per personalizzare il comando par2, soprattutto se scegli un programma diverso dal par2cmdline predefinito

### Come compilare
#### Dipendenze:
- build-essential (compilatore C++, libstdc++, make, ...)
- qt5-default (librerie e header di Qt5)
- qt5-qmake (per generare i file moc e creare il Makefile)
- libssl (v1.0.2 o v1.1), ma dovrebbe essere già installata sul tuo sistema

#### Compilazione:
- vai nella cartella src
- qmake
- make

Facile! Dovrebbe aver generato l'eseguibile **ngPost**</br>
puoi copiarlo in una cartella del tuo PATH così sarà accessibile da ovunque


Essendo fatto in C++/Qt, puoi compilarlo ed eseguirlo su qualsiasi sistema operativo (Linux / Windows / MacOS / Android). <br/>
Per compilare su un altro sistema, la cosa più semplice è [installare Qt](https://www.qt.io/download) e caricare il progetto in QtCreator<br/>

### Documentazione completa
Le opzioni da riga di comando, l'elenco completo delle parole chiave di configurazione e le guide di installazione sono quelle del progetto originale: vedi il [README di ngPost](https://github.com/mbruel/ngPost#readme). Le sue guide di installazione si riferiscono ai binari originali, non a questo fork.

### Licenza
**ngPost** è pubblicato con licenza **GPL v3**
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

### Domande / Problemi / Richieste
- per domande su questo fork puoi contattare il suo manutentore su GitHub: [thenormal](https://github.com/thenormal)
- per il programma originale fai riferimento al [progetto originale](https://github.com/mbruel/ngPost) e al suo autore Matthieu Bruel

### Lingue supportate
Per ora ngPost è tradotto in cinese, olandese, inglese, francese, tedesco, italiano, portoghese e spagnolo (la traduzione italiana è stata aggiunta in questo fork).<br/>
Se vuoi tradurre ngPost nella tua lingua è facile (c'è una comoda interfaccia: QtLinguist). I file di traduzione sono nella cartella **src/lang**.

### Ringraziamenti
- Matthieu Bruel (mbruel) per aver creato ngPost e averlo pubblicato con licenza GPL v3
- Uukrull per i suoi test intensivi e i suggerimenti e per aver costruito tutti i pacchetti MacOS.
- awsms per i suoi test su un vero server con connessione da 10 Gb/s che hanno aiutato a migliorare la velocità di upload di ngPost e il supporto al multi-threading
- animetosho per aver sviluppato ParPar, il generatore di par2 più veloce di sempre!
- demanuel per lo sviluppo di NewsUP, che è stato il mio primo poster
- noobcoder1983, tensai e poi yuppie per la traduzione tedesca
- tiriclote per la traduzione spagnola
- hunesco per la traduzione portoghese
- Peng per la traduzione cinese
- tutti gli utenti di ngPost ;)

### Sostenere l'autore originale
ngPost è stato creato da Matthieu Bruel. Se usi l'app e vuoi contribuire allo sforzo, fallo sul [progetto originale](https://github.com/mbruel/ngPost).
