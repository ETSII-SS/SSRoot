@echo off
rem =============================================================================
rem  SS010-Init.bat
rem  Software de Sistemas - 3o Ingenieria de Computadores - ETSII / Univ. Sevilla
rem  Curso 2026-27   (rama 2026_27 del repositorio SSRoot)
rem
rem  COMO SE USA
rem    1. Descargue SOLO este archivo desde el repositorio, carpeta
rem       _pub\_utils\ , y dejelo en cualquier sitio del ordenador
rem       (Descargas, Escritorio, un pendrive...).
rem    2. Ejecutelo con doble clic.
rem    3. Le preguntara donde quiere instalar el arbol de practicas.
rem       Si pulsa ENTRAR sin escribir nada, creara SSRoot en la MISMA CARPETA
rem       donde este este archivo.
rem    4. Cuando termine, puede borrar el archivo descargado: dentro de
rem       SSRoot\_pub\_utils\ hay una copia que se actualiza con el repositorio.
rem
rem    Tambien puede ejecutarse desde SSRoot\_pub\_utils\ una vez instalado: en
rem    ese caso detecta que ya existe SSRoot, no pregunta nada y solo actualiza.
rem    Es IDEMPOTENTE: puede ejecutarse tantas veces como se quiera.
rem
rem  ESTRUCTURA DE DIRECTORIOS QUE DEJA PREPARADA
rem
rem    SSRoot\                  <- clon del repositorio. Aqui esta .git
rem       .gitignore
rem       _pub\                 material de la asignatura. NO LO MODIFIQUE.
rem          _comm\             inc\ lib\ bin\ comunes a todos los proyectos
rem          _utils\            los scripts .bat de la asignatura
rem             SS010-Init.bat  <- este archivo
rem          Practicas\  Teoria\  Test\
rem       _priv\                SU carpeta de trabajo. Repositorio Git propio,
rem                             local y sin servidor, ignorado por el publico.
rem
rem  QUE HACE
rem    1. Localiza git.exe (PATH, Git para Windows o el que instala VS2026).
rem    2. Pregunta la ruta y clona el repositorio en SSRoot, o lo actualiza si
rem       ya existe.
rem    3. Se situa en la rama del curso. NO PREGUNTA: si el repositorio esta
rem       en otra rama, cambia a la del curso sin consultar.
rem    4. Crea _priv y la convierte en repositorio Git local (git init).
rem    5. Fija la variable de entorno AAComm a SSRoot\_pub\_comm\
rem    6. Guarda la configuracion en SSRoot\_priv\SS-Config.cmd
rem
rem  OJO CON AAComm
rem    El valor DEBE terminar en barra invertida, porque los proyectos la usan
rem    concatenada sin separador: $(AAComm)inc , $(AAComm)lib\sslib\... ,
rem    $(AAComm)bin\sslib\... . Si se fija sin la barra final, los proyectos no
rem    encuentran los includes ni las bibliotecas. Por eso el setx de mas abajo
rem    lleva doble barra.
rem
rem  LO QUE NO HACE
rem    No usa hojas de propiedades. No modifica la variable PATH: no hace falta,
rem    los scripts localizan git.exe por su cuenta, y modificar PATH con setx es
rem    peligroso porque puede truncarse a 1024 caracteres.
rem
rem  IMPORTANTE
rem    Si Visual Studio estaba abierto, CIERRELO Y VUELVA A ABRIRLO: los
rem    procesos ya en marcha no ven el nuevo valor de AAComm.
rem =============================================================================

rem --- Trampolin -------------------------------------------------------------
rem  Cuando se ejecuta desde SSRoot\_pub\_utils\ , este script actualiza el
rem  repositorio en el que el mismo esta guardado. Si git lo sustituyera
rem  mientras se ejecuta, cmd.exe seguiria leyendolo por posicion y haria cosas
rem  impredecibles. Para evitarlo, se copia a la carpeta temporal y se ejecuta
rem  desde alli, pasando su carpeta original como segundo argumento.
rem ---------------------------------------------------------------------------
if /i "%~1"=="__RUN" goto :Main
copy /y "%~f0" "%TEMP%\SS010-Init-ejec.bat" >nul 2>&1
if errorlevel 1 goto :Main
cmd /c ""%TEMP%\SS010-Init-ejec.bat" __RUN "%~dp0"" & exit /b

