#ifdef __EMSCRIPTEN__
// For emscripten, instead of using glad we use its built-in support for OpenGL.
// ./emsdk/upstream/emscripten/emcc main.cpp -Iglm --preload-file 7.2.camera.fs --preload-file 7.2.camera.vs --preload-file container.jpg -s USE_WEBGL2=3 -s FULL_ES3=3 -s USE_GLFW=3 -s WASM=3 -o modern.html
// ./emsdk/upstream/emscripten/emcc main.cpp Texture.cpp -Iglm -std=c++20 --preload-file 7.2.camera.fs --preload-file 7.2.camera.vs --preload-file container.jpg -s USE_WEBGL2=3 -s FULL_ES3=3 -s USE_GLFW=3 -s WASM=3 -o modern.html
// optimized + debug symbols for profiling
// ./emsdk/upstream/emscripten/emcc main.cpp Texture.cpp -Iglm -std=c++20 -O3 -flto --preload-file 7.2.camera.fs --preload-file 7.2.camera.vs --preload-file container.jpg -s MAX_WEBGL_VERSION=2 -s USE_GLFW=3 -s WASM=3 --profiling -o modern.html
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include "_glfwGetKeyName.h"
#else
#include <glad/glad.h>
#endif


// Grass: https://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch07.html
// OpenGL Instancing: https://learnopengl.com/Advanced-OpenGL/Instancing
// Load 3D model: https://marcelbraghetto.github.io/a-simple-triangle/2019/04/14/part-09/
// Anti-aliasing: https://learnopengl.com/Advanced-OpenGL/Anti-Aliasing

#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "filesystem.h"
#include "shader_m.h"
#include "View.h"
#include "Plane.h"
#include "Frog.h"
#include "Stork.h"
#include "Firebug.h"

#include <iostream>
#include <functional>

#pragma comment(lib, "glad.lib")
#pragma comment(lib, "glfw3dll.lib")
// dependencies
// vcpkg install glfw3:x64-windows glm:x64-windows glad:x64-windows

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

// camera
glm::vec3 View::eye    = glm::vec3(0, 2, 0);
glm::vec3 View::lookat = glm::vec3(0, 0, 1);
glm::vec3 View::up     = glm::vec3(0, 1, 0);
glm::mat4 View::model;
float View::fov = 54.0f;
std::stack<glm::mat4> View::modelStack;
Shader* View::activeShader = nullptr;

MaterialBase* MaterialBase::currentMaterial = nullptr;
bool MaterialBase::allowCaching = true;

Texture Texture::grass;
Texture Texture::grass_frog;
Texture Texture::golya_;
GLuint Texture::textures[16];

Material Material::frog_body = Material(Color(0.05, 0.2, 0.01), Color(0.1, 0.5, 0.1), Color(0.1, 0.5, 0.1));
Material Material::frog_leg = Material(Color(0.05, 0.2, 0.01), Color(0.1, 0.65, 0.1), Color(0.00, 0.00, 0.0));
Material Material::frog_eye = Material(Color(0.05, 0.2, 0.01), Color(0.3, 0.5, 0.3), Color(0.10, 0.10, 0.10));

Material Material::golya_beak = Material(Color(0.1, 0.1, 0.1), Color(0.95292, 0.247, 0.247), Color(0.4, 0.1, 0.1));
Material Material::golya_eye = Material(Color(0.1, 0.1, 0.1), Color(0, 0, 0), Color(0, 0, 0));

Material Material::firebug_body = Material(Color(3, 2.8, 1), Color(0.4, 0.34, 0.125), Color(0.3, 0.3, 0.1));

Material Material::texture_default = Material(Color(0.1, 0.1, 0.1), Color(1, 1, 1), Color(0.5, 0.5, 0.5));
Material Material::shadow = Material(Color(0.05, 0.05, 0.05), Color(0, 0, 0), Color(0, 0, 0));

std::unordered_map<int, VertexBuffer> Ellipsoid::resToVBuf;

const int MultiCylinder::max_resolution = 20;
glm::vec3 MultiCylinder::buffer[max_resolution * 5];
float MultiCylinder::sincosbuffer[max_resolution * 2];
std::optional<VertexBuffer> MultiCylinder::vbuf;

