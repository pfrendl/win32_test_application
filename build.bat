@echo off

if not exist .\build mkdir .\build
pushd .\build
g++ -o main.exe ..\main.cpp ..\graphics.cpp ..\datatypes.cpp ..\random.cpp ..\collision_detection.cpp ..\physics.cpp -luser32 -lgdi32
popd
