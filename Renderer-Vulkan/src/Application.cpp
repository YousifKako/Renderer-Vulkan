#include <chrono>
#include <stdexcept>
#include <array>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <Application.hpp>
#include <Rendering/RenderSystem.hpp>
#include <KeyboardMovementController.hpp>
#include <Camera.hpp>

Application::Application()
{
    this->loadObjects();
}

Application::~Application() { }

void
Application::run()
{
    RenderSystem renderSystem = { this->device, this->renderer.getSwapChainRenderPass() };
    Camera camera             = { };

    //camera.setViewDirection(glm::vec3(0.0f), glm::vec3(0.5f, 0.0f, 1.0f));
    //camera.setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));

    KeyboardMovementController cameraController = { };
    Object viewerObject                         = Object::createObject();
    auto currentTime                            = std::chrono::high_resolution_clock::now();

    while (!window.shouldClose())
    {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;
        frameTime = glm::min(frameTime, 0.2f);

        cameraController.moveInPlaneXZ(this->window.getGLFWwindow(), frameTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = this->renderer.getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(55.0f), aspect, 0.1f, 20.0f);

        if (auto commandBuffer = this->renderer.beginFrame())
        {
            this->renderer.beginSwapChainRenderPass(commandBuffer);
            renderSystem.renderObjects(commandBuffer, this->objects, camera);
            this->renderer.endSwapChainRenderPass(commandBuffer);
            this->renderer.endFrame();
        }
    }

    vkDeviceWaitIdle(this->device.device());
}

void
Application::loadObjects()
{
    std::shared_ptr<Model> model    = Model::createModelFromFile(this->device, "Assets/Scenes/Test.obj");
    auto objects                    = Object::createObject();
    objects.model                   = model;
    objects.color                   = { 0.1f, 0.8f, 0.1f };
    objects.transform.translation   = { 0.0f, 0.0f, 2.5f };
    objects.transform.scale         = { 0.5f, 0.5f, 0.5f };

    this->objects.push_back(std::move(objects));
}