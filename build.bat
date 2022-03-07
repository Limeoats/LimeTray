windres Limetray/Resource.rc -O coff -o build/icon.res
clang -Wall -o build/limetray.exe Limetray/src/limetray.c build/icon.res -lgdi32 -luser32 -lshell32 -lcomctl32
