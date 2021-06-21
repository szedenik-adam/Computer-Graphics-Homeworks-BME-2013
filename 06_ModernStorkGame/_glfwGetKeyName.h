#pragma once
#ifdef __EMSCRIPTEN__

#include <GLFW/glfw3.h>

const char* _glfwPlatformGetScancodeName(int scancode);
int _glfwPlatformGetKeyScancode(int key);

const char* _glfwGetKeyName(int key, int scancode)
{
    if (key != GLFW_KEY_UNKNOWN)
    {
        if (key != GLFW_KEY_KP_EQUAL &&
            (key < GLFW_KEY_KP_0 || key > GLFW_KEY_KP_ADD) &&
            (key < GLFW_KEY_APOSTROPHE || key > GLFW_KEY_WORLD_2))
        {
            return NULL;
        }

        scancode = _glfwPlatformGetKeyScancode(key);
    }

    return _glfwPlatformGetScancodeName(scancode);
}
int _glfwPlatformGetKeyScancode(int key)
{
    return key;
}
const char* _glfwPlatformGetScancodeName(int scancode)
{
    switch (scancode)
    {
    case GLFW_KEY_APOSTROPHE:
        return "'";
    case GLFW_KEY_COMMA:
        return ",";
    case GLFW_KEY_MINUS:
    case GLFW_KEY_KP_SUBTRACT:
        return "-";
    case GLFW_KEY_PERIOD:
    case GLFW_KEY_KP_DECIMAL:
        return ".";
    case GLFW_KEY_SLASH:
    case GLFW_KEY_KP_DIVIDE:
        return "/";
    case GLFW_KEY_SEMICOLON:
        return ";";
    case GLFW_KEY_EQUAL:
    case GLFW_KEY_KP_EQUAL:
        return "=";
    case GLFW_KEY_LEFT_BRACKET:
        return "[";
    case GLFW_KEY_RIGHT_BRACKET:
        return "]";
    case GLFW_KEY_KP_MULTIPLY:
        return "*";
    case GLFW_KEY_KP_ADD:
        return "+";
    case GLFW_KEY_BACKSLASH:
    case GLFW_KEY_WORLD_1:
    case GLFW_KEY_WORLD_2:
        return "\\";
    case GLFW_KEY_0:
    case GLFW_KEY_KP_0:
        return "0";
    case GLFW_KEY_1:
    case GLFW_KEY_KP_1:
        return "1";
    case GLFW_KEY_2:
    case GLFW_KEY_KP_2:
        return "2";
    case GLFW_KEY_3:
    case GLFW_KEY_KP_3:
        return "3";
    case GLFW_KEY_4:
    case GLFW_KEY_KP_4:
        return "4";
    case GLFW_KEY_5:
    case GLFW_KEY_KP_5:
        return "5";
    case GLFW_KEY_6:
    case GLFW_KEY_KP_6:
        return "6";
    case GLFW_KEY_7:
    case GLFW_KEY_KP_7:
        return "7";
    case GLFW_KEY_8:
    case GLFW_KEY_KP_8:
        return "8";
    case GLFW_KEY_9:
    case GLFW_KEY_KP_9:
        return "9";
    case GLFW_KEY_A:
        return "a";
    case GLFW_KEY_B:
        return "b";
    case GLFW_KEY_C:
        return "c";
    case GLFW_KEY_D:
        return "d";
    case GLFW_KEY_E:
        return "e";
    case GLFW_KEY_F:
        return "f";
    case GLFW_KEY_G:
        return "g";
    case GLFW_KEY_H:
        return "h";
    case GLFW_KEY_I:
        return "i";
    case GLFW_KEY_J:
        return "j";
    case GLFW_KEY_K:
        return "k";
    case GLFW_KEY_L:
        return "l";
    case GLFW_KEY_M:
        return "m";
    case GLFW_KEY_N:
        return "n";
    case GLFW_KEY_O:
        return "o";
    case GLFW_KEY_P:
        return "p";
    case GLFW_KEY_Q:
        return "q";
    case GLFW_KEY_R:
        return "r";
    case GLFW_KEY_S:
        return "s";
    case GLFW_KEY_T:
        return "t";
    case GLFW_KEY_U:
        return "u";
    case GLFW_KEY_V:
        return "v";
    case GLFW_KEY_W:
        return "w";
    case GLFW_KEY_X:
        return "x";
    case GLFW_KEY_Y:
        return "y";
    case GLFW_KEY_Z:
        return "z";
    }

    return NULL;
}
#endif