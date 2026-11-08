<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost Deluxe

[English](README.md) | [Français](README_FR.md) | [Deutsch](README_DE.md) | **Italiano** | [Español](README_ES.md) | [Nederlands](README_NL.md) | [Português](README_PT.md) | [中文](README_ZH.md)

**ngPost Deluxe** è una versione rivista e ridisegnata del famoso uploader Usenet **ngPost**.<br/>
È un fork di [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, licenza GPL v3). Il merito del programma originale va al suo autore, Matthieu Bruel.<br/>

### Download
**Installer**: scarica `ngPostDeluxe-v1.1-win64-setup.exe` (Windows 64 bit) o `ngPostDeluxe-v1.1-win32-setup.exe` (Windows 32 bit) dalla pagina [Releases](https://github.com/thenormal/ngPost-Deluxe/releases/latest) e segui i passaggi. [7-Zip](https://www.7-zip.org/) è incluso: non serve installare altro. Le impostazioni e la cronologia restano in `%APPDATA%\ngPost Deluxe` e sopravvivono agli aggiornamenti e alla disinstallazione.<br/>
Windows può mostrare "Il PC è stato protetto da Windows" perché l'installer non è firmato digitalmente: clicca su **Ulteriori informazioni**, poi su **Esegui comunque**.<br/>
**Versione portatile**: `ngPostDeluxe-v1.1-win64.zip` / `ngPostDeluxe-v1.1-win32.zip`, senza installazione: decomprimi la cartella dove vuoi e avvia `ngPostDeluxe.exe` (le impostazioni stanno nel file `ngPost.conf` accanto al programma).

### Docker / Unraid
La versione a riga di comando è disponibile anche come immagine Docker che controlla una cartella e posta tutto quello che ci arriva
(per NAS o server), con un template per Unraid: `ghcr.io/thenormal/ngpost-deluxe`. Vedi [docker/README.md](docker/README.md) (in inglese).

### Screenshot
| Quick Post (tema chiaro) | Cronologia |
|---|---|
| ![Quick Post (tema chiaro)](docs/screenshots/quick-post.png) | ![Cronologia](docs/screenshots/history.png) |
| **Impostazioni (tema scuro)** | **Server (tema scuro)** |
| ![Impostazioni (tema scuro)](docs/screenshots/settings-dark.png) | ![Server (tema scuro)](docs/screenshots/servers-dark.png) |

### Cosa cambia in questo fork
-   **ngPost Deluxe v1.1**: **immagine Docker** della versione a riga di comando (controlla una cartella e posta tutto quello che ci arriva, con un template per **Unraid**), il pulsante Post è centrato in Quick Post e Auto Posting, icone dei tab e tab finiti leggibili nel tema scuro
-   **Correzione di sicurezza**: possibile overflow dell'heap nel buffer di codifica yEnc (con certi dati l'output poteva essere più grande del buffer)
-   **Correzione yEnc**: uno spazio o un tab a fine riga ora viene protetto correttamente (alcuni server lo eliminavano, corrompendo i dati)
-   **Password più sicure**: le password e i nomi degli archivi generati usano il generatore casuale crittografico del sistema operativo (Qt >= 5.10) invece di `std::rand()`
-   **NZB_POST_CMD**: i segnaposto possono contenere spazi (un percorso come `C:\Le mie cartelle\file.nzb` resta UN SOLO argomento) e il comando viene avviato in background (ngPost non lo aspetta più, quindi il suo codice di uscita non viene più registrato)
-   **RAR_EXTRA e PAR2_ARGS**: analizzati rispettando le virgolette. 7-Zip viene riconosciuto dal nome del programma e non dall'intero percorso
-   **ParPar** (un generatore di par2 molto più veloce) è incluso per Windows a 64 bit (v0.4.6, pubblico dominio) e usato automaticamente quando `parpar.exe` si trova nella cartella di ngPost. par2cmdline a 64 bit è stato tolto dal pacchetto, quello a 32 bit resta per Windows a 32 bit
-   **Nuova interfaccia**: barra laterale (Quick Post, Post automatico, Server, Impostazioni, Cronologia), pagine a carte, opzioni avanzate nascoste finché non servono, una nuova icona e una schermata vuota che spiega cosa fare. **Tema chiaro e scuro**, oppure automatico come Windows (chiave `theme`). Imposta la variabile d'ambiente `NGPOST_NO_THEME` per tornare all'aspetto nativo
-   **Interfaccia in italiano**: la GUI/CMD è ora tradotta anche in italiano (era già disponibile in cinese, olandese, inglese, francese, tedesco, portoghese e spagnolo)
-   **Nuovo README**: riscritto per questo fork e disponibile in 8 lingue (inglese, francese, tedesco, italiano, spagnolo, olandese, portoghese, cinese)
-   **ngPost Deluxe v1.0**: nuovo nome e nuova versione (il programma per Windows ora si chiama `ngPostDeluxe.exe`; il file di configurazione non cambia), finestra About ridisegnata, i pulsanti per le donazioni originali sono sostituiti da un pulsante **Ko-fi** per questo fork (nella finestra About e nelle Impostazioni)
-   **Installer per Windows** (64 bit e 32 bit, con **7-Zip incluso**) e pacchetti ZIP portatili, creati automaticamente su GitHub e pubblicati nelle Releases (`ngPostDeluxe.exe`)
-   **Compressione più veloce su Windows**: l'output di 7-Zip / rar ora viene letto man mano che arriva (un post da 115 MB aspettava 200 s, ora 1 s)
-   **Nuova sezione Server**: elenco dei server con pallino di stato e casella attivo/disattivo, scheda di dettaglio (password visibile a richiesta) e pulsante **Prova connessione** che verifica l'accesso. Modificare i server durante un upload non può più mandare in crash ngPost
-   **Salvataggio automatico**: ogni modifica delle impostazioni viene salvata da sola (il file precedente resta come `ngPost.conf.bak`)
-   **Scheda Auto Posting**: nuovi pulsanti **Rimuovi completati**, **Rimuovi tutto** e **Chiudi le schede terminate**
-   **Password dell'archivio fuori dalla riga di comando**: 7-Zip la riceve sullo standard input e rar tramite la variabile d'ambiente `RAR`, quindi non è più visibile nell'elenco dei processi
-   **Diagnostica**: ngPost registra durata e velocità di ogni fase (compressione, par2, upload), gli errori e i blocchi, e il pulsante **Report diagnostico** salva un report per analizzare un problema. Password, nomi utente e nomi dei file vengono mascherati, nulla viene inviato
-   **Windows**: la finestra della console si chiude da sola quando ngPost viene avviato con un doppio clic; il pacchetto include OpenSSL 1.1.1w (ultima versione supportata da Qt 5)
-   **Pagina Cronologia**: ogni upload viene registrato (`ngPost_history.csv` accanto alla configurazione, oppure `POST_HISTORY`) con stato, numero di file e durata. La pagina li elenca con ricerca, filtro per periodo, totali, password nascoste finché non le chiedi, copia, esportazione e cartella dell'NZB. Attenzione: il file contiene le password degli archivi in chiaro
-   **Quick Post**: una nota ricorda che tutti gli elementi dell'elenco vengono postati come un'unica release (un NZB, un archivio)

