# Computer Graphics Homeworks (BME 2013)
Some multi-platform OpenGL C++ applications (2 of them are games).

## Online Demo
[Angry Birds](https://szedenik-adam.github.io/Computer-Graphics-Homeworks-BME-2013/www/AngryBirds.html) | [Splines](https://szedenik-adam.github.io/Computer-Graphics-Homeworks-BME-2013/www/Splines.html) | [Stork Game](https://szedenik-adam.github.io/Computer-Graphics-Homeworks-BME-2013/www/StorkGame.html)
------------ | ------------- | -------------

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

## Build with emscripten on Linux
1. Install emsdk dependencies
```
sudo apt install python3 cmake git
```
2. Download emsdk
```
git clone https://github.com/emscripten-core/emsdk.git
```
3. Build emscripten
```
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
cd ..
```
4. Download repository
```
git clone https://github.com/szedenik-adam/Computer-Graphics-Homeworks-BME-2013
```
5. Build
```
cd Computer-Graphics-Homeworks-BME-2013/01_AngryBirds
./emsdk/upstream/emscripten/emcc main.cpp -s LEGACY_GL_EMULATION=1 -s GL_UNSAFE_OPTS=1 -s GL_FFP_ONLY=1 -s MAX_WEBGL_VERSION=2 --profiling -O3 -flto -o AngryBirds.html
cd ../../Computer-Graphics-Homeworks-BME-2013/02_Splines
./emsdk/upstream/emscripten/emcc main.cpp -s LEGACY_GL_EMULATION=1 -s GL_UNSAFE_OPTS=1 -s GL_FFP_ONLY=1 -s MAX_WEBGL_VERSION=2 --profiling -O3 -flto -o Splines.html
cd ../../Computer-Graphics-Homeworks-BME-2013/06_ModernStorkGame
./emsdk/upstream/emscripten/emcc main.cpp Texture.cpp -Iglm -std=c++20 -O3 -flto --preload-file shader.fs --preload-file shader.vs -s MAX_WEBGL_VERSION=2 -s USE_GLFW=3 -s WASM=3 --profiling -o StorkGame.html
```
Note: ommit optimalization flags (```-O3 -flto```) when developing, since they are greatly increasing the compile time.

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
- **\<space\>**: Start animation.
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

### 04 Stork
3D Stork model whose body is made using two Catmul-Rom splines. The body is basically a curved and variable width cylinder (one spline defines the body's middle axis, the other one sets the body's thickness). The legs are regular cylinders and the beak is a simple cone. There is also a frog which is made of only ellipsoids and a firefly that emits light.

**Keyboard controls**
- **C**: Enables the rotation of the stork and frog.
- **Z/H**: Move camera up/down.
- **R/D/F/G**: Move camera horizontally.
- **0/1/2**: Decrease the stork's spine and body width's splines' specific control point's weight.
- **3**: Decreases camera movement speed.
- **4**: Increases camera movement speed.

![3D Stork](/extras/20131202_golya_most_extreme.png)

### 05 Stork Game
Pick up and eat the frogs with the stork.

**Keyboard controls**
- **W**: Walk forward.
- **\<space\>**: Grab with beak.
- **B**: Turn left.
- **J**: Turn right.
- **Z/H**: Move camera up/down.
- **R/D/F/G**: Move camera horizontally.

![Stork Gameplay](/extras/storkgame.gif)
