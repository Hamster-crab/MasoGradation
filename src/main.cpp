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

// 頂点データ（四角形）
float vertices[] = {
    -0.5f, 0.0f, -0.5f,  // 左下
     0.5f, 0.0f, -0.5f,  // 右下
     0.5f, 0.0f,  0.5f,  // 右上
    -0.5f, 0.0f,  0.5f   // 左上
};

unsigned int indices[] = {
    0, 1, 2, // 三角形1
    2, 3, 0  // 三角形2
};

// シェーダープログラム
const char* vertexShaderSource = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
)";

const char* fragmentShaderSource = R"(
    #version 330 core
    uniform vec3 tileColor;
    out vec4 FragColor;

    void main() {
        FragColor = vec4(tileColor, 1.0);
    }
)";

// シェーダー作成関数
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "Shader Compilation Failed\n" << infoLog << std::endl;
    }
    return shader;
}

unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// GLFWの初期化と設定
void setupGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

void setupOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);  // 空のような青色
}

// メイン
int main() {
    setupGLFW();
    GLFWwindow* window = glfwCreateWindow(800, 600, "50x50 Tile Grid", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    setupOpenGL();

    Camera camera(glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -30.0f);
    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // マウス入力
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    double lastX = 800.0f / 2.0f;
    double lastY = 600.0f / 2.0f;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        // ESCキーでウィンドウを閉じる
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        // 時間計算
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // マウス入力
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float xOffset = xpos - lastX;
        float yOffset = lastY - ypos; // y軸反転
        lastX = xpos;
        lastY = ypos;

        camera.processMouseMovement(xOffset, yOffset);

        // キー入力に基づいてカメラを操作
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.processKeyboard("FORWARD", deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.processKeyboard("BACKWARD", deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.processKeyboard("LEFT", deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.processKeyboard("RIGHT", deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) camera.processKeyboard("UP", deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.processKeyboard("DOWN", deltaTime);

        // 描画処理
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // モデル・ビュー・射影行列の設定
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 view = camera.getViewMatrix();

        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // タイルの描画
        for (blockSampleX = 0; blockSampleX <= 0; ++blockSampleX) {
            for (blockSampleZ = 0; blockSampleZ <= 0; ++blockSampleZ) {
                glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(blockSampleX, 0.0f, blockSampleZ));
                unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                
                glBindVertexArray(VAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        std::cout << "サンプルブロックのx : " << blockSampleX
                  << "\n" << "サンプルブロックのy : " << blockSampleY
                  << "\n" << "サンプルブロックのz : " << blockSampleZ
                  << "\n" << "カメラの向き : " << camera.front.x
                  << "\n" << "プレイヤーのx : " << camera.position.x
                  << "\n" << "プレイヤーのy : " << camera.position.y 
                  << "\n" << "プレイヤーのz : " << camera.position.z
                  << "\n"
                  << std::endl;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

