@echo off
rem =============================================================================
rem  SS030-Restaurar.bat
rem  Software de Sistemas - 3o Ingenieria de Computadores - ETSII / Univ. Sevilla
rem  Curso 2026-27
rem
rem  QUE HACE
rem    Recupera su trabajo a partir de una copia creada con SS020-Guardar.bat.
rem
rem    Acepta los dos formatos:
rem      .bundle  recupera TODO, incluido el historial de revisiones. Es el
rem               formato recomendado.
rem      .zip     recupera solo los archivos, sin historial. Sirve como
rem               ultimo recurso si ha perdido el .bundle.
rem
rem    Uso normal: llega a un equipo nuevo (o recien borrado), ejecuta
rem    SS010-Init.bat y despues este script, indicando donde tiene la copia
rem    (pendrive, OneDrive...).
rem
rem  SEGURIDAD
rem    Si ya tiene trabajo en _priv, este script NO lo sobrescribe: restaura en
rem    una carpeta aparte, _priv-restaurado-<fecha>, y le explica como comparar.
rem    Nunca se pierde nada.
rem
rem  UBICACION
rem    SSRoot\_pub\_utils\ , o cualquier subcarpeta de SSRoot: busca SSRoot
rem    hacia arriba.
rem =============================================================================

setlocal EnableExtensions DisableDelayedExpansion
for /f "tokens=2 delims=:." %%c in ('chcp 2^>nul') do set "SS_OLDCP=%%c"
if defined SS_OLDCP set "SS_OLDCP=%SS_OLDCP: =%"
chcp 65001 >nul 2>&1
title Software de Sistemas - Restaurar mi trabajo

rem =============================================================================
set "SS_PRIV=_priv"
rem =============================================================================

set "SCRIPTDIR=%~dp0"
if "%SCRIPTDIR:~-1%"=="\" set "SCRIPTDIR=%SCRIPTDIR:~0,-1%"

echo ==============================================================
echo  Software de Sistemas - Restaurar mi trabajo
echo ==============================================================
echo.

call :LocalizarSSRoot
if defined SSROOT goto :HaySSRoot
echo [ERROR] No se encuentra SSRoot a partir de:
echo         %SCRIPTDIR%
echo         Ejecute antes SS010-Init.bat.
goto :Fin
:HaySSRoot
set "PRIVDIR=%SSROOT%\%SS_PRIV%"
echo   SSRoot: %SSROOT%
echo.

call :FindGit
if errorlevel 1 goto :Fin

set "SS_UVUS="
set "SS_DEST="
if exist "%PRIVDIR%\SS-Config.cmd" call "%PRIVDIR%\SS-Config.cmd" >nul 2>&1

rem ===================== 1. Localizar el archivo de copia =====================
echo --- 1. Donde esta su copia -------------------------------------------------
echo   Indique la CARPETA donde tiene la copia (pendrive, OneDrive...) o la
echo   ruta completa del propio archivo.
if defined SS_DEST echo   Ultima carpeta usada al guardar: %SS_DEST%
echo.
:PedirRuta
set "SSANS="
if defined SS_DEST set /p "SSANS=  Carpeta o archivo (ENTRAR = la ultima usada): "
if not defined SS_DEST set /p "SSANS=  Carpeta o archivo: "
if defined SSANS goto :RutaEscrita
if defined SS_DEST set "RUTA=%SS_DEST%"
if defined RUTA goto :RutaListaOk
echo   Debe escribir algo.
goto :PedirRuta
:RutaEscrita
set RUTA=%SSANS:"=%
:RutaListaOk
if "%RUTA:~-1%"=="\" set "RUTA=%RUTA:~0,-1%"

set "COPIA="
if exist "%RUTA%\" goto :EsCarpeta
if exist "%RUTA%" set "COPIA=%RUTA%"
if defined COPIA goto :CopiaElegida
echo   [ERROR] No existe: %RUTA%
goto :PedirRuta

:EsCarpeta
echo.
echo   Copias encontradas en %RUTA% (la mas reciente primero):
echo.
set "N=0"
for /f "delims=" %%F in ('dir /b /o-n "%RUTA%\SS-*.bundle" 2^>nul') do call :Listar "%RUTA%\%%F"
if not "%N%"=="0" goto :HayCopias
echo   No hay ningun .bundle. Buscando archivos .zip...
echo.
for /f "delims=" %%F in ('dir /b /o-n "%RUTA%\SS-*.zip" 2^>nul') do call :Listar "%RUTA%\%%F"
if "%N%"=="0" goto :SinCopias
echo.
echo   [AVISO] Solo hay copias en zip: se recuperaran los archivos, pero NO el
echo           historial de revisiones.

:HayCopias
echo.
set "SSANS="
set /p "SSANS=  ENTRAR para usar la mas reciente, o escriba el nombre del archivo: "
if not defined SSANS goto :CopiaElegida
set NOMBRE=%SSANS:"=%
set "COPIA=%RUTA%\%NOMBRE%"
if exist "%COPIA%" goto :CopiaElegida
if exist "%NOMBRE%" set "COPIA=%NOMBRE%"
if exist "%COPIA%" goto :CopiaElegida
echo   [ERROR] No existe ese archivo.
goto :PedirRuta

:SinCopias
echo   [ERROR] No hay ningun archivo SS-*.bundle ni SS-*.zip en esa carpeta.
echo           Compruebe la ruta, o escriba la ruta completa del archivo.
goto :PedirRuta

:CopiaElegida
echo.
echo   Archivo elegido: %COPIA%
set "TIPO=BUNDLE"
for %%F in ("%COPIA%") do if /i "%%~xF"==".zip" set "TIPO=ZIP"
echo   Formato        : %TIPO%
echo.

