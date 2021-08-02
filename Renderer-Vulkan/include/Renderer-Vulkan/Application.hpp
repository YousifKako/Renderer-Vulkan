#pragma once

#include <memory>

#include <Window.hpp>
#include <Device.hpp>
#include <Objects/Object.hpp>
#include <Rendering/Renderer.hpp>

class Application
{
private:
    static constexpr uint32_t WIDTH    = 800;
    static constexpr uint32_t HEIGHT   = 600;

    Window window                      = Window(WIDTH, HEIGHT, "Renderer in Vulkan");
    Device device                      = Device(window);
    Renderer renderer                  = { window, device };

    std::vector<Object> objects        = { };

    void loadObjects();

public:
    Application();
    ~Application();

    // Delete copy constructor and copy operator
    Application(const Application&)            = delete;
    Application& operator=(const Application&) = delete;

    void run();
};