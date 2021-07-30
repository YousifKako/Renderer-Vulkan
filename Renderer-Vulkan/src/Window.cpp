#include <stdexcept>
#include <iostream>

#include <Window.hpp>

void Window::frameBufferResizedCallback(GLFWwindow* window_, int width, int height)
{
    auto window                = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window_));
    window->frameBufferResized = true;
    window->width              = width;
    window->height             = height;
}

Window::Window(const uint32_t& width, const uint32_t& height, const std::string& name) :
    width(width), height(height), name(name)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    this->window = glfwCreateWindow(this->width, this->height, this->name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(this->window, this);
    glfwSetFramebufferSizeCallback(this->window, this->frameBufferResizedCallback);
}

Window::~Window()
{
    glfwDestroyWindow(this->window);
    glfwTerminate();
}

bool
Window::shouldClose()
{
    return glfwWindowShouldClose(this->window);
}

void
Window::createWindowSurface(const VkInstance& instance, VkSurfaceKHR* const surface)
{
    if (glfwCreateWindowSurface(instance, this->window, nullptr, surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to Create Window Surface");
}

VkExtent2D
Window::getExtent()
{
    return { this->width, this->height };
}