rem ========================= 2. Elegir el destino =============================
echo --- 2. Preparando la carpeta de destino ------------------------------------
set "DESTDIR=%PRIVDIR%"
set "APARTE=0"
if not exist "%PRIVDIR%\" goto :DestinoListo
set "VACIA=1"
for /f "delims=" %%X in ('dir /b /a "%PRIVDIR%" 2^>nul') do set "VACIA=0"
if "%VACIA%"=="1" goto :BorrarVacia

set "STAMP="
for /f "usebackq delims=" %%T in (`powershell -NoProfile -Command "Get-Date -Format yyyyMMdd-HHmm" 2^>nul`) do set "STAMP=%%T"
if not defined STAMP set "STAMP=sinfecha-%RANDOM%"
set "DESTDIR=%SSROOT%\%SS_PRIV%-restaurado-%STAMP%"
set "APARTE=1"
echo   [AVISO] Ya hay trabajo en %SS_PRIV%. No se toca.
echo           La copia se restaurara en una carpeta aparte:
echo           %DESTDIR%
goto :DestinoListo

:BorrarVacia
rmdir "%PRIVDIR%" 2>nul
if exist "%PRIVDIR%\" set "DESTDIR=%SSROOT%\%SS_PRIV%-restaurado"

:DestinoListo
echo.
echo --- 3. Restaurando ---------------------------------------------------------
if "%TIPO%"=="ZIP" goto :RestaurarZip

rem --- Restaurar desde .bundle ---
"%SSGIT%" -C "%SSROOT%" bundle verify "%COPIA%"
if errorlevel 1 goto :ErrorVerify
echo   [ OK ] Es un bundle valido y contiene un historial completo.
echo.
"%SSGIT%" clone "%COPIA%" "%DESTDIR%"
if errorlevel 1 goto :ErrorRestaurar
if not exist "%DESTDIR%\.git" goto :ErrorRestaurar
rem El origen apuntaria al fichero .bundle, que puede desaparecer. Se elimina
rem para que el repositorio quede como lo que es: local y sin servidor.
"%SSGIT%" -C "%DESTDIR%" remote remove origin >nul 2>&1
goto :Identidad

rem --- Restaurar desde .zip ---
:RestaurarZip
echo   Extrayendo el zip...
if not exist "%DESTDIR%\" mkdir "%DESTDIR%" 2>nul
if not exist "%DESTDIR%\" goto :ErrorRestaurar
powershell -NoProfile -Command "Expand-Archive -LiteralPath '%COPIA%' -DestinationPath '%DESTDIR%' -Force"
if errorlevel 1 goto :ErrorRestaurar
echo   [ OK ] Archivos extraidos.
echo   Creando un repositorio nuevo (el zip no lleva historial)...
"%SSGIT%" -C "%DESTDIR%" init -b main >nul 2>&1
if not exist "%DESTDIR%\.git" "%SSGIT%" -C "%DESTDIR%" init >nul 2>&1
if not exist "%DESTDIR%\.git" goto :ErrorRestaurar

:Identidad
if not defined SS_UVUS goto :SinIdentidad
"%SSGIT%" -C "%DESTDIR%" config --local user.name "%SS_UVUS%"
"%SSGIT%" -C "%DESTDIR%" config --local user.email "%SS_UVUS%@alum.us.es"
:SinIdentidad
if "%TIPO%"=="BUNDLE" goto :Informe
"%SSGIT%" -C "%DESTDIR%" add -A >nul 2>&1
"%SSGIT%" -C "%DESTDIR%" commit -m "Restaurado desde zip por SS030-Restaurar.bat" >nul 2>&1

:Informe
echo.
echo =================================================================
echo  TRABAJO RESTAURADO
echo =================================================================
echo.
echo   Carpeta : %DESTDIR%
echo.
"%SSGIT%" -C "%DESTDIR%" --no-pager log --oneline -5
echo.
if "%APARTE%"=="0" goto :FinOk

echo   ATENCION: se ha restaurado FUERA de %SS_PRIV% para no perder lo que ya
echo   tenia. Ahora tiene dos carpetas:
echo       %PRIVDIR%
echo           lo que habia en este equipo
echo       %DESTDIR%
echo           lo que venia en la copia
echo.
echo   Revise las dos, quedese con la que quiera y renombrela a %SS_PRIV%. Si no
echo   sabe cual conservar, no borre ninguna y pregunte al profesor.
echo.
echo   Cuando termine, ejecute SS010-Init.bat otra vez para rehacer la
echo   configuracion.
goto :Fin

:FinOk
echo   Ya puede seguir trabajando. Recuerde ejecutar SS020-Guardar.bat al
echo   terminar la sesion.
goto :Fin

rem =============================================================================
:Listar
set /a N+=1
echo     %N%. %~nx1   (%~z1 bytes)
if "%N%"=="1" set "COPIA=%~1"
exit /b 0

:ErrorVerify
echo.
echo [ERROR] El archivo no es un bundle valido o esta corrupto:
echo         %COPIA%
echo         Pruebe con otra copia: por eso SS020-Guardar.bat conserva las
echo         anteriores, con la fecha en el nombre.
goto :Fin

:ErrorRestaurar
echo.
echo [ERROR] No se ha podido restaurar en:
echo         %DESTDIR%
echo         Compruebe que esa carpeta no existe o esta vacia, y que tiene
echo         permiso de escritura.
goto :Fin

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
set "PF86=%ProgramFiles(x86)%"
if not defined PF86 set "PF86=%ProgramFiles%"
for /f "delims=" %%G in ('where git.exe 2^>nul') do if not defined SSGIT set "SSGIT=%%G"
if defined SSGIT exit /b 0
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
echo [ERROR] No se ha encontrado git.exe. Vea el Anexo A.
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