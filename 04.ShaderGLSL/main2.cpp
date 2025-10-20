#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

const char* vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "layout (location = 1) in vec3 aColor;\n"
                                 "out vec3 ourColor;\n"
                                 "void main()\n"
                                 "{\n"
                                 "gl_Position = vec4(aPos, 1.0f);\n"
                                 "ourColor = aColor;\n"
                                 "}\n";
const char* fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "in vec3 ourColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "FragColor = vec4(ourColor, 1.0f);\n"
                                   "}\n";

static void ProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // 这是创建的窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // 设置视口
    // 从左下到右上
    // 这是渲染窗口
    glViewport(0, 0, 800, 600);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
        {
            glViewport(0, 0, width, height);
        });

    // 定义顶点教程
    float vertices[] = {
        // 位置              // 颜色
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // 右下
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // 左下
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // 顶部
    };

    // 创建VBO
    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);

    // 绑定 VAO 对象
    glBindVertexArray(VAO);

    // 绑定缓冲
    // 填充数据
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // 最后一个参数用来将绘制数据放到合适位置，不变的数据放普速存储区，变得多的放在高速存储区

    // 设置顶点位置属性指针
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 设置顶点颜色属性指针
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // 创建顶点和片段着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); // 创建一个顶点着色器类型的 着色器
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // 附加着色器代码
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // 检测是否编译成功
    int isSuccess;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isSuccess);
    if (!isSuccess)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "EROOR::SHADER::VERTEX::COMPILATION_FAILED\n"
            << infoLog
            << std::endl;
    }

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isSuccess);
    if (!isSuccess)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cout << "EROOR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
            << infoLog
            << std::endl;
    }
    
    // 创建程序对象
    unsigned int shaderProgram = glCreateProgram();
    // 将着色器附加到程序对象上
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    // 链接
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isSuccess);
    if (!isSuccess)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "EROOR::SHADER_PROGRAM_LINK_FAILED\n"
            << infoLog
            << std::endl;
    }

    // 使用着色器程序
    glUseProgram(shaderProgram);

    // 删除着色器
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 设置填充绘制模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    while (!glfwWindowShouldClose(window))
    {
        ProcessInput(window);

        // 渲染指令
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        //glDrawArrays(GL_POINTS, 0, 3);
        //glDrawArrays(GL_LINE_LOOP, 0, 3);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 资源释放
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}