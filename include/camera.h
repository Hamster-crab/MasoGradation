#ifndef CAMERA_H
#define CAMERA_H

#include "../include/glad/glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

// カメラクラス（Minecraft風操作）
class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;

    Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch)
        : position(startPosition), worldUp(startUp), yaw(startYaw), pitch(startPitch) {
        front = glm::vec3(0.0f, 0.0f, -1.0f);
        updateCameraVectors();
        movementSpeed = 10.0f;
        mouseSensitivity = 0.1f;
    }

    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, position + front, up);
    }

    void processKeyboard(const std::string& direction, float deltaTime) {
        float velocity = movementSpeed * deltaTime;

        // 上下の移動は視点に依存しない
        if (direction == "UP") position += glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
        if (direction == "DOWN") position -= glm::vec3(0.0f, 1.0f, 0.0f) * velocity;

        // 前後左右の移動は視点に依存する
        if (direction == "FORWARD") position += glm::vec3(front.x, 0.0f, front.z) * velocity; // Y軸の影響を排除
        if (direction == "BACKWARD") position -= glm::vec3(front.x, 0.0f, front.z) * velocity; // Y軸の影響を排除
        if (direction == "LEFT") position -= glm::vec3(right.x, 0.0f, right.z) * velocity; // Y軸の影響を排除
        if (direction == "RIGHT") position += glm::vec3(right.x, 0.0f, right.z) * velocity; // Y軸の影響を排除
    }

    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true) {
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

private:
    void updateCameraVectors() {
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(newFront);
        right = glm::normalize(glm::cross(front, worldUp)); // 地面に平行な右方向ベクトル
        up = glm::normalize(glm::cross(right, front));      // 上方向ベクトル
    }
};

#endif // CAMERA_H

