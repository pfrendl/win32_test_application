@echo off

if not exist .\build mkdir .\build
pushd .\build
nvcc -o test.exe ..\test.cpp ..\datatypes.cpp ..\collision_detection.cpp ..\random.cpp user32.lib gdi32.lib
popd
