<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost-2026

[English](README.md) | [Français](README_FR.md) | [Deutsch](README_DE.md) | [Italiano](README_IT.md) | [Español](README_ES.md) | [Nederlands](README_NL.md) | **Português** | [中文](README_ZH.md)

**ngPost-2026** é uma versão revisitada e redesenhada do famoso uploader de Usenet **ngPost**.<br/>
É um fork de [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, licença GPL v3). Todo o mérito do programa original pertence ao autor, Matthieu Bruel.<br/>

### O que muda neste fork
-   **Correção de segurança**: possível estouro de heap no buffer de codificação yEnc (com certos dados, a saída podia exceder o buffer)
-   **Correção yEnc**: um espaço ou tab no fim de uma linha agora é escapado corretamente (alguns servidores o removiam, corrompendo os dados)
-   **Senhas mais seguras**: as senhas e nomes de arquivo gerados usam o gerador aleatório criptográfico do sistema operacional (Qt >= 5.10) em vez de `std::rand()`
-   **NZB_POST_CMD**: os marcadores podem conter espaços (um caminho como `C:\Minhas pastas\arquivo.nzb` continua sendo UM argumento) e o comando é iniciado em segundo plano (o ngPost não espera mais por ele, então o código de saída deixa de ser registrado)
-   **RAR_EXTRA e PAR2_ARGS**: são interpretados respeitando as aspas duplas. O 7-Zip é reconhecido pelo nome do programa e não pelo caminho completo
-   **ParPar** (um gerador de par2 muito mais rápido) está incluído para Windows 64 bits (v0.4.6, domínio público) e é usado automaticamente quando `parpar.exe` está na pasta do ngPost. O par2cmdline de 64 bits foi removido do pacote; a versão de 32 bits continua para Windows 32 bits
-   **Tema claro moderno**: a GUI tem um visual limpo inspirado no Windows 11 (estilo Fusion + folha de estilo). Defina a variável de ambiente `NGPOST_NO_THEME` para voltar ao visual nativo
-   **Interface em italiano**: a GUI/CMD agora também está traduzida para italiano (já disponível em chinês, holandês, inglês, francês, alemão, português e espanhol)
-   **Novo README**: reescrito para este fork e disponível em 8 idiomas (inglês, francês, alemão, italiano, espanhol, holandês, português, chinês)

Este fork ainda não publica binários: compile-o a partir do código-fonte (veja abaixo).<br/>
**Usuários de Windows**: por enquanto não há instalador pronto para este fork, o programa precisa ser compilado (por exemplo com o Qt Creator). Os instaladores do projeto original não incluem as alterações acima e ainda usam o antigo par2cmdline.<br/>

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
- **PAR2_CMD**: para mudar o gerador de par2 e poder usar o [Parpar](https://github.com/animetosho/ParPar) ou o [Multipar](http://hp.vector.co.jp/authors/VA021385/). (o par2cmdline é o gerador incluído por padrão; no Windows 64 bits o ParPar é usado automaticamente se `parpar.exe` estiver na pasta do ngPost)
- **PAR2_ARGS**: para personalizar o comando par2, sobretudo se escolher um programa diferente do par2cmdline padrão

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
Para compilar em outro sistema operacional, o mais fácil é [instalar o Qt](https://www.qt.io/download) e carregar o projeto no QtCreator<br/>

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

### Apoie o autor original
O ngPost foi criado por Matthieu Bruel. Se você usa o aplicativo e quer contribuir com o esforço, faça-o pelo [projeto original](https://github.com/mbruel/ngPost).
