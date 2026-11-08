<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost Deluxe

[English](README.md) | [Français](README_FR.md) | [Deutsch](README_DE.md) | [Italiano](README_IT.md) | [Español](README_ES.md) | [Nederlands](README_NL.md) | **Português** | [中文](README_ZH.md)

**Um poster de Usenet rápido, gratuito e moderno para Windows, Linux e Docker.**<br/>
Comprima, adicione par2, ofusque e publique os seus ficheiros a partir de uma interface limpa (temas claro e escuro) ou da linha de comandos.

[![Latest release](https://img.shields.io/github/v/release/thenormal/ngPost-Deluxe)](https://github.com/thenormal/ngPost-Deluxe/releases/latest)
[![Build](https://github.com/thenormal/ngPost-Deluxe/actions/workflows/build-windows.yml/badge.svg)](https://github.com/thenormal/ngPost-Deluxe/actions)
[![Licence: GPL v3](https://img.shields.io/badge/licence-GPL%20v3-blue)](LICENSE)

[![Download](https://img.shields.io/badge/Download-latest%20release-2ea44f?style=for-the-badge)](https://github.com/thenormal/ngPost-Deluxe/releases/latest)

**ngPost Deluxe** é uma versão revisitada e redesenhada do famoso uploader de Usenet **ngPost**.<br/>
É um fork de [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, licença GPL v3). Todo o mérito do programa original pertence ao autor, Matthieu Bruel.<br/>

### Primeiros passos
| Instalador do Windows | Windows portátil | Docker / Unraid |
|---|---|---|
| Transfira `ngPostDeluxe-v…-win64-setup.exe` (ou `win32`) da página [Releases](https://github.com/thenormal/ngPost-Deluxe/releases/latest). O 7-Zip está incluído. | Descompacte `ngPostDeluxe-v…-win64.zip` (ou `win32`) onde quiser e inicie `ngPostDeluxe.exe`. As definições estão no `ngPost.conf` junto ao programa. | Imagem `ghcr.io/thenormal/ngpost-deluxe` com um modelo para Unraid. Ver [docker/README.md](docker/README.md). |

> O Windows pode mostrar "O Windows protegeu o seu PC" porque o instalador não está assinado digitalmente: clique em **Mais informações** e depois em **Executar mesmo assim**.

### Início rápido
1. Instale e inicie o ngPost Deluxe.
2. Abra **Servers**, adicione o seu servidor Usenet e prima **Test connection**.
3. Abra **Quick Post** e arraste os seus ficheiros ou pastas para a janela.
4. Prima **Post Files**. O ficheiro NZB é escrito na pasta escolhida nas definições e cada envio aparece em **History**.

Já usa o ngPost? Copie o seu `ngPost.conf` para `%APPDATA%\ngPost Deluxe` e os seus servidores e definições ficam lá.

### Capturas de ecrã
| Quick Post (tema claro) | Histórico |
|---|---|
| ![Quick Post (tema claro)](docs/screenshots/quick-post.png) | ![Histórico](docs/screenshots/history.png) |
| **Definições (tema escuro)** | **Servidores (tema escuro)** |
| ![Definições (tema escuro)](docs/screenshots/settings-dark.png) | ![Servidores (tema escuro)](docs/screenshots/servers-dark.png) |

### O que muda neste fork

**Interface**
-   **Nova interface**: barra lateral (Quick Post, Postagem Automática, Servidores, Definições, Histórico), páginas com cartões, opções avançadas ocultas até serem precisas, um novo ícone e um ecrã vazio que explica o que fazer. **Temas claro e escuro**, ou automático como o Windows (chave `theme`). Defina a variável de ambiente `NGPOST_NO_THEME` para voltar ao aspeto nativo
-   **Interface em italiano**: a GUI/CMD agora também está traduzida para italiano (já disponível em chinês, holandês, inglês, francês, alemão, português e espanhol)
-   **ngPost Deluxe v1.0**: novo nome e nova versão (o programa para Windows chama-se agora `ngPostDeluxe.exe`; o ficheiro de configuração não muda), janela Sobre redesenhada, os botões de donativo originais foram substituídos por um botão **Ko-fi** para este fork (na janela Sobre e nas Definições)
-   **Nova secção Servidores**: lista dos servidores com um indicador de estado e uma caixa ativo/inativo, ficha de detalhe (palavra-passe visível a pedido) e um botão **Testar ligação** que verifica o acesso. Alterar os servidores durante um envio já não pode fazer o ngPost falhar
-   **Separador Auto Posting**: novos botões **Remover concluídos**, **Remover tudo** e **Fechar separadores concluídos**
-   **Página Histórico**: cada upload é registado (`ngPost_history.csv` ao lado da configuração, ou `POST_HISTORY`) com o estado, o número de ficheiros e a duração. A página lista-os com pesquisa, filtro por período, totais, palavras-passe ocultas até serem pedidas, cópia, exportação e pasta do NZB. Atenção: o ficheiro contém as palavras-passe dos arquivos em texto simples
-   **Quick Post**: uma nota lembra que todos os elementos da lista são publicados como uma só release (um NZB, um arquivo)

**Velocidade**
-   **ParPar** (um gerador de par2 muito mais rápido) está incluído para Windows 64 bits (v0.4.6, domínio público) e é usado automaticamente quando `parpar.exe` está na pasta do ngPost. O par2cmdline de 64 bits foi removido do pacote; a versão de 32 bits continua para Windows 32 bits
-   **Compressão mais rápida no Windows**: a saída do 7-Zip / rar passa a ser lida à medida que chega (um post de 115 MB esperava 200 s, agora 1 s)

**Pacotes Windows e Docker**
-   **ngPost Deluxe v1.1**: **imagem Docker** da versão de linha de comandos (vigia uma pasta e publica tudo o que lá chega, com um modelo para **Unraid**), o botão Post está centrado em Quick Post e Auto Posting, os ícones dos separadores e os separadores terminados ficam legíveis no tema escuro
-   **Instalador para Windows** (64 e 32 bits, com **7-Zip incluído**) e pacotes ZIP portáteis, criados automaticamente no GitHub e publicados nas Releases (`ngPostDeluxe.exe`)
-   **Windows**: a janela da consola fecha-se sozinha quando o ngPost é iniciado com duplo clique; o pacote inclui o OpenSSL 1.1.1w (última versão suportada pelo Qt 5)

**Segurança e correções**
-   **Correção de segurança**: possível estouro de heap no buffer de codificação yEnc (com certos dados, a saída podia exceder o buffer)
-   **Correção yEnc**: um espaço ou tab no fim de uma linha agora é escapado corretamente (alguns servidores o removiam, corrompendo os dados)
-   **Senhas mais seguras**: as senhas e nomes de arquivo gerados usam o gerador aleatório criptográfico do sistema operacional (Qt >= 5.10) em vez de `std::rand()`
-   **Palavra-passe do arquivo fora da linha de comandos**: o 7-Zip recebe-a na entrada padrão e o rar através da variável de ambiente `RAR`, por isso já não aparece na lista de processos

**Definições e ferramentas**
-   **NZB_POST_CMD**: os marcadores podem conter espaços (um caminho como `C:\Minhas pastas\arquivo.nzb` continua sendo UM argumento) e o comando é iniciado em segundo plano (o ngPost não espera mais por ele, então o código de saída deixa de ser registrado)
-   **RAR_EXTRA e PAR2_ARGS**: são interpretados respeitando as aspas duplas. O 7-Zip é reconhecido pelo nome do programa e não pelo caminho completo
-   **Novo README**: reescrito para este fork e disponível em 8 idiomas (inglês, francês, alemão, italiano, espanhol, holandês, português, chinês)
-   **Gravação automática**: cada alteração das definições é gravada automaticamente (o ficheiro anterior é mantido como `ngPost.conf.bak`)
-   **Diagnóstico**: o ngPost regista a duração e a velocidade de cada fase (compressão, par2, envio), os erros e os bloqueios, e o botão **Relatório de diagnóstico** guarda um relatório para analisar um problema. Palavras-passe, utilizadores e nomes de ficheiros são ocultados, nada é enviado

Os instaladores e pacotes portáteis para Windows e a imagem Docker estão em [Releases](https://github.com/thenormal/ngPost-Deluxe/releases/latest). Nos outros sistemas, compile-o a partir do código-fonte (ver abaixo).<br/>

### Principais funções
-   **Poster de Usenet em linha de comando e com uma GUI agradável** para arquivos binários, desenvolvido em **C++11/Qt5**
-   foi projetado para ser **o mais rápido possível** e oferecer TODAS as funções importantes **para postar dados de forma simples e segura**.
-   pode **compactar** (com o seu programa rar externo) e **gerar os arquivos par2** antes de postar!
-   tem uma **fila** para preparar vários posts (principalmente na GUI com abas).
-   **empacota o post seguinte enquanto envia o atual** (compactação + geração de par2)
-   pode **automatizar** posts **varrendo pastas** e postando cada arquivo/pasta separadamente depois de compactá-los (opcionalmente com nome de arquivo e senha aleatórios) e gerar o par2!
-   pode **monitorar pastas e postar cada novo arquivo/pasta** separadamente depois de compactá-los.
-   pode **apagar automaticamente arquivos/pastas após o post** (apenas no monitoramento da GUI e com --auto e --monitor juntos na linha de comando)
-   pode **executar um COMANDO ou script** depois de cada post (veja NZB_POST_CMD no arquivo de configuração)
-   pode **desligar o computador** quando todos os posts terminarem
-   está **traduzido para chinês, holandês, inglês, francês, alemão, italiano, português e espanhol**.

### Palavras-chave de configuração alteradas neste fork
O arquivo de configuração funciona como no projeto original (padrão: **~/.ngPost** no Linux e Mac, **ngPost.conf** na pasta de instalação no Windows). Aqui estão listadas apenas as palavras-chave cujo comportamento mudou:

- **NZB_POST_CMD**: comando executado após cada post. Os marcadores podem conter espaços e o comando é iniciado em segundo plano (o código de saída não é registrado)
- **RAR_EXTRA**: para personalizar o comando rar (não é preciso indicar 'a', '-idp' nem '-r'). Desnecessário para o 7-zip, exceto se quiser mudar o nível de compressão.
- **PAR2_PATH**: para mudar o gerador de par2 e poder usar o [Parpar](https://github.com/animetosho/ParPar) ou o [Multipar](http://hp.vector.co.jp/authors/VA021385/). (o par2cmdline é o gerador incluído por padrão; no Windows 64 bits o ParPar é usado automaticamente se `parpar.exe` estiver na pasta do ngPost)
- **PAR2_ARGS**: para personalizar o comando par2, sobretudo se escolher um programa diferente do par2cmdline padrão
- **theme**: tema da interface, `AUTO` (como o Windows), `LIGHT` ou `DARK`
- **history**: `ON` (predefinido) ou `OFF`, histórico dos uploads mostrado na página Histórico. Sem **POST_HISTORY** o ficheiro é `ngPost_history.csv` ao lado da configuração. São adicionadas novas colunas no fim de cada linha: estado, número de ficheiros, duração e ficheiro NZB

### Compilar
#### Linux
Dependências (Ubuntu / Debian):
```
sudo apt install build-essential qtbase5-dev qt5-qmake libssl-dev
```
Compilar a versão gráfica, ou apenas a de linha de comandos (não são precisas as bibliotecas gráficas do Qt):
```
# GUI
cd src && qmake ngPost.pro && make -j$(nproc)

# command line only, in its own folder
mkdir build-cmd && cd build-cmd && qmake ../src/ngPost_cmd.pro && make -j$(nproc)
```
O executável **ngPost** é criado na pasta onde compilou: copie-o para uma pasta do seu PATH.

#### Windows
Instale o [Qt 5.15](https://www.qt.io/download) com MinGW e abra `src/ngPost.pro` no Qt Creator, ou a partir de uma linha de comandos do Qt: `mkdir build && cd build && qmake ../src/ngPost.pro CONFIG+=release && mingw32-make`. O programa chama-se `ngPostDeluxe.exe`. O instalador é criado com o [Inno Setup](https://jrsoftware.org/isinfo.php) (`installer/ngPostDeluxe.iss`); todos os passos dos pacotes oficiais estão em `.github/workflows/build-windows.yml`.

#### Docker
Versão de linha de comandos num contentor (ver [docker/README.md](docker/README.md)):
```
docker build -t ngpost-deluxe .
```

#### Outros sistemas
Como é feito em C++/Qt, pode ser compilado e executado em qualquer sistema (macOS, Android...): o mais fácil é [instalar o Qt](https://www.qt.io/download) e abrir `src/ngPost.pro` no Qt Creator.

### Documentação completa
As opções da linha de comando, a lista completa das palavras-chave de configuração e os guias de instalação são os do projeto original: veja o [README do ngPost](https://github.com/mbruel/ngPost#readme). Os guias de instalação de lá se referem aos binários originais, não a este fork.

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

### Apoie este fork
O ngPost Deluxe é gratuito. Se lhe for útil, pode apoiar o seu desenvolvimento no Ko-fi:<br/>
[![Ko-fi](https://img.shields.io/badge/Ko--fi-support%20ngPost%20Deluxe-FF5E5B?logo=ko-fi&logoColor=white)](https://ko-fi.com/thenormal)

### Apoie o autor original
O ngPost foi criado por Matthieu Bruel. Se você usa o aplicativo e quer contribuir com o esforço, faça-o pelo [projeto original](https://github.com/mbruel/ngPost).
