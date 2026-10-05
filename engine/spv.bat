@echo off
setlocal

rem spv.bat - Compile GLSL shader using glslc
rem Usage: spv.bat <shader_path> [output_path]

rem ANSI colours (Windows 10+ terminals)
for /f %%a in ('echo prompt $E ^| cmd') do set "ESC=%%a"
set "RED=%ESC%[0;31m"
set "GREEN=%ESC%[0;32m"
set "YELLOW=%ESC%[1;33m"
set "NC=%ESC%[0m"

if /i "%~1"=="--help" goto :help
if /i "%~1"=="-h" goto :help
if "%~1"=="" (
    echo %RED%Error: Shader path is required%NC%
    goto :usage_fail
)

set "SHADER_PATH=%~1"

if not exist "%SHADER_PATH%" (
    echo %RED%Error: Shader file not found: %SHADER_PATH%%NC%
    exit /b 1
)

where glslc >nul 2>nul
if errorlevel 1 (
    echo %RED%Error: glslc not found%NC%
    exit /b 1
)

rem Output path: second argument, or input path with .spv extension
if not "%~2"=="" (
    set "OUTPUT_PATH=%~2"
) else (
    set "OUTPUT_PATH=%~dpn1.spv"
)

rem Create the output directory if needed
for %%I in ("%OUTPUT_PATH%") do set "OUTPUT_DIR=%%~dpI"
if not exist "%OUTPUT_DIR%" (
    mkdir "%OUTPUT_DIR%"
    echo %YELLOW%Created directory: %OUTPUT_DIR%%NC%
)

rem Detect shader stage from extension (informational)
set "EXT=%~x1"
set "STAGE=unknown"
if /i "%EXT%"==".vert"  set "STAGE=vertex"
if /i "%EXT%"==".frag"  set "STAGE=fragment"
if /i "%EXT%"==".comp"  set "STAGE=compute"
if /i "%EXT%"==".geom"  set "STAGE=geometry"
if /i "%EXT%"==".tesc"  set "STAGE=tessellation control"
if /i "%EXT%"==".tese"  set "STAGE=tessellation evaluation"
if /i "%EXT%"==".mesh"  set "STAGE=mesh"
if /i "%EXT%"==".task"  set "STAGE=task"
if /i "%EXT%"==".rgen"  set "STAGE=ray generation"
if /i "%EXT%"==".rint"  set "STAGE=ray intersection"
if /i "%EXT%"==".rahit" set "STAGE=ray any-hit"
if /i "%EXT%"==".rchit" set "STAGE=ray closest-hit"
if /i "%EXT%"==".rmiss" set "STAGE=ray miss"
if /i "%EXT%"==".rcall" set "STAGE=ray callable"

echo %GREEN%Compiling shader:%NC% %SHADER_PATH%
echo %GREEN%Stage:%NC% %STAGE%
echo %GREEN%Output:%NC% %OUTPUT_PATH%

glslc --target-env=vulkan1.3 "%SHADER_PATH%" -o "%OUTPUT_PATH%"
if errorlevel 1 (
    echo %RED%Compilation failed%NC%
    exit /b 1
)

if not exist "%OUTPUT_PATH%" (
    echo %RED%Compilation failed: output file was not created%NC%
    exit /b 1
)

for %%I in ("%OUTPUT_PATH%") do set "SIZE=%%~zI"
echo %GREEN%Compilation successful%NC%
echo %GREEN%Output size:%NC% %SIZE% bytes

where spirv-dis >nul 2>nul
if not errorlevel 1 echo %YELLOW%View SPIR-V:%NC% spirv-dis "%OUTPUT_PATH%"

exit /b 0

:help
call :usage
exit /b 0

:usage_fail
call :usage
exit /b 1

:usage
echo Usage: %~nx0 ^<shader_path^> [output_path]
echo.
echo Arguments:
echo   shader_path    Path to the GLSL shader file
echo   output_path    Optional output path for the SPIR-V binary
echo                  Defaults to input filename with .spv extension
echo.
echo Examples:
echo   %~nx0 shader.vert
echo   %~nx0 shader.frag compiled\shader.frag.spv
echo   %~nx0 --help
exit /b 0