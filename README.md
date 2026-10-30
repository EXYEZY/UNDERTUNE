# UNDERTUNE VST3 (Windows x64)

Proyecto fuente de un prototipo independiente de corrección vocal para FL Studio.

## Incluye
- VST3 de efecto para audio mono/estéreo.
- Modos FEID (respuesta suavizada, objetivo equivalente aproximado a 20 ms) y under (corrección agresiva, objetivo de 0 ms).
- Selector de 12 tonalidades y escalas Major/Minor.
- Bypass y estado de parámetros guardado en el proyecto.
- Interfaz personalizada en negro, morado y rosa, con ilustración de Marin Kitagawa integrada como recurso gráfico.
- CMake + JUCE 8.0.6; compilación en Windows mediante `build_windows.bat`.

## Compilar
Instala Visual Studio 2022 con “Desktop development with C++”, CMake 3.22+ y conexión a internet para que CMake descargue JUCE. Ejecuta `build_windows.bat`. Si compila correctamente, localiza `UNDERTUNE.vst3` dentro de `build/UNDERTUNE_artefacts/Release/VST3/`.

## Instalar
Copia la carpeta completa `UNDERTUNE.vst3` a `C:\Program Files\Common Files\VST3\` y en FL Studio usa `Options > Manage plugins > Find installed plugins`.

## Limitaciones
Es un prototipo experimental, no afiliado a Antares y no usa su código propietario. La detección de pitch y el desplazamiento granular pueden producir artefactos; no se garantiza equivalencia sonora con Auto-Tune Classic. Debe compilarse y probarse en Windows antes de considerarlo listo para producción.
