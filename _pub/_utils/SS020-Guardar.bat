@echo off
rem =============================================================================
rem  SS020-Guardar.bat
rem  Software de Sistemas - 3o Ingenieria de Computadores - ETSII / Univ. Sevilla
rem  Curso 2026-27
rem
rem  QUE HACE
rem    Guarda su trabajo de la carpeta _priv en DOS archivos:
rem
rem      SS-<uvus>-<aaaammdd>-<hhmm>.bundle
rem          Copia completa, con todo el historial de revisiones. Es la que se
rem          usa para recuperar el trabajo, con SS030-Restaurar.bat.
rem
rem      SS-<uvus>-<aaaammdd>-<hhmm>.zip
rem          Los mismos archivos en un zip normal, que puede abrir con un doble
rem          clic para comprobar que esta todo, o entregar donde se le pida.
rem          No lleva historial.
rem
rem    Los dos salen de Git, asi que NO incluyen la salida de compilacion:
rem    Debug\, Release\, x64\, .vs\, *.obj, *.pdb quedan fuera porque estan en
rem    el .gitignore de _priv. Por eso ocupan tan poco.
rem
rem    EJECUTELO AL TERMINAR CADA SESION DE LABORATORIO. Los equipos del aula se
rem    borran: si no se lleva los archivos, pierde el trabajo.
rem
rem  IMPORTANTE
rem    Solo se guarda el trabajo del que se ha hecho "commit". Si tiene cambios
rem    sin registrar, el script se ofrece a registrarlos antes.
rem
rem  UBICACION
rem    SSRoot\_pub\_utils\ , o cualquier subcarpeta de SSRoot: busca SSRoot
rem    hacia arriba.
rem =============================================================================

setlocal EnableExtensions DisableDelayedExpansion
for /f "tokens=2 delims=:." %%c in ('chcp 2^>nul') do set "SS_OLDCP=%%c"
if defined SS_OLDCP set "SS_OLDCP=%SS_OLDCP: =%"
chcp 65001 >nul 2>&1
title Software de Sistemas - Guardar mi trabajo

rem =============================================================================
set "SS_CURSO=2026-27"
set "SS_PRIV=_priv"
rem =============================================================================

set "SCRIPTDIR=%~dp0"
if "%SCRIPTDIR:~-1%"=="\" set "SCRIPTDIR=%SCRIPTDIR:~0,-1%"

echo ==============================================================
echo  Software de Sistemas - Guardar mi trabajo
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

if exist "%PRIVDIR%\.git" goto :HayPriv
echo [ERROR] No existe su repositorio de trabajo:
echo         %PRIVDIR%
echo         Ejecute antes SS010-Init.bat.
goto :Fin
:HayPriv

call :FindGit
if errorlevel 1 goto :Fin

set "SS_UVUS="
set "SS_DEST="
set "SS_CURSO_CFG="
if exist "%PRIVDIR%\SS-Config.cmd" call "%PRIVDIR%\SS-Config.cmd" >nul 2>&1
if defined SS_UVUS goto :UvusOk
:PedirUvus
set "SSANS="
set /p "SSANS=Escriba su UVUS (sin @us.es) y pulse ENTRAR: "
if not defined SSANS goto :PedirUvus
set SS_UVUS=%SSANS:"=%
set "SS_UVUS=%SS_UVUS: =%"
if not defined SS_UVUS goto :PedirUvus
:UvusOk

echo   SSRoot     : %SSROOT%
echo   Mi trabajo : %PRIVDIR%
echo   UVUS       : %SS_UVUS%
echo.

rem ================== 1. Registrar los cambios pendientes =====================
echo --- 1. Revisando su trabajo ------------------------------------------------
set "SUCIO="
for /f "usebackq delims=" %%S in (`"%SSGIT%" -C "%PRIVDIR%" status --porcelain 2^>nul`) do set "SUCIO=1"
if not defined SUCIO goto :NadaPendiente

echo   Tiene cambios sin registrar (sin commit). Las copias SOLO guardan lo
echo   registrado, asi que estos cambios NO se guardarian:
echo.
"%SSGIT%" -C "%PRIVDIR%" status --short
echo.
set "RESP="
set /p "RESP=  Registrarlos ahora en una revision automatica? (S/N) [S]: "
if /i "%RESP%"=="N" goto :SinRegistrar
"%SSGIT%" -C "%PRIVDIR%" add -A
"%SSGIT%" -C "%PRIVDIR%" commit -m "SS020-Guardar: copia de seguridad automatica" >nul 2>&1
if errorlevel 1 goto :ErrorCommit
echo   [ OK ] Cambios registrados.
goto :HayCommits
:SinRegistrar
echo   [AVISO] Se guardara solo el trabajo ya registrado. Lo demas se perdera.
goto :HayCommits
:ErrorCommit
echo   [AVISO] No se ha podido registrar automaticamente. Hagalo con TortoiseGit
echo           (Commit) y vuelva a ejecutar este script.
goto :HayCommits

:NadaPendiente
echo   [ OK ] No hay cambios pendientes de registrar.

:HayCommits
"%SSGIT%" -C "%PRIVDIR%" rev-parse --verify HEAD >nul 2>&1
if not errorlevel 1 goto :ConHistorial
echo.
echo [ERROR] Su repositorio de trabajo no tiene ninguna revision todavia: no hay
echo         nada que guardar. Cree algun archivo, haga un Commit y repita.
goto :Fin
:ConHistorial
echo.

