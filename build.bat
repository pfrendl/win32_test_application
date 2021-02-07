@echo off

if not exist .\build mkdir .\build
pushd .\build
nvcc -o main.exe ..\main.cpp ..\graphics.cpp ..\datatypes.cpp ..\random.cpp ..\collision_detection.cpp user32.lib gdi32.lib
popd
