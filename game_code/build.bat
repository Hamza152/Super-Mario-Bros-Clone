@echo off

pushd ..\build\
cl -Zi ..\game_code\main.cpp user32.lib gdi32.lib Winmm.lib
popd