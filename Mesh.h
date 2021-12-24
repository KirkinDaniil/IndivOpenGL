#pragma once
#include <iostream>
#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
struct vec3 {
    GLfloat x;
    GLfloat y;
    GLfloat z;
public:
    vec3(GLfloat x, GLfloat y, GLfloat z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};
struct vec2 {
    GLfloat x;
    GLfloat y;
};
class Mesh {
public:
    Mesh();
    /// <summary>
    /// use default shaders
    /// </summary>
    /// <param name="mesh_filename"> filename of .obj mesh file with full path</param>
    /// <param name="texture_filename"> file name with image of texture (.jpg)</param>
    Mesh(std::string mesh_filename, const char* texture_filename = "image.jpg");

    Mesh(std::string mesh_filename,const char* vertex_shader, const char* fragment_shader) {
        InitShader(vertex_shader,fragment_shader);
        SetupMesh(mesh_filename);
        glEnable(GL_DEPTH_TEST);
    }
    Mesh(std::string mesh_filename, const char* texture_filename, const char* vertex_shader, const char* fragment_shader) {
        InitShader(vertex_shader, fragment_shader);
        InitTexture(texture_filename);
        SetupMesh(mesh_filename);
        glEnable(GL_DEPTH_TEST);
    }

    void Draw();

    void Draw(vec3 lightpos, vec3 view, float lightXAngle, float lightYAngle, float intens);

    void Draw(const char* vertex_shader, const char* fragment_shader){
    }

    void IncXPos(float d);

    void IncYPos(float d);

    void IncXSize(float d);

    void IncYSize(float d);

    void Release();

private:

    struct Vertex {
        // position
        vec3 Position;
        // normal
        vec3 Normal;
        // texCoords
        vec2 TexCoords;

    };

    // ID шейдерной программы
    GLuint Program;

    // ID юниформ переменной перемешения
    GLint Unif_posX;
    GLint Unif_posY;
    GLint Unif_sizeX;
    GLint Unif_sizeY;
    GLint unifTexture;
    GLint unifLightPos;
    GLint unifViewVec;
    GLint Attrib_light;
    // ID Vertex Buffer Object
    GLuint VBO;
    GLuint VAO;
    GLuint IBO;

    std::vector<Vertex> vertices;
    std::vector<vec3> positions;
    std::vector<vec3> normals;
    std::vector<vec2> uvs;

    std::vector<unsigned int> indices;
    // ID текстуры
    GLint textureHandle;
    // SFML текстура
    sf::Texture textureData;

    float posX = 0.0f, posY = 0.0f;
    float sizeX = 1.0f, sizeY = 1.0f;

    // Исходный код вершинного шейдера
    const char* VertexShaderSource = R"(
    #version 330 core
    uniform float positX;
    uniform float positY;
    uniform float sizeX;
    uniform float sizeY;
    uniform vec3 lightPos;
    uniform vec3 view;

    layout (location = 0)in vec3 coord;
    layout (location = 1)in vec3 normal;
    layout (location = 2)in vec2 uv;
    out vec2 textcoord;
    out float constant;
    void main() {       
        float x_angle = 1 + positX;
        float y_angle = 1 + positY;
        
        
        vec3 position = coord * mat3(
            1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle)
        ) * mat3(
            cos(y_angle), 0, sin(y_angle),
            0, 1, 0,
            -sin(y_angle), 0, cos(y_angle)
        ) * mat3(
            sizeX, 0, 0,
            0, sizeY, 0,
            0, 0, sizeX
        );
        vec3 normal2 = normal * mat3(
            1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle)
        ) * mat3(
            cos(y_angle), 0, sin(y_angle),
            0, 1, 0,
            -sin(y_angle), 0, cos(y_angle)
        )* mat3(
            sizeX, 0, 0,
            0, sizeY, 0,
            0, 0, sizeX
        );
        
