<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost Deluxe

[English](README.md) | [Français](README_FR.md) | [Deutsch](README_DE.md) | [Italiano](README_IT.md) | **Español** | [Nederlands](README_NL.md) | [Português](README_PT.md) | [中文](README_ZH.md)

**Un cargador de Usenet rápido, gratuito y moderno para Windows, Linux y Docker.**<br/>
Comprime, añade par2, ofusca y publica tus archivos desde una interfaz limpia (temas claro y oscuro) o desde la línea de comandos.

[![Latest release](https://img.shields.io/github/v/release/thenormal/ngPost-Deluxe)](https://github.com/thenormal/ngPost-Deluxe/releases/latest)
[![Build](https://github.com/thenormal/ngPost-Deluxe/actions/workflows/build-windows.yml/badge.svg)](https://github.com/thenormal/ngPost-Deluxe/actions)
[![Licence: GPL v3](https://img.shields.io/badge/licence-GPL%20v3-blue)](LICENSE)

[![Download](https://img.shields.io/badge/Download-latest%20release-2ea44f?style=for-the-badge)](https://github.com/thenormal/ngPost-Deluxe/releases/latest)

**ngPost Deluxe** es una versión revisada y rediseñada del famoso cargador de Usenet **ngPost**.<br/>
Es un fork de [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, licencia GPL v3). Todo el mérito del programa original corresponde a su autor, Matthieu Bruel.<br/>

### Primeros pasos
| Instalador de Windows | Windows portátil | Docker / Unraid |
|---|---|---|
| Descarga `ngPostDeluxe-v…-win64-setup.exe` (o `win32`) desde la página [Releases](https://github.com/thenormal/ngPost-Deluxe/releases/latest). 7-Zip está incluido. | Descomprime `ngPostDeluxe-v…-win64.zip` (o `win32`) donde quieras e inicia `ngPostDeluxe.exe`. Los ajustes están en el `ngPost.conf` junto al programa. | Imagen `ghcr.io/thenormal/ngpost-deluxe` con una plantilla para Unraid. Ver [docker/README.md](docker/README.md). |

> Windows puede mostrar «Windows protegió su PC» porque el instalador no está firmado digitalmente: haz clic en **Más información** y luego en **Ejecutar de todas formas**.

### Inicio rápido
1. Instala e inicia ngPost Deluxe.
2. Abre **Servers**, añade tu servidor de Usenet y pulsa **Test connection**.
3. Abre **Quick Post** y arrastra tus archivos o carpetas a la ventana.
4. Pulsa **Post Files**. El archivo NZB se escribe en la carpeta elegida en los ajustes y cada subida aparece en **History**.

¿Ya usas ngPost? Copia tu `ngPost.conf` en `%APPDATA%\ngPost Deluxe` y tendrás tus servidores y ajustes.

### Capturas de pantalla
| Quick Post (tema claro) | Historial |
|---|---|
| ![Quick Post (tema claro)](docs/screenshots/quick-post.png) | ![Historial](docs/screenshots/history.png) |
| **Ajustes (tema oscuro)** | **Servidores (tema oscuro)** |
| ![Ajustes (tema oscuro)](docs/screenshots/settings-dark.png) | ![Servidores (tema oscuro)](docs/screenshots/servers-dark.png) |

### Qué cambia en este fork

**Interfaz**
-   **Nueva interfaz**: barra lateral (Quick Post, Publicación Automática, Servidores, Ajustes, Historial), páginas con tarjetas, opciones avanzadas ocultas hasta que se necesitan, un nuevo icono y una pantalla vacía que explica qué hacer. **Temas claro y oscuro**, o automático como Windows (clave `theme`). Define la variable de entorno `NGPOST_NO_THEME` para volver al aspecto nativo
-   **Interfaz en italiano**: la GUI/CMD ahora también está traducida al italiano (ya estaba disponible en chino, neerlandés, inglés, francés, alemán, portugués y español)
-   **ngPost Deluxe v1.0**: nuevo nombre y nueva versión (el programa de Windows ahora es `ngPostDeluxe.exe`; el archivo de configuración no cambia), ventana Acerca de rediseñada, los botones de donación originales se sustituyen por un botón de **Ko-fi** para este fork (en la ventana Acerca de y en los Ajustes)
-   **Nueva sección Servidores**: lista de servidores con un indicador de estado y una casilla activado/desactivado, ficha de detalle (contraseña visible a petición) y un botón **Probar conexión** que comprueba el acceso. Modificar los servidores durante una subida ya no puede bloquear ngPost
-   **Pestaña Auto Posting**: nuevos botones **Quitar completados**, **Quitar todo** y **Cerrar pestañas terminadas**
-   **Página Historial**: cada subida se registra (`ngPost_history.csv` junto a la configuración, o `POST_HISTORY`) con su estado, número de archivos y duración. La página las muestra con búsqueda, filtro por periodo, totales, contraseñas ocultas hasta que se piden, copia, exportación y carpeta del NZB. Cuidado: el archivo contiene las contraseñas de los archivos comprimidos en texto claro
-   **Quick Post**: una nota recuerda que todos los elementos de la lista se publican como una sola release (un NZB, un archivo comprimido)

**Velocidad**
-   **ParPar** (un generador de par2 mucho más rápido) se incluye para Windows de 64 bits (v0.4.6, dominio público) y se usa automáticamente cuando `parpar.exe` está en la carpeta de ngPost. Se ha quitado par2cmdline de 64 bits del paquete; el de 32 bits se mantiene para Windows de 32 bits
-   **Compresión más rápida en Windows**: la salida de 7-Zip / rar ahora se lee a medida que llega (un post de 115 MB esperaba 200 s, ahora 1 s)

**Paquetes de Windows y Docker**
-   **ngPost Deluxe v1.1**: **imagen Docker** de la versión en línea de comandos (vigila una carpeta y publica todo lo que llega, con una plantilla para **Unraid**), el botón Post está centrado en Quick Post y Auto Posting, los iconos de las pestañas y las pestañas terminadas se leen bien en el tema oscuro
-   **Instalador de Windows** (64 y 32 bits, con **7-Zip incluido**) y paquetes ZIP portátiles, creados automáticamente en GitHub y publicados en las Releases (`ngPostDeluxe.exe`)
-   **Windows**: la ventana de consola se cierra sola cuando ngPost se inicia con doble clic; el paquete incluye OpenSSL 1.1.1w (última versión soportada por Qt 5)

**Seguridad y correcciones**
-   **Corrección de seguridad**: posible desbordamiento de memoria en el búfer de codificación yEnc (con ciertos datos la salida podía ser mayor que el búfer)
-   **Corrección yEnc**: un espacio o tabulador al final de una línea ahora se escapa correctamente (algunos servidores lo eliminaban y corrompían los datos)
-   **Contraseñas más seguras**: las contraseñas y nombres de archivo generados usan el generador aleatorio criptográfico del sistema operativo (Qt >= 5.10) en lugar de `std::rand()`
-   **Contraseña del archivo fuera de la línea de comandos**: 7-Zip la recibe por su entrada estándar y rar mediante la variable de entorno `RAR`, así que ya no aparece en la lista de procesos

**Ajustes y herramientas**
-   **NZB_POST_CMD**: los marcadores pueden contener espacios (una ruta como `C:\Mis carpetas\archivo.nzb` sigue siendo UN SOLO argumento) y el comando se lanza en segundo plano (ngPost ya no lo espera, así que su código de salida ya no se registra)
-   **RAR_EXTRA y PAR2_ARGS**: se analizan respetando las comillas. 7-Zip se detecta por el nombre del programa y no por toda la ruta
-   **Nuevo README**: reescrito para este fork y disponible en 8 idiomas (inglés, francés, alemán, italiano, español, neerlandés, portugués, chino)
-   **Guardado automático**: cada cambio de los ajustes se guarda automáticamente (el archivo anterior se conserva como `ngPost.conf.bak`)
-   **Diagnóstico**: ngPost registra la duración y la velocidad de cada fase (compresión, par2, subida), los errores y los bloqueos, y el botón **Informe de diagnóstico** guarda un informe para analizar un problema. Contraseñas, usuarios y nombres de archivo se ocultan, no se envía nada

Los instaladores y paquetes portátiles para Windows y la imagen Docker están en [Releases](https://github.com/thenormal/ngPost-Deluxe/releases/latest). En los demás sistemas, compílalo desde el código fuente (ver abajo).<br/>

### Funciones principales
-   **Cargador de Usenet por línea de comandos y con una bonita interfaz gráfica** para binarios, desarrollado en **C++11/Qt5**
-   está diseñado para ser **lo más rápido posible** y ofrecer TODAS las funciones principales **para publicar datos de forma fácil y segura**.
-   puede **comprimir** (con tu programa rar externo) y **generar los par2** antes de publicar!
-   tiene una **cola de publicación** para preparar varias publicaciones (sobre todo en la interfaz gráfica con pestañas).
-   **prepara la siguiente publicación mientras sube la actual** (compresión + generación de par2)
-   puede **automatizar publicaciones** **escaneando carpetas** y publicando cada archivo/carpeta por separado tras comprimirlos (con un posible nombre de archivo y contraseña aleatorios) y generar los par2!
-   puede **vigilar carpetas y publicar cada nuevo archivo/carpeta** por separado tras comprimirlos.
-   puede **borrar automáticamente archivos/carpetas una vez publicados** (solo con la vigilancia en la interfaz gráfica y con --auto y --monitor a la vez en la línea de comandos)
-   puede **ejecutar un COMANDO o un script tras cada publicación** (ver NZB_POST_CMD en el archivo de configuración)
-   puede **apagar el ordenador** cuando todas las publicaciones han terminado
-   está **traducido al chino, neerlandés, inglés, francés, alemán, italiano, portugués y español**.

### Palabras clave de configuración modificadas en este fork
El archivo de configuración funciona como en el proyecto original (por defecto: **~/.ngPost** en Linux y Mac, **ngPost.conf** en la carpeta de instalación en Windows). Aquí solo se listan las palabras clave cuyo comportamiento ha cambiado:

- **NZB_POST_CMD**: comando ejecutado después de cada post. Los marcadores pueden contener espacios y el comando se ejecuta en segundo plano (su código de salida no se registra)
- **RAR_EXTRA**: para personalizar el comando rar (no hace falta poner 'a', '-idp' ni '-r'). No hace falta para 7-zip, salvo que quieras cambiar el nivel de compresión.
- **PAR2_PATH**: para cambiar el generador de par2 y poder usar [Parpar](https://github.com/animetosho/ParPar) o [Multipar](http://hp.vector.co.jp/authors/VA021385/). (par2cmdline es el generador incluido por defecto; ParPar se usa automáticamente en Windows de 64 bits si `parpar.exe` está en la carpeta de ngPost)
- **PAR2_ARGS**: para personalizar el comando par2, sobre todo si eliges otro programa distinto del par2cmdline por defecto
- **theme**: tema de la interfaz, `AUTO` (como Windows), `LIGHT` o `DARK`
- **history**: `ON` (por defecto) u `OFF`, historial de subidas mostrado en la página Historial. Sin **POST_HISTORY** el archivo es `ngPost_history.csv` junto a la configuración. Se añaden nuevas columnas al final de cada línea: estado, número de archivos, duración y archivo NZB

### Cómo compilar
#### Linux
Dependencias (Ubuntu / Debian):
```
sudo apt install build-essential qtbase5-dev qt5-qmake libssl-dev
```
Compilar la versión gráfica, o solo la de línea de comandos (no hacen falta las bibliotecas gráficas de Qt):
```
# GUI
cd src && qmake ngPost.pro && make -j$(nproc)

# command line only, in its own folder
mkdir build-cmd && cd build-cmd && qmake ../src/ngPost_cmd.pro && make -j$(nproc)
```
El ejecutable **ngPost** se crea en la carpeta donde has compilado: cópialo a una carpeta de tu PATH.

#### Windows
Instala [Qt 5.15](https://www.qt.io/download) con MinGW y abre `src/ngPost.pro` en Qt Creator, o desde un símbolo del sistema de Qt: `mkdir build && cd build && qmake ../src/ngPost.pro CONFIG+=release && mingw32-make`. El programa se llama `ngPostDeluxe.exe`. El instalador se crea con [Inno Setup](https://jrsoftware.org/isinfo.php) (`installer/ngPostDeluxe.iss`); todos los pasos de los paquetes oficiales están en `.github/workflows/build-windows.yml`.

#### Docker
Versión de línea de comandos en un contenedor (ver [docker/README.md](docker/README.md)):
```
docker build -t ngpost-deluxe .
```

#### Otros sistemas
Al estar hecho en C++/Qt, se puede compilar y ejecutar en cualquier sistema (macOS, Android...): lo más fácil es [instalar Qt](https://www.qt.io/download) y abrir `src/ngPost.pro` en Qt Creator.

### Documentación completa
Las opciones de línea de comandos, la lista completa de palabras clave de configuración y las guías de instalación son las del proyecto original: consulta el [README de ngPost](https://github.com/mbruel/ngPost#readme). Sus guías de instalación se refieren a los binarios originales, no a este fork.

### FAQ

**Windows dice «Windows protegió su PC». ¿Es seguro?**<br/>
El instalador no está firmado digitalmente (una firma cuesta dinero), por eso SmartScreen avisa ante cualquier programa desconocido. Pulsa **Más información** y luego **Ejecutar de todos modos**. El programa es de código abierto y los paquetes se generan automáticamente a partir de este repositorio.

**¿Dónde están mis ajustes y mi historial?**<br/>
Con el instalador: en `%APPDATA%\ngPost Deluxe` (`ngPost.conf` y `ngPost_history.csv`). Con la versión portátil: `ngPost.conf` junto al programa. Si hay un `ngPost.conf` junto a `ngPostDeluxe.exe`, se usa en lugar del de `%APPDATA%`. El historial se puede mover con `POST_HISTORY`.

**¿Cómo conservo los ajustes del ngPost original?**<br/>
Cierra ngPost Deluxe, copia tu antiguo `ngPost.conf` en `%APPDATA%\ngPost Deluxe` y, si contiene una línea `PAR2_PATH` que apunta a una carpeta antigua, bórrala (el programa usará su propio ParPar). También se lee tu antiguo `ngPost_history.csv`: cópialo junto a la configuración o mantén su ruta en `POST_HISTORY`.

**Algo no funciona (compresión, par2, subida…). ¿Qué puedo hacer?**<br/>
Pulsa **Diagnostic report** en el registro y adjunta el archivo a una [issue](https://github.com/thenormal/ngPost-Deluxe/issues). Las contraseñas, los nombres de usuario y los nombres de archivo están enmascarados y no se envía nada.

**¿Tengo que instalar 7-Zip o rar?**<br/>
No: 7-Zip está incluido en el instalador y en el paquete portátil. En los ajustes puedes elegir otro 7-Zip o WinRAR.

**¿El historial es privado?**<br/>
Contiene las contraseñas de los archivos en texto claro: mantén privados `ngPost_history.csv` y `ngPost.conf` (que contiene el acceso a tu servidor) y no los adjuntes a una issue pública.

### Licencia
[**ngPost**](https://github.com/mbruel/ngPost) se distribuye con la licencia **GPL v3** (ver [LICENSE](LICENSE)). Copyright (C) 2020 Matthieu Bruel. ngPost Deluxe es un fork mantenido por [thenormal](https://github.com/thenormal).

### Preguntas / Problemas / Peticiones
- para preguntas sobre este fork, puedes contactar con su mantenedor en GitHub: [thenormal](https://github.com/thenormal)
- para el programa original, consulta el [proyecto original](https://github.com/mbruel/ngPost) y a su autor Matthieu Bruel

### Idiomas admitidos
Por ahora ngPost está traducido al chino, neerlandés, inglés, francés, alemán, italiano, portugués y español (la traducción al italiano se añadió en este fork).<br/>
Si quieres traducir ngPost a tu idioma, es fácil (hay una buena interfaz para ello: QtLinguist). Los archivos de traducción están en la carpeta **src/lang**.

### Agradecimientos
- Matthieu Bruel (mbruel) por haber creado ngPost y publicarlo bajo GPL v3
- Uukrull por sus intensas pruebas y comentarios y por construir todos los paquetes de MacOS.
- awsms por sus pruebas en un servidor de verdad con conexión de 10 Gb/s que ayudaron a mejorar la velocidad de subida de ngPost y la compatibilidad con multihilo
- animetosho por haber desarrollado ParPar, el generador de par2 más rápido que existe!
- demanuel por el desarrollo de NewsUP, que fue mi primer cargador
- noobcoder1983, tensai y luego yuppie por la traducción al alemán
- tiriclote por la traducción al español
- hunesco por la traducción al portugués
- Peng por la traducción al chino
- todos los usuarios de ngPost ;)

### Apoyar este fork
ngPost Deluxe es gratuito. Si te resulta útil, puedes apoyar su desarrollo en Ko-fi:<br/>
[![Ko-fi](https://img.shields.io/badge/Ko--fi-support%20ngPost%20Deluxe-FF5E5B?logo=ko-fi&logoColor=white)](https://ko-fi.com/thenormal)

### Apoyar al autor original
ngPost fue creado por Matthieu Bruel. Si usas la aplicación y quieres contribuir al esfuerzo, hazlo en el [proyecto original](https://github.com/mbruel/ngPost).
