// Lab12.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <gl/glew.h>
#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "Mesh.h"
int main()
{
#pragma region FONG_SHADERS
    const char* FongVertexShaderSource = R"(
    #version 330 core
    uniform float positX;
    uniform float positY;
    uniform float sizeX;
    uniform float sizeY;

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
        )* mat3(
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
        uniform vec3 lightParams;
  
        uniform vec3 lightPos = vec3(0,0,-1); 
        uniform vec3 view;
        uniform sampler2D textureData;

        void main() {

        vec3 lightPosit = lightPos * mat3( 1, 0, 0,
        0, cos(lightParams[0]), -sin(lightParams[0]),
        0, sin(lightParams[0]), cos(lightParams[0]) ) *
        mat3( cos(lightParams[1]), 0, sin(lightParams[1]),
        0, 1, 0,
        -sin(lightParams[1]), 0, cos(lightParams[1])
        );

        vec3 lightColor = vec3(1.0,0.5,0.5);
        vec3 objectColor = texture(textureData, textcoord).rgb;
            // ambient
            float ambient = 0.4;
            //vec3 ambient = ambientStrength * lightColor;
  	
            // diffuse 
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPosit - FragPos);
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
            if (lightParams[2] == 0.0f)
            {
                ambient = 0.0f; diffuse = 0.0f; specular = 0.0f;
            }
            vec3 result = (ambient + diffuse + specular) * lightColor * objectColor;
            FragColor = vec4(result, 1.0);
        }
        )";
#pragma endregion

    const char* GrassVertexShaderSource = R"(
    #version 330 core
    uniform float positX;
    uniform float positY;
    uniform float sizeX;
    uniform float sizeY;

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
        )* mat3(
            sizeX, 0, 0,
            0, sizeY, 0,
            0, 0, sizeX
        )* mat3(
            1, 0, 0,
            0, 1, 0,
            0, 0, -1
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
        Normal = normal2;
        FragPos = position;
    }
    )";

    // Исходный код фрагментного шейдера
    const char* GrassFragShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        in vec2 textcoord;
        in vec3 Normal;  
        in vec3 FragPos;  
        uniform vec3 lightParams;
  
        uniform vec3 lightPos = vec3(0,0,-1); 
        uniform vec3 view;
        uniform sampler2D textureData;

        void main() {

        vec3 lightPosit = lightPos * mat3( 1, 0, 0,
        0, cos(lightParams[0]), -sin(lightParams[0]),
        0, sin(lightParams[0]), cos(lightParams[0]) ) *
        mat3( cos(lightParams[1]), 0, sin(lightParams[1]),
        0, 1, 0,
        -sin(lightParams[1]), 0, cos(lightParams[1])
        );

        vec3 lightColor = vec3(1.0,0.5,0.5);
        vec3 objectColor = texture(textureData, textcoord).rgb;
            // ambient
            float ambient = 0.1;
            //vec3 ambient = ambientStrength * lightColor;
  	
            // diffuse 
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPosit - FragPos);
            float diffuse = max(dot(norm, lightDir), 0.0);
            //vec3 diffuse = diff * lightColor;
    
            // specular
            float specularStrength = 0.7;
            vec3 viewDir = normalize(view - FragPos);
            vec3 reflectDir = reflect(lightDir, norm);  
            //vec3 specular = specularStrength * spec * lightColor;  
            //vec3 specular = vec3(0,0,0); 
            if (lightParams[2] < 0.9f)
            {
                ambient = 0.0f; diffuse = 0.0f;
            }
            vec3 result = (ambient + diffuse) * lightColor * objectColor;
            FragColor = vec4(result, 1.0);
        }
 )";

    
    sf::Window window(sf::VideoMode(600, 600), "My OpenGL window", sf::Style::Default, sf::ContextSettings(24));
    window.setVerticalSyncEnabled(true);

    window.setActive(true);

    // Инициализация glew
    glewInit();
    Mesh grass = Mesh("grass.obj", "grass.png", GrassVertexShaderSource, GrassFragShaderSource);
    Mesh grass2 = Mesh("grass.obj", "grass.png", GrassVertexShaderSource, GrassFragShaderSource);
    Mesh truck = Mesh("bus2.obj","bus2.png", FongVertexShaderSource, FongFragShaderSource);
    Mesh road = Mesh("road.obj", "road.png", FongVertexShaderSource, FongFragShaderSource);

    truck.IncXPos(6.0f);
    truck.IncYPos(-4.1f);
    road.IncXPos(5.8f);
    road.IncYPos(-4.15f);
    grass.IncXPos(7.1f);
    grass.IncYPos(-3.7f);
    grass2.IncXPos(3.45f);
    grass2.IncYPos(-7.7f);
    //road.IncXSize(1.0f);
    road.IncYSize(1.0f);
    //grass.IncXSize(1.0f);
    //grass.IncYSize(1.0f);
    // lighting
    float lightXAngle = 0.0f;
    float lightYAngle = 0.0f;
    float lightIntens = 1.0f;
    vec3 lightPos(1.2f, 1.0f, 0.0f);
    vec3 view(0.5f, 0.0f, 0.0f);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::Resized) {
                glViewport(0, 0, event.size.width, event.size.height);
            }
            // обработка нажатий клавиш
            else if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                case (sf::Keyboard::A): lightXAngle += 0.05f; break;
                case (sf::Keyboard::D): lightXAngle -= 0.05f; break;
                case (sf::Keyboard::W): lightYAngle += 0.05f; break;
                case (sf::Keyboard::S): lightYAngle += 0.05f; break;
                case (sf::Keyboard::F): lightIntens = lightIntens == 0.0? 1.0f : 0.0f; break;
                default: break;
                }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        grass.Draw(lightPos, view, lightXAngle, lightYAngle, lightIntens);
        grass2.Draw(lightPos, view, lightXAngle, lightYAngle, lightIntens);
        truck.Draw(lightPos, view, lightXAngle, lightYAngle, lightIntens);
        road.Draw(lightPos, view, lightXAngle, lightYAngle, lightIntens);

        window.display();
    }

    truck.Release();
    return 0;
}

