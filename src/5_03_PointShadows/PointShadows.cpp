#define STB_IMAGE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// glm数学库
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// imgui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// 用于创建立方体等元素
#include <geometry/BoxGeometry.h>
#include <geometry/SphereGeometry.h>
#include <geometry/PlaneGeometry.h>

#include <tools/stb_image.h>
#include <tools/shader.h>
#include <tools/camera.h>

#include <iostream>
#include <string>
#include <string_view>
#include <format>
#include <unordered_set>

static void processInput(GLFWwindow* window);
static void keyCallback(GLFWwindow* window, GLint key, GLint scancode, GLint action, GLint mods);
static void mouseCallback(GLFWwindow* window, GLdouble posX, GLdouble posY);

static GLuint loadTexture(std::string_view path);
static void renderScene(const Shader& shader);
static void renderQuad();

GLint SCREEN_WIDTH = 1280;
GLint SCREEN_HEIGHT = 720;

constexpr GLuint SHADOW_WIDTH = 1024;
constexpr GLuint SHADOW_HEIGHT = 1024;

// 摄像机
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
GLfloat lastX = SCREEN_WIDTH / 2.0f;
GLfloat lastY = SCREEN_HEIGHT / 2.0f;
bool isFirstMouse = true;
bool isMouseCaptured = true; // 初始为捕获状态（隐藏鼠标，控制视角）
bool useShadows = true;

// 时机
GLfloat deltaTime = 0.0f; // 当前帧与上一帧的时间差
GLfloat prevFrameTime = 0.0f; // 上一针的时间

// 几何形状
std::unique_ptr<BoxGeometry> cubeGeometry;

GLuint quadVAO = 0;
GLuint quadVBO;