:Main
setlocal EnableExtensions DisableDelayedExpansion
for /f "tokens=2 delims=:." %%c in ('chcp 2^>nul') do set "SS_OLDCP=%%c"
if defined SS_OLDCP set "SS_OLDCP=%SS_OLDCP: =%"
chcp 65001 >nul 2>&1
title Software de Sistemas - Instalar el espacio de trabajo

rem =============================================================================
rem  CONFIGURACION DEL CURSO. Es lo unico que hay que revisar cada curso.
rem =============================================================================
set "SS_CURSO=2026-27"
set "SS_RAMA=%SS_CURSO:-=_%"
set "URL_REPO=https://github.com/ETSII-SS/SSRoot.git"
set "SS_PRIV=_priv"
set "SS_UTILS=_pub\_utils"
rem =============================================================================

set "SS_AVISOS=0"

set "SCRIPTDIR=%~dp0"
if /i "%~1"=="__RUN" set "SCRIPTDIR=%~2"
if "%SCRIPTDIR:~-1%"=="\" set "SCRIPTDIR=%SCRIPTDIR:~0,-1%"

echo =================================================================
echo  Software de Sistemas - Curso %SS_CURSO%
echo  Instalar el espacio de trabajo de practicas
echo =================================================================
echo.

rem ============================ 1. Localizar git ==============================
echo --- 1. Comprobando git -----------------------------------------------------
call :FindGit
if errorlevel 1 goto :Fin
echo   [ OK ] git: %SSGIT%
for /f "usebackq delims=" %%V in (`"%SSGIT%" --version 2^>nul`) do echo   [ OK ] %%V
if "%SSGIT_INPATH%"=="0" echo   [ .. ] git no esta en el PATH. No hace falta para estos scripts.
echo.

rem ================= 2. Donde instalar: preguntar la ruta =====================
echo --- 2. Donde instalar ------------------------------------------------------

rem  Primero: si este script ya se esta ejecutando desde dentro de un SSRoot
rem  instalado, no hay nada que preguntar.
call :LocalizarSSRoot
if not defined SSROOT goto :PreguntarRuta
echo   Este script se esta ejecutando desde un SSRoot ya instalado:
echo       %SSROOT%
echo   No se pregunta la ruta: solo se actualiza.
goto :Actualizar

:PreguntarRuta
set "SSPROP=%SCRIPTDIR%\SSRoot"
echo   Escriba la carpeta donde quiere instalar el arbol de practicas.
echo.
echo   Si pulsa ENTRAR sin escribir nada, se creara en la carpeta donde esta
echo   este archivo, es decir en:
echo       %SSPROP%
echo.
echo   Recomendacion: una ruta corta, local y sin espacios, p.ej. C:\SSRoot
echo   Evite OneDrive: la sincronizacion en la nube y Git se estorban.
echo.
:PedirRuta
set "SSANS="
set /p "SSANS=  Carpeta de instalacion (ENTRAR = %SSPROP%): "
if not defined SSANS goto :UsarPropuesta
set SSROOT=%SSANS:"=%
goto :RutaElegida
:UsarPropuesta
set "SSROOT=%SSPROP%"

:RutaElegida
if not defined SSROOT goto :PedirRuta
if "%SSROOT:~-1%"=="\" set "SSROOT=%SSROOT:~0,-1%"
if "%SSROOT:~-1%"==":" set "SSROOT=%SSROOT%\"
echo.
echo   Instalando en: %SSROOT%

rem  Si ya hay un repositorio ahi, actualizar en vez de clonar.
if exist "%SSROOT%\.git" goto :YaHayRepo

rem  Si la carpeta existe y tiene contenido, git clone fallaria.
if not exist "%SSROOT%\" goto :Clonar
set "VACIA=1"
for /f "delims=" %%X in ('dir /b /a "%SSROOT%" 2^>nul') do set "VACIA=0"
if "%VACIA%"=="0" goto :ErrorNoVacia

