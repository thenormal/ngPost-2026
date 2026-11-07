<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost-2026

[English](README.md) | [Français](README_FR.md) | [Deutsch](README_DE.md) | [Italiano](README_IT.md) | [Español](README_ES.md) | [Nederlands](README_NL.md) | [Português](README_PT.md) | **中文**

**ngPost-2026** 是著名 Usenet 上传工具 **ngPost** 的重新审视与重新设计版本。<br/>
它是 [mbruel/ngPost](https://github.com/mbruel/ngPost)（v4.16，GPL v3 许可证）的分支（fork）。原始程序的全部功劳归于其作者 Matthieu Bruel。<br/>

### 本分支的不同之处
-   **安全修复**：修复 yEnc 编码缓冲区中可能的堆溢出（对于某些数据，输出可能超出缓冲区大小）
-   **更安全的密码**：生成的压缩包密码和名称使用操作系统的加密级随机数生成器（Qt >= 5.10），而不是 `std::rand()`
-   **NZB_POST_CMD**：占位符可以包含空格（例如 `C:\我的文件夹\文件.nzb` 这样的路径仍然是同一个参数），并且命令在后台启动（ngPost 不再等待它结束，因此不再记录其退出码）
-   **RAR_EXTRA 和 PAR2_ARGS**：解析时遵循双引号。7-Zip 通过程序名称而不是完整路径来识别
-   为 64 位 Windows 附带 **ParPar**（快得多的 par2 生成器，v0.4.6，公有领域），当 ngPost 文件夹中存在 `parpar.exe` 时自动使用。已从软件包中移除 64 位 par2cmdline，32 位版本仍保留给 32 位 Windows

本分支目前不发布二进制文件：请从源代码编译（见下文）。安装章节中的下载链接是原始 ngPost v4.16 的链接，不包含上述修改。<br/>
<br/>

面向二进制文件的**命令行 Usenet 上传工具，并带有美观的图形界面**，使用 **C++11/Qt5** 开发<br/>
它的设计目标是**尽可能快**，并提供所有重要功能，**让数据发布简单又安全**。<br/>
它可以在发布前**压缩**（使用你的外部 rar 程序）并**生成 par2 文件**！<br/>
它有**队列**，可以准备多个帖子（主要用于带标签页的图形界面）。<br/>
它会在**上传当前帖子的同时打包下一个帖子**（压缩 + 生成 par2）<br/>
它可以**自动化**发布：**扫描文件夹**，将每个文件/文件夹压缩后单独发布（可选随机压缩包名称和密码），并生成 par2！<br/>
它可以**监视文件夹**，将每个新文件/文件夹压缩后单独发布。<br/>
它可以在**发布后自动删除文件/文件夹**（仅限图形界面的监视模式，以及命令行中同时使用 --auto 和 --monitor）<br/>
它可以在每次发布后**执行命令或脚本**（见配置文件中的 NZB_POST_CMD）<br/>
它可以在所有帖子完成后**关闭计算机**<br/>
它已**翻译为中文、荷兰语、英语、法语、德语、意大利语、葡萄牙语和西班牙语**。<br/>


![ngPost_v4.3](pics/ngPost_v4.3.png)

[原版 ngPost 的发布版本可在此获取](https://github.com/mbruel/ngPost/releases/tag/v4.16)，支持：Linux 64 位、Windows（32 位和 64 位）、MacOS 和 Raspbian（RPI 4）。不久也将支持 Android，之后是 iOS……

以下是 ngPost 的主要功能和优势：

-   **完全隐藏 Article 头部信息**：主题是 UUID（与 msg-id 相同），并使用随机发件人。**注意**：因此如果你丢失了 NZB 文件，就无法再在 Usenet（或索引站）上找到你的帖子。但这种方法**完全安全**，**无需混淆文件名或使用密码**。
-   **队列**：发布内容的同时可以准备多个帖子。当然，也可以取消等待中的帖子 ;)
-   **自动发布**：扫描文件夹，压缩后发布每个文件/文件夹。（见**命令行选项 --auto** 和图形界面的“Auto Posting”标签页）
-   **使用 RAR 或 7zip 压缩**（外部命令），带随机**名称混淆**和密码，并在发布前**生成 par2**
-   **将发布历史保存到 csv 文件**：这样你可以在简单的 Excel 表格中看到日期、**文件名**、大小、上传速度，尤其是**压缩包名称和密码** ;)
-   在 64 位 Windows 上，当 ngPost 文件夹中存在 `parpar.exe` 时自动使用 **ParPar**（比 par2cmdline 快得多；Windows 64 位版本位于 `release/libs`）。32 位 Windows 仍附带 **par2cmdline**，你也可以通过配置文件中的 PAR2_CMD 和 PAR2_ARGS 关键字使用其他程序（如 Multipar）
-   支持**多个文件**和**多个文件夹**
-   支持**多个服务器**（通过配置文件或图形界面），每个服务器可有**多个连接**（支持 SSL）
-   将这些连接分配到**多个线程**。默认是计算机的核心数，但你可以自行设置。
-   **在主线程中准备 Article**（yEnc 编码），使上传线程**始终可以随时写入**。（每个连接预先准备 2 个 Article）
-   在上传线程中使用**异步套接字**
-   **将磁盘访问降到最低**：要发布的文件只在主线程中打开一次并逐个处理（无需同时打开多个文件，Article 会分配给所有连接）
-   生成**随机发件人**（from），也可以使用固定发件人
-   **生成 nzb 文件**
-   **处理中断**：如果发布被中断，**会用已发布的文件创建 nzb**，并**列出未发布的文件**，这样你只需重新发布这些文件，然后手动合并 nzb。（在命令行中，应用程序会正常退出）
-   **在 nzb 头部添加元数据**（通常是密码）
-   出错时以**不同的 UUID** 重新发布 Article
-   套接字出错时重新连接（与 Article 使用相同的 Retry 参数）
-   ……


