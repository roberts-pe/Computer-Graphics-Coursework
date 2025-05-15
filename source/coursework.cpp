//note to self: texture fixed, mess with lighting

#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/maths.hpp>
#include <common/camera.hpp>
#include <common/model.hpp>
#include <common/light.hpp>

// Function prototypes
void keyboardInput(GLFWwindow* window);
void mouseInput(GLFWwindow* window);

// Frame timers
float previousTime = 0.0f;
float deltaTime = 0.0f;

// Camera setup
Camera camera(glm::vec3(0.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f));
float radius = 4.0f;
float sensitivity = 0.002f;
// Debug mode
int debugMode = 0;

struct Object {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    float angle = 0.0f;
    std::string name;
};

int main(void) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Computer Graphics Coursework", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        glfwTerminate();
        return -1;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    unsigned int shaderID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");
    glUseProgram(shaderID);

    // Models
    Model Die("../assets/Die.obj");
    Model sphere("../assets/sphere.obj");
    Model Cube("../assets/inCube.obj");

    // Textures
    Die.addTexture("../assets/Die_Diffuse.jpg", "diffuse");
    Die.addTexture("../assets/Die_Normal.jpg", "normal");
    Die.addTexture("../assets/Die_Specular.jpg", "specular");
    Die.addTexture("../assets/Die_Glossiness.jpg", "glossiness");

    sphere.addTexture("../assets/white.jpg", "diffuse");
    sphere.addTexture("../assets/normal.jpg", "normal");
    sphere.addTexture("../assets/white.jpg", "roughness");
    sphere.addTexture("../assets/white.jpg", "metallic");
    sphere.addTexture("../assets/white.jpg", "specular");

    Cube.addTexture("../assets/AngryCube.jpg", "diffuse");

    // Material properties
    Die.ka = 0.01f; Die.kd = 0.6f; Die.ks = 0.4f;
    sphere.ka = 0.05f; sphere.kd = 0.5f; sphere.ks = 1.0f;
    Cube.ka = 0.05f; Cube.kd = 0.8f; Cube.ks = 0.0f;

    // Lights
    Light lightSources;
    lightSources.addDirectionalLight(glm::vec3(-0.3f, -1.0f, -0.2f), glm::vec3(5.0f, 3.5f, 2.0f));
    lightSources.addPointLight(glm::vec3(1.5f, 0.5f, 1.0f), glm::vec3(0.2f, 0.5f, 1.2f), 1.0f, 0.1f, 0.02f);
    lightSources.addSpotLight(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f, 8.0f, 0.0f), 1.0f, 0.1f, 0.02f, cos(glm::radians(20.0f)));

    // Scene objects
    glm::vec3 DiePositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 0.0f)
    };

    std::vector<Object> objects;
    Object obj;
    obj.name = "Die";
    for (unsigned int i = 0; i < 2; i++) {
        obj.position = DiePositions[i];
        obj.rotation = glm::vec3(1.0f, 1.0f, 1.0f);
        obj.scale = glm::vec3(0.5f);
        obj.angle = Maths::radians(20.0f * i);
        objects.push_back(obj);
    }

    // Camera orientation
    camera.yaw = glm::radians(0.0f);
    camera.pitch = glm::radians(-45.0f);
    camera.target = glm::vec3(0.0f);

    while (!glfwWindowShouldClose(window)) {
        float time = glfwGetTime();
        deltaTime = time - previousTime;
        previousTime = time;

        keyboardInput(window);
        mouseInput(window);

        glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.eye.x = camera.target.x + radius * cos(camera.pitch) * sin(camera.yaw);
        camera.eye.y = camera.target.y + radius * sin(camera.pitch);
        camera.eye.z = camera.target.z + radius * cos(camera.pitch) * cos(camera.yaw);
		camera.calculateMatrices();


        glUseProgram(shaderID);
        glUniform1i(glGetUniformLocation(shaderID, "debugMode"), debugMode);

        // Draw skybox
        glDepthMask(GL_FALSE);
        glm::mat4 skyboxModel = glm::mat4(1.0f);
        skyboxModel = glm::rotate(skyboxModel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        skyboxModel = glm::scale(skyboxModel, glm::vec3(0.04f));
        glm::mat4 MV_skybox = glm::mat4(glm::mat3(camera.view)) * skyboxModel;
        glm::mat4 MVP_skybox = camera.projection * MV_skybox;
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, &MVP_skybox[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "MV"), 1, GL_FALSE, &MV_skybox[0][0]);
        glDisable(GL_CULL_FACE);
        Cube.draw(shaderID);
        glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);

        // Scene lighting
        lightSources.toShader(shaderID, camera.view);
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "V"), 1, GL_FALSE, &camera.view[0][0]);

        for (Object& o : objects) {
            if (o.name == "Die") o.angle += 0.5f * deltaTime;

            glm::mat4 translate = Maths::translate(o.position);
            glm::mat4 scale = Maths::scale(o.scale);
            glm::mat4 rotate = Maths::rotate(o.angle, o.rotation);
            glm::mat4 model = translate * rotate * scale;
            glm::mat4 MV = camera.view * model;
            glm::mat4 MVP = camera.projection * MV;

            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MVP"), 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shaderID, "MV"), 1, GL_FALSE, &MV[0][0]);

            if (o.name == "Die") Die.draw(shaderID);
        }

        lightSources.draw(shaderID, camera.view, camera.projection, sphere);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void keyboardInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    glm::vec3 forward = glm::normalize(camera.target - camera.eye);
    glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));
    float speed = 5.0f * deltaTime;



    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.target += speed * forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.target -= speed * forward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.target -= speed * right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.target += speed * right;

    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) debugMode = 0;
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) debugMode = 1;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) debugMode = 2;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) debugMode = 3;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) debugMode = 4;
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) debugMode = 5;
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) debugMode = 6;
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) debugMode = 7;
    if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) debugMode = 8;
    if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) debugMode = 9;
}

void mouseInput(GLFWwindow* window) {
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    glfwSetCursorPos(window, 1920 / 2, 1080 / 2);

    camera.yaw += sensitivity * float(xPos - 1920 / 2);
    camera.pitch += sensitivity * float(1080 / 2 - yPos);

    float pitchLimit = glm::radians(89.0f);
    camera.pitch = glm::clamp(camera.pitch, -pitchLimit, pitchLimit);
}
