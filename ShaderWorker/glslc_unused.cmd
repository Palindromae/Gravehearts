@echo off
set VULKAN_DIR=%1
@rem %~2 ==> '~' removes speech marks that surround the file path
set FILE=%~2
@rem https://stackoverflow.com/questions/3215501/batch-remove-file-extension
set FILE_NO_EXT=%~n2
set OUT=%3

if not exist %OUT% mkdir %OUT%

rem echo %VULKAN_DIR%
rem echo %FILE%
rem echo %FILE_NO_EXT%
rem echo %OUT%


if not exist %OUT% md %OUT%

echo %VULKAN_DIR%\Bin\glslc.exe %FILE%  --target-env vulkan1.3 -o %OUT%\%FILE_NO_EXT%.spv
%VULKAN_DIR%\Bin\glslc.exe %FILE% -o %OUT%\%FILE_NO_EXT%.spv
