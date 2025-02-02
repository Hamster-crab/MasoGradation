#define STB_TRUETYPE_IMPLEMENTATION
#include "../include/stb_truetype.h"
#include <fstream>
#include "../include/glad/glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"
#include <filesystem>


namespace fs = std::filesystem;

double blockSampleX, blockSampleY, blockSampleZ = 0;
int Bamen = 0; //0 = タイトル , 1 = ゲーム内
bool Menu = false;


std::string getBasePath() {
    // 実行ファイルのパスを取得
    fs::path exePath = fs::current_path();
    return exePath.string();
}


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

  uniform mat4 model;       // モデル行列
  uniform mat4 view;        // ビュー行列
  uniform mat4 projection;  // 射影行列
  uniform vec3 color;       // 色

  out vec3 fragColor;

  void main() {
      gl_Position = projection * view * model * vec4(aPos, 1.0);
      fragColor = color;
  }

)";

const char* fragmentShaderSource = R"(
    #version 330 core
    in vec3 fragColor;
    out vec4 FragColor;

    void main() {
        FragColor = vec4(fragColor, 1.0);
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


void drawTile(unsigned int shaderProgram, unsigned int VAO, float x, float y, float z, float rotateAngle, float colorR, float colorG, float colorB, glm::vec3 rotateAxis) {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
    model = glm::rotate(model, glm::radians(rotateAngle), rotateAxis);
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f)); // サイズ調整

    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    unsigned int colorLoc = glGetUniformLocation(shaderProgram, "color");
    glUniform3f(colorLoc, colorR, colorG, colorB); // 緑色

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

// グローバル変数としてフォントのデータを持つ
std::vector<stbtt_bakedchar> charData(256); // ASCII 文字に対応するキャラクター情報
GLuint fontTexture;
int textureWidth = 512;
int textureHeight = 512;

GLuint createFontTexture(const std::string &fontPath, const int fontSize) {
    // フォントファイルの読み込み
    std::ifstream fontFile(fontPath, std::ios::binary);  // std::ifstream のインスタンス作成
    if (!fontFile.is_open()) {
        std::cerr << "フォントファイルを開けませんでした: " << fontPath << std::endl;
        return 0;
    }

    // フォントデータをバッファに読み込む
    std::vector<unsigned char> fontBuffer((std::istreambuf_iterator<char>(fontFile)), std::istreambuf_iterator<char>());
    
    stbtt_fontinfo font;
    stbtt_InitFont(&font, fontBuffer.data(), stbtt_GetFontOffsetForIndex(fontBuffer.data(), 0));

    // テクスチャ用のビットマップ領域
    std::vector<unsigned char> bitmap(textureWidth * textureHeight, 0); // 黒色で初期化

    // 文字を 256 個レンダリング（ASCII）
    float scale = stbtt_ScaleForPixelHeight(&font, fontSize);
    stbtt_BakeFontBitmap(fontBuffer.data(), 0, fontSize, bitmap.data(), textureWidth, textureHeight, 32, 256, charData.data());

    // OpenGL のテクスチャを生成
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // ビットマップを OpenGL テクスチャにアップロード
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, textureWidth, textureHeight, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.data());

    // テクスチャの設定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}

// 文字列を描画する関数
void drawTextTwoDimensional(const std::string &text, float x, float y, GLuint fontTexture, int fontSize) {
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    std::vector<float> vertices;

    // 各文字を描画
    for (char c : text) {
        if (c < 32 || c >= 128) continue; // 32文字以上か128文字未満の文字を描画（ASCII）
        int index = c - 32;  // ' ' からのオフセット

        stbtt_bakedchar &charInfo = charData[index];

        // 文字の座標とテクスチャ座標を計算
        float x0 = x + charInfo.xoff;
        float y0 = y + charInfo.yoff;
        float x1 = x0 + charInfo.xadvance;
        float y1 = y0 + charInfo.yoff + charInfo.y1;

        // 頂点の座標とテクスチャ座標
        vertices.push_back(x0); vertices.push_back(y0); vertices.push_back(0.0f); // 左下
        vertices.push_back(charInfo.x0 / textureWidth); vertices.push_back(charInfo.y0 / textureHeight);

        vertices.push_back(x1); vertices.push_back(y0); vertices.push_back(0.0f); // 右下
        vertices.push_back(charInfo.x1 / textureWidth); vertices.push_back(charInfo.y0 / textureHeight);

        vertices.push_back(x1); vertices.push_back(y1); vertices.push_back(0.0f); // 右上
        vertices.push_back(charInfo.x1 / textureWidth); vertices.push_back(charInfo.y1 / textureHeight);

        vertices.push_back(x0); vertices.push_back(y1); vertices.push_back(0.0f); // 左上
        vertices.push_back(charInfo.x0 / textureWidth); vertices.push_back(charInfo.y1 / textureHeight);

        x += charInfo.xadvance;  // 次の文字へ
    }

    // VBO と VAO のバインド
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    // 頂点データの設定
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // テクスチャをバインドして描画
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTexture);

    // 描画
    glDrawArrays(GL_QUADS, 0, vertices.size() / 5);

    // クリーンアップ
    glBindVertexArray(0);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}


