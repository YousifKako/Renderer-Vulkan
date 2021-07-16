#include <stdexcept>

#include <Window.hpp>

Window::Window(const uint32_t& width, const uint32_t& height, const std::string& name) :
    width(width), height(height), name(name)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    this->window = glfwCreateWindow(this->width, this->height, this->name.c_str(), nullptr, nullptr);
}

Window::~Window()
{
    glfwDestroyWindow(this->window);
    glfwTerminate();
}

bool
Window::should_close()
{
    return glfwWindowShouldClose(this->window);
}

void Window::create_window_surface(const VkInstance& instance, VkSurfaceKHR* const surface)
{
    if (glfwCreateWindowSurface(instance, this->window, nullptr, surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to Create Window Surface");
}

VkExtent2D
Window::get_extent()
{
    return { this->width, this->height };
}