:Clonar
echo   Clonando la rama %SS_RAMA% (unos 17 MB, puede tardar un minuto)...
echo.
"%SSGIT%" clone --branch "%SS_RAMA%" "%URL_REPO%" "%SSROOT%"
if errorlevel 1 goto :ErrorClonar
echo.
echo   [ OK ] Repositorio clonado.
call :LeeRama
goto :RamaOk

:YaHayRepo
echo   [ .. ] Ya hay un repositorio en esa carpeta. Se actualizara.

:Actualizar
rem  La rama se asegura ANTES de actualizar. Si el clonado se hizo sin indicar
rem  la rama, el repositorio esta en "main", y actualizar ahi no sirve de nada.
call :AsegurarRama
if errorlevel 1 goto :RamaNo

echo   Actualizando el material de la asignatura...
"%SSGIT%" -C "%SSROOT%" pull --ff-only
if errorlevel 1 goto :AvisoPull
echo   [ OK ] Material al dia.
goto :RamaOk
:AvisoPull
echo   [AVISO] No se ha podido actualizar. Sin red, o ha modificado archivos del
echo           material (es de solo lectura). Su carpeta %SS_PRIV% no se ve
echo           afectada en ningun caso.
set "SS_AVISOS=1"
goto :RamaOk

:RamaNo
echo   [AVISO] No se ha podido cambiar a la rama "%SS_RAMA%". Sigue en "%RAMA%",
echo           asi que el material puede no ser el de este curso.
echo           Suele deberse a modificaciones locales en archivos de _pub, que es
echo           de solo lectura. Avise al profesor.
set "SS_AVISOS=1"
goto :RevisarRuta

:RamaOk
echo   [ OK ] Rama del curso: %RAMA%

:RevisarRuta
echo "%SSROOT%" | find /i "OneDrive" >nul
if errorlevel 1 goto :SinOneDrive
echo   [AVISO] SSRoot esta dentro de OneDrive: puede provocar conflictos de
echo           sincronizacion y compilaciones lentas. Preferible C:\SSRoot.
set "SS_AVISOS=1"
:SinOneDrive
if "%SSROOT%"=="%SSROOT: =%" goto :SinEspacios
echo   [AVISO] La ruta de SSRoot contiene espacios. Funciona, pero algunas
echo           herramientas de compilacion dan problemas.
set "SS_AVISOS=1"
:SinEspacios

if exist "%SSROOT%\_pub\_comm\inc\" goto :CommOk
echo   [AVISO] No existe %SSROOT%\_pub\_comm\inc
echo           AAComm apuntaria a una carpeta incompleta. Avise al profesor.
set "SS_AVISOS=1"
:CommOk
echo.

rem =============== 3. Carpeta de trabajo del alumno ===========================
echo --- 3. Carpeta de trabajo %SS_PRIV% ----------------------------------------
set "PRIVDIR=%SSROOT%\%SS_PRIV%"
if exist "%PRIVDIR%\" goto :PrivExiste
mkdir "%PRIVDIR%" 2>nul
if not exist "%PRIVDIR%\" goto :ErrorMkdirPriv
echo   [ OK ] Carpeta creada: %PRIVDIR%
goto :PrivGit
:PrivExiste
echo   [ OK ] Ya existe: %PRIVDIR%

:PrivGit
"%SSGIT%" -C "%SSROOT%" check-ignore -q "%SS_PRIV%"
if errorlevel 1 echo   [AVISO] %SS_PRIV% NO esta ignorada por el repositorio. Falta la linea "%SS_PRIV%/" en el .gitignore de SSRoot: avise al profesor.
if exist "%PRIVDIR%\.git" goto :PrivGitYa

echo   Creando su repositorio Git local...
"%SSGIT%" -C "%PRIVDIR%" init -b main >nul 2>&1
if not exist "%PRIVDIR%\.git" "%SSGIT%" -C "%PRIVDIR%" init >nul 2>&1
if not exist "%PRIVDIR%\.git" goto :ErrorInitPriv
echo   [ OK ] Repositorio local creado. No tiene servidor: todo queda en este PC.
goto :PrivIgnore

