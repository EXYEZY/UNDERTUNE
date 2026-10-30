@echo off
setlocal
where cmake >nul 2>nul
if errorlevel 1 (
  echo ERROR: CMake no esta instalado o no esta en PATH.
  echo Instala CMake 3.22 o superior y Visual Studio 2022 con C++.
  pause
  exit /b 1
)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto fail
cmake --build build --config Release --target UNDERTUNE_VST3
if errorlevel 1 goto fail
echo.
echo Compilacion terminada. Busca UNDERTUNE.vst3 dentro de build\UNDERTUNE_artefacts\Release\VST3\
pause
exit /b 0
:fail
echo.
echo La compilacion fallo. Revisa que Visual Studio 2022 C++ este instalado y que tengas internet para descargar JUCE.
pause
exit /b 1
