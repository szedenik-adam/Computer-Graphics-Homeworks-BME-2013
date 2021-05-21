# Computer Graphics Homeworks (BME 2013)
Some multi-platform OpenGL C++ applications (2 of them are games).

## Build on Windows
1. Download or clone the repository.
2. (optional) Build external dependencies and copy them to the external_dependencies folder: **vcpkg install glew:x64-windows freeglut:x64-windows**
3. Open the solution (**ComputerGraphicsHomework.sln**) with Visual Studio.
4. **Build** menu > **Build Solution**.

## Build on Linux
1. Download or clone the repository.
```
git clone https://github.com/szedenik-adam/Computer-Graphics-Homeworks-BME-2013
```
2. Install external dependencies.
```
sudo apt install libglu1-mesa-dev freeglut3-dev
```
3. Step into the repository's directory.
```
cd Computer-Graphics-Homeworks-BME-2013
```
4. Compile with GCC.
```
g++ 01_AngryBirds/main.cpp -o AngryBirds -lglut -lGLU -lGL
g++ 02_Splines/main.cpp -o Splines -lglut -lGLU -lGL
g++ 03_RayTracing/main.cpp -o RayTracing -lglut -lGLU -lGL
g++ 04_Stork/main.cpp -o Stork -lglut -lGLU -lGL
g++ 05_StorkGame/main.cpp -o StorkGame -lglut -lGLU -lGL
```
