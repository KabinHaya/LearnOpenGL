#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <geometry/BoxGeometry.h>
#include <geometry/SphereGeometry.h>
#include <geometry/PlaneGeometry.h>
#include <tools/shader.h>
#include <tools/stb_image.h>
#include <tools/camera.h>
#include <tools/mesh.h>
#include <tools/model.h>

#include <iostream>
#include <string>
#include <string_view>
#include <format>

static void processInput(GLFWwindow* window);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouseCallback(GLFWwindow* window, double posX, double posY);

static unsigned int loadTexture(std::string_view path);

int SCREEN_WIDTH = 1280;
int SCREEN_HEIGHT = 720;

// 摄像机
Camera camera(glm::vec3(0.0f, 0.0f, 55.0f));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool isFirstMouse = true;
bool isMouseCaptured = true; // 初始为捕获状态（隐藏鼠标，控制视角）

// 时机
float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float prevFrameTime = 0.0f; // 上一针的时间

int main()
{
    const char* glslVersion = "#version 330";

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 这是创建的窗口
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    /*
        回调函数注册
        1.注册窗口变化监听
        2.注册鼠标事件
    */
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
        {
            glViewport(0, 0, width, height);
        });

    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);

    glfwSetScrollCallback(window, [](GLFWwindow* window, double offsetX, double offsetY)
        {
            camera.ProcessMouseScroll(static_cast<float>(offsetY));
        });
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ------------------------------------------------------------
    // 创建 imgui 上下文
    ImGui::CreateContext();

    // 设置样式
    ImGui::StyleColorsDark();
    // 设置平台和渲染器
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);
    // ------------------------------------------------------------

    // 设置视口
    // 从左下到右上
    // 这是渲染窗口
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    ImVec4 bgColor = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);

    Shader planetShader(SHADER_DIR "/planet.vert", SHADER_DIR "/planet.frag");
    Shader meteoriteShader(SHADER_DIR "/meteorite.vert", SHADER_DIR "/meteorite.frag");

    Model planetModel(ASSETS_DIR "/model/planet/planet.obj");
    Model rockModel(ASSETS_DIR "/model/rock/rock.obj");
    
    unsigned int amount = 1000;
    std::vector<glm::mat4> modelMatrices(amount);
    srand(static_cast<unsigned int>(glfwGetTime())); // 初始化随机种子
    float radius = 50.0f;
    float offset = 2.5f;
    for (unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1.位移：分布在半径为radius的圆形上，偏移的范围是 [-offset, offset]
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = glm::sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.4f; // 让行星带的高度比x和z的宽度要小
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = glm::cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        // 2.缩放：在0.05和0.25之间缩放
        float scale = static_cast<float>(rand() % 20) / 100.0f + 0.05f;
        model = glm::scale(model, glm::vec3(scale));

        // 3.旋转：绕着一个（半）随机选择的旋转轴向量进行随机的旋转
        float rotAngle = static_cast<float>(rand() % 360);
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        // 4.添加到矩阵的数组中
        modelMatrices[i] = model;
    }

    // 配置实例化数组
    unsigned int instancingBuffer;
    glGenBuffers(1, &instancingBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, instancingBuffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), modelMatrices.data(), GL_STATIC_DRAW);

    for (unsigned int i = 0; i < rockModel.meshes.size(); i++)
    {
        unsigned int VAO = rockModel.meshes[i].VAO;
        glBindVertexArray(VAO);
        // 给矩阵设置属性指针
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        float currentFrameTime = static_cast<float>(glfwGetTime());
        deltaTime = currentFrameTime - prevFrameTime;
        prevFrameTime = currentFrameTime;

        // 开始 ImGui 帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::Begin("ImGui");
            ImGui::Text("ESC: Exit  L: Lock/Unlock Cursor");
            ImGui::Text("WASD: Movement  Space: Up  LCtrl: Down");
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("FOV: %.1f", camera.Zoom);
            ImGui::Text("x: %.1f, y: %.1f, z: %.1f", camera.Position.x, camera.Position.y, camera.Position.z);
            ImGui::Text("Actual resolution");
            ImGui::SliderInt("Width", &SCREEN_WIDTH, 800, 1920);
            ImGui::SliderInt("Height", &SCREEN_HEIGHT, 600, 1080);
        ImGui::End();

        // ------------------------------------------------------------
        // 渲染指令
        glClearColor(bgColor.x, bgColor.y, bgColor.z, bgColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 1.0f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();;
        glm::mat4 model = glm::mat4(1.0f);

        // 设置星球        
        planetShader.use();
        planetShader.setMat4("projection", projection);
        planetShader.setMat4("view", view);
        model = glm::scale(model, glm::vec3(4.0f));
        model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
        planetShader.setMat4("model", model);

        planetModel.Draw(planetShader);

        // 设置岩石
        meteoriteShader.use();
        meteoriteShader.setMat4("projection", projection);
        meteoriteShader.setMat4("view", view);
        meteoriteShader.setInt("diffuseTexture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, rockModel.textures_loaded[0].id);
        for (unsigned int i = 0; i < rockModel.meshes.size(); i++)
        {
            glBindVertexArray(rockModel.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(rockModel.meshes[i].indices.size()), GL_UNSIGNED_INT, 0, amount);
            glBindVertexArray(0);
        }

        // ImGui 渲染
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 资源释放

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    std::unordered_set<Camera_Movement> operations;

    // 问题在于斜着走会更快，不过这个暂时不用考虑
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        operations.insert(Camera_Movement::FORWARD);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        operations.insert(Camera_Movement::BACKWARD);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        operations.insert(Camera_Movement::LEFT);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        operations.insert(Camera_Movement::RIGHT);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        operations.insert(Camera_Movement::UP);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        operations.insert(Camera_Movement::DOWN);

    if (!operations.empty())
        camera.ProcessKeyboard(operations, deltaTime);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    { // 仅在按下瞬间触发
        isMouseCaptured = !isMouseCaptured;
        if (isMouseCaptured)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            isFirstMouse = true;
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void mouseCallback(GLFWwindow* window, double posXIn, double posYIn)
{
    if (!isMouseCaptured)
        return; // 如果鼠标未被捕获（即已释放），不处理视角移动

    float posX = static_cast<float>(posXIn);
    float posY = static_cast<float>(posYIn);

    if (isFirstMouse)
    {
        lastX = posX;
        lastY = posY;
        isFirstMouse = false;
    }

    float offsetX = posX - lastX;
    float offsetY = lastY - posY;

    lastX = posX;
    lastY = posY;

    camera.ProcessMouseMovement(offsetX, offsetY);
}

unsigned int loadTexture(std::string_view path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    // 图像y轴翻转
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path.data(), &width, &height, &nrComponents, 0);
    if (data)
    {
        /*
            jpg 和 png格式不一样
            jpg只有3个通道
            png有4个通道，第4个通道设置透明度
        */
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }
    stbi_image_free(data);

    return textureID;
}