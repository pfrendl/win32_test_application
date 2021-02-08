@echo off

if not exist .\build mkdir .\build
pushd .\build
g++ -o test.exe ..\test.cpp ..\datatypes.cpp ..\collision_detection.cpp ..\random.cpp -luser32 -lgdi32
popd
