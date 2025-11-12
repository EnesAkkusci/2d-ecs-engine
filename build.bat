@echo off
setlocal

rem Root directory
set ROOT=%~dp0

if "%~1"==""        goto :help
if "%1"=="help"     goto :help
if "%1"=="lsp"      goto :lsp
if "%1"=="compile"  goto :compile
if "%1"=="run"      goto :run
if "%1"=="clean"    goto :clean
if "%1"=="restart"  goto :restart
if "%1"=="test"     goto :test
if "%1"=="release"  goto :release
if "%1"=="play"     goto :play
if "%1"=="init"     goto :init
goto :eof

:restart
call :clean
call :init
call :compile
call :run
goto :eof

:test
call :lsp
call :compile
call :run
goto :eof

:init
if not exist "%ROOT%build" (
	mkdir "%ROOT%build"
)
pushd "%ROOT%build"
cmake ..
popd
call :lsp
goto :eof

:lsp
rem Generating the compile_commands file for clangd LSP
if not exist "%ROOT%make" (
	mkdir "%ROOT%make"
)
pushd "%ROOT%make"
cmake -G "Unix Makefiles" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
move /Y compile_commands.json "%ROOT%"
popd
goto :eof

:compile
pushd "%ROOT%build"
cmake --build . --config Debug
popd
goto :eof

:run
pushd "%ROOT%"
".\build\Debug\main.exe"
popd
goto :eof

:clean
pushd "%ROOT%"
if exist compile_commands.json del /Q compile_commands.json
if exist build rmdir /S /Q build
mkdir build
if exist make rmdir /S /Q make
mkdir make
if exist GAME rmdir /S /Q GAME
popd
goto :eof

:release
pushd "%ROOT%build"
cmake --build . --config Release
popd
goto :eof

:play
call :release
pushd "%ROOT%build\Release"
".\main.exe"
popd
goto :eof

:help
echo Usage: %~n0 ^<command^>
echo.
echo Commands:
echo   lsp       Generate compile_commands.json
echo   compile   Compile the Debug version
echo   run       Run Debug executable
echo   clean     Remove build/make/GAME dirs and compile_commands.json
echo   restart   clean + init + compile + run
echo   test      lsp + compile + run
echo   release   Compile the Release version
echo   play      Run the Release executable
echo   init      Initialize the build system and generate compile_commands.json
goto :eof
