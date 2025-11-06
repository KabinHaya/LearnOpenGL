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
static void scrollCallback(GLFWwindow* window, double offsetX, double offsetY);
static unsigned int loadTexture(std::string_view path);

const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;

// 摄像机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool isFirstMouse = true;
bool isMouseCaptured = true; // 初始为捕获状态（隐藏鼠标，控制视角）

// 时机
float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float prevFrameTime = 0.0f; // 上一针的时间


// 绘制原则
// 1.先绘制所有不透明的物体
// 2.对所有透明的物体排序
// 3.按顺序绘制所有透明的物体，以由远到近的方式进行
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
    glfwSetScrollCallback(window, scrollCallback);
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    std::vector<glm::vec3> cubePositions
    {
        glm::vec3( 1.5f,  0.0f,  0.0f),
        glm::vec3(-1.5f,  0.0f, -1.0f)
    };

    // 草或者窗户
    std::vector<glm::vec3> windowPositions
    {
        glm::vec3(-1.5f, 0.0f, -0.48f),
        glm::vec3( 1.5f, 0.0f,  0.51f),
        glm::vec3( 0.0f, 0.0f,  0.7f),
        glm::vec3(-0.3f, 0.0f, -2.3f),
        glm::vec3( 0.5f, 0.0f, -0.6f)
    };

    std::vector<glm::vec3> pointLightPositions
    {
        glm::vec3( 2.0f,  1.5f, -1.0f),
        glm::vec3(-2.0f,  1.5f, -1.0f),
        glm::vec3( 2.0f,  1.5f, -3.0f),
        glm::vec3(-2.0f,  1.5f, -3.0f)
    };

    ImVec4 bgColor = ImVec4(0.15f, 0.15f, 0.20f, 1.0f);

    Shader sceneShader(std::string(SHADER_DIR) + "/scene.vert", std::string(SHADER_DIR) + "/scene.frag");
    Shader lightingShader(std::string(SHADER_DIR) + "/lighting.vert", std::string(SHADER_DIR) + "/lighting.frag");
    
    BoxGeometry boxGeometry(1.0f, 1.0f, 1.0f);
    SphereGeometry sphereGeometry(0.1f, 10.0f, 10.0f);
    PlaneGeometry planeGeometry(1.0f, 1.0f);
        
    unsigned int boxMap    =  loadTexture(std::string(ASSETS_DIR) + "/texture/metal.png");
    unsigned int floorMap  =  loadTexture(std::string(ASSETS_DIR) + "/texture/wood.png");
    unsigned int windowMap =  loadTexture(std::string(ASSETS_DIR) + "/texture/blending_transparent_window.png");    

    sceneShader.use();
    sceneShader.setInt("tempTex", 0);
    // sceneShader.setInt("material.diffuse", 0);
    // sceneShader.setInt("material.specular", 0);

    // 定向光
    sceneShader.setVec3("dirLight.direction", -1.0f, -1.0f, -1.0f);
    sceneShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    sceneShader.setVec3("dirLight.diffuse", 0.6f, 0.6f, 0.6f);
    sceneShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    // 4个点光源
    for (size_t i = 0; i < pointLightPositions.size(); ++i)
    {
        sceneShader.setVec3(std::format("pointLights[{}].position", i), pointLightPositions[i]);
        sceneShader.setVec3(std::format("pointLights[{}].ambient", i), 0.05f, 0.05f, 0.05f);
        sceneShader.setVec3(std::format("pointLights[{}].diffuse", i), 0.8f, 0.8f, 0.8f);
        sceneShader.setVec3(std::format("pointLights[{}].specular", i), 1.0f, 1.0f, 1.0f);
        sceneShader.setFloat(std::format("pointLights[{}].constant", i), 1.0f);
        sceneShader.setFloat(std::format("pointLights[{}].linear", i), 0.09f);
        sceneShader.setFloat(std::format("pointLights[{}].quadratic", i), 0.032f);
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
        ImGui::End();

        // ------------------------------------------------------------
        // 渲染指令
        glClearColor(bgColor.x, bgColor.y, bgColor.z, bgColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        std::map<float, glm::vec3> sorted; // 先给窗户排个序
        for (size_t i = 0; i < windowPositions.size(); i++)
        {
            float distance = glm::length(camera.Position - windowPositions[i]);
            sorted[distance] = windowPositions[i];
        }

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        // 创建灯光
        lightingShader.use();
        glBindVertexArray(sphereGeometry.VAO);
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        for (unsigned int i = 0; i < pointLightPositions.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            lightingShader.setMat4("model", model);
            glDrawElements(GL_TRIANGLES, static_cast<int>(sphereGeometry.indices.size()), GL_UNSIGNED_INT, 0);
        }

        sceneShader.use();
        sceneShader.setMat4("projection", projection);
        sceneShader.setMat4("view", view);
        sceneShader.setVec3("viewPos", camera.Position);

        glActiveTexture(GL_TEXTURE0);

        // 创建地面
        glBindVertexArray(planeGeometry.VAO);        
        glBindTexture(GL_TEXTURE_2D, floorMap);
        sceneShader.setFloat("material.shininess", 2.0f);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.5f));
        model = glm::scale(model, glm::vec3(10.0f));
        sceneShader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, static_cast<int>(planeGeometry.indices.size()), GL_UNSIGNED_INT, 0);

        // 创建箱子
        glBindVertexArray(boxGeometry.VAO);
        glBindTexture(GL_TEXTURE_2D, boxMap);
        sceneShader.setFloat("material.shininess", 32.0f);
        for (unsigned int i = 0; i < cubePositions.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            sceneShader.setMat4("model", model);
            glDrawElements(GL_TRIANGLES, static_cast<int>(boxGeometry.indices.size()), GL_UNSIGNED_INT, 0);
        }

        // 创建窗户，由远到近
        glBindVertexArray(planeGeometry.VAO);
        glBindTexture(GL_TEXTURE_2D, windowMap);
        sceneShader.setFloat("material.shininess", 16.0f);
        for (auto iter = sorted.rbegin(); iter != sorted.rend(); iter++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, iter->second);
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            sceneShader.setMat4("model", model);
            glDrawElements(GL_TRIANGLES, static_cast<int>(planeGeometry.indices.size()), GL_UNSIGNED_INT, 0);
        }

        // ImGui 渲染
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 资源释放
    boxGeometry.dispose();
    sphereGeometry.dispose();
    planeGeometry.dispose();

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 问题在于斜着走会更快，不过这个暂时不用考虑
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::DOWN, deltaTime);
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

void scrollCallback(GLFWwindow* window, double offsetX, double offsetY)
{
    camera.ProcessMouseScroll(static_cast<float>(offsetY));
}

unsigned int loadTexture(std::string_view path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

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