<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost Deluxe

[English](README.md) | [Français](README_FR.md) | [Deutsch](README_DE.md) | [Italiano](README_IT.md) | **Español** | [Nederlands](README_NL.md) | [Português](README_PT.md) | [中文](README_ZH.md)

**ngPost Deluxe** es una versión revisada y rediseñada del famoso cargador de Usenet **ngPost**.<br/>
Es un fork de [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, licencia GPL v3). Todo el mérito del programa original corresponde a su autor, Matthieu Bruel.<br/>

### Descarga
**Instalador**: descarga `ngPostDeluxe-v1.1-win64-setup.exe` (Windows 64 bits) o `ngPostDeluxe-v1.1-win32-setup.exe` (Windows 32 bits) desde la página [Releases](https://github.com/thenormal/ngPost-Deluxe/releases/latest) y sigue los pasos. [7-Zip](https://www.7-zip.org/) está incluido: no hay que instalar nada más. Tus ajustes y el historial se guardan en `%APPDATA%\ngPost Deluxe` y se conservan al actualizar y al desinstalar.<br/>
Windows puede mostrar «Windows protegió su PC» porque el instalador no está firmado digitalmente: haz clic en **Más información** y luego en **Ejecutar de todas formas**.<br/>
**Versión portátil**: `ngPostDeluxe-v1.1-win64.zip` / `ngPostDeluxe-v1.1-win32.zip`, sin instalación: descomprime la carpeta donde quieras y ejecuta `ngPostDeluxe.exe` (los ajustes están en el `ngPost.conf` junto al programa).

### Docker / Unraid
La versión en línea de comandos también está disponible como imagen Docker que vigila una carpeta y publica todo lo que llega (para NAS o servidores), con una plantilla para Unraid: `ghcr.io/thenormal/ngpost-deluxe`. Ver [docker/README.md](docker/README.md) (en inglés).

### Capturas de pantalla
| Quick Post (tema claro) | Historial |
|---|---|
| ![Quick Post (tema claro)](docs/screenshots/quick-post.png) | ![Historial](docs/screenshots/history.png) |
| **Ajustes (tema oscuro)** | **Servidores (tema oscuro)** |
| ![Ajustes (tema oscuro)](docs/screenshots/settings-dark.png) | ![Servidores (tema oscuro)](docs/screenshots/servers-dark.png) |

### Qué cambia en este fork
-   **ngPost Deluxe v1.1**: **imagen Docker** de la versión en línea de comandos (vigila una carpeta y publica todo lo que llega, con una plantilla para **Unraid**), el botón Post está centrado en Quick Post y Auto Posting, los iconos de las pestañas y las pestañas terminadas se leen bien en el tema oscuro
-   **Corrección de seguridad**: posible desbordamiento de memoria en el búfer de codificación yEnc (con ciertos datos la salida podía ser mayor que el búfer)
-   **Corrección yEnc**: un espacio o tabulador al final de una línea ahora se escapa correctamente (algunos servidores lo eliminaban y corrompían los datos)
-   **Contraseñas más seguras**: las contraseñas y nombres de archivo generados usan el generador aleatorio criptográfico del sistema operativo (Qt >= 5.10) en lugar de `std::rand()`
-   **NZB_POST_CMD**: los marcadores pueden contener espacios (una ruta como `C:\Mis carpetas\archivo.nzb` sigue siendo UN SOLO argumento) y el comando se lanza en segundo plano (ngPost ya no lo espera, así que su código de salida ya no se registra)
-   **RAR_EXTRA y PAR2_ARGS**: se analizan respetando las comillas. 7-Zip se detecta por el nombre del programa y no por toda la ruta
-   **ParPar** (un generador de par2 mucho más rápido) se incluye para Windows de 64 bits (v0.4.6, dominio público) y se usa automáticamente cuando `parpar.exe` está en la carpeta de ngPost. Se ha quitado par2cmdline de 64 bits del paquete; el de 32 bits se mantiene para Windows de 32 bits
-   **Nueva interfaz**: barra lateral (Quick Post, Publicación Automática, Servidores, Ajustes, Historial), páginas con tarjetas, opciones avanzadas ocultas hasta que se necesitan, un nuevo icono y una pantalla vacía que explica qué hacer. **Temas claro y oscuro**, o automático como Windows (clave `theme`). Define la variable de entorno `NGPOST_NO_THEME` para volver al aspecto nativo
-   **Interfaz en italiano**: la GUI/CMD ahora también está traducida al italiano (ya estaba disponible en chino, neerlandés, inglés, francés, alemán, portugués y español)
-   **Nuevo README**: reescrito para este fork y disponible en 8 idiomas (inglés, francés, alemán, italiano, español, neerlandés, portugués, chino)
-   **ngPost Deluxe v1.0**: nuevo nombre y nueva versión (el programa de Windows ahora es `ngPostDeluxe.exe`; el archivo de configuración no cambia), ventana Acerca de rediseñada, los botones de donación originales se sustituyen por un botón de **Ko-fi** para este fork (en la ventana Acerca de y en los Ajustes)
-   **Instalador de Windows** (64 y 32 bits, con **7-Zip incluido**) y paquetes ZIP portátiles, creados automáticamente en GitHub y publicados en las Releases (`ngPostDeluxe.exe`)
-   **Compresión más rápida en Windows**: la salida de 7-Zip / rar ahora se lee a medida que llega (un post de 115 MB esperaba 200 s, ahora 1 s)
-   **Nueva sección Servidores**: lista de servidores con un indicador de estado y una casilla activado/desactivado, ficha de detalle (contraseña visible a petición) y un botón **Probar conexión** que comprueba el acceso. Modificar los servidores durante una subida ya no puede bloquear ngPost
-   **Guardado automático**: cada cambio de los ajustes se guarda automáticamente (el archivo anterior se conserva como `ngPost.conf.bak`)
-   **Pestaña Auto Posting**: nuevos botones **Quitar completados**, **Quitar todo** y **Cerrar pestañas terminadas**
-   **Contraseña del archivo fuera de la línea de comandos**: 7-Zip la recibe por su entrada estándar y rar mediante la variable de entorno `RAR`, así que ya no aparece en la lista de procesos
-   **Diagnóstico**: ngPost registra la duración y la velocidad de cada fase (compresión, par2, subida), los errores y los bloqueos, y el botón **Informe de diagnóstico** guarda un informe para analizar un problema. Contraseñas, usuarios y nombres de archivo se ocultan, no se envía nada
-   **Windows**: la ventana de consola se cierra sola cuando ngPost se inicia con doble clic; el paquete incluye OpenSSL 1.1.1w (última versión soportada por Qt 5)
-   **Página Historial**: cada subida se registra (`ngPost_history.csv` junto a la configuración, o `POST_HISTORY`) con su estado, número de archivos y duración. La página las muestra con búsqueda, filtro por periodo, totales, contraseñas ocultas hasta que se piden, copia, exportación y carpeta del NZB. Cuidado: el archivo contiene las contraseñas de los archivos comprimidos en texto claro
-   **Quick Post**: una nota recuerda que todos los elementos de la lista se publican como una sola release (un NZB, un archivo comprimido)

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

### Licencia
**ngPost** se publica bajo licencia **GPL v3**
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
