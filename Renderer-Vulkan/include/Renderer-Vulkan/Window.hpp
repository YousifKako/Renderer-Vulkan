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
    GLFWwindow* window = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    const std::string name = "";
    bool frame_buffer_resized = false;

    static void frame_buffer_resized_callback(GLFWwindow* window, int width, int height);

public:
    Window(const uint32_t& width, const uint32_t& height, const std::string& name);
    ~Window();

    // Delete copy constructor and copy operator
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool should_close();
    void create_window_surface(const VkInstance& instance, VkSurfaceKHR* const surface);
    VkExtent2D get_extent();
    bool was_window_resized() { return this->frame_buffer_resized; }
    void reset_window_resized_flag() { this->frame_buffer_resized = false; }
};