const int SalamiGolya::leg_rad_count = 6;
const float SalamiGolya::upperlegdist = 0.9;
const float SalamiGolya::lowerlegdist = 0.98;
const float SalamiGolya::upperlegX = 0.7;
const float SalamiGolya::upperlegY = -0.4;
const float SalamiGolya::upperlegZ = 0.25;//   invisible tail[0]                     tail[1]           base point[2]            neck bottom[3]                    neck end[4]                       head[5]            beak_start[6]          beak_end[7]
const PointW SalamiGolya::spine_vals[8] = { PointW(-2.0,-1.7,0,1), PointW(-1.0,-0.3,0   ,0.4), PointW(0  ,-0.0,0  ,1), PointW(2.2, 0.6 ,0   ,1), PointW(2  , 2.4, 0   ,0.2543), PointW(2.4 ,2.2 ,0   ,0.116), PointW(2.7,2.2,0  ,4.1), PointW(4,2.2,0,0.2) };
const PointW SalamiGolya::body_vals[8] = { PointW(0, 0  ,0,1), PointW(0.0, 0.1,0.02,0.4), PointW(0.5, 0.8,0.6,1), PointW(0.20,0.25,0.25,1), PointW(0.20,0.20,0.20,0.2543), PointW(0.30,0.26,0.20,0.116), PointW(0.1,0.1,0.1,4.1), PointW(0,0  ,0,0.2) };
bool SalamiGolya::leg_rads_inited = false;
float SalamiGolya::leg_rads[SalamiGolya::leg_rad_count];

TCRSpline MovingGolya::walking;
glm::vec3 MovingGolya::standing_main;
glm::vec3 MovingGolya::standing_float;
bool MovingGolya::loaded = false;
float MovingGolya::walk_phase_delay = 1.5;

Shader* shaderPtr = nullptr;

Frog frogs[40]; int frog_count = 40;
FireBug firebug;
MovingGolya golya;

double wtime = 0, oldtime = 0;
char navigation = 0; double navtime = 0; float navspeed = 5.0f;
char golya_animation = 0; double gatime = 0;
double frog_ai_time = 0;

#ifdef __EMSCRIPTEN__
bool fullscreen = false;
void emscGetContainerSize(double* width, double* height) {
    emscripten_get_element_css_size(".emscripten_border", width, height);
}
EM_BOOL emscWindowSizeChanged(int eventType, const EmscriptenUiEvent* uiEvent, void* userData) {
    double width, height;
    emscGetContainerSize(&width, &height);
    printf("Div size changed to: %lf x %lf\n", width, height);

    GLFWwindow* window = (GLFWwindow*)userData;
    //if (glfwGetWindowMonitor(window) != nullptr) // Not working for fullscreen check.
    if(!fullscreen)
    glfwSetWindowSize(window, width-2, height-2);
    return true;
}
EM_BOOL emscFullscreenChanged(int eventType, const EmscriptenFullscreenChangeEvent* fullscreenChangeEvent, void* userData)
{
    fullscreen = fullscreenChangeEvent->isFullscreen;
    if (fullscreen)printf("changed to FULLSCREEN\n"); else printf("changed to windowed\n");
    return true;
}
#endif

void onKeyboard(GLFWwindow* window, int key, int scancode, int mods)
{
    if (key == 'r') { navigation |= 1; navtime = wtime; }
    if (key == 'f') { navigation |= 2; navtime = wtime; }
    if (key == 'd') { navigation |= 4; navtime = wtime; }
    if (key == 'g') { navigation |= 8; navtime = wtime; }
    if (key == 'z') { navigation |= 16; navtime = wtime; }
    if (key == 'h') { navigation |= 32; navtime = wtime; }
    if (key == '0') { for (int i = 0; i < frog_count; i++) frogs[i].ellipsoids[0].rotation.x += 0.05; }
    if (key == '1') { for (int i = 0; i < frog_count; i++) frogs[i].ellipsoids[0].rotation.x -= 0.05; }
    if (key == '2') { for (int i = 0; i < frog_count; i++) frogs[i].ellipsoids[0].rotation.x += 0.01; }
    if (key == '3') { for (int i = 0; i < frog_count; i++) frogs[i].ellipsoids[0].rotation.x -= 0.01; }
    if (key == '4') { MaterialBase::ToggleCaching(); }
    if (key == ' ') { golya.StartBeakAttack1(); }
    if (key == 'w') { golya.StartWalking(); }
    if (key == 'b') { golya_animation |= 1;  gatime = wtime; }
    if (key == 'j') { golya_animation |= 2;  gatime = wtime; }

    /*if (key == GLFW_KEY_SPACE) { golya.StartBeakAttack1(); }
    if (key == GLFW_KEY_W) { golya.StartWalking(); }
    if (key == GLFW_KEY_B) { golya_animation |= 1;  gatime = wtime; }
    if (key == GLFW_KEY_J) { golya_animation |= 2;  gatime = wtime; }*/
}