        gl_Position = vec4(position, 1.0);
        textcoord = uv;
        constant = dot(normal2,position-view);
    }
    )";

    // Исходный код фрагментного шейдера
    const char* FragShaderSource = R"(
        #version 330 core
        in vec2 textcoord;
        in float constant;
        out vec4 color;
        uniform sampler2D textureData;
        void main() {
            color = constant*texture(textureData, textcoord);
        }
        )";
    // Проверка ошибок OpenGL, если есть то вывод в консоль тип ошибки
    void checkOpenGLerror();

    // Функция печати лога шейдера
    void ShaderLog(unsigned int shader);

    void InitVBO();

    void InitShader();

    void InitShader(const char* vertex_shader, const char* fragment_shader) {
        // Создаем вершинный шейдер
        GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
        // Передаем исходный код
        glShaderSource(vShader, 1, &vertex_shader, NULL);
        // Компилируем шейдер
        glCompileShader(vShader);
        std::cout << "vertex shader \n";
        ShaderLog(vShader);

        // Создаем фрагментный шейдер
        GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
        // Передаем исходный код
        glShaderSource(fShader, 1, &fragment_shader, NULL);
        // Компилируем шейдер
        glCompileShader(fShader);
        std::cout << "fragment shader \n";
        ShaderLog(fShader);

        // Создаем программу и прикрепляем шейдеры к ней
        Program = glCreateProgram();
        glAttachShader(Program, vShader);
        glAttachShader(Program, fShader);

        // Линкуем шейдерную программу
        glLinkProgram(Program);
        // Проверяем статус сборки
        int link_ok;
        glGetProgramiv(Program, GL_LINK_STATUS, &link_ok);
        if (!link_ok)
        {
            std::cout << "error attach shaders \n";
            return;
        }

        const char* unifX_name = "positX";
        Unif_posX = glGetUniformLocation(Program, unifX_name);
        if (Unif_posX == -1)
        {
            std::cout << "could not bind uniform " << unifX_name << std::endl;
            return;
        }

        const char* unifY_name = "positY";
        Unif_posY = glGetUniformLocation(Program, unifY_name);
        if (Unif_posY == -1)
        {
            std::cout << "could not bind uniform " << unifY_name << std::endl;
            return;
        }

        const char* unifSizeX_name = "sizeX";
        Unif_sizeX = glGetUniformLocation(Program, unifSizeX_name);
        if (Unif_sizeX == -1)
        {
            std::cout << "could not bind uniform " << unifSizeX_name << std::endl;
            return;
        }

        const char* unifSizeY_name = "sizeY";
        Unif_sizeY = glGetUniformLocation(Program, unifSizeY_name);
        if (Unif_sizeY == -1)
        {
            std::cout << "could not bind uniform " << unifSizeY_name << std::endl;
            return;
        }

        unifTexture = glGetUniformLocation(Program, "textureData");
        if (unifTexture == -1)
        {
            std::cout << "could not bind uniform textureData" << std::endl;
            return;
        }
        unifLightPos = glGetUniformLocation(Program, "lightPos");
        if (unifTexture == -1)
        {
            std::cout << "could not bind uniform lightPos" << std::endl;
            return;
        }
        unifViewVec = glGetUniformLocation(Program, "view");
        if (unifTexture == -1)
        {
            std::cout << "could not bind uniform view" << std::endl;
            return;
        }

        const char* attr_name = "lightParams";
        Attrib_light = glGetUniformLocation(Program, attr_name);
        if (Attrib_light == -1)
        {
            std::cout << "could not bind attrib " << attr_name << std::endl;
            return;
        }

        checkOpenGLerror();
    }
    void InitTexture(const char* filename = "image.jpg");

    void Init();

    // Освобождение шейдеров
    void ReleaseShader();

    // Освобождение буфера
    void ReleaseVBO();

    void ReadVectorsFromFile(std::string filename);

    void SetupMesh(std::string filename);
};