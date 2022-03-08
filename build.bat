@REM @echo off

@REM set DEBUG=1
@REM set "PROJECT_NAME=Limetray"

@REM if not exist "./build" mkdir build
@REM pushd build
@REM del . /F /Q

@REM windres ../Limetray/Resource.rc -O coff -o icon.res

@REM set "FLAGS=-o"
@REM set "DEBUG_FLAGS=-g -O0"

@REM if %DEBUG% equ 1 (
@REM     set "FLAGS=%DEBUG_FLAGS% %FLAGS%"
@REM )

@REM echo Building %PROJECT_NAME%...
@REM REM clang -Wall %FLAGS% limetray.exe ../Limetray/src/*.c icon.res -lgdi32 -luser32 -lshell32 -lcomctl32
@REM gcc -Wall %FLAGS% limetray.exe ../Limetray/src/*.c icon.res -lgdi32 -luser32 -lshell32 -lcomctl32
@REM echo Build completed

@echo off
pushd build
make
popd