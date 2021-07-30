#pragma once

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

class Window
{
private:
    GLFWwindow* window      = nullptr;
    uint32_t width          = 0;
    uint32_t height         = 0;
    const std::string name  = "";
    bool frameBufferResized = false;

    static void frameBufferResizedCallback(GLFWwindow* window, int width, int height);

public:
    Window(const uint32_t& width, const uint32_t& height, const std::string& name);
    ~Window();

    // Delete copy constructor and copy operator
    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose();
    void createWindowSurface(const VkInstance& instance, VkSurfaceKHR* const surface);
    VkExtent2D getExtent();
    bool wasWindowResized()       { return this->frameBufferResized;  }
    void resetWindowResizedFlag() { this->frameBufferResized = false; }
};