#include <limits>

#include <KeyboardMovementController.hpp>

void
KeyboardMovementController::moveInPlaneXZ(GLFWwindow* window, float dt, Object& object) {
    glm::vec3 rotate{ 0 };
    if (glfwGetKey(window, this->keys.lookRight) == GLFW_PRESS) rotate.y += 1.0f;
    if (glfwGetKey(window, this->keys.lookLeft)  == GLFW_PRESS) rotate.y -= 1.0f;
    if (glfwGetKey(window, this->keys.lookUp)    == GLFW_PRESS) rotate.x += 1.0f;
    if (glfwGetKey(window, this->keys.lookDown)  == GLFW_PRESS) rotate.x -= 1.0f;

    if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
        object.transform.rotation += this->lookSpeed * dt * glm::normalize(rotate);

    // limit pitch values between about +/- 85ish degrees
    object.transform.rotation.x = glm::clamp(object.transform.rotation.x, -1.5f, 1.5f);
    object.transform.rotation.y = glm::mod(object.transform.rotation.y, glm::two_pi<float>());

    float yaw = object.transform.rotation.y;
    const glm::vec3 forwardDir{ sin(yaw), 0.0f, cos(yaw) };
    const glm::vec3 rightDir{ forwardDir.z, 0.0f, -forwardDir.x };
    const glm::vec3 upDir{ 0.0f, -1.0f, 0.0f };

    glm::vec3 moveDir{ 0.0f };
    if (glfwGetKey(window, this->keys.moveForward)  == GLFW_PRESS) moveDir += forwardDir;
    if (glfwGetKey(window, this->keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
    if (glfwGetKey(window, this->keys.moveRight)    == GLFW_PRESS) moveDir += rightDir;
    if (glfwGetKey(window, this->keys.moveLeft)     == GLFW_PRESS) moveDir -= rightDir;
    if (glfwGetKey(window, this->keys.moveUp)       == GLFW_PRESS) moveDir += upDir;
    if (glfwGetKey(window, this->keys.moveDown)     == GLFW_PRESS) moveDir -= upDir;

    if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
        object.transform.translation += this->moveSpeed * dt * glm::normalize(moveDir);
}