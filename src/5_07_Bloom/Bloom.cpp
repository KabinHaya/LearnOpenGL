#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
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
static void drawMesh(const BufferGeometry& geometry);

const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;

// 摄像机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool isFirstMouse = true;
bool isMouseCaptured = true; // 初始为捕获状态（隐藏鼠标，控制视角）
bool useBloom = true;

// 时机
float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float prevFrameTime = 0.0f; // 上一针的时间

// 基于4_05
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);

    std::vector<glm::vec3> cubePositions
    {
        glm::vec3( 1.5f,  0.0f,  0.0f),
        glm::vec3(-1.5f,  0.0f, -1.0f)
    };

    std::vector<glm::vec3> pointLightPositions
    {
        glm::vec3( 0.0f, 0.5f,  1.5f),
        glm::vec3(-4.0f, 0.5f, -3.0f),
        glm::vec3( 3.0f, 0.5f,  1.0f),
        glm::vec3(-0.8f, 2.4f, -1.0f)
    };

    std::vector<glm::vec3> pointLightColors
    {
        glm::vec3(2.0f, 1.0f, 1.0f),
        glm::vec3(4.0f, 0.0f, 4.0f),
        glm::vec3(1.0f, 1.0f, 4.0f),
        glm::vec3(0.0f, 2.0f, 0.0f)
    };

    ImVec4 bgColor = ImVec4(0.02f, 0.02f, 0.03f, 1.0f);

    Shader sceneShader(SHADER_DIR "/scene.vert", SHADER_DIR "/scene.frag");
    Shader lightObjShader(SHADER_DIR "/lightObj.vert", SHADER_DIR "/lightObj.frag");
    Shader blurShader(SHADER_DIR "/blur.vert", SHADER_DIR "/blur.frag");
    Shader bloomFinalShader(SHADER_DIR "/bloomFinal.vert", SHADER_DIR "/bloomFinal.frag");
    
    BoxGeometry boxGeometry(1.0f, 1.0f, 1.0f);
    BoxGeometry pointLightGeometry(0.2f, 0.2f, 0.2f);
    PlaneGeometry floorGeometry(10.0f, 10.0f);
    PlaneGeometry frameGeometry(2.0f, 2.0f);
        
    unsigned int boxMap     =  loadTexture(ASSETS_DIR "/texture/container2.png");
    unsigned int boxSpecMap = loadTexture(ASSETS_DIR "/texture/container2_specular.png");
    unsigned int floorMap   =  loadTexture(ASSETS_DIR "/texture/wood.png");

    float exposure = 0.5f;
    float bloomThreshold = 1.0f;

    sceneShader.use();
    sceneShader.setInt("material.diffuse", 0);
    sceneShader.setInt("material.specular", 1);

    // 4个点光源
    for (size_t i = 0; i < pointLightPositions.size(); ++i)
    {
        sceneShader.setVec3(std::format("pointLights[{}].position", i), pointLightPositions[i]);
        sceneShader.setVec3(std::format("pointLights[{}].ambient", i), 0.01f, 0.01f, 0.01f);
        sceneShader.setVec3(std::format("pointLights[{}].diffuse", i), pointLightColors[i]);
        sceneShader.setVec3(std::format("pointLights[{}].specular", i), 0.1f, 0.1f, 0.1f);
        sceneShader.setFloat(std::format("pointLights[{}].constant", i), 1.0f);
        sceneShader.setFloat(std::format("pointLights[{}].linear", i), 0.09f);
        sceneShader.setFloat(std::format("pointLights[{}].quadratic", i), 0.032f);
    }

    // 帧缓冲设置
    blurShader.use();
    blurShader.setInt("image", 0);
    bloomFinalShader.use();
    bloomFinalShader.setInt("screenTex", 0);
    bloomFinalShader.setInt("bloomBlur", 1);

    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // ------------------------------------------------------------
    // 创建两个颜色缓冲区，一个用于渲染，一个用于亮度提取
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr); // GL_RGB16F  GL_FLOAT 更改
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // 使用帧缓冲区的颜色附件
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std:: endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ------------------------------------------------------------
    // 创建用于模糊的帧缓冲区
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; ++i)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT,
            0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            pingpongColorbuffers[i], 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
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
            ImGui::SliderFloat("HDR Exposure", &exposure, 0.0f, 1.0f);
            ImGui::Checkbox("Bloom", &useBloom);
            ImGui::SliderFloat("Bloom Threshold", &bloomThreshold, 0.0f, 5.0f);
        ImGui::End();

        // ------------------------------------------------------------
        // 1.将场景渲染到缓冲区
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

        glClearColor(bgColor.x, bgColor.y, bgColor.z, bgColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        // 创建灯光
        // 绘制灯光物体
        lightObjShader.use();
        lightObjShader.setFloat("bloomThreshold", bloomThreshold);
        lightObjShader.setMat4("projection", projection);
        lightObjShader.setMat4("view", view);
        for (unsigned int i = 0; i < pointLightPositions.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            lightObjShader.setMat4("model", model);
            lightObjShader.setVec3("lightColor", pointLightColors[i]);
            drawMesh(pointLightGeometry);
        }

        // 绘制场景
        sceneShader.use();
        sceneShader.setMat4("projection", projection);
        sceneShader.setMat4("view", view);
        sceneShader.setVec3("viewPos", camera.Position);
        sceneShader.setFloat("bloomThreshold", bloomThreshold);


        // 创建地面
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorMap);
        sceneShader.setFloat("uvScale", 4.0f);
        sceneShader.setFloat("material.shininess", 32.0f);
        model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -0.5f));
        sceneShader.setMat4("model", model);
        drawMesh(floorGeometry);

        // 创建箱子
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, boxMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, boxSpecMap);
        sceneShader.setFloat("uvScale", 1.0f);
        sceneShader.setFloat("material.shininess", 4.0f);
        for (unsigned int i = 0; i < cubePositions.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            sceneShader.setMat4("model", model);
            drawMesh(boxGeometry);
        }

        // ------------------------------------------------------------
        // 2.高斯模糊去处理明亮的片段
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        bool isHorizontal = true;
        bool isFirstIteration = true;
        unsigned int amount = 10;
        blurShader.use();
        glActiveTexture(GL_TEXTURE0);
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[isHorizontal]);
            blurShader.setInt("isHorizontal", isHorizontal);
            glBindTexture(GL_TEXTURE_2D, isFirstIteration ? colorBuffers[1] : pingpongColorbuffers[!isHorizontal]);  // bind texture of other framebuffer (or scene if first iteration)            
            drawMesh(frameGeometry);
            isHorizontal = !isHorizontal;
            if (isFirstIteration)
                isFirstIteration = false;
        }

        // ------------------------------------------------------------
        // 3.绘制hdr输出的texture
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        bloomFinalShader.use();
        bloomFinalShader.setFloat("exposure", exposure);
        bloomFinalShader.setBool("useBloom", useBloom);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!isHorizontal]);
        drawMesh(frameGeometry);

        // ImGui 渲染
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    std::unordered_set<Camera_Movement> operations{};

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

    camera.ProcessKeyboard(operations, deltaTime);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_L && action == GLFW_RELEASE)
    {
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

// 绘制物体
void drawMesh(const BufferGeometry& geometry)
{
    glBindVertexArray(geometry.VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(geometry.indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
