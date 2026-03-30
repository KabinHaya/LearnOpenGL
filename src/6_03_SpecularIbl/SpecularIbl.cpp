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
#include <format>

static void processInput(GLFWwindow* window);
static void keyCallback(GLFWwindow* window, GLint key, GLint scancode, GLint action, GLint mods);
static void mouseCallback(GLFWwindow* window, GLdouble posX, GLdouble posY);

static GLuint loadTexture(std::string_view path);
static GLuint loadHdrTexture(std::string_view path);
static void drawMesh(BufferGeometry geometry);
static void drawLightObject(Shader shader, BufferGeometry geometry, glm::vec3 position);
static void renderQuad();


int SCREEN_WIDTH = 1280;
int SCREEN_HEIGHT = 720;

// 摄像机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
GLfloat lastX = SCREEN_WIDTH / 2.0f;
GLfloat lastY = SCREEN_HEIGHT / 2.0f;
bool isFirstMouse = true;
bool isMouseCaptured = true; // 初始为捕获状态（隐藏鼠标，控制视角）

// 时机
GLfloat deltaTime = 0.0f; // 当前帧与上一帧的时间差
GLfloat prevFrameTime = 0.0f; // 上一针的时间

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
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, GLint width, GLint height)
        {
            glViewport(0, 0, width, height);
            SCREEN_WIDTH = width;
            SCREEN_HEIGHT = height;
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
    glDepthFunc(GL_LEQUAL);

    //确保立方体正确采样，去除边缘接缝
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    std::vector<glm::vec3> lightPositions
    {
        glm::vec3(-10.0f,  10.0f, 10.0f),
        glm::vec3( 10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3( 10.0f, -10.0f, 10.0f)
    };

    // 点光源颜色
    std::vector<glm::vec3> lightColors
    {
        glm::vec3(300.0f),
        glm::vec3(300.0f),
        glm::vec3(300.0f),
        glm::vec3(300.0f)
    };

    Shader sceneShader(SHADER_DIR "/pbr.vert", SHADER_DIR "/pbr.frag");
    Shader lightObjShader(SHADER_DIR "/lightObj.vert", SHADER_DIR "/lightObj.frag");
    Shader equirectangularToCubemapShader(SHADER_DIR "/cubemap.vert", SHADER_DIR "/cubemap.frag");
    Shader irradianceShader(SHADER_DIR "/irradiance.vert", SHADER_DIR "/irradiance.frag");

    Shader prefilterShader(SHADER_DIR "/prefilter.vert", SHADER_DIR "/prefilter.frag");
    Shader brdfShader(SHADER_DIR "/brdf.vert", SHADER_DIR "/brdf.frag");
    Shader testBrdfShader(SHADER_DIR "/testBrdf.vert", SHADER_DIR "/testBrdf.frag");

    Shader backgroundShader(SHADER_DIR "/background.vert", SHADER_DIR "/background.frag");
    
    SphereGeometry pointLightGeometry(0.05f, 10.0f, 10.0f);
    SphereGeometry objectGeometry(1.0f, 64.0f, 64.0f);      // 圆球
    BoxGeometry envCubeGeometry(5.0f, 5.0f, 5.0f);
    PlaneGeometry quadGeometry(2.0f, 2.0f);

    Model ourModel(ASSETS_DIR "/model/cerberus/Cerberus.obj");

    GLuint hdrMap = loadHdrTexture(ASSETS_DIR "/texture/Alexs_Apt_2k.hdr");
    //GLuint hdrMap = loadHdrTexture(ASSETS_DIR "/texture/Ditch-River_2k.hdr");

    ImVec4 bgColor = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);

    float spacing = 2.5;

    float albedoFactor[3] = { 0.5f, 0.0f, 0.0f };
    float metallicFactor = 0.5f;
    float roughnessFactor = 0.5f;
    float aoFactor = 1.0f;

    sceneShader.use();
    sceneShader.setInt("irradianceMap", 0);
    sceneShader.setInt("prefilterMap", 1);
    sceneShader.setInt("brdfLUT", 2);
    sceneShader.setInt("material.albedoMap", 3);
    sceneShader.setInt("material.normalMap", 4);
    sceneShader.setInt("material.metallicMap", 5);
    sceneShader.setInt("material.roughnessMap", 6);
    sceneShader.setInt("material.aoMap", 7);

    backgroundShader.use();
    backgroundShader.setInt("envMap", 0);

    GLuint albedoMap1 = loadTexture(ASSETS_DIR "/texture/solar/TexturesCom_PaintedConcreteFloor_1K_albedo.png");
    GLuint normalMap1 = loadTexture(ASSETS_DIR "/texture/solar/TexturesCom_PaintedConcreteFloor_1K_normal.png");
    GLuint metallicMap1 = loadTexture(ASSETS_DIR "/texture/solar/TexturesCom_PaintedConcreteFloor_1K_metallic.png");
    GLuint roughnessMap1 = loadTexture(ASSETS_DIR "/texture/solar/TexturesCom_PaintedConcreteFloor_1K_roughness.png");
    GLuint aoMap1 = loadTexture(ASSETS_DIR "/texture/solar/TexturesCom_PaintedConcreteFloor_1K_ao.png");

    GLuint albedoMap2 = loadTexture(ASSETS_DIR "/texture/gold/fancy-scaled-gold_albedo.png");
    GLuint normalMap2 = loadTexture(ASSETS_DIR "/texture/gold/fancy-scaled-gold_normal-ogl.png");
    GLuint metallicMap2 = loadTexture(ASSETS_DIR "/texture/gold/fancy-scaled-gold_metallic.png");
    GLuint roughnessMap2 = loadTexture(ASSETS_DIR "/texture/gold/fancy-scaled-gold_roughness.png");
    GLuint aoMap2 = loadTexture(ASSETS_DIR "/texture/gold/fancy-scaled-gold_ao.png");

    // ------------------------------------------------------------
    GLuint captureFBO;
    GLuint captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // ------------------------------------------------------------
    GLuint envCubemap;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (GLuint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // ------------------------------------------------------------
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] =
    {
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // ------------------------------------------------------------
    equirectangularToCubemapShader.use();
    equirectangularToCubemapShader.setInt("equirectangularMap", 0);
    equirectangularToCubemapShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrMap);

    glViewport(0, 0, 512, 512);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (GLuint i = 0; i < 6; ++i)
    {
        equirectangularToCubemapShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawMesh(envCubeGeometry);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ------------------------------------------------------------
    GLuint irradianceMap;
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (GLuint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    // ------------------------------------------------------------
    irradianceShader.use();
    irradianceShader.setInt("envMap", 0);
    irradianceShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glViewport(0, 0, 32, 32);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (GLuint i = 0; i < 6; ++i)
    {
        irradianceShader.setMat4("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawMesh(envCubeGeometry);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ------------------------------------------------------------
    GLuint prefilterMap;
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (GLuint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // ------------------------------------------------------------
    prefilterShader.use();
    prefilterShader.setInt("envMap", 0);
    prefilterShader.setMat4("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    GLuint maxMipLevels = 5;
    for (GLuint mip = 0; mip < maxMipLevels; ++mip)
    {
        GLuint mipWidth = static_cast<GLuint>(128 * std::pow(0.5, mip));
        GLuint mipHeight = static_cast<GLuint>(128 * std::pow(0.5, mip));
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader.setFloat("roughness", roughness);
        for (GLuint i = 0; i < 6; ++i)
        {
            prefilterShader.setMat4("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            drawMesh(envCubeGeometry);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ------------------------------------------------------------
    GLuint brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);

    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    brdfShader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawMesh(quadGeometry);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);    

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    const char* skyboxes[] = { "Env Cubemap", "Irradiance Map", "Prefilter Map"};
    int skyboxIdx = 0;

    bool enableTestBrdf = false;
    bool useTexture = true;

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
            ImGui::Combo("Skybox", &skyboxIdx, skyboxes, IM_ARRAYSIZE(skyboxes));
            ImGui::Checkbox("Test BRDF", &enableTestBrdf);
            ImGui::Checkbox("Texture", &useTexture);
            if (!useTexture)
            {
                ImGui::SliderFloat3("Albedo", albedoFactor, 0.0f, 1.0f);
                ImGui::SliderFloat("Roughness", &roughnessFactor, 0.0f, 1.0f);
                ImGui::SliderFloat("Metallic", &metallicFactor, 0.0f, 1.0f);
                ImGui::SliderFloat("AO", &aoFactor, 0.0f, 1.0f);
            }
        ImGui::End();


        // glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

        // ------------------------------------------------------------
        // 渲染指令
        glClearColor(bgColor.x, bgColor.y, bgColor.z, bgColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        std::vector<glm::vec3> newPositions(4);
        for (GLuint i = 0; i < 4; ++i)
        {
            newPositions[i] = lightPositions[i] + glm::vec3(sin(currentFrameTime * 5.0) * 5.0, 0.0, 0.0);
        }

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT), 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        lightObjShader.use();
        lightObjShader.setMat4("projection", projection);
        lightObjShader.setMat4("view", view);
        for (GLuint i = 0; i < 4; ++i)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, newPositions[i]);
            lightObjShader.setMat4("model", model);
            lightObjShader.setVec3("lightColor", lightColors[i]);
            drawMesh(pointLightGeometry);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

        if (useTexture)
        {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, albedoMap1);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, normalMap1);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, metallicMap1);
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, roughnessMap1);
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_2D, aoMap1);
        }

        sceneShader.use();
        sceneShader.setBool("useTexture", useTexture);
        sceneShader.setVec3("albedoFactor", albedoFactor[0], albedoFactor[1], albedoFactor[2]);
        sceneShader.setFloat("metallicFactor", metallicFactor);
        sceneShader.setFloat("roughnessFactor", roughnessFactor);
        sceneShader.setFloat("aoFactor", aoFactor);

        sceneShader.setVec3("camPos", camera.Position);
        sceneShader.setMat4("projection", projection);
        sceneShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-1 * spacing, 0.0f, 0.0f));
        sceneShader.setMat4("model", model);
        drawMesh(objectGeometry);

        if (useTexture)
        {
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, albedoMap2);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, normalMap2);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, metallicMap2);
            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D, roughnessMap2);
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_2D, aoMap2);
        }

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0 * spacing, 0.0f, 0.0f));
        sceneShader.setMat4("model", model);
        drawMesh(objectGeometry);

        // TODO: 正确读取模型的材质
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1 * spacing, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        sceneShader.setMat4("model", model);
        ourModel.Draw(sceneShader);

        for (GLuint i = 0; i < lightPositions.size(); ++i)
        {
            sceneShader.setVec3(std::format("pointLights[{}].position", i), newPositions[i]);
            sceneShader.setVec3(std::format("pointLights[{}].color", i), lightColors[i]);
        }
        
        backgroundShader.use();
        backgroundShader.setMat4("projection", projection);
        backgroundShader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        switch (skyboxIdx)
        {
        case 0:
            glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
            break;
        case 1:
            glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
            break;
        case 2:
            glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
            break;
        }

        drawMesh(envCubeGeometry);

        if (enableTestBrdf)
        {
            glViewport(0, 0, 512, 512);
            testBrdfShader.use();
            testBrdfShader.setInt("brdfTexture", 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
            drawMesh(quadGeometry);
            glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        }

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

void keyCallback(GLFWwindow* window, GLint key, GLint scancode, GLint action, GLint mods)
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

void mouseCallback(GLFWwindow* window, GLdouble posXIn, GLdouble posYIn)
{
    if (!isMouseCaptured)
        return; // 如果鼠标未被捕获（即已释放），不处理视角移动

    GLfloat posX = static_cast<GLfloat>(posXIn);
    GLfloat posY = static_cast<GLfloat>(posYIn);

    if (isFirstMouse)
    {
        lastX = posX;
        lastY = posY;
        isFirstMouse = false;
    }

    GLfloat offsetX = posX - lastX;
    GLfloat offsetY = lastY - posY;

    lastX = posX;
    lastY = posY;

    camera.ProcessMouseMovement(offsetX, offsetY);
}

GLuint loadTexture(std::string_view path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    // 图像y轴翻转
    stbi_set_flip_vertically_on_load(true);
    GLint width, height, nrComponents;
    stbi_uc* data = stbi_load(path.data(), &width, &height, &nrComponents, 0);
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

GLuint loadHdrTexture(std::string_view path)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float* data = stbi_loadf(path.data(), &width, &height, &nrComponents, 0);
    if (data)
    {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cout << "Failed to load HDR image at path: " << path << std::endl;
    }
    stbi_image_free(data);

    return textureID;
}

// 绘制物体
void drawMesh(BufferGeometry geometry)
{
    glBindVertexArray(geometry.VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(geometry.indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}