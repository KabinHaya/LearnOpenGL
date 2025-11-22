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
#include <random>

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
bool useSSAO = true;

// 时机
float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float prevFrameTime = 0.0f; // 上一针的时间

static float ourLerp(float a, float b, float f)
{
    return a + f * (b - a);
}

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
    glEnable(GL_CULL_FACE);

    float lightPos[3] = { 2.0f, 4.0f, -2.0f };
    float lightColor[3] = { 0.2f, 0.2f, 0.7f };

    ImVec4 bgColor = ImVec4(0.02f, 0.02f, 0.03f, 1.0f);

    Shader shaderGeometryPass(SHADER_DIR "/geometryPass.vert", SHADER_DIR "/geometryPass.frag");
    Shader shaderLightingPass(SHADER_DIR "/lightingPass.vert", SHADER_DIR "/lightingPass.frag");
    Shader shaderSSAO(SHADER_DIR "/ssao.vert", SHADER_DIR "/ssao.frag");
    Shader shaderSSAOBlur(SHADER_DIR "/ssaoBlur.vert", SHADER_DIR "/ssaoBlur.frag");
    Shader shaderLightObj(SHADER_DIR "/lightObj.vert", SHADER_DIR "/lightObj.frag");
    
    SphereGeometry pointLightGeometry(0.05f);
    BoxGeometry roomGeometry(15.0f, 15.0f, 15.0f);
    PlaneGeometry frameGeometry(2.0f, 2.0f);    

    // stbi_set_flip_vertically_on_load(true);
    Model ourModel(ASSETS_DIR "/model/backpack/backpack.obj");
    // Model ourModel(ASSETS_DIR "/model/nanosuit/nanosuit.obj");

    // ------------------------------------------------------------
    // 创建GBuffer
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPosition, gNormal, gAlbedo;

    // 位置颜色缓冲
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // 法线颜色缓冲
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // 颜色 + 颜色缓冲
    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    // 深度缓冲
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ------------------------------------------------------------
    // 还要创建帧缓冲区来保存SSAO处理阶段的数据
    unsigned int ssaoFBO, ssaoBlurFBO;
    glGenFramebuffers(1, &ssaoFBO);
    glGenFramebuffers(1, &ssaoBlurFBO);
    unsigned int ssaoColorBuffer, ssaoColorBufferBlur;

    // SSAO 颜色缓冲
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;

    // 模糊阶段
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);

    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ------------------------------------------------------------
    // 生成采样的核
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f); // 生成在范围[0, 1]之间的随机浮点数
    std::default_random_engine generator;
    std::vector<glm::vec3> ssaoKernel;
    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        // 缩放样本，使得样本尽量靠近核的中心
        scale = ourLerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    // ------------------------------------------------------------
    // 生成噪声材质
    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; ++i)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0,
            randomFloats(generator) * 2.0 - 1.0,
            0.0f);
        ssaoNoise.push_back(noise);
    }
    unsigned int noiseTexture;
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, ssaoNoise.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // ------------------------------------------------------------
    shaderLightingPass.use();
    shaderLightingPass.setInt("gPosition", 0);
    shaderLightingPass.setInt("gNormal", 1);
    shaderLightingPass.setInt("gAlbedo", 2);
    shaderLightingPass.setInt("ssao", 3);
    shaderLightingPass.setFloat("shininess", 32.0f);

    shaderSSAO.use();
    shaderSSAO.setInt("gPosition", 0);
    shaderSSAO.setInt("gNormal", 1);
    shaderSSAO.setInt("texNoise", 2);

    shaderSSAOBlur.use();
    shaderSSAOBlur.setInt("ssaoInput", 0);

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
            ImGui::SliderFloat3("Light Position", lightPos, -15.0f, 15.0f);
            ImGui::SliderFloat3("Light Color", lightColor, 0.0f, 2.0f);
            ImGui::Checkbox("SSAO", &useSSAO);
        ImGui::End();

        glm::vec3 curLightPos(lightPos[0], lightPos[1], lightPos[2]);
        glm::vec3 curLightColor(lightColor[0], lightColor[1], lightColor[2]);

        // ------------------------------------------------------------
        // 1. 几何阶段：将场景的几何/颜色数据渲染到 G-Buffer 中
        // glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0);
        glClearColor(0, 0, 0, 1.0);

        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        shaderGeometryPass.use();
        shaderGeometryPass.setMat4("projection", projection);
        shaderGeometryPass.setMat4("view", view);
        
        // 正方体房间
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0, 7.0f, 0.0f));
        shaderGeometryPass.setMat4("model", model);
        shaderGeometryPass.setBool("invertedNormals", true); // 在房间内，反转法向量
        glCullFace(GL_FRONT);
        drawMesh(roomGeometry);
        glCullFace(GL_BACK);
        shaderGeometryPass.setBool("invertedNormals", false);

        // 在地板上的背包
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
        shaderGeometryPass.setMat4("model", model);
        ourModel.Draw(shaderGeometryPass);

        // ------------------------------------------------------------
        // 2.生成 SSAO 贴图
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
        glClear(GL_COLOR_BUFFER_BIT);

        shaderSSAO.use();
        for (unsigned int i = 0; i < 64; ++i)
            shaderSSAO.setVec3(std::format("samples[{}]", i), ssaoKernel[i]);
        shaderSSAO.setMat4("projection", projection);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        drawMesh(frameGeometry);

        // ------------------------------------------------------------
        // 3.模糊 SSAO 材质来去除噪声
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        shaderSSAOBlur.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
        drawMesh(frameGeometry);

        // ------------------------------------------------------------
        // 4.光照阶段: 传统延迟布林冯光照 + SSAO
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderLightingPass.use();
        shaderLightingPass.use();
        shaderLightingPass.setBool("useSSAO", useSSAO);

        glm::vec3 lightPosView = glm::vec3(camera.GetViewMatrix() * glm::vec4(curLightPos, 1.0));

        shaderLightingPass.setVec3(std::format("pointLights[{}].position", 0), lightPosView);
        shaderLightingPass.setVec3(std::format("pointLights[{}].ambient", 0), 0.3f, 0.3f, 0.3f);
        shaderLightingPass.setVec3(std::format("pointLights[{}].diffuse", 0), curLightColor);
        shaderLightingPass.setVec3(std::format("pointLights[{}].specular", 0), 0.1f, 0.1f, 0.1f);

        const float constant = 1.0f; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
        const float linear = 0.09f;
        const float quadratic = 0.032f;
        shaderLightingPass.setFloat(std::format("pointLights[{}].constant", 0), constant);
        shaderLightingPass.setFloat(std::format("pointLights[{}].linear", 0), linear);
        shaderLightingPass.setFloat(std::format("pointLights[{}].quadratic", 0), quadratic);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedo);
        glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
        drawMesh(frameGeometry);

        // ------------------------------------------------------------
        // 4.5. 将几何阶段的深度缓冲区内容复制到默认帧缓冲区的深度缓冲区中
        // 延迟结合正向渲染
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // 指定默认的帧缓冲为写缓冲
        // 复制gbuffer的深度信息到默认帧缓冲的深度缓冲
        glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

        // ------------------------------------------------------------
        // 5. 在场景之上渲染光源
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 绘制灯光物体
        shaderLightObj.use();
        shaderLightObj.setMat4("projection", projection);
        shaderLightObj.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, curLightPos);

        shaderLightObj.setMat4("model", model);
        shaderLightObj.setVec3("lightColor", curLightColor);

        drawMesh(pointLightGeometry);

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

// 绘制物体
void drawMesh(const BufferGeometry& geometry)
{
    glBindVertexArray(geometry.VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(geometry.indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
