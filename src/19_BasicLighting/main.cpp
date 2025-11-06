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
#include <tools/shader.h>
#include <tools/stb_image.h>
#include <tools/camera.h>

#include <iostream>
#include <string>
#include <format>

static void processInput(GLFWwindow* window);
static void mouseCallback(GLFWwindow* window, double posX, double posY);
static void scrollCallback(GLFWwindow* window, double offsetX, double offsetY);

const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;

// 摄像机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool isFirstMouse = true;

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
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    Shader ourShader(std::string(SHADER_DIR) + "/vertex.glsl", std::string(SHADER_DIR) + "/fragment.glsl");
    Shader lightObjShader(std::string(SHADER_DIR) + "/lightObjVert.glsl", std::string(SHADER_DIR) + "/lightObjFrag.glsl");
    
    BoxGeometry boxGeometry(1.0f, 1.0f, 1.0f);
    SphereGeometry sphereGeometry(0.1f, 10.0f, 10.0f);

    // 生成纹理
    unsigned int texture1, texture2;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    // 纹理1

    // 设置环绕和过滤方式
    float borderColor[] = { 0.3f, 0.1f, 0.7f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 图像y轴翻转
    stbi_set_flip_vertically_on_load(true);

    // 加载图片

    std::string imgPath = std::string(ASSETS_DIR) + "/texture/container.jpg";
    
    int width, height, nrChannels;
    unsigned char* data = stbi_load(imgPath.c_str(), &width, &height, &nrChannels, 0);


    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);

    // 纹理2

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    imgPath = std::string(ASSETS_DIR) + "/texture/awesomeface.png";

    data = stbi_load(imgPath.c_str(), &width, &height, &nrChannels, 0);


    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);

    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // 光照信息
    glm::vec3 lightPosition = glm::vec3(1.0f, 1.5f, 0.0f); // 光照位置
    ourShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    ourShader.setFloat("ambientStrength", 0.1f);

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
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        // ------------------------------------------------------------
        // 渲染指令
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ------------------------------------------------------------
        // 设置灯光物体的着色器
        lightObjShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 curLightPos = glm::vec3(lightPosition.x * glm::sin(glfwGetTime()), lightPosition.y, lightPosition.z);
        model = glm::translate(model, curLightPos);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT), 0.1f, 100.0f);
        
        lightObjShader.setMat4("model", model);
        lightObjShader.setMat4("view", view);
        lightObjShader.setMat4("projection", projection);
        
        glBindVertexArray(sphereGeometry.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<int>(sphereGeometry.indices.size()), GL_UNSIGNED_INT, 0);

        // ------------------------------------------------------------
        // 设置物体的着色器
        ourShader.use();
        model = glm::mat4(1.0f);
        model = glm::rotate(model, static_cast<float>(glfwGetTime()) * glm::radians(-45.0f), glm::vec3(1.0f, 1.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.4f));

        ourShader.setVec3("lightPos", curLightPos);
        ourShader.setMat4("model", model);
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);
        ourShader.setVec3("viewPos", camera.Position);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glBindVertexArray(boxGeometry.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<int>(boxGeometry.indices.size()), GL_UNSIGNED_INT, 0);
        
        // ImGui 渲染
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 资源释放
    boxGeometry.dispose();
    sphereGeometry.dispose();

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

void mouseCallback(GLFWwindow* window, double posXIn, double posYIn)
{
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
