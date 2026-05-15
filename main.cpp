#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "renderer/Shader.h"
#include "renderer/Framebuffer.h"
#include "camera/Camera.h"
#include "terrain/TerrainManager.h"
#include "environment/EnvironmentManager.h"
#include "environment/Sky.h"
#include "environment/Water.h"
#include "ecosystem/Vegetation.h"
#include "presentation/PresentationManager.h"
#include "database/AssetDatabase.h"
#include "gui/ImGuiManager.h"
#include "gui/GUIManager.h"

#include <iostream>

// --- Settings ---
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// --- Global Engine State ---
Camera camera(glm::vec3(0.0f, 30.0f, 100.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool cursorEnabled = false;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
PresentationManager presManager;

// Rendering State
bool wireframe = false;
float exposure = 1.2f;
float bloomStrength = 0.5f;
glm::vec3 tint = glm::vec3(1.0f, 1.0f, 1.0f);

// --- Callbacks ---
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow* window);

int main() {
    // --- Initialize GLFW ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Procedural Environmental Simulator v1.0", NULL, NULL);
    if (window == NULL) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- Systems Initialization ---
    ImGuiManager::Init(window);
    GUIManager gui;
    AssetDatabase db;
    db.LoadDatabase("assets/database/assets.json", "assets/database/biomes.json");

    Shader terrainShader("assets/shaders/terrain.vs", "assets/shaders/terrain.fs");
    Shader skyShader("assets/shaders/sky.vs", "assets/shaders/sky.fs");
    Shader waterShader("assets/shaders/water.vs", "assets/shaders/water.fs");
    Shader instancedShader("assets/shaders/instanced.vs", "assets/shaders/instanced.fs");
    Shader postShader("assets/shaders/postprocess.vs", "assets/shaders/postprocess.fs");
    
    TerrainGenerator generator;
    generator.database = &db;
    generator.UpdateSettings();
    TerrainManager terrainManager(generator);
    EnvironmentManager envManager;
    Sky sky(2500.0f);
    Water water(3000.0f);
    Framebuffer mainFBO(SCR_WIDTH, SCR_HEIGHT);
    Mesh* quad = Mesh::CreateQuad();

    InstancedMesh* treeAsset = VegetationGenerator::CreateTree();
    InstancedMesh* grassAsset = VegetationGenerator::CreateGrass();

    // --- Render Loop ---
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        presManager.Update(camera, currentFrame, deltaTime);
        envManager.Update(deltaTime);
        terrainManager.Update(camera.Position);

        // --- 1. World Rendering (HDR FBO) ---
        mainFBO.Bind();
        glClearColor(envManager.fogColor.r, envManager.fogColor.g, envManager.fogColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 5000.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // Sky
        skyShader.use();
        skyShader.setMat4("projection", projection); skyShader.setMat4("view", view);
        skyShader.setVec3("skyColorTop", envManager.skyColorTop);
        skyShader.setVec3("skyColorBottom", envManager.skyColorBottom);
        skyShader.setVec3("sunDir", envManager.sunDir);
        sky.Draw();

        // Terrain
        if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        terrainShader.use();
        terrainShader.setMat4("projection", projection); terrainShader.setMat4("view", view);
        terrainShader.setMat4("model", glm::mat4(1.0f));
        terrainShader.setVec3("lightDir", envManager.sunDir);
        terrainShader.setVec3("sunColor", envManager.sunColor);
        terrainShader.setVec3("ambientColor", envManager.ambientColor);
        terrainShader.setVec3("viewPos", camera.Position);
        terrainShader.setFloat("fogDensity", envManager.fogDensity);
        terrainShader.setVec3("fogColor", envManager.fogColor);
        terrainManager.Draw(terrainShader);

        // Ecosystem
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        instancedShader.use();
        instancedShader.setMat4("projection", projection); instancedShader.setMat4("view", view);
        instancedShader.setVec3("lightDir", envManager.sunDir); instancedShader.setVec3("sunColor", envManager.sunColor);
        instancedShader.setVec3("ambientColor", envManager.ambientColor); instancedShader.setVec3("viewPos", camera.Position);
        instancedShader.setFloat("fogDensity", envManager.fogDensity); instancedShader.setVec3("fogColor", envManager.fogColor);
        instancedShader.setFloat("time", currentFrame);
        instancedShader.setFloat("windSpeed", 2.0f); instancedShader.setFloat("windStrength", 0.2f);

        for (auto const& [key, chunk] : terrainManager.chunks) {
            if (!chunk->treeTransforms.empty()) { treeAsset->UpdateInstances(chunk->treeTransforms); treeAsset->DrawInstanced(); }
            if (!chunk->grassTransforms.empty()) { grassAsset->UpdateInstances(chunk->grassTransforms); grassAsset->DrawInstanced(); }
        }

        // Water
        waterShader.use();
        waterShader.setMat4("projection", projection); waterShader.setMat4("view", view);
        waterShader.setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0, envManager.waterHeight, 0)));
        waterShader.setVec3("viewPos", camera.Position); waterShader.setVec3("skyColorTop", envManager.skyColorTop);
        waterShader.setFloat("fogDensity", envManager.fogDensity); waterShader.setVec3("fogColor", envManager.fogColor);
        waterShader.setFloat("time", currentFrame);
        waterShader.setFloat("waveSpeed", envManager.waveSpeed); waterShader.setFloat("waveStrength", envManager.waveStrength);
        water.Draw();

        mainFBO.Unbind();

        // --- 2. Post-Processing & GUI ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        postShader.use();
        postShader.setFloat("exposure", exposure); postShader.setFloat("bloomStrength", bloomStrength);
        postShader.setVec3("tint", tint);
        glBindTexture(GL_TEXTURE_2D, mainFBO.textureColorBuffer);
        quad->Draw();

        ImGuiManager::BeginFrame();
        gui.Render(generator, terrainManager, envManager, presManager, camera, exposure, bloomStrength, tint, wireframe, deltaTime);
        ImGuiManager::EndFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete treeAsset; delete grassAsset; delete quad;
    ImGuiManager::Shutdown();
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (!cursorEnabled && !presManager.cinematicMode) {
        camera.MovementSpeed = 50.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera.ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera.ProcessKeyboard(DOWN, deltaTime);
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS) {
        cursorEnabled = !cursorEnabled;
        glfwSetInputMode(window, GLFW_CURSOR, cursorEnabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (cursorEnabled || presManager.cinematicMode) return;
    float xpos = (float)xposIn; float ypos = (float)yposIn;
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    float xoffset = xpos - lastX; float yoffset = lastY - ypos;
    lastX = xpos; lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { camera.ProcessMouseScroll((float)yoffset); }
