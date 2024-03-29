﻿// Вращающийся треугольник с текстурой (можно вращать стрелочками)

#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <vector>
#include "Mesh.h"


// В C и C++ есть оператор #, который позволяет превращать параметры макроса в строку
#define TO_STRING(x) #x



struct ShaderInformation {
    // Переменные с индентификаторами ID
    // ID шейдерной программы
    GLuint shaderProgram;
    // ID атрибута вершин
    GLint attribVertex;
    // ID атрибута текстурных координат
    GLint attribTexture;
    // ID юниформа текстуры
    GLint unifTexture;
    // ID юниформа сдвига
    GLint unifShift;
};

struct GameObject {
    // Количество вершин в буферах
    GLfloat buffers_size;
    // ID буфера вершин
    GLuint vertexVBO;
    // ID буфера текстурных координат
    GLuint textureVBO;
    // ID текстуры
    GLuint textureHandle;

    // Велечина сдвига
    GLfloat shift[2];
};


sf::Texture textureData;
std::vector <GameObject> gameObjects;
ShaderInformation shaderInformation;
// Массив VBO что бы их потом удалить
std::vector <GLuint> VBOArray;

int numberOfSquares = 3;

// Вершина
struct Vertex
{
    GLfloat x;
    GLfloat y;
};

const char* FongVertexShaderSource = R"(
    #version 330 core
    uniform float positX;
    uniform float positY;

    layout (location = 0)in vec3 coord;
    layout (location = 1)in vec3 normal;
    layout (location = 2)in vec2 uv;

    out vec2 textcoord;
    out vec3 Normal;  
    out vec3 FragPos;

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
        );
        vec3 normal2 = normal * mat3(
            1, 0, 0,
            0, cos(x_angle), -sin(x_angle),
            0, sin(x_angle), cos(x_angle)
        ) * mat3(
            cos(y_angle), 0, sin(y_angle),
            0, 1, 0,
            -sin(y_angle), 0, cos(y_angle)
        );
        
        gl_Position = vec4(position, 1.0);
        textcoord = uv;
        Normal = normal2;
        FragPos = position;
    }
    )";

// Исходный код фрагментного шейдера
const char* FongFragShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        in vec2 textcoord;
        in vec3 Normal;  
        in vec3 FragPos;  
  
        uniform vec3 lightPos = vec3(0,0,-1); 
        uniform vec3 view;
        uniform sampler2D textureData;

        void main() {
        vec3 lightColor = vec3(1,0.5,0.5);
        vec3 objectColor = texture(textureData, textcoord).rgb;
            // ambient
            float ambient = 0.1;
            //vec3 ambient = ambientStrength * lightColor;
  	
            // diffuse 
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diffuse = max(dot(norm, lightDir), 0.0);
            //vec3 diffuse = diff * lightColor;
    
            // specular
            float specularStrength = 0.7;
            vec3 viewDir = normalize(view - FragPos);
            vec3 reflectDir = reflect(lightDir, norm);  
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            //vec3 specular = specularStrength * spec * lightColor;  
            //vec3 specular = vec3(0,0,0);
            float specular = specularStrength * spec ;  
            vec3 result = (ambient + diffuse + specular) * lightColor * objectColor;
            FragColor = vec4(result, 1.0);
        }
)";

const char* VertexShaderSource = TO_STRING(
    #version 330 core\n

    uniform vec2 shift;

    in vec2 vertCoord;
    in vec2 texureCoord;

    out vec2 tCoord;

    void main() {
        tCoord = texureCoord;
        gl_Position = vec4(vertCoord + shift, 0.0, 1.0);
    }
);

const char* FragShaderSource = TO_STRING(
    #version 330 core\n

    uniform sampler2D textureData;
    in vec2 tCoord;
    out vec4 color;

    void main() {
        color = texture(textureData, tCoord);
    }
);


void Init();
void GameTick(int tick);
void Draw(GameObject gameObject);
void Release();


int main() {
    sf::Window window(sf::VideoMode(600, 600), "Ride of the Metallica", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);
    Mesh truck = Mesh("file.obj", "pink.jpg", FongVertexShaderSource, FongFragShaderSource);
    Mesh road = Mesh("road.obj", "road.png", FongVertexShaderSource, FongFragShaderSource);
    
    vec3 lightPos(1.2f, 1.0f, 0.0f);
    vec3 view(0.5f, 0.0f, 0.0f);
    window.setActive(true);

    glewInit();

    Init();

    // Счётчик кадров
    int tickCounter = 0;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::Resized) {
                glViewport(0, 0, event.size.width, event.size.height);
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GameTick(tickCounter);
        // Отрисовываем все объекты сцены
        for (GameObject& object: gameObjects)
            Draw(object);

        tickCounter++;
        truck.Draw(lightPos, view);
        road.Draw(lightPos, view);
        window.display();
    }

    Release();
    return 0;
}


// Проверка ошибок OpenGL, если есть то вывод в консоль тип ошибки
void checkOpenGLerror() {
    GLenum errCode;
    // Коды ошибок можно смотреть тут
    // https://www.khronos.org/opengl/wiki/OpenGL_Error
    if ((errCode = glGetError()) != GL_NO_ERROR)
        std::cout << "OpenGl error!: " << errCode << std::endl;
}