Non ci sono ancora release ufficiali di questo fork. Un pacchetto **Windows 64bit** pronto all'uso viene compilato automaticamente da GitHub Actions a ogni modifica (scheda *Actions* > esecuzione *Build Windows (x64)* > *Artifacts*, serve un account GitHub). Sugli altri sistemi va compilato dai sorgenti (vedi sotto).<br/>

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
- **theme**: tema dell'interfaccia, `AUTO` (come Windows), `LIGHT` o `DARK`
- **history**: `ON` (predefinito) o `OFF`, cronologia degli upload mostrata nella pagina Cronologia. Senza **POST_HISTORY** il file è `ngPost_history.csv` accanto alla configurazione. In fondo a ogni riga ci sono nuove colonne: stato, numero di file, durata e file NZB

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

### Sostenere questo fork
ngPost Deluxe è gratuito. Se ti è utile, puoi sostenerne lo sviluppo su Ko-fi:<br/>
[![Ko-fi](https://img.shields.io/badge/Ko--fi-support%20ngPost%20Deluxe-FF5E5B?logo=ko-fi&logoColor=white)](https://ko-fi.com/thenormal)

### Sostenere l'autore originale
ngPost è stato creato da Matthieu Bruel. Se usi l'app e vuoi contribuire allo sforzo, fallo sul [progetto originale](https://github.com/mbruel/ngPost).
