@echo off

mkdir build
pushd build
cl -I ..\code -I ../deps/include -FC -Zi ..\code\game.cpp user32.lib gdi32.lib Shell32.lib SDL2.lib SDL2main.lib SDL2_ttf.lib /link /LIBPATH:../deps/lib  /SUBSYSTEM:CONSOLE
popd