:PrivGitYa
echo   [ OK ] %SS_PRIV% ya es un repositorio Git.

:PrivIgnore
if exist "%PRIVDIR%\.gitignore" goto :Uvus
set "GI=%PRIVDIR%\.gitignore"
>"%GI%"  echo # Carpeta de trabajo de Software de Sistemas. Generado por SS010-Init.bat.
>>"%GI%" echo # Se ignora la salida de compilacion: ocupa mucho y se regenera sola.
>>"%GI%" echo .vs/
>>"%GI%" echo ipch/
>>"%GI%" echo [Dd]ebug/
>>"%GI%" echo [Rr]elease/
>>"%GI%" echo x64/
>>"%GI%" echo x86/
>>"%GI%" echo Win32/
>>"%GI%" echo *.obj
>>"%GI%" echo *.pdb
>>"%GI%" echo *.ilk
>>"%GI%" echo *.tlog
>>"%GI%" echo *.lastbuildstate
>>"%GI%" echo # Configuracion propia de este equipo: no debe viajar en las copias.
>>"%GI%" echo SS-Config.cmd
>>"%GI%" echo # NO se ignora *.vcxproj.user: la Practica 0 lo estudia y guarda las
>>"%GI%" echo # opciones de depuracion (Anexo F).
echo   [ OK ] .gitignore de %SS_PRIV% creado.

rem ============================== 4. UVUS =====================================
:Uvus
echo.
echo --- 4. Identificacion ------------------------------------------------------
set "SS_UVUS="
set "SS_DEST="
if exist "%PRIVDIR%\SS-Config.cmd" call "%PRIVDIR%\SS-Config.cmd" >nul 2>&1
if defined SS_UVUS echo   UVUS guardado de una ejecucion anterior: %SS_UVUS%
if defined SS_UVUS goto :UvusListo
:PedirUvus
set "SSANS="
set /p "SSANS=  Escriba su UVUS (sin @us.es) y pulse ENTRAR: "
if not defined SSANS goto :PedirUvus
set SS_UVUS=%SSANS:"=%
set "SS_UVUS=%SS_UVUS: =%"
if not defined SS_UVUS goto :PedirUvus
:UvusListo

set "IDNAME="
for /f "usebackq delims=" %%N in (`"%SSGIT%" -C "%PRIVDIR%" config --get user.name 2^>nul`) do set "IDNAME=%%N"
if defined IDNAME goto :PrimerCommit
"%SSGIT%" -C "%PRIVDIR%" config --local user.name "%SS_UVUS%"
"%SSGIT%" -C "%PRIVDIR%" config --local user.email "%SS_UVUS%@alum.us.es"
echo   [ OK ] Identidad Git de %SS_PRIV%: %SS_UVUS% ^<%SS_UVUS%@alum.us.es^>

:PrimerCommit
"%SSGIT%" -C "%PRIVDIR%" rev-parse --verify HEAD >nul 2>&1
if not errorlevel 1 goto :PrivListo
"%SSGIT%" -C "%PRIVDIR%" add -A >nul 2>&1
"%SSGIT%" -C "%PRIVDIR%" commit -m "Inicio del curso: carpeta de trabajo creada por SS010-Init.bat" >nul 2>&1
if errorlevel 1 echo   [AVISO] No se ha podido hacer la primera revision. Hagala con TortoiseGit.
if not errorlevel 1 echo   [ OK ] Primera revision (commit) hecha.
:PrivListo
echo.

