#include "block.h"
#include <iostream>

// キューブの面に適用するテクスチャの読み込み
Block::Block(const std::string& front, const std::string& back, const std::string& left, 
             const std::string& right, const std::string& top, const std::string& bottom) {
    // テクスチャの読み込み
    textures[0] = loadTexture(front);  // front
    textures[1] = loadTexture(back);   // back
    textures[2] = loadTexture(left);   // left
    textures[3] = loadTexture(right);  // right
    textures[4] = loadTexture(top);    // top
    textures[5] = loadTexture(bottom); // bottom

    // キューブの頂点データ
    float vertices[] = {
        // 位置               // テクスチャ座標
        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,  // front
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f,  
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f,  
        -0.5f,  0.5f,  0.5f,   0.0f, 1.0f,  
        
        -0.5f, -0.5f, -0.5f,   1.0f, 0.0f,  // back
         0.5f, -0.5f, -0.5f,   0.0f, 0.0f,  
         0.5f,  0.5f, -0.5f,   0.0f, 1.0f,  
        -0.5f,  0.5f, -0.5f,   1.0f, 1.0f,  

        -0.5f, -0.5f, -0.5f,   0.0f, 0.0f,  // left
        -0.5f, -0.5f,  0.5f,   1.0f, 0.0f,  
        -0.5f,  0.5f,  0.5f,   1.0f, 1.0f,  
        -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,  

         0.5f, -0.5f, -0.5f,   0.0f, 0.0f,  // right
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f,  
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f,  
         0.5f,  0.5f, -0.5f,   0.0f, 1.0f,  

        -0.5f,  0.5f,  0.5f,   0.0f, 0.0f,  // top
         0.5f,  0.5f,  0.5f,   1.0f, 0.0f,  
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f,  
        -0.5f,  0.5f, -0.5f,   0.0f, 1.0f,  

        -0.5f, -0.5f,  0.5f,   0.0f, 0.0f,  // bottom
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f,  
         0.5f, -0.5f, -0.5f,   1.0f, 1.0f,  
        -0.5f, -0.5f, -0.5f,   0.0f, 1.0f   
    };

    unsigned int indices[] = {
        0, 1, 2,  0, 2, 3,  // front
        4, 5, 6,  4, 6, 7,  // back
        8, 9, 10, 8, 10, 11, // left
        12, 13, 14, 12, 14, 15, // right
        16, 17, 18, 16, 18, 19, // top
        20, 21, 22, 20, 22, 23  // bottom
    };

    // VAO, VBO, EBOの設定
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

// デストラクタでリソースを解放
Block::~Block() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

// キューブを描画
void Block::draw() {
    glBindVertexArray(VAO);

    for (int i = 0; i < 6; ++i) {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(i * 6 * sizeof(unsigned int)));
    }
}

// テクスチャの読み込み
GLuint Block::loadTexture(const std::string& path) {
    int width, height, channels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "テクスチャの読み込みに失敗しました: " << path << std::endl;
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    return texture;
}

