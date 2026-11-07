<img align="left" width="80" height="80" src="src/resources/icons/ngPost.png" alt="ngPost">

# ngPost-2026

[English](README.md) | [Français](README_FR.md) | [Deutsch](README_DE.md) | [Italiano](README_IT.md) | **Español** | [Nederlands](README_NL.md) | [Português](README_PT.md) | [中文](README_ZH.md)

**ngPost-2026** es una versión revisada y rediseñada del famoso cargador de Usenet **ngPost**.<br/>
Es un fork de [mbruel/ngPost](https://github.com/mbruel/ngPost) (v4.16, licencia GPL v3). Todo el mérito del programa original corresponde a su autor, Matthieu Bruel.<br/>

### Qué cambia en este fork
-   **Corrección de seguridad**: posible desbordamiento de memoria en el búfer de codificación yEnc (con ciertos datos la salida podía ser mayor que el búfer)
-   **Corrección yEnc**: un espacio o tabulador al final de una línea ahora se escapa correctamente (algunos servidores lo eliminaban y corrompían los datos)
-   **Contraseñas más seguras**: las contraseñas y nombres de archivo generados usan el generador aleatorio criptográfico del sistema operativo (Qt >= 5.10) en lugar de `std::rand()`
-   **NZB_POST_CMD**: los marcadores pueden contener espacios (una ruta como `C:\Mis carpetas\archivo.nzb` sigue siendo UN SOLO argumento) y el comando se lanza en segundo plano (ngPost ya no lo espera, así que su código de salida ya no se registra)
-   **RAR_EXTRA y PAR2_ARGS**: se analizan respetando las comillas. 7-Zip se detecta por el nombre del programa y no por toda la ruta
-   **ParPar** (un generador de par2 mucho más rápido) se incluye para Windows de 64 bits (v0.4.6, dominio público) y se usa automáticamente cuando `parpar.exe` está en la carpeta de ngPost. Se ha quitado par2cmdline de 64 bits del paquete; el de 32 bits se mantiene para Windows de 32 bits
-   **Tema claro moderno**: la GUI tiene un aspecto limpio inspirado en Windows 11 (estilo Fusion + hoja de estilos). Define la variable de entorno `NGPOST_NO_THEME` para recuperar el aspecto nativo
-   **Interfaz en italiano**: la GUI/CMD ahora también está traducida al italiano (ya estaba disponible en chino, neerlandés, inglés, francés, alemán, portugués y español)
-   **Nuevo README**: reescrito para este fork y disponible en 8 idiomas (inglés, francés, alemán, italiano, español, neerlandés, portugués, chino)
-   **ngPost 2026 v5.0 beta**: nuevo nombre y nueva versión, ventana Acerca de rediseñada, se han quitado los botones de donación (para apoyar al autor original, ver el final de esta página)
-   **Nueva sección Servidores**: lista de servidores con un indicador de estado y una casilla activado/desactivado, ficha de detalle (contraseña visible a petición) y un botón **Probar conexión** que comprueba el acceso. Modificar los servidores durante una subida ya no puede bloquear ngPost
-   **Guardado automático**: cada cambio de los ajustes se guarda automáticamente (el archivo anterior se conserva como `ngPost.conf.bak`)
-   **Pestaña Auto Posting**: nuevos botones **Quitar completados**, **Quitar todo** y **Cerrar pestañas terminadas**
-   **Contraseña del archivo fuera de la línea de comandos**: 7-Zip la recibe por su entrada estándar y rar mediante la variable de entorno `RAR`, así que ya no aparece en la lista de procesos
-   **Diagnóstico**: ngPost registra la duración y la velocidad de cada fase (compresión, par2, subida), los errores y los bloqueos, y el botón **Informe de diagnóstico** guarda un informe para analizar un problema. Contraseñas, usuarios y nombres de archivo se ocultan, no se envía nada
-   **Windows**: la ventana de consola se cierra sola cuando ngPost se inicia con doble clic; el paquete incluye OpenSSL 1.1.1w (última versión soportada por Qt 5)

Todavía no hay versiones oficiales de este fork. GitHub Actions genera automáticamente un paquete **Windows 64bit** listo para usar en cada cambio (pestaña *Actions* > ejecución *Build Windows (x64)* > *Artifacts*, se necesita una cuenta de GitHub). En los demás sistemas, compílalo desde el código fuente (ver abajo).<br/>

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
- **PAR2_CMD**: para cambiar el generador de par2 y poder usar [Parpar](https://github.com/animetosho/ParPar) o [Multipar](http://hp.vector.co.jp/authors/VA021385/). (par2cmdline es el generador incluido por defecto; ParPar se usa automáticamente en Windows de 64 bits si `parpar.exe` está en la carpeta de ngPost)
- **PAR2_ARGS**: para personalizar el comando par2, sobre todo si eliges otro programa distinto del par2cmdline por defecto

### Cómo compilar
#### Dependencias:
- build-essential (compilador C++, libstdc++, make, ...)
- qt5-default (bibliotecas y cabeceras de Qt5)
- qt5-qmake (para generar los archivos moc y crear el Makefile)
- libssl (v1.0.2 o v1.1), pero debería estar ya instalada en tu sistema

#### Compilación:
- ve a la carpeta src
- qmake
- make

¡Fácil! Debería haber generado el ejecutable **ngPost**</br>
puedes copiarlo a una carpeta de tu PATH para que sea accesible desde cualquier sitio


Como está hecho en C++/Qt, puedes compilarlo y ejecutarlo en cualquier sistema operativo (Linux / Windows / MacOS / Android). <br/>
Para compilar en otro sistema, lo más fácil es [instalar Qt](https://www.qt.io/download) y cargar el proyecto en QtCreator<br/>

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

### Apoyar al autor original
ngPost fue creado por Matthieu Bruel. Si usas la aplicación y quieres contribuir al esfuerzo, hazlo en el [proyecto original](https://github.com/mbruel/ngPost).