rem ================= 5. Variable de entorno AAComm ============================
echo --- 5. Variable de entorno AAComm -----------------------------------------
set "AACOMM_DIR=%SSROOT%\_pub\_comm"
rem  La doble barra final es intencionada: hace que setx almacene el valor
rem  terminado en UNA barra, que es lo que necesitan los proyectos, porque la
rem  concatenan sin separador:  $(AAComm)inc
setx AAComm "%AACOMM_DIR%\\" >nul 2>&1
if errorlevel 1 goto :ErrorSetx
set "AAComm=%AACOMM_DIR%\"
echo   [ OK ] AAComm = %AACOMM_DIR%\
echo   [ !! ] Si Visual Studio estaba abierto, cierrelo y vuelva a abrirlo:
echo          los programas ya en marcha no ven el nuevo valor.
goto :AACommListo
:ErrorSetx
echo   [ERROR] No se ha podido fijar AAComm con setx.
set "SS_AVISOS=1"
:AACommListo
echo.

rem ================= 6. Guardar configuracion =================================
echo --- 6. Guardando configuracion --------------------------------------------
set "CFG=%PRIVDIR%\SS-Config.cmd"
>"%CFG%"  echo @rem Configuracion local generada por los scripts de Software de Sistemas.
>>"%CFG%" echo @rem Especifica de este equipo. Ignorada por Git: no viaja en las copias.
>>"%CFG%" echo set "SS_CURSO=%SS_CURSO%"
>>"%CFG%" echo set "SS_RAMA=%SS_RAMA%"
>>"%CFG%" echo set "SS_UVUS=%SS_UVUS%"
if defined SS_DEST >>"%CFG%" echo set "SS_DEST=%SS_DEST%"
if exist "%CFG%" echo   [ OK ] %CFG%
echo.

rem ============================== Resumen =====================================
echo =================================================================
if "%SS_AVISOS%"=="0" echo  INSTALACION TERMINADA, SIN AVISOS
if "%SS_AVISOS%"=="1" echo  INSTALACION TERMINADA, CON AVISOS (lea el texto anterior)
echo =================================================================
echo.
echo   SSRoot        : %SSROOT%
echo   No modificar  : %SSROOT%\_pub
echo   Scripts       : %SSROOT%\%SS_UTILS%
echo   Su trabajo    : %PRIVDIR%
echo   AAComm        : %AACOMM_DIR%\
echo.
echo   A PARTIR DE AHORA use los scripts de %SSROOT%\%SS_UTILS%
echo   Ya puede borrar la copia que descargo de este archivo.
echo.
set "RESP="
set /p "RESP=  Abrir la carpeta de scripts? (S/N) [S]: "
if /i not "%RESP%"=="N" if exist "%SSROOT%\%SS_UTILS%\" start "" explorer "%SSROOT%\%SS_UTILS%"
goto :Fin

rem =============================================================================
:ErrorNoVacia
echo.
echo   [ERROR] La carpeta ya existe y tiene contenido, pero no es un
echo           repositorio Git:
echo             %SSROOT%
echo           git no puede clonar sobre una carpeta con contenido.
echo           Elija otra ruta, o vacie esa carpeta.
echo.
goto :PedirRuta

:ErrorClonar
echo.
echo   [ERROR] No se ha podido clonar el repositorio.
echo           Compruebe:
echo             - la conexion de red,
echo             - que existe la rama %SS_RAMA% en
echo               %URL_REPO%
echo             - y que tiene permiso de escritura en la ruta elegida.
goto :Fin

:ErrorMkdirPriv
echo   [ERROR] No se ha podido crear %PRIVDIR%
echo           Compruebe que tiene permiso de escritura en %SSROOT%
goto :Fin

:ErrorInitPriv
echo   [ERROR] No se ha podido crear el repositorio Git de %SS_PRIV%.
goto :Fin

rem =============================================================================
rem  :AsegurarRama
rem  Deja el repositorio en la rama del curso. NO PREGUNTA NADA: si esta en otra
rem  rama, cambia a la del curso. Deja en %RAMA% la rama en la que ha quedado.
rem  Devuelve errorlevel 1 si no ha podido cambiar (p.ej. hay modificaciones
rem  locales en archivos del material).
rem =============================================================================
:AsegurarRama
call :LeeRama
if /i "%RAMA%"=="%SS_RAMA%" exit /b 0
echo   [ .. ] El repositorio esta en la rama "%RAMA%". Se cambia a "%SS_RAMA%".
"%SSGIT%" -C "%SSROOT%" fetch --prune origin >nul 2>&1
"%SSGIT%" -C "%SSROOT%" switch "%SS_RAMA%" 2>nul
if errorlevel 1 "%SSGIT%" -C "%SSROOT%" checkout "%SS_RAMA%" 2>nul
call :LeeRama
if /i not "%RAMA%"=="%SS_RAMA%" exit /b 1
echo   [ OK ] Cambiado a la rama %SS_RAMA%.
exit /b 0

