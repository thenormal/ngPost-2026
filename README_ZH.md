<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost Deluxe

[English](README.md) | [Français](README_FR.md) | [Deutsch](README_DE.md) | [Italiano](README_IT.md) | [Español](README_ES.md) | [Nederlands](README_NL.md) | [Português](README_PT.md) | **中文**

**ngPost Deluxe** 是著名 Usenet 上传工具 **ngPost** 的重新审视与重新设计版本。<br/>
它是 [mbruel/ngPost](https://github.com/mbruel/ngPost)（v4.16，GPL v3 许可证）的分支（fork）。原始程序的全部功劳归于其作者 Matthieu Bruel。<br/>

### 本分支的不同之处
-   **安全修复**：修复 yEnc 编码缓冲区中可能的堆溢出（对于某些数据，输出可能超出缓冲区大小）
-   **yEnc 修复**：行尾的空格或制表符现在会被正确转义（某些服务器会将其去除，导致数据损坏）
-   **更安全的密码**：生成的压缩包密码和名称使用操作系统的加密级随机数生成器（Qt >= 5.10），而不是 `std::rand()`
-   **NZB_POST_CMD**：占位符可以包含空格（例如 `C:\我的文件夹\文件.nzb` 这样的路径仍然是同一个参数），并且命令在后台启动（ngPost 不再等待它结束，因此不再记录其退出码）
-   **RAR_EXTRA 和 PAR2_ARGS**：解析时遵循双引号。7-Zip 通过程序名称而不是完整路径来识别
-   为 64 位 Windows 附带 **ParPar**（快得多的 par2 生成器，v0.4.6，公有领域），当 ngPost 文件夹中存在 `parpar.exe` 时自动使用。已从软件包中移除 64 位 par2cmdline，32 位版本仍保留给 32 位 Windows
-   **全新界面**：侧边栏（快速发布、自动发布、服务器、设置、历史记录），卡片式页面，高级选项在需要时才显示，新图标，以及说明操作方法的空白页面。**浅色和深色主题**，或跟随 Windows 自动切换（`theme` 键）。设置环境变量 `NGPOST_NO_THEME` 可恢复原生外观
-   **意大利语界面**：图形界面/命令行现在也已翻译为意大利语（此前已有中文、荷兰语、英语、法语、德语、葡萄牙语和西班牙语）
-   **新的 README**：为本分支重写，提供 8 种语言（英语、法语、德语、意大利语、西班牙语、荷兰语、葡萄牙语、中文）
-   **ngPost Deluxe v5.0 beta**：新名称和新版本（Windows 程序现在名为 `ngPostDeluxe.exe`；配置文件不变），重新设计的“关于”窗口，原来的捐赠按钮已替换为本分支的 **Ko-fi** 按钮（在“关于”窗口和设置中）
-   **全新的服务器区域**：服务器列表带状态指示点和启用/禁用复选框，详情表单（密码可按需显示），以及检查登录的 **测试连接** 按钮。发布过程中修改服务器不会再导致 ngPost 崩溃
-   **自动保存**：每次修改设置都会自动保存（之前的文件保留为 `ngPost.conf.bak`）
-   **Auto Posting 标签页**：新增 **移除已完成**、**全部移除** 和 **关闭已完成的标签页** 按钮
-   **压缩包密码不再出现在命令行中**：7-Zip 通过标准输入接收密码，rar 通过 `RAR` 环境变量接收，因此进程列表中不再可见
-   **诊断**：ngPost 会记录每个阶段（压缩、par2、上传）的耗时和速度、错误和界面卡顿，**诊断报告** 按钮可保存报告用于分析问题。密码、用户名和文件名都会被隐藏，不会发送任何内容
-   **Windows**：双击启动 ngPost 时控制台窗口会自动关闭；安装包附带 OpenSSL 1.1.1w（Qt 5 支持的最新版本）
-   **历史记录页面**：每次上传都会被记录（配置文件旁边的 `ngPost_history.csv`，或 `POST_HISTORY`），包括状态、文件数和耗时。该页面提供搜索、按时间段筛选、汇总、默认隐藏的密码、复制、导出以及打开 NZB 所在文件夹。注意：该文件以明文保存压缩包密码
-   **快速发布**：提示列表中的所有项目会作为一个发布一起发布（一个 NZB，一个压缩包）

本分支尚未发布正式版本。每次修改后，GitHub Actions 都会自动构建可直接运行的 **Windows 64 位** 安装包（*Actions* 标签页 > *Build Windows (x64)* 运行 > *Artifacts*，需要 GitHub 账号）。其他系统请从源码编译（见下文）。<br/>

### 主要功能
-   面向二进制文件的**命令行 Usenet 上传工具，并带有美观的图形界面**，使用 **C++11/Qt5** 开发
-   它的设计目标是**尽可能快**，并提供所有重要功能，**让数据发布简单又安全**。
-   它可以在发布前**压缩**（使用你的外部 rar 程序）并**生成 par2 文件**！
-   它有**队列**，可以准备多个帖子（主要用于带标签页的图形界面）。
-   它会在**上传当前帖子的同时打包下一个帖子**（压缩 + 生成 par2）
-   它可以**自动化**发布：**扫描文件夹**，将每个文件/文件夹压缩后单独发布（可选随机压缩包名称和密码），并生成 par2！
-   它可以**监视文件夹**，将每个新文件/文件夹压缩后单独发布。
-   它可以在**发布后自动删除文件/文件夹**（仅限图形界面的监视模式，以及命令行中同时使用 --auto 和 --monitor）
-   它可以在每次发布后**执行命令或脚本**（见配置文件中的 NZB_POST_CMD）
-   它可以在所有帖子完成后**关闭计算机**
-   它已**翻译为中文、荷兰语、英语、法语、德语、意大利语、葡萄牙语和西班牙语**。

### 本分支中修改过的配置关键字
配置文件的使用方式与原始项目相同（默认：Linux 和 Mac 为 **~/.ngPost**，Windows 为安装文件夹中的 **ngPost.conf**）。这里只列出行为发生变化的关键字：

- **NZB_POST_CMD**：每次发布后执行的命令。占位符可以包含空格，命令在后台运行（不记录其退出码）
- **RAR_EXTRA**：用于自定义 rar 命令（无需指定 'a'、'-idp' 或 '-r'）。7-zip 不需要，除非你想更改压缩级别。
- **PAR2_CMD**：用于更改 par2 生成器，以便使用 [Parpar](https://github.com/animetosho/ParPar) 或 [Multipar](http://hp.vector.co.jp/authors/VA021385/)。（par2cmdline 是默认附带的生成器；在 64 位 Windows 上，如果 ngPost 文件夹中有 `parpar.exe`，则自动使用 ParPar）
- **PAR2_ARGS**：用于自定义 par2 命令，特别是在选择了不同于默认 par2cmdline 的程序时
- **theme**：界面主题，`AUTO`（跟随 Windows）、`LIGHT` 或 `DARK`
- **history**：`ON`（默认）或 `OFF`，上传历史记录显示在历史记录页面。未设置 **POST_HISTORY** 时，文件为配置文件旁边的 `ngPost_history.csv`。每行末尾新增列：状态、文件数、耗时和 NZB 文件

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
要在其他操作系统上编译，最简单的方法是[安装 Qt](https://www.qt.io/download)，并在 QtCreator 中加载项目<br/>

### 完整文档
命令行选项、完整的配置关键字列表和安装指南与原始项目相同：请参阅 [ngPost 的 README](https://github.com/mbruel/ngPost#readme)。其中的安装指南针对的是原始二进制文件，而不是本分支。

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

### 支持本分支
ngPost Deluxe 是免费的。如果它对你有用，你可以在 Ko-fi 上支持它的开发：<br/>
[![Ko-fi](https://img.shields.io/badge/Ko--fi-support%20ngPost%20Deluxe-FF5E5B?logo=ko-fi&logoColor=white)](https://ko-fi.com/thenormal)

### 支持原作者
ngPost 由 Matthieu Bruel 创建。如果你使用该应用并希望为这项工作做出贡献，请通过[原始项目](https://github.com/mbruel/ngPost)进行。
