#pragma once

#include <Objects/Object.hpp>
#include <Window.hpp>

class KeyboardMovementController
{
public:
    struct KeyMappings
    {
        int moveLeft     = GLFW_KEY_A;
        int moveRight    = GLFW_KEY_D;
        int moveForward  = GLFW_KEY_W;
        int moveBackward = GLFW_KEY_S;
        //int moveUp       = GLFW_KEY_E;
        int moveUp       = GLFW_KEY_SPACE;
        int moveDown     = GLFW_KEY_LEFT_CONTROL;
        //int moveDown     = GLFW_KEY_Q;
        int lookLeft     = GLFW_KEY_LEFT;
        int lookRight    = GLFW_KEY_RIGHT;
        int lookUp       = GLFW_KEY_UP;
        int lookDown     = GLFW_KEY_DOWN;
    };

    void moveInPlaneXZ(GLFWwindow* window, float dt, Object& object);

    const KeyMappings keys = { };
    float moveSpeed        = 3.0f;
    float lookSpeed        = 2.0f;
};