// テクスチャ作成関数
unsigned int createTexture(const char* texturePath, int width, int height) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // 画像データの読み込み
    int nrChannels;
    unsigned char* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
        std::cout << "STB error: " << stbi_failure_reason() << std::endl;
    }
    stbi_image_free(data);

    return textureID;
}

// テクスチャを描画する関数
void renderTexture(unsigned int textureID, float x, float y, float width, float height, unsigned int shaderProgram, unsigned int VAO) {
    // 使用するテクスチャを設定
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // シェーダープログラムを使用
    glUseProgram(shaderProgram);

    // モデル行列を作成して位置を変更
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(x, y, 0.0f));  // x, y の位置に移動

    // シェーダーのモデル行列に送信
    unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // VAOをバインドして描画
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}



// メイン
int main() {
    setupGLFW();
    GLFWwindow* window = glfwCreateWindow(800, 600, "MasoGradation", NULL, NULL);
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

    // フォントテクスチャを作成
    fontTexture = createFontTexture("/home/kitanohideaki/Documents/Git/MasoGradation/fonts/main.ttf", 48);

    unsigned int textureID = createTexture("resources/Title/Title.png", 100, 100);

    std::cout << "ロード中\n";

    while (!glfwWindowShouldClose(window)) {
        // ESCキーでウィンドウを閉じる
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
        if (Bamen == 0)
        {
            // 描画処理
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(shaderProgram);

            renderTexture(textureID, 1.0f, 1.0f, 100, 100, shaderProgram, VAO);

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        else if (Bamen == 1)
        {
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

            // 文字列を描画
            drawTextTwoDimensional("Hello", 50.0f, 300.0f, fontTexture, 48);

            std::cout << "A" << std::endl;

            // まわり
            // タイルを描画
            drawTile(shaderProgram, VAO, blockSampleX, blockSampleY, blockSampleZ + 1, 90.0f, 0.7f, 0.2f, 0.1f, glm::vec3(1.0f, 0.0f, 0.0f));
            drawTile(shaderProgram, VAO, blockSampleX, blockSampleY, blockSampleZ, 90.0f, 1.0f, 1.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.0f));
            drawTile(shaderProgram, VAO, blockSampleX - 0.5, blockSampleY, blockSampleZ + 0.5, 90.0f, 1.0f, 0.0f, 0.4f, glm::vec3(0.0f, 0.0f, 1.0f));
            drawTile(shaderProgram, VAO, blockSampleX + 0.5, blockSampleY, blockSampleZ + 0.5, 90.0f, 0.3f, 1.0f, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));
            drawTile(shaderProgram, VAO, blockSampleX, blockSampleY + 0.5, blockSampleZ + 0.5, 0.5f, 0.0f, 0.0f, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
            drawTile(shaderProgram, VAO, blockSampleX, blockSampleY - 0.5, blockSampleZ + 0.5, 180.0f, 0.5f, 0.2f, 0.2f, glm::vec3(1.0f, 0.0f, 0.0f));
        
            drawTextTwoDimensional("Hello", 5.0f, 5.0f, 3.0f, 3.0f);


            //std::cout << "サンプルブロックのx : " << blockSampleX
            //            << "\n" << "サンプルブロックのy : " << blockSampleY
            //                  << "\n" << "サンプルブロックのz : " << blockSampleZ
            //                  << "\n" << "カメラの向き : " << camera.front.x
            //                  << "\n" << "プレイヤーのx : " << camera.position.x
            //                  << "\n" << "プレイヤーのy : " << camera.position.y
            //                  << "\n" << "プレイヤーのz : " << camera.position.z
            //                  << "\n"
            //                  << std::endl;

            glfwSwapBuffers(window);
            glfwPollEvents();
            }
        }
    glfwTerminate();
    return 0;
}