rem ========================= 2. Carpeta de destino ============================
echo --- 2. Donde quiere guardar las copias -------------------------------------
echo   Escriba la carpeta de destino. Recomendado: su OneDrive o un pendrive,
echo   NO una carpeta de este equipo (los del aula se borran).
if defined SS_DEST echo   Ultima carpeta usada: %SS_DEST%
echo.
:PedirDest
set "SSANS="
if defined SS_DEST set /p "SSANS=  Carpeta de destino (ENTRAR = la ultima usada): "
if not defined SS_DEST set /p "SSANS=  Carpeta de destino: "
if defined SSANS goto :DestEscrito
if defined SS_DEST set "DEST=%SS_DEST%"
if defined DEST goto :DestListo
echo   Debe escribir una carpeta.
goto :PedirDest
:DestEscrito
set DEST=%SSANS:"=%
:DestListo
if "%DEST:~-1%"=="\" set "DEST=%DEST:~0,-1%"

if exist "%DEST%\" goto :DestExiste
echo.
echo   La carpeta %DEST% no existe.
set "RESP="
set /p "RESP=  Crearla? (S/N) [S]: "
if /i "%RESP%"=="N" goto :PedirDest
mkdir "%DEST%" 2>nul
if not exist "%DEST%\" goto :ErrorDest
:DestExiste
echo   [ OK ] Destino: %DEST%
echo.

rem ============================ 3. Crear las copias ===========================
set "STAMP="
for /f "usebackq delims=" %%T in (`powershell -NoProfile -Command "Get-Date -Format yyyyMMdd-HHmm" 2^>nul`) do set "STAMP=%%T"
if not defined STAMP set "STAMP=sinfecha-%RANDOM%"

set "BUNDLE=%DEST%\SS-%SS_UVUS%-%STAMP%.bundle"
set "ZIP=%DEST%\SS-%SS_UVUS%-%STAMP%.zip"

echo --- 3. Copia completa con historial (.bundle) ------------------------------
"%SSGIT%" -C "%PRIVDIR%" bundle create "%BUNDLE%" --all
if errorlevel 1 goto :ErrorBundle
if not exist "%BUNDLE%" goto :ErrorBundle
echo.
echo   Comprobando integridad...
"%SSGIT%" -C "%PRIVDIR%" bundle verify "%BUNDLE%"
if errorlevel 1 goto :ErrorVerify
echo.

echo --- 4. Copia legible en zip (sin historial) --------------------------------
rem  "git archive" empaqueta SOLO los archivos rastreados, asi que la salida de
rem  compilacion (Debug\, Release\, x64\, .vs\, *.obj, *.pdb) queda fuera sola:
rem  esta en el .gitignore de _priv. No hay que mantener lista de exclusiones.
"%SSGIT%" -C "%PRIVDIR%" archive --format=zip -o "%ZIP%" HEAD
if errorlevel 1 goto :ErrorZip
if not exist "%ZIP%" goto :ErrorZip
echo   [ OK ] zip creado.
echo.

set "SS_DEST=%DEST%"
call :GuardarConfig

set "TAMB="
set "TAMZ="
for %%F in ("%BUNDLE%") do set "TAMB=%%~zF"
for %%F in ("%ZIP%") do set "TAMZ=%%~zF"

echo =================================================================
echo  TRABAJO GUARDADO
echo =================================================================
echo.
echo   Con historial : %BUNDLE%
echo                   %TAMB% bytes
echo   Legible (zip) : %ZIP%
echo                   %TAMZ% bytes
echo.
echo   Ninguna de las dos incluye la salida de compilacion: se regenera al
echo   compilar, no hay que llevarsela.
echo.
echo   COMPRUEBELO: abra la carpeta de destino y confirme que los dos archivos
echo   estan ahi antes de irse del laboratorio.
echo.
echo   Para recuperar el trabajo en otro equipo: copie el archivo .bundle,
echo   ejecute SS010-Init.bat en el equipo nuevo y despues SS030-Restaurar.bat.
echo.
set "RESP="
set /p "RESP=  Abrir la carpeta de destino para comprobarlo? (S/N) [S]: "
if /i not "%RESP%"=="N" start "" explorer "%DEST%"
goto :Fin

rem =============================================================================
:ErrorDest
echo   [ERROR] No se ha podido crear la carpeta %DEST%
goto :Fin

:ErrorBundle
echo.
echo [ERROR] No se ha podido crear la copia con historial:
echo         %BUNDLE%
echo         Compruebe que tiene permiso de escritura en el destino y espacio
echo         libre suficiente.
goto :Fin

:ErrorVerify
echo.
echo [ERROR] El archivo .bundle se ha creado pero no pasa la comprobacion de
echo         integridad. NO se fie de esta copia: repita en otra carpeta.
goto :Fin

:ErrorZip
echo.
echo [AVISO] No se ha podido crear el zip, pero la copia con historial
echo         (.bundle) si esta creada y verificada:
echo         %BUNDLE%
echo         Con ese archivo puede recuperar todo su trabajo.
goto :Fin

rem =============================================================================
:GuardarConfig
set "CFG=%PRIVDIR%\SS-Config.cmd"
>"%CFG%"  echo @rem Configuracion local generada por los scripts de Software de Sistemas.
>>"%CFG%" echo @rem Especifica de este equipo. Ignorada por Git: no viaja en las copias.
>>"%CFG%" echo set "SS_CURSO_CFG=%SS_CURSO%"
>>"%CFG%" echo set "SS_UVUS=%SS_UVUS%"
if defined SS_DEST >>"%CFG%" echo set "SS_DEST=%SS_DEST%"
exit /b 0

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