// Функция печати лога шейдера
void ShaderLog(unsigned int shader)
{
    int infologLen = 0;
    int charsWritten = 0;
    char* infoLog;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLen);
    if (infologLen > 1)
    {
        infoLog = new char[infologLen];
        if (infoLog == NULL)
        {
            std::cout << "ERROR: Could not allocate InfoLog buffer" << std::endl;
            exit(1);
        }
        glGetShaderInfoLog(shader, infologLen, &charsWritten, infoLog);
        std::cout << "InfoLog: " << infoLog << "\n\n\n";
        delete[] infoLog;
    }
}


void InitObjects()
{
    GLuint vertexVBO;
    GLuint textureVBO;
    glGenBuffers(1, &vertexVBO);
    glGenBuffers(1, &textureVBO);
    VBOArray.push_back(vertexVBO);
    VBOArray.push_back(textureVBO);

    // Объявляем вершины треугольника
    Vertex triangle[] = {
        { -0.5f, -0.5f },
        { +0.5f, -0.5f },
        { +0.5f, +0.5f },

        { +0.5f, +0.5f },
        { -0.5f, +0.5f },
        { -0.5f, -0.5f },
    };

    // Объявляем текстурные координаты
    Vertex texture[] = {
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 1.0f, 0.0f },

        { 1.0f, 0.0f },
        { 0.0f, 0.0f },
        { 0.0f, 1.0f  },
    };

    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, textureVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texture), texture, GL_STATIC_DRAW);

    checkOpenGLerror();

    // Добавляем три одинаковых объект, менять им расположение мы будем потом при обработке каждого кадра
    for (int i = 0; i < numberOfSquares; ++i)
        gameObjects.push_back(GameObject{
                6,  // количество вершин в каждом буфере
                vertexVBO,
                textureVBO,
                textureData.getNativeHandle(), {0, 0} });
}


/*void InitShader() {
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &VertexShaderSource, NULL);
    glCompileShader(vShader);
    std::cout << "vertex shader \n";
    ShaderLog(vShader);

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &FragShaderSource, NULL);
    glCompileShader(fShader);
    std::cout << "fragment shader \n";
    ShaderLog(fShader);

    shaderInformation.shaderProgram = glCreateProgram();
    glAttachShader(shaderInformation.shaderProgram, vShader);
    glAttachShader(shaderInformation.shaderProgram, fShader);

    glLinkProgram(shaderInformation.shaderProgram);
    int link_status;
    glGetProgramiv(shaderInformation.shaderProgram, GL_LINK_STATUS, &link_status);
    if (!link_status)
    {
        std::cout << "error attach shaders \n";
        return;
    }

    shaderInformation.attribVertex =
        glGetAttribLocation(shaderInformation.shaderProgram, "vertCoord");
    if (shaderInformation.attribVertex == -1)
    {
        std::cout << "could not bind attrib vertCoord" << std::endl;
        return;
    }

    shaderInformation.attribTexture =
        glGetAttribLocation(shaderInformation.shaderProgram, "texureCoord");
    if (shaderInformation.attribTexture == -1)
    {
        std::cout << "could not bind attrib texureCoord" << std::endl;
        return;
    }

    shaderInformation.unifTexture =
        glGetUniformLocation(shaderInformation.shaderProgram, "textureData");
    if (shaderInformation.unifTexture == -1)
    {
        std::cout << "could not bind uniform textureData" << std::endl;
        return;
    }

    shaderInformation.unifShift = glGetUniformLocation(shaderInformation.shaderProgram, "shift");
    if (shaderInformation.unifShift == -1)
    {
        std::cout << "could not bind uniform angle" << std::endl;
        return;
    }
    checkOpenGLerror();
}*/


void InitTexture()
{
    const char* filename = "road.png";
    if (!textureData.loadFromFile(filename))
    {
        std::cout << "could not load texture" << std::endl;
    }
}

void Init() {
    //InitShader();
    InitTexture();
    InitObjects();
}


// Обработка шага игрового цикла
void GameTick(int tick) {
    int frequency = 100;
    for (int i = 0; i < numberOfSquares; ++i)
        gameObjects[i].shift[1] = 1.5f - ((tick + (frequency * i)) % (numberOfSquares * frequency)) / (float)frequency;
}


void Draw(GameObject gameObject) {
    glUseProgram(shaderInformation.shaderProgram);
    glUniform2fv(shaderInformation.unifShift, 1, gameObject.shift);

    glActiveTexture(GL_TEXTURE0);
    sf::Texture::bind(&textureData);
    glUniform1i(shaderInformation.unifTexture, 0);

    // Подключаем VBO
    glEnableVertexAttribArray(shaderInformation.attribVertex);
    glBindBuffer(GL_ARRAY_BUFFER, gameObject.vertexVBO);
    glVertexAttribPointer(shaderInformation.attribVertex, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(shaderInformation.attribTexture);
    glBindBuffer(GL_ARRAY_BUFFER, gameObject.textureVBO);
    glVertexAttribPointer(shaderInformation.attribTexture, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Передаем данные на видеокарту(рисуем)
    glDrawArrays(GL_TRIANGLES, 0, gameObject.buffers_size);

    // Отключаем массив атрибутов
    glDisableVertexAttribArray(shaderInformation.attribVertex);
    // Отключаем шейдерную программу
    glUseProgram(0);
    checkOpenGLerror();
}


void ReleaseShader() {
    glUseProgram(0);
    glDeleteProgram(shaderInformation.shaderProgram);
}

void ReleaseVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Чистим все выделенные VBO
    for (GLuint VBO: VBOArray)
        glDeleteBuffers(1, &VBO);
}

void Release() {
    ReleaseShader();
    ReleaseVBO();
}
