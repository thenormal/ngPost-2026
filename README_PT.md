<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost-2026

[English](README.md) | [Français](README_FR.md) | [Deutsch](README_DE.md) | [Italiano](README_IT.md) | [Español](README_ES.md) | [Nederlands](README_NL.md) | **Português** | [中文](README_ZH.md)

**ngPost-2026** é uma versão revisitada e redesenhada do famoso uploader de Usenet **ngPost**.<br/>
É um fork de [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, licença GPL v3). Todo o mérito do programa original pertence ao autor, Matthieu Bruel.<br/>

### O que muda neste fork
-   **Correção de segurança**: possível estouro de heap no buffer de codificação yEnc (com certos dados, a saída podia exceder o buffer)
-   **Senhas mais seguras**: as senhas e nomes de arquivo gerados usam o gerador aleatório criptográfico do sistema operacional (Qt >= 5.10) em vez de `std::rand()`
-   **NZB_POST_CMD**: os marcadores podem conter espaços (um caminho como `C:\Minhas pastas\arquivo.nzb` continua sendo UM argumento) e o comando é iniciado em segundo plano (o ngPost não espera mais por ele, então o código de saída deixa de ser registrado)
-   **RAR_EXTRA e PAR2_ARGS**: são interpretados respeitando as aspas duplas. O 7-Zip é reconhecido pelo nome do programa e não pelo caminho completo
-   **ParPar** (um gerador de par2 muito mais rápido) está incluído para Windows 64 bits (v0.4.6, domínio público) e é usado automaticamente quando `parpar.exe` está na pasta do ngPost. O par2cmdline de 64 bits foi removido do pacote; a versão de 32 bits continua para Windows 32 bits

Este fork ainda não publica binários: compile-o a partir do código-fonte (veja abaixo). Os links de download nas seções de instalação são os do ngPost original v4.16 e não incluem as alterações acima.<br/>
<br/>

**Poster de Usenet em linha de comando e com uma GUI agradável** para arquivos binários, desenvolvido em **C++11/Qt5**<br/>
foi projetado para ser **o mais rápido possível** e oferecer TODAS as funções importantes **para postar dados de forma simples e segura**.<br/>
pode **compactar** (com o seu programa rar externo) e **gerar os arquivos par2** antes de postar!<br/>
tem uma **fila** para preparar vários posts (principalmente na GUI com abas).<br/>
**empacota o post seguinte enquanto envia o atual** (compactação + geração de par2)<br/>
pode **automatizar** posts **varrendo pastas** e postando cada arquivo/pasta separadamente depois de compactá-los (opcionalmente com nome de arquivo e senha aleatórios) e gerar o par2!<br/>
pode **monitorar pastas e postar cada novo arquivo/pasta** separadamente depois de compactá-los.<br/>
pode **apagar automaticamente arquivos/pastas após o post** (apenas no monitoramento da GUI e com --auto e --monitor juntos na linha de comando)<br/>
pode **executar um COMANDO ou script** depois de cada post (veja NZB_POST_CMD no arquivo de configuração)<br/>
pode **desligar o computador** quando todos os posts terminarem<br/>
está **traduzido para chinês, holandês, inglês, francês, alemão, italiano, português e espanhol**.<br/>


![ngPost_v4.3](pics/ngPost_v4.3.png)

[As versões do ngPost original estão disponíveis](https://github.com/mbruel/ngPost/releases/tag/v4.16) para: Linux 64 bits, Windows (32 e 64 bits), MacOS e Raspbian (RPI 4). Em breve também para Android e depois iOS...

Estas são as principais funções e vantagens do ngPost:

-   **ocultação total do cabeçalho do Article**: o assunto é um UUID (como o msg-id) e é usado um remetente aleatório. **Atenção**: por isso você não conseguirá mais encontrar o seu post na Usenet (nem em indexadores) se perder o arquivo NZB. Mas este método é **totalmente seguro**, **não é preciso ofuscar os arquivos nem usar senha**.
-   **Fila**: você pode preparar vários posts enquanto posta algo. Claro, é possível cancelar os posts em espera ;)
-   **Automação de posts**: varra uma pasta e poste cada arquivo/pasta depois de compactar. (veja a opção **--auto na linha de comando** e a aba "Auto Posting" na GUI)
-   **compactar com RAR ou 7zip** (comando externo) com **ofuscação de nome** aleatória e senha, e **gerar par2** antes de postar
-   **histórico dos posts em um arquivo csv**: assim você tem data, **nome do arquivo**, tamanho, velocidade de envio e sobretudo **nome do arquivo compactado e senha** em uma simples planilha ;)
-   **ParPar** (muito mais rápido que o par2cmdline) é usado automaticamente no Windows 64 bits quando `parpar.exe` está na pasta do ngPost (a versão para Windows 64 bits está em `release/libs`). O **par2cmdline** continua incluído para Windows 32 bits, e você também pode usar outro programa (como o Multipar) com as palavras-chave PAR2_CMD e PAR2_ARGS do arquivo de configuração
-   suporta **vários arquivos** e **várias pastas**
-   suporta **vários servidores** (pelo arquivo de configuração ou pela GUI), cada um com **várias conexões** (com SSL)
-   distribui essas conexões em **várias threads**. Por padrão, o número de núcleos do computador, mas você pode definir o número.
-   **prepara os Articles na thread principal** (codificação yEnc) para que as threads de envio estejam **sempre prontas para escrever**. (cada conexão tem 2 Articles prontos de antemão)
-   usa **sockets assíncronos** nas threads de envio
-   **limita o acesso ao disco** ao mínimo: os arquivos a postar são abertos apenas uma vez na thread principal e processados um a um (não é preciso abrir vários ao mesmo tempo, os Articles são distribuídos entre todas as conexões)
-   gera um **remetente (from) aleatório**, mas você também pode usar um fixo
-   **gera o arquivo nzb**
-   **trata interrupções**: se um post for interrompido, **o nzb é criado com os arquivos já postados** e **os arquivos não postados são listados**, para que você poste só esses e depois una os nzb manualmente. (na linha de comando, o aplicativo seria encerrado de forma limpa)
-   **adicionar metadados no cabeçalho do nzb** (geralmente uma senha)
-   repostar os Articles em caso de erro **com outro UUID**
-   reconectar em caso de erro em um socket (mesmo parâmetro Retry dos Articles)
-   ...