void onKeyboardUp(GLFWwindow* window, int key, int scancode, int mods)
{
    if (key == 'r') { navigation &= ~1; navtime = wtime; }
    if (key == 'f') { navigation &= ~2; navtime = wtime; }
    if (key == 'd') { navigation &= ~4; navtime = wtime; }
    if (key == 'g') { navigation &= ~8; navtime = wtime; }
    if (key == 'z') { navigation &= ~16; navtime = wtime; }
    if (key == 'h') { navigation &= ~32; navtime = wtime; }
    if (key == 'w') { golya.StopWalking(); }
    if (key == 'b') { golya_animation &= ~1; }
    if (key == 'j') { golya_animation &= ~2; }

    /*if (key == GLFW_KEY_W) { golya.StopWalking(); }
    if (key == GLFW_KEY_B) { golya_animation &= ~1;  gatime = wtime; }
    if (key == GLFW_KEY_J) { golya_animation &= ~2;  gatime = wtime; }*/
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
#ifndef __EMSCRIPTEN__
    const char* keyName = glfwGetKeyName(key, scancode);
#else
    const char* keyName = _glfwGetKeyName(key, scancode);
#endif
    if (keyName && strlen(keyName) == 1) key = keyName[0];

    if (action == GLFW_PRESS) { 
        onKeyboard(window, key, scancode, mods);
    }
    if (action == GLFW_RELEASE) { 
        onKeyboardUp(window, key, scancode, mods);
    }
}

void OnIdle(double deltat)
{
    if (navigation) {
        float navDelta = deltat; //wtime - navtime;
        float cameraDistance = navspeed * navDelta;
        if (navigation & 1)  View::Move( cameraDistance * View::lookat);
        if (navigation & 2)  View::Move(-cameraDistance * View::lookat);
        if (navigation & 4)  View::Move(-glm::normalize(glm::cross(View::lookat, View::up)) * cameraDistance);
        if (navigation & 8)  View::Move( glm::normalize(glm::cross(View::lookat, View::up)) * cameraDistance);
        if (navigation & 16) View::Move( cameraDistance * View::up);
        if (navigation & 32) View::Move(-cameraDistance * View::up);
        navtime = wtime;
    }
    float golyat = deltat / 12.f *1000;
    while (golyat > 0.2) { golya.Animate(0.2); golyat -= 0.2; } golya.Animate(golyat);

    if (golya_animation) {
        if (golya_animation & 1) golya.RotateLeft(20 * deltat);
        if (golya_animation & 2) golya.RotateRight(20 * deltat);
    }
    for (int i = 0; i < frog_count; i++) { frogs[i].Animate(deltat); }

    if (frog_ai_time < deltat) {
        frog_ai_time = ((rand() & 4095) + 1000)/1000.f; // Wait 1-5 sec.
        int nearest = 0; float dist = glm::distance(golya.pos, frogs[0].pos);
        for (int i = 0; i < frog_count; i++) { float dist2 = glm::distance(golya.pos, frogs[i].pos); if (dist > dist2) { dist = dist2; nearest = i; } }
        if (dist < 7) { frogs[nearest].JumpAway(golya.pos); } // Jump with the nearest frog.
        else { frog_ai_time = 0.2; } // Check again 200 ms later.
    }
    else frog_ai_time -= deltat;
}

