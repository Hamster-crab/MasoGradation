#ifndef BLOCK_H
#define BLOCK_H

#include "../include/glad/glad/glad.h"
#include <GLFW/glfw3.h>
#include "../include/stb_image.h"
#include <string>

class Block {
public:
    Block(const std::string& front, const std::string& back, const std::string& left, 
          const std::string& right, const std::string& top, const std::string& bottom);
    ~Block();
    void draw();
    
private:
    GLuint loadTexture(const std::string& path);
    GLuint VAO, VBO, EBO;
    GLuint textures[6];
};

#endif // BLOCK_H