### Compilar
#### Dependências:
- build-essential (compilador C++, libstdc++, make, ...)
- qt5-default (bibliotecas e cabeçalhos do Qt5)
- qt5-qmake (para gerar os arquivos moc e criar o Makefile)
- libssl (v1.0.2 ou v1.1), mas já deveria estar no seu sistema

#### Compilação:
- vá para a pasta src
- qmake
- make

Fácil! Deve ter gerado o executável **ngPost**</br>
você pode copiá-lo para uma pasta do seu PATH para tê-lo disponível em qualquer lugar


Como é feito em C++/Qt, você pode compilá-lo e executá-lo em qualquer sistema operacional (Linux / Windows / MacOS / Android). <br/>
Até agora só foram feitas versões para Linux x64, Windows x64 (a partir do Windows 7) e MacOS</br>
Para compilar em outro sistema operacional, o mais fácil é [instalar o Qt](https://www.qt.io/download) e carregar o projeto no QtCreator<br/>

### Uso na linha de comando (as descrições das opções são a saída original em inglês do programa)
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

### Arquivo de configuração e palavras-chave exclusivas dele
O arquivo de configuração padrão para Linux e Mac é: **~/.ngPost** (sem extensão .conf)<br/>
Se quiser, na linha de comando você pode usar outro arquivo com a opção -c.<br/>
[Aqui está o exemplo a seguir](https://github.com/mbruel/ngPost/blob/master/ngPost.conf).<br/>
<br/>
A maioria das palavras-chave da configuração pode ser usada na linha de comando, mas algumas só existem no arquivo de configuração:
- **MONITOR_NZB_FOLDERS**: cada post de monitoramento vai para uma pasta própria criada dentro de nzbPath (Req/Issue #15)
- **inputDir**: pasta padrão aberta para escolher arquivos na GUI (pode mudar se você alterar a pasta Auto na GUI e SALVAR)
- **POST_HISTORY**: arquivo csv ao qual cada post bem-sucedido acrescenta data, nome do arquivo, tamanho, velocidade de envio, nome do arquivo compactado e senha
- **GROUP_POLICY**: define como postar quando há vários grupos (em todos, um por post ou um por arquivo)
- **NZB_RM_ACCENTS**: remove acentos e caracteres especiais dos nomes dos arquivos nzb
- **AUTO_CLOSE_TABS**: fecha as abas Quick Post após um post bem-sucedido (para a GUI)
- **RESUME_WAIT**: tempo de espera (segundos) antes de tentar retomar automaticamente um Post após perda de rede (mín.: 30)
- **NO_RESUME_AUTO**: interrompe um post se a rede cair
- **PREPARE_PACKING**: com vários posts na fila, prepara o empacotamento do seguinte enquanto o atual é enviado

As seguintes são para posters experientes:
- **RAR_EXTRA**: para personalizar o comando rar (não é preciso indicar 'a', '-idp' nem '-r'). Desnecessário para o 7-zip, exceto se quiser mudar o nível de compressão.
- **PAR2_CMD**: para mudar o gerador de par2 e poder usar o [Parpar](https://github.com/animetosho/ParPar) ou o [Multipar](http://hp.vector.co.jp/authors/VA021385/). (o par2cmdline é o gerador incluído por padrão; no Windows 64 bits o ParPar é usado automaticamente se `parpar.exe` estiver na pasta do ngPost)
- **PAR2_ARGS**: para personalizar o comando par2, sobretudo se escolher um programa diferente do par2cmdline padrão
- **TMP_RAM**: usa uma pasta temporária com limite de tamanho, normalmente uma partição tmpfs, para os posts que couberem nela (caso contrário usa TMP_DIR)
- **TMP_RAM_RATIO**: fator aplicado ao tamanho dos arquivos de origem para compensar a geração do par2

### Versão portátil Linux 64 bits (compilada com Qt v5.12.6)
Se não quiser compilar nem instalar as dependências, também pode usar a versão portátil, que inclui tudo.<br/>
- baixe [ngPost_v4.16-x86_64.AppImage](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16-x86_64.AppImage) ou [ngPost_v4.16_cmd-x86_64.AppImage](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16_cmd-x86_64.AppImage) só para linha de comando (a primeira versão faz as duas coisas: CMD e GUI)
- chmod 755 ngPost_v4.16-x86_64.AppImage
- execute-o com a mesma sintaxe da seção acima
- se quiser guardar o arquivo de configuração, edite o arquivo **~/.ngPost** seguindo [este modelo](https://raw.githubusercontent.com/mbruel/ngPost/master/ngPost.conf) (sem a extensão .conf)

PS: para sistemas mais antigos com GLIBC < 2.24, aqui está uma versão compilada no Debian8 com GLIBC 2.19 e Qt v5.8.0: [ngPost_v4.16-x86_64_debian8.AppImage](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16-x86_64_debian8.AppImage)


### Versão Raspbian (armhf para Raspberry PI)
- baixe [ngPost_v4.16-armhf.AppImage](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16-armhf.AppImage)
- chmod 755 ngPost_v4.16-armhf.AppImage
- execute-o com a mesma sintaxe da seção acima
- se quiser guardar o arquivo de configuração, edite o arquivo **~/.ngPost** seguindo [este modelo](https://raw.githubusercontent.com/mbruel/ngPost/master/ngPost.conf) (sem a extensão .conf)

**Como o rar não está disponível no RPI, foi adicionado suporte ao 7zip.** Use RAR_PATH para apontar para o 7z e RAR_EXTRA para as opções do 7zip. Você deve ter algo como:
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


### Instalador para Windows
- basta usar o pacote [ngPost_v4.16_x64_setup.exe](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16_x64_setup.exe) ou [ngPost_v4.16_x86_setup.exe](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16_x86_setup.exe) para a versão de 32 bits
- edite o **ngPost.conf** (na pasta de instalação) para adicionar os dados do seu servidor (pode indicar vários).
- execute o **ngPost.exe** (versão GUI)
- ou use-o na linha de comando: **ngPost.exe** -i "seu arquivo ou pasta"

se quiser, pode indicar todos os parâmetros do servidor na linha de comando (veja a seção acima)<br/>
Por padrão:
- o ngPost carrega o arquivo de configuração 'ngPost.conf' que está na pasta
- também grava o arquivo nzb nessa pasta. (o nome é o do último arquivo de entrada na linha de comando)

**Problema conhecido sem suporte a SSL:** talvez seja preciso instalar os [msvc2015 redistributables](https://www.microsoft.com/en-us/download/details.aspx?id=48145), pois a libssl depende das suas APIs<br/>


### Versão MacOS compilada no High Sierra (v10.13)
- baixe [ngPost_v4.16.1.dmg](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16.1.dmg)
- execute-o com a mesma sintaxe da seção acima
- se quiser guardar o arquivo de configuração, edite o arquivo **~/.ngPost** seguindo [este modelo](https://raw.githubusercontent.com/mbruel/ngPost/master/ngPost.conf) (sem a extensão .conf)


### Alternativas

Uma lista de posters de Usenet do GitHub do Nyuu [está aqui](https://github.com/animetosho/Nyuu/wiki/Usenet-Uploaders).


### Licença
O **ngPost** é publicado sob a licença **GPL v3**
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


### Perguntas / Problemas / Pedidos
- para perguntas sobre este fork, entre em contato com o mantenedor no GitHub: [thenormal](https://github.com/thenormal)
- para o programa original, consulte o [projeto original](https://github.com/mbruel/ngPost) e o autor Matthieu Bruel


### Idiomas suportados
Atualmente o ngPost está traduzido para chinês, holandês, inglês, francês, alemão, italiano, português e espanhol (a tradução italiana foi adicionada neste fork).<br/>
Se quiser traduzir o ngPost para o seu idioma, é fácil (há uma GUI prática para isso: QtLinguist). Os arquivos de tradução estão na pasta **src/lang**.


### Agradecimentos
- Matthieu Bruel (mbruel) por criar o ngPost e publicá-lo sob a GPL v3
- Uukrull pelos testes intensivos, pelo feedback e por compilar todos os pacotes de MacOS.
- awsms pelos testes em um servidor real com conexão de 10 Gb/s, que ajudaram a melhorar a velocidade de envio do ngPost e o suporte a multithreading
- animetosho pelo desenvolvimento do ParPar, o gerador de par2 mais rápido de todos!
- demanuel pelo desenvolvimento do NewsUP, que foi o meu primeiro poster
- noobcoder1983, tensai e depois yuppie pela tradução alemã
- tiriclote pela tradução espanhola
- hunesco pela tradução portuguesa
- Peng pela tradução chinesa
- todos os usuários do ngPost ;)


### Apoie o autor original
O ngPost foi criado por Matthieu Bruel. Se você usa o aplicativo e quer contribuir com o esforço, faça-o pelo [projeto original](https://github.com/mbruel/ngPost).