std::function<void()> rloop;
void render_loop() { rloop(); }

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    /*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/
    // OpenGL ES 2.0
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    int windowWidth = SCR_WIDTH, windowHeight = SCR_HEIGHT;
#ifdef __EMSCRIPTEN__
    double dw, dh;
    emscGetContainerSize(&dw, &dh);
    windowWidth = dw; windowHeight = dh;
#endif
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Stork Game", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

#ifndef __EMSCRIPTEN__
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
#else
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, window, false, emscWindowSizeChanged);

    emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, false, emscFullscreenChanged);
#endif

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader shader("7.2.camera.vs", "7.2.camera.fs");
    shaderPtr = &shader;

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    /*float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,    0.f, 0.f, -1.f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,    0.f, 0.f, -1.f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    0.f, 0.f, -1.f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    0.f, 0.f, -1.f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,    0.f, 0.f, -1.f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,    0.f, 0.f, -1.f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,    0.f, 0.f, 1.f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,    0.f, 0.f, 1.f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,    0.f, 0.f, 1.f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,    0.f, 0.f, 1.f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,    0.f, 0.f, 1.f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,    0.f, 0.f, 1.f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,    -1.f, 0.f, 0.f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    -1.f, 0.f, 0.f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,    -1.f, 0.f, 0.f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,    -1.f, 0.f, 0.f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,    -1.f, 0.f, 0.f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,    -1.f, 0.f, 0.f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,    1.f, 0.f, 0.f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    1.f, 0.f, 0.f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,    1.f, 0.f, 0.f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,    1.f, 0.f, 0.f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,    1.f, 0.f, 0.f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,    1.f, 0.f, 0.f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,    0.f, -1.f, 0.f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,    0.f, -1.f, 0.f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,    0.f, -1.f, 0.f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,    0.f, -1.f, 0.f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,    0.f, -1.f, 0.f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,    0.f, -1.f, 0.f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,    0.f, 1.f, 0.f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,    0.f, 1.f, 0.f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,    0.f, 1.f, 0.f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,    0.f, 1.f, 0.f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,    0.f, 1.f, 0.f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,    0.f, 1.f, 0.f,
    };
    // world space positions of our cubes
    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  5.0f,  7.0f),
        glm::vec3(2.0f,  6.0f, 15.0f),
        glm::vec3(-1.5f, 5.2f,  2.5f),
        glm::vec3(-3.8f, 5.0f, 12.3f),
        glm::vec3(2.4f,  2.4f,  3.5f),
        glm::vec3(-1.7f, 5.0f,  7.5f),
        glm::vec3(1.3f,  2.0f,  2.5f),
        glm::vec3(1.5f,  8.0f,  7.5f),
        glm::vec3(1.5f,  5.2f,  1.5f),
        glm::vec3(-1.3f, 7.0f,  4.5f)
    };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); 
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // load and create a texture 
    // -------------------------
    unsigned int texture1, texture2;
    // texture 1
    // ---------
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load(FileSystem::getPath("container.jpg").c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    // texture 2
    // ---------
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    data = stbi_load(FileSystem::getPath("resources/textures/awesomeface.png").c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        // note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);*/

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    View::SetShader(shader);
//    shader.setInt("texture1", 0);
//    shader.setInt("texture2", 1);

    // pass projection matrix to shader (as projection matrix rarely changes there's no need to do this per frame)
    // -----------------------------------------------------------------------------------------------------------
    //glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    //shader.setMat4("projection", projection);
    View::Init(shader, windowWidth, windowHeight);
    
    Texture::TextureInit();
    Plane ground = Plane(glm::vec3(-30, 0, -20), glm::vec3(0, 1, 0), 60, 60, 2, 2);
    ground.material = &(Texture::grass);

    // Init frogs.
    frogs[0].size = 0.3;
    frogs[0].pos = glm::vec3(1, 0.22, 5);
    frogs[0].rotation = glm::vec3(0, 40, 0);
    for (int i = 1; i < (sizeof(frogs) / sizeof(Frog)); i++) {
        float angle = ((rand() % 10000) * 2 * PI) / 10000;
        float dist = ((rand() % 10000) / 10000.f) * 10;
        frogs[i].pos = glm::vec3(dist * cos(angle), 0.22, dist * sin(angle))  ;//+ golya.pos;
        frogs[i].rotation.y = rand() % 360; int j;
        frogs[i].size = 0.3;
        for (j = 0; j < 40; j++) {
            if (i != j && glm::distance(frogs[i].pos, frogs[j].pos) < 1) break;
        }
        if (j < 40) { i--; }
    }

    int fps = 0;
    // render loop
    // -----------
    rloop = [&]
    {
        // per-frame time logic
        // --------------------
        wtime = glfwGetTime();
        float deltat = wtime - oldtime;

        fps++;
        if ((int)wtime != (int)oldtime) { printf("FPS: %d\n", fps); fps = 0; }

        oldtime = wtime;
        

        OnIdle(deltat);

        // input
        // -----
        //processInput(window);

        // render
        // ------
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        View::Clear();

        // bind textures on corresponding texture units
        /*glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);*/

        // activate shader
        View::SetShader(shader);

        // camera/view transformation
        //glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        //shader.setMat4("view", view);
        View::ResetTransform();

        // render boxes
        /*shader.setInt("texture1", 0);
        shader.setVec3("material.ambient", 0, 0, 0);
        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i*wtime;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            shader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }*/
        glm::mat4 model = glm::mat4(1.0f); // reset model transform.
        shader.setMat4("model", model);
        firebug.Draw();
        ground.Draw();

        int drawnfrogs = 0;
        glm::vec3 dir = View::lookat; float cos_fov = cos(View::fov * PI / 180); //béka-kilóg tesztre inkább MVP matrix-ot kellene használni (ezt:http://stackoverflow.com/questions/6301085/how-to-check-if-an-object-lies-outside-the-clipping-volume-in-opengl)
        for (int i = 0; i < frog_count; i++)
        {
            if (glm::dot(dir, glm::normalize(frogs[i].pos - View::eye)) >= cos_fov) { float distance = glm::distance(View::eye, frogs[i].pos); frogs[i].Draw(distance); drawnfrogs++; }
        }
        golya.Draw();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    };

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(render_loop, 0, true);
#else
    while (!glfwWindowShouldClose(window))
        render_loop();
#endif

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    //glDeleteVertexArrays(1, &VAO);
    //glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    if (width == 0 && height == 0) { printf("framebuffer_size_callback called with 0,0!"); return; }
    View::Init(*shaderPtr, width, height);
}