int main()
{

    const GLchar* glslVersion = "#version 330";

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

    ImVec4 bgColor = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);

    Shader sceneShader(SHADER_DIR "/scene.vert", SHADER_DIR "/scene.frag");
    Shader lightObjShader(SHADER_DIR "/lightObj.vert", SHADER_DIR "/lightObj.frag");
    Shader simpleDepthShader(SHADER_DIR "/pointShadowsDepth.vert", SHADER_DIR "/pointShadowsDepth.frag", SHADER_DIR "/pointShadowsDepth.geom");

    cubeGeometry  = std::make_unique<BoxGeometry>(1.0f, 1.0f, 1.0f);
    SphereGeometry sphereGeometry(0.01f, 10.0f, 10.0f);

    GLuint woodTexture = loadTexture(ASSETS_DIR "/texture/wood.png");

    GLfloat lightPos[3] = { 0.0f, 0.0f, 0.0f };

    // 为渲染的深度贴图创建一个帧缓冲对象
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    
    // 创建一个2D纹理，提供给帧缓冲的深度缓冲使用
    GLuint depthCubeMap;
    glGenTextures(1, &depthCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
    for (GLuint i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr); // 纹理格式指定为GL_DEPTH_COMPONENT
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // 把生成的深度纹理作为帧缓冲的深度缓冲
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    sceneShader.use();
    sceneShader.setInt("diffuseTexture", 0);
    sceneShader.setInt("shadowMap", 1);    

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        GLfloat currFrameTime = static_cast<GLfloat>(glfwGetTime());
        deltaTime = currFrameTime - prevFrameTime;
        prevFrameTime = currFrameTime;        

        // 开始 ImGui 帧
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        ImGui::Begin("ImGui");
            ImGui::Text("ESC: Exit  L: Lock/Unlock Cursor");
            ImGui::Text("WASD: Movement  Space: Up  LCtrl: Down");
            ImGui::Text("U: Turn On/Off Shadows");
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Text("FOV: %.1f", camera.Zoom);
            ImGui::Text("x: %.1f, y: %.1f, z: %.1f", camera.Position.x, camera.Position.y, camera.Position.z);
            ImGui::SliderInt("Screen Width", &SCREEN_WIDTH, 800, 1920);
            ImGui::SliderInt("Screen Height", &SCREEN_HEIGHT, 600, 1080);
            ImGui::SliderFloat3("Light Position", lightPos, -5.0f, 5.0f);
            ImGui::Checkbox("Use Shadows", &useShadows);
        ImGui::End();

        glm::vec3 curLightPos(lightPos[0], lightPos[1], lightPos[2]);

        // ------------------------------------------------------------
        // 渲染指令
        glClearColor(bgColor.x, bgColor.y, bgColor.z, bgColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1.将场景深度渲染为纹理（从灯光的角度）
        GLfloat aspect = (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT;
        GLfloat nearPlane = 1.0f;
        GLfloat farPlane  = 25.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(curLightPos, curLightPos + glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3( 0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(curLightPos, curLightPos + glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3( 0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(curLightPos, curLightPos + glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3( 0.0f,  0.0f,  1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(curLightPos, curLightPos + glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3( 0.0f,  0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(curLightPos, curLightPos + glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3( 0.0f, -1.0f,  0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(curLightPos, curLightPos + glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3( 0.0f, -1.0f,  0.0f)));

        simpleDepthShader.use();
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            for (GLuint i = 0; i < 6; ++i)
            {
                simpleDepthShader.setMat4(std::format("shadowMatrices[{}]", i), shadowTransforms[i]);
            }
            simpleDepthShader.setFloat("farPlane", farPlane);
            simpleDepthShader.setVec3("lightPos", curLightPos);
            renderScene(simpleDepthShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2.使用生成的深度/阴影贴图，正常渲染场景
        // 重置视口大小
        glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        sceneShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        sceneShader.setMat4("projection", projection);
        sceneShader.setMat4("view", view);
        // 设置其它属性
        sceneShader.setVec3("viewPos", camera.Position);
        sceneShader.setVec3("lightPos", curLightPos);
        sceneShader.setFloat("farPlane", farPlane);        
        sceneShader.setBool("useShadows", useShadows);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
        renderScene(sceneShader);

        // 渲染灯光
        lightObjShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, curLightPos);
        lightObjShader.setMat4("projection", projection);
        lightObjShader.setMat4("view", view);
        lightObjShader.setMat4("model", model);
        glBindVertexArray(sphereGeometry.VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sphereGeometry.indices.size()), GL_UNSIGNED_INT, 0);

        // ImGui 渲染
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 资源释放
    cubeGeometry->dispose();
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);

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

    if (key == GLFW_KEY_U && action == GLFW_RELEASE)
    {
        useShadows = !useShadows;
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

void renderScene(const Shader& shader)
{
    // ------------------------------------------------------------
    // Room cube
    glBindVertexArray(cubeGeometry->VAO);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(10.0f));
    shader.setMat4("model", model);
    shader.setFloat("uvScale", 4.0f);
    glDisable(GL_CULL_FACE);
    shader.setInt("isReverseNormals", 1);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(cubeGeometry->indices.size()), GL_UNSIGNED_INT, 0);
    shader.setInt("isReverseNormals", 0);
    glEnable(GL_CULL_FACE);

    // ------------------------------------------------------------
    // cubes
    std::vector<glm::vec3> cubePositions
    {
        glm::vec3( 4.0f, -3.5f,  0.0f),
        glm::vec3( 2.0f,  3.0f,  1.0f),
        glm::vec3(-3.0f, -1.0f,  0.0f),
        glm::vec3(-1.5f,  1.0f,  1.5f),
        glm::vec3(-1.5f,  2.0f, -3.0f)
    };

    std::vector<GLfloat> rotateAngles
    {
         0.0f,
         0.0f,
         0.0f,
         0.0f,
        60.0f
    };

    std::vector<GLfloat> scaleFactors
    {
        1.0f,
        1.5f,
        1.0f,
        1.0f,
        1.5f
    };    
    
    glBindVertexArray(cubeGeometry->VAO);
    shader.setFloat("uvScale", 1.0f);
    for (GLuint i = 0; i < cubePositions.size(); ++i)
    {
        model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);
        model = glm::rotate(model, glm::radians(rotateAngles[i]), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
        model = glm::scale(model, glm::vec3(scaleFactors[i]));
        shader.setMat4("model", model);
        
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(cubeGeometry->indices.size()), GL_UNSIGNED_INT, 0);
    }
}

void renderQuad()
{
    if (quadVAO == 0)
    {
        GLfloat quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}