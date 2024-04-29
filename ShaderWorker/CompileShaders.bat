@echo off
set local enableextensions

set OUT=%1\shaders
set GLSLCCMD=%CD%\shaders\glslc.cmd
set scriptpath=%~dp0

if not exist "%OUT%" mkdir "%OUT%"
echo "%OUT%"

for %%d in (%scriptpath%TEMP) do (
	echo "%%d"
	pushd "%%d"
	for %%g in (.comp, .rgen, .rmiss, .rchit, .rint) do (
	forfiles /m *%%g /c "cmd /c %GLSLCCMD% %VK_SDK_PATH% @PATH %OUT%\ComputeShaders"
	)
	popd
)

