
@echo off
cls
pushd build

set files=../main.cpp ../game.cpp ../math.cpp
set opt=-g -lkernel32 -luser32 -lgdi32
gcc %files% -o evolution.exe %opt%

echo "Finished building"
popd