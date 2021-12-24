#include "Mesh.h"
Mesh::Mesh() {
    Init();
}

Mesh::Mesh(std::string mesh_filename, const char* texture_filename) {
    InitShader();
    InitTexture(texture_filename);
    SetupMesh(mesh_filename);
    glEnable(GL_DEPTH_TEST);
}

void Mesh::Draw() {
    glUseProgram(Program);
    glUniform1f(Unif_posX, posX);
    glUniform1f(Unif_posY, posY);
    glUniform1f(Unif_sizeX, sizeX);
    glUniform1f(Unif_sizeY, sizeY);
    glActiveTexture(GL_TEXTURE0);
    // Обёртка SFML на opengl функцией glBindTexture
    sf::Texture::bind(&textureData);
    glUniform1i(unifTexture, 0);
    glBindVertexArray(VAO);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

}

void Mesh::Draw(vec3 lightpos, vec3 view, float lightXAngle, float lightYAngle, float lightIntens) {
    glUseProgram(Program);
    glUniform1f(Unif_posX, posX);
    glUniform1f(Unif_posY, posY);
    glUniform1f(Unif_sizeX, sizeX);
    glUniform1f(Unif_sizeY, sizeY);
    glUniform3f(Attrib_light, lightXAngle, lightYAngle, lightIntens);

    glUniform3f(unifViewVec, view.x, view.y, view.z);
    glActiveTexture(GL_TEXTURE0);
    // Обёртка SFML на opengl функцией glBindTexture
    sf::Texture::bind(&textureData);
    glUniform1i(unifTexture, 0);
    glBindVertexArray(VAO);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

}

void Mesh::IncXPos(float d) {
    posX += d;
}

void Mesh::IncYPos(float d) {
    posY += d;
}

void Mesh::IncXSize(float d) {
    sizeX += d;
}

void Mesh::IncYSize(float d) {
    sizeY += d;
}

void Mesh::Release() {
    ReleaseShader();
    ReleaseVBO();
}

// Проверка ошибок OpenGL, если есть то вывод в консоль тип ошибки

inline void Mesh::checkOpenGLerror() {
    GLenum errCode;
    // Коды ошибок можно смотреть тут
    // https://www.khronos.org/opengl/wiki/OpenGL_Error
    if ((errCode = glGetError()) != GL_NO_ERROR)
        std::cout << "OpenGl error!: " << errCode << std::endl;
}

// Функция печати лога шейдера

inline void Mesh::ShaderLog(unsigned int shader)
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

inline void Mesh::InitVBO()
{
    float vertices[] = {
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    glGenVertexArrays(1, &VAO);
    glGenVertexArrays(1, &VBO);
    glGenBuffers(1, &IBO);
    // 1. bind Vertex Array Object
    glBindVertexArray(VAO);
    // 2. copy our vertices array in a vertex buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // 3. copy our index array in a element buffer for OpenGL to use
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // 4. then set the vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

inline void Mesh::InitShader() {
    // Создаем вершинный шейдер
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    // Передаем исходный код
    glShaderSource(vShader, 1, &VertexShaderSource, NULL);
    // Компилируем шейдер
    glCompileShader(vShader);
    std::cout << "vertex shader \n";
    ShaderLog(vShader);

    // Создаем фрагментный шейдер
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    // Передаем исходный код
    glShaderSource(fShader, 1, &FragShaderSource, NULL);
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
    Attrib_light = glGetAttribLocation(Program, attr_name);
    if (Attrib_light == -1)
    {
        std::cout << "could not bind attrib" << attr_name << std::endl;
        return;
    }
    checkOpenGLerror();
}

inline void Mesh::InitTexture(const char* filename)
{
    // Загружаем текстуру из файла
    if (!textureData.loadFromFile(filename))
    {
        // Не вышло загрузить картинку
        return;
    }
    // Теперь получаем openGL дескриптор текстуры
    textureHandle = textureData.getNativeHandle();
}

inline void Mesh::Init() {
    InitShader();
    InitVBO();
    glEnable(GL_DEPTH_TEST);
}

// Освобождение шейдеров

inline void Mesh::ReleaseShader() {
    // Передавая ноль, мы отключаем шейдрную программу
    glUseProgram(0);
    // Удаляем шейдерную программу
    glDeleteProgram(Program);
}

// Освобождение буфера

inline void Mesh::ReleaseVBO()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VBO);
}

inline void Mesh::ReadVectorsFromFile(std::string filename) {
    std::ifstream infile(filename);
    std::string descr;
    while (infile >> descr) {
        if (descr == "v")
        {
            float x, y, z;
            infile >> x >> y >> z;
            x = x / 20;
            y = y / 20;
            z = z / 20;
            positions.push_back({ x,y,z });
        }
        else if (descr == "vt")
        {
            float x, y;
            infile >> x >> y;
            uvs.push_back({ x,y });
        }
        else if (descr == "vn")
        {
            float x, y, z;
            infile >> x >> y >> z;
            normals.push_back({ x,y,z });
        }
        else if (descr == "f")
        {
            // get triangle
            for (size_t i = 0; i < 3; i++)
            {
                int p_ind;
                char c;
                // read vertex index
                infile >> p_ind;
                infile >> c;
                // read uv index
                int uv_ind;

                infile >> uv_ind;
                infile >> c;
                // read normal index
                int n_ind;

                infile >> n_ind;

                vertices.push_back({ positions[p_ind - 1],normals[n_ind - 1],uvs[uv_ind - 1] });
            }
        }
    }

    indices = std::vector<unsigned int>(vertices.size());

    for (size_t i = 0; i < indices.size(); i++)
    {
        indices[i] = i;
    }
}

inline void Mesh::SetupMesh(std::string filename)
{
    ReadVectorsFromFile(filename);
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &IBO);

    glBindVertexArray(VAO);
    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(vec3), &positions[0], GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);


    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
    checkOpenGLerror();
}
