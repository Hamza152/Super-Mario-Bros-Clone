@echo off

pushd ..\build\
cl -Zi ..\editor_code\MarioEditor.cpp user32.lib gdi32.lib Winmm.lib
popd