### 编译
#### 依赖项：
- build-essential（C++ 编译器、libstdc++、make 等）
- qt5-default（Qt5 库和头文件）
- qt5-qmake（用于生成 moc 文件并创建 Makefile）
- libssl（v1.0.2 或 v1.1），但系统中应该已经有了

#### 构建：
- 进入 src 文件夹
- qmake
- make

很简单！这样应该会生成可执行文件 **ngPost**</br>
你可以把它复制到 PATH 中的某个文件夹，以便在任何地方使用


由于使用 C++/Qt 编写，你可以在任何操作系统（Linux / Windows / MacOS / Android）上编译和运行。<br/>
目前只制作了 Linux x64、Windows x64（Windows 7 起）和 MacOS 的发布版本</br>
要在其他操作系统上编译，最简单的方法是[安装 Qt](https://www.qt.io/download)，并在 QtCreator 中加载项目<br/>

### 命令行用法（选项说明为程序输出的英文原文）
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

### 配置文件及仅限配置文件的关键字
Linux 和 Mac 的默认配置文件为：**~/.ngPost**（没有 .conf 扩展名）<br/>
如果需要，在命令行中可以用 -c 选项使用其他文件。<br/>
[这里是可供参考的示例](https://github.com/mbruel/ngPost/blob/master/ngPost.conf)。<br/>
<br/>
配置中的大多数关键字都可以在命令行中使用，但有几个只存在于配置文件中：
- **MONITOR_NZB_FOLDERS**：每个监视帖子都放入在 nzbPath 中创建的独立文件夹（Req/Issue #15）
- **inputDir**：图形界面中选择文件时默认打开的文件夹（如果你在图形界面中更改 Auto 文件夹并保存，它可能会改变）
- **POST_HISTORY**：csv 文件，每次成功发布都会向其中追加日期、文件名、大小、上传速度、压缩包名称和密码
- **GROUP_POLICY**：指定有多个组时如何发布（发到所有组、每个帖子一个组，或每个文件一个组）
- **NZB_RM_ACCENTS**：从 nzb 文件名中移除重音符号和特殊字符
- **AUTO_CLOSE_TABS**：成功发布后关闭 Quick Post 标签页（用于图形界面）
- **RESUME_WAIT**：网络中断后自动尝试恢复 Post 之前的等待时间（秒）（最小值：30）
- **NO_RESUME_AUTO**：网络中断时停止发布
- **PREPARE_PACKING**：队列中有多个帖子时，在上传当前帖子的同时准备打包下一个

以下适用于有经验的发布者：
- **RAR_EXTRA**：用于自定义 rar 命令（无需指定 'a'、'-idp' 或 '-r'）。7-zip 不需要，除非你想更改压缩级别。
- **PAR2_CMD**：用于更改 par2 生成器，以便使用 [Parpar](https://github.com/animetosho/ParPar) 或 [Multipar](http://hp.vector.co.jp/authors/VA021385/)。（par2cmdline 是默认附带的生成器；在 64 位 Windows 上，如果 ngPost 文件夹中有 `parpar.exe`，则自动使用 ParPar）
- **PAR2_ARGS**：用于自定义 par2 命令，特别是在选择了不同于默认 par2cmdline 的程序时
- **TMP_RAM**：为能放得下的帖子使用有大小限制的临时文件夹，通常是 tmpfs 分区（否则使用 TMP_DIR）
- **TMP_RAM_RATIO**：应用于源文件大小的系数，用于补偿 par2 的生成

### 便携式 Linux 64 位发布版（使用 Qt v5.12.6 编译）
如果你不想编译也不想安装依赖项，也可以使用包含所有内容的便携版。<br/>
- 下载 [ngPost_v4.16-x86_64.AppImage](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16-x86_64.AppImage)，或仅命令行版本 [ngPost_v4.16_cmd-x86_64.AppImage](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16_cmd-x86_64.AppImage)（第一个版本同时支持命令行和图形界面）
- chmod 755 ngPost_v4.16-x86_64.AppImage
- 使用与上一节相同的语法运行
- 如果想保留配置文件，请按照[此模板](https://raw.githubusercontent.com/mbruel/ngPost/master/ngPost.conf)编辑 **~/.ngPost** 文件（没有 .conf 扩展名）

PS：对于 GLIBC < 2.24 的旧系统，这里有一个在 Debian8（GLIBC 2.19、Qt v5.8.0）上编译的版本：[ngPost_v4.16-x86_64_debian8.AppImage](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16-x86_64_debian8.AppImage)


### Raspbian 发布版（适用于 Raspberry PI 的 armhf）
- 下载 [ngPost_v4.16-armhf.AppImage](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16-armhf.AppImage)
- chmod 755 ngPost_v4.16-armhf.AppImage
- 使用与上一节相同的语法运行
- 如果想保留配置文件，请按照[此模板](https://raw.githubusercontent.com/mbruel/ngPost/master/ngPost.conf)编辑 **~/.ngPost** 文件（没有 .conf 扩展名）

**由于 RPI 上没有 rar，已添加对 7zip 的支持。**使用 RAR_PATH 指向 7z，并用 RAR_EXTRA 设置 7zip 选项。你应该会得到类似这样的内容：
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


### Windows 安装程序
- 直接使用安装包 [ngPost_v4.16_x64_setup.exe](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16_x64_setup.exe)，32 位版本使用 [ngPost_v4.16_x86_setup.exe](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16_x86_setup.exe)
- 编辑 **ngPost.conf**（在安装文件夹中）以添加你的服务器信息（可以指定多个）。
- 运行 **ngPost.exe**（图形界面版本）
- 或在命令行中使用：**ngPost.exe** -i "你的文件或文件夹"

如果需要，你可以在命令行中指定所有服务器参数（见上一节）<br/>
默认情况下：
- ngPost 会加载文件夹中的配置文件 'ngPost.conf'
- 它也会把 nzb 文件写入该文件夹。（名称为命令行中最后一个输入文件的名称）

**不支持 SSL 的已知问题：**你可能需要安装 [msvc2015 redistributables](https://www.microsoft.com/en-us/download/details.aspx?id=48145)，因为 libssl 依赖其 API<br/>


### 在 High Sierra（v10.13）上构建的 MacOS 发布版
- 下载 [ngPost_v4.16.1.dmg](https://github.com/mbruel/ngPost/releases/download/v4.16/ngPost_v4.16.1.dmg)
- 使用与上一节相同的语法运行
- 如果想保留配置文件，请按照[此模板](https://raw.githubusercontent.com/mbruel/ngPost/master/ngPost.conf)编辑 **~/.ngPost** 文件（没有 .conf 扩展名）


### 替代方案

Nyuu 的 GitHub 上有一份 Usenet 上传工具列表，[见此处](https://github.com/animetosho/Nyuu/wiki/Usenet-Uploaders)。


### 许可证
**ngPost** 以 **GPL v3** 许可证发布
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


### 问题 / 故障 / 请求
- 关于本分支的问题，请在 GitHub 上联系维护者：[thenormal](https://github.com/thenormal)
- 关于原始程序，请参阅[原始项目](https://github.com/mbruel/ngPost)及其作者 Matthieu Bruel


### 支持的语言
目前 ngPost 已翻译为中文、荷兰语、英语、法语、德语、意大利语、葡萄牙语和西班牙语（意大利语翻译是在本分支中添加的）。<br/>
如果你想把 ngPost 翻译成你的语言，很容易（有一个方便的图形界面工具：QtLinguist）。翻译文件位于 **src/lang** 文件夹。


### 致谢
- Matthieu Bruel (mbruel)：创建 ngPost 并以 GPL v3 发布
- Uukrull：大量测试、反馈，并构建了所有 MacOS 软件包。
- awsms：在 10 Gb/s 连接的真实服务器上进行测试，帮助提升了 ngPost 的上传速度和多线程支持
- animetosho：开发了史上最快的 par2 生成器 ParPar！
- demanuel：开发了 NewsUP，那是我的第一个上传工具
- noobcoder1983、tensai，以及后来的 yuppie：德语翻译
- tiriclote：西班牙语翻译
- hunesco：葡萄牙语翻译
- Peng：中文翻译
- 所有 ngPost 用户 ;)


### 支持原作者
ngPost 由 Matthieu Bruel 创建。如果你使用该应用并希望为这项工作做出贡献，请通过[原始项目](https://github.com/mbruel/ngPost)进行。
