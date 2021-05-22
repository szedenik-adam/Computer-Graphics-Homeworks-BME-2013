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

## Usage
### 01 Angry Birds
Drag the red bird with the mouse from the slingshot to the left and try to hit the green bird.

**Keyboard controls**
- **S**: Toggle gravity of the green bird making hitting it easier.
- **E**: Toggle automatic vertical movement of the green bird.
- **R/D/F/G**: Manual movement of the green bird (up/down movement only allowed when the automatic movement is disabled).

![Angry Birds Gameplay](/extras/angrybirds.gif)

### 02 Splines
2D Catmull-Rom and Bezier Spline drawing based on user defined control points and weights.
Use the mouse to set the control points.
Clicking with the mouse sets the control points' location and the mouse click's duration sets their weights.

**Keyboard controls**
- **T**: Toggle animated point tracking.
- **Z/H**: Zoom in/out.
- **R/D/F/G**: Move camera.
- **C**: Clear screen.

![Splines Demo](/extras/splines.gif)

### 03 Ray Tracing
Calculates an image by tracing the path of light for each pixel.
Supports cylinder and paraboloid shapes and point light sources.
Surface types can be smooth (metallic, glass) or diffuse (plastic).
Renders very slowly since the CPU does the majority of the calculations (do not use this in production).

**Keyboard controls**
- **+**: Increases the glass' refraction coefficient by 0.1.
- **0**: Resets the glass' refraction coefficient to 1.
- **\<space\>**: Rerender scene.
- **W/A/S/D**: Move camera.

![Ray Tracing Scene](/extras/20131108_g3_remote1_hidef.png)
