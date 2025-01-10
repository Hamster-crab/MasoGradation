// camera.cpp
#include "camera.h"

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch)
    : position(startPosition), worldUp(startUp), yaw(startYaw), pitch(startPitch) {
    front = glm::vec3(0.0f, 0.0f, -1.0f);
    updateCameraVectors();
    movementSpeed = 10.0f;
    mouseSensitivity = 0.1f;
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(position, position + front, up);
}

void Camera::processKeyboard(const std::string& direction, float deltaTime) {
    float velocity = movementSpeed * deltaTime;

    if (direction == "UP") position += glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
    if (direction == "DOWN") position -= glm::vec3(0.0f, 1.0f, 0.0f) * velocity;

    if (direction == "FORWARD") position += glm::vec3(front.x, 0.0f, front.z) * velocity;
    if (direction == "BACKWARD") position -= glm::vec3(front.x, 0.0f, front.z) * velocity;
    if (direction == "LEFT") position -= glm::vec3(right.x, 0.0f, right.z) * velocity;
    if (direction == "RIGHT") position += glm::vec3(right.x, 0.0f, right.z) * velocity;
}

void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;

    yaw += xOffset;
    pitch += yOffset;

    if (constrainPitch) {
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