rem =============================================================================
rem  :LeeRama   Deja en %RAMA% la rama actual del repositorio.
rem =============================================================================
:LeeRama
set "RAMA="
for /f "usebackq delims=" %%B in (`"%SSGIT%" -C "%SSROOT%" branch --show-current 2^>nul`) do set "RAMA=%%B"
if not defined RAMA set "RAMA=(no se ha podido leer)"
exit /b 0

rem =============================================================================
rem  :LocalizarSSRoot
rem  Busca SSRoot hacia arriba desde la carpeta del script: la primera carpeta
rem  que contenga .git y _pub. Sirve para detectar que el script se esta
rem  ejecutando desde _pub\_utils\ de una instalacion ya hecha.
rem  Deja SSROOT sin definir si no lo encuentra.
rem =============================================================================
:LocalizarSSRoot
set "SSROOT="
set "D=%SCRIPTDIR%"
for /l %%L in (1,1,5) do call :SubirNivel
exit /b 0
:SubirNivel
if defined SSROOT exit /b 0
if not defined D exit /b 0
if not exist "%D%\.git" goto :Arriba
if not exist "%D%\_pub\" goto :Arriba
set "SSROOT=%D%"
exit /b 0
:Arriba
set "PADRE="
for %%I in ("%D%") do set "PADRE=%%~dpI"
if not defined PADRE goto :FinNivel
if "%PADRE:~-1%"=="\" set "PADRE=%PADRE:~0,-1%"
if /i "%PADRE%"=="%D%" goto :FinNivel
set "D=%PADRE%"
exit /b 0
:FinNivel
set "D="
exit /b 0

rem =============================================================================
:FindGit
set "SSGIT="
set "SSGIT_INPATH=0"
set "PF86=%ProgramFiles(x86)%"
if not defined PF86 set "PF86=%ProgramFiles%"
for /f "delims=" %%G in ('where git.exe 2^>nul') do if not defined SSGIT set "SSGIT=%%G"
if not defined SSGIT goto :BuscarGitDisco
set "SSGIT_INPATH=1"
exit /b 0
:BuscarGitDisco
if exist "%ProgramFiles%\Git\cmd\git.exe" set "SSGIT=%ProgramFiles%\Git\cmd\git.exe"
if defined SSGIT exit /b 0
if exist "%PF86%\Git\cmd\git.exe" set "SSGIT=%PF86%\Git\cmd\git.exe"
if defined SSGIT exit /b 0
if exist "%LocalAppData%\Programs\Git\cmd\git.exe" set "SSGIT=%LocalAppData%\Programs\Git\cmd\git.exe"
if defined SSGIT exit /b 0
set "VSWHERE=%PF86%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" set "VSWHERE=%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :NoHayGit
for /f "usebackq delims=" %%P in (`"%VSWHERE%" -latest -products * -property installationPath 2^>nul`) do call :ProbarGitVS "%%P"
if defined SSGIT exit /b 0
:NoHayGit
echo   [ERROR] No se ha encontrado git.exe en este equipo.
echo           Instale una de estas dos cosas:
echo             - Git para Windows: https://git-scm.com/download/win
echo             - o, en Visual Studio Installer, Componentes individuales,
echo               buscar "git" y marcar "Git para Windows" (Anexo A).
exit /b 1
:ProbarGitVS
if defined SSGIT exit /b 0
if exist "%~1\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer\Git\cmd\git.exe" set "SSGIT=%~1\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer\Git\cmd\git.exe"
exit /b 0

rem =============================================================================
:Fin
echo.
if defined SS_OLDCP chcp %SS_OLDCP% >nul 2>&1
pause
endlocal
exit /b 0
