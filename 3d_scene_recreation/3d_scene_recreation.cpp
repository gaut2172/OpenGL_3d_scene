// Author: Joshua Gauthier
// Also credit to Southern New Hampshire University

#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "Cylinder.h"
#include "Sphere.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility functions


using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Breakfast"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 500;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao[6];         // Handle for the vertex array object
        GLuint vbos[10];     // Handles for the vertex buffer objects
        GLuint nIndices;    // Number of indices of the mesh
    };

    // plane structure
    struct plane {
        vector<float> verts;
        vector<int> indices;
    };

    struct cube {
        vector<float> verts;
        vector<int> indices;
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    // Texture
    GLuint gTextureTable, gTextureCup, gTextureTea, gTextureLemon, gTextureOrange, gTextureCloth, gTexturePlate;
    glm::vec2 gUVScale(1.0f, 1.0f);
    // Shader program
    GLuint gProgramId;
    //GLuint gProgramId2;

    // camera
    // constructor format: Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH)
    Camera gCamera(glm::vec3(-4.0f, 6.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -70.0, -60.0);
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;
    float addedSpeed = 0.05f;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // cylinders
    Cylinder cylinder1(1.0f, 1.5f, 2.0f, 25, 8, true);
    Cylinder cylinder2(1.35f, 1.35f, 0.1f, 25, 8, true);
    Cylinder cylinder3(1.4f, 1.9, 0.25f, 25, 8, true);

    // sphere
    //Sphere::Sphere(float radius, int sectors, int stacks, bool smooth)
    Sphere sphere1(0.9, 36, 18, true);

    // plane
    plane plane1 = {};

    // cube
    cube cube1 = {};

    // ortho and perspective global variables
    glm::mat4 projection;
    bool select_ortho = false;

    // lighting global variables
    //--------------------------
    // light 1
    glm::vec3 gLightPosition1(3.0f, 5.0f, -5.0f);
    glm::vec3 gLightColor1(1.0f, 1.0f, 1.0f);
    GLfloat light_1_strength = 1.0f;
    // light 2
    glm::vec3 gLightPosition2(-4.0f, 3.0f, 3.0f);
    glm::vec3 gLightColor2(0.5f, 0.5f, 1.0f);
    GLfloat light_2_strength = 1.0f;
    // base object color
    //glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    // light components
    glm::vec3 gAmbientStrength(glm::vec3(0.1f));
    glm::vec3 gDiffuseStrength(glm::vec3(1.0f));
    float gSpecularIntensity = 0.6f;

}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void setupCupBuffers(GLMesh& mesh);
void setupPlaneBuffers(GLMesh& mesh);
void setupHandleBuffers(GLMesh& mesh);
void setupTeaBuffers(GLMesh& mesh);
void setupSphereBuffers(GLMesh& mesh);
void setupPlateBuffers(GLMesh& mesh);
void createPlaneMesh();
void createCubeMesh();
void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateTexture(const char* filename, GLuint& textureId, int textureUnit);
void UDestroyTexture(GLuint textureId);
void flipImageVertically(unsigned char* image, int width, int height, int channels);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

// for debugging
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
    GLsizei length, const char* message, const void* userParam);


/* Cube Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);



/* Cube Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 lightPos1;
uniform vec3 lightColor1;
uniform float light_1_strength;
uniform vec3 lightPos2;
uniform vec3 lightColor2;
uniform float light_2_strength;
uniform vec3 ambientStrength;
uniform float specularIntensity;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform sampler2D uTextureExtra;
uniform bool multipleTextures;
uniform vec2 uvScale;
//uniform vec3 objectColor;

void main()
{
    // Texture holds the color to be used for all three components of Phong lighting model
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);
    // if there is a second image
    if (multipleTextures) {
        // find the color of the second texture based on this fragment's tex coord 
        vec4 extraTexture = texture(uTextureExtra, vertexTextureCoordinate);
        // if this location is not fully transparent, use its color
        if (extraTexture.a != 0.0) {
            textureColor = extraTexture;
        }
    }

    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    // FIRST LIGHT:
    //-------------
    //Calculate Ambient lighting*/
    vec3 ambient = ambientStrength * lightColor1; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos1 - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor1; // Generate diffuse light color

    //Calculate Specular lighting*/
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor1;

    // SECOND LIGHT:
    //--------------
    // ambient lighting - add first and second light ambient numbers
    ambient += light_2_strength * (ambientStrength * lightColor2);

    // diffuse lighting
    lightDirection = normalize(lightPos2 - vertexFragmentPos);
    impact = max(dot(norm, lightDirection), 0.0);
    // add first and second light diffuses
    diffuse += light_2_strength * (impact * lightColor2);

    // specular lighting
    reflectDir = reflect(-lightDirection, norm);
    specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    // add first and second light speculars
    specular += light_2_strength * (specularIntensity * specularComponent * lightColor2);

    // CALCULATE PHONG RESULT
    //-----------------------
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);




/* ------------------- MAIN -------------------*/
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    // Load table texture
    const char* texFilename1 = "resources/textures/wood.jpg";
    if (!UCreateTexture(texFilename1, gTextureTable, 0))
    {
        cout << "Failed to load texture " << texFilename1 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename1 << endl;
    }

    // Load mug texture
    const char* texFilename2 = "resources/textures/marble.jpg";
    if (!UCreateTexture(texFilename2, gTextureCup, 0))
    {
        cout << "Failed to load texture " << texFilename2 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename2 << endl;
    }

    // Load tea texture
    const char* texFilename3 = "resources/textures/tea.png";
    if (!UCreateTexture(texFilename3, gTextureTea, 0))
    {
        cout << "Failed to load texture " << texFilename3 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename3 << endl;
    }

    // Load lemon texture
    const char* texFilename4 = "resources/textures/lemon.png";
    // use unit GL_TEXTURE1 instead of GL_TEXTURE0, so I can use two textures on one object
    if (!UCreateTexture(texFilename4, gTextureLemon, 1))
    {
        cout << "Failed to load texture " << texFilename3 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename4 << endl;
    }

    // Load orange texture
    const char* texFilename5 = "resources/textures/orange.jpg";
    if (!UCreateTexture(texFilename5, gTextureOrange, 0))
    {
        cout << "Failed to load texture " << texFilename5 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename5 << endl;
    }

    // Load cloth texture
    const char* texFilename6 = "resources/textures/knit.jpg";
    if (!UCreateTexture(texFilename6, gTextureCloth, 0))
    {
        cout << "Failed to load texture " << texFilename6 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename6 << endl;
    }

    // Load plate texture
    const char* texFilename7 = "resources/textures/plate.png";
    if (!UCreateTexture(texFilename7, gTexturePlate, 0))
    {
        cout << "Failed to load texture " << texFilename7 << endl;
        return EXIT_FAILURE;
    }
    else {
        cout << "Successfully loaded texture " << texFilename7 << endl;
    }



    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);
    // We set the texture as texture unit 1
    glUniform1i(glGetUniformLocation(gProgramId, "uTextureExtra"), 1);


    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // one iteration of this loop is one frame. 60FPS means this loop repeats 60 times per second
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    // Release textures
    UDestroyTexture(gTextureTable);
    UDestroyTexture(gTextureCup);
    UDestroyTexture(gTextureTea);
    UDestroyTexture(gTextureLemon);
    UDestroyTexture(gTextureOrange);
    UDestroyTexture(gTextureCloth);
    UDestroyTexture(gTexturePlate);

    // Release shader programs
    UDestroyShaderProgram(gProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


/* ------------------- Initialize GLFW, GLEW, window, and everything else -------------------*/
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // for debugging
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);
    glfwSetKeyCallback(*window, key_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    // create the plane mesh
    createPlaneMesh();
    // create cube mesh
    createCubeMesh();

    // generate all the vertex arrays
    glGenVertexArrays(6, gMesh.vao);

    // Create all the buffers: 2 for each mesh: first one for the vertex data, second one for the indices
    glGenBuffers(10, gMesh.vbos);

    // set up all the GPU buffer objects
    setupCupBuffers(gMesh);
    setupPlaneBuffers(gMesh);
    setupHandleBuffers(gMesh);
    setupTeaBuffers(gMesh);
    setupSphereBuffers(gMesh);
    setupPlateBuffers(gMesh);


    return true;
}


/* ------------------- Process key input for current frame -------------------*/
// called every render loop, making it a very fast input reader
void UProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime, addedSpeed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime, addedSpeed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime, addedSpeed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime, addedSpeed);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime, addedSpeed);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime, addedSpeed);

}


/* ------------------- GLFW keyboard callback for outside render loop -------------------*/
// used for slower, individual key presses (because it is not inside the render loop)
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        select_ortho = !select_ortho;

    }
}


/* ------------------- glfw callback for changing window size -------------------*/
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


/* ------------------- glfw callback for mouse movement -------------------*/
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


/* ------------------- glfw callback for mouse scroll wheel -------------------*/
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset == 1) {
        addedSpeed += 0.1f;
        if (addedSpeed > 1.0f) {
            addedSpeed = 1.0f;
        }
    }
    else if (yoffset == -1) {
        addedSpeed -= 0.2f;
        if (addedSpeed < 0) {
            addedSpeed = 0.03f;
        }
    }
}


/* ------------------- glfw callback for mouse buttons -------------------*/
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}

/* ------------------- Function for rendering frame -------------------*/
void URender()
{

    // for debugging
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();


    // create projection with either perspective or Orthographic matrix
    if (select_ortho) {
        // creates an orthographic view matrix
        projection = glm::ortho(-(float)WINDOW_WIDTH * 0.01f, (float)WINDOW_WIDTH * 0.01f, -(float)WINDOW_HEIGHT * 0.01f, (float)WINDOW_HEIGHT * 0.01f, 0.001f, 1000.0f);
    }
    else {
        // Creates a perspective projection
        projection = glm::perspective(45.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }

    // Set the shader to be used
    glUseProgram(gProgramId);

    //---------------------- CUP CYLINDER ----------------------

    // 1. Scales the object by 2
    glm::mat4 scale = glm::mat4(1.0f);
    // 2. Rotates shape 
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, -1.5708f, glm::vec3(1.0, 0.0f, 0.0f));
    // 3. Place object at the origin
    glm::mat4 translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(0.0f, 1.00f, 0.0f));
    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = translation * rotation * scale;

    // specify color of cylinder
    GLfloat myColor[] = { 0.7f, 0.7f, 0.7f, 1.0f };

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // set texture uniform variable in shader program
    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Reference matrix uniforms from the shader program for the color, light color, light position, and camera position
    GLint lightColor1Loc = glGetUniformLocation(gProgramId, "lightColor1");
    GLint lightColor2Loc = glGetUniformLocation(gProgramId, "lightColor2");
    GLint lightPosition1Loc = glGetUniformLocation(gProgramId, "lightPos1");
    GLint lightPosition2Loc = glGetUniformLocation(gProgramId, "lightPos2");
    GLint lightStrength1Loc = glGetUniformLocation(gProgramId, "light_1_strength");
    GLint lightStrength2Loc = glGetUniformLocation(gProgramId, "light_2_strength");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");
    GLint ambientStrengthLoc = glGetUniformLocation(gProgramId, "ambientStrength");
    GLint diffuseStrengthLoc = glGetUniformLocation(gProgramId, "diffuseStrength");
    GLint specularIntensityLoc = glGetUniformLocation(gProgramId, "specularIntensity");

    // Pass color, light, and camera data to the shader program's corresponding uniforms
    glUniform3f(lightColor1Loc, gLightColor1.r, gLightColor1.g, gLightColor1.b);
    glUniform3f(lightColor2Loc, gLightColor2.r, gLightColor2.g, gLightColor2.b);
    glUniform3f(lightPosition1Loc, gLightPosition1.x, gLightPosition1.y, gLightPosition1.z);
    glUniform3f(lightPosition2Loc, gLightPosition2.x, gLightPosition2.y, gLightPosition2.z);
    glUniform1f(lightStrength1Loc, light_1_strength);
    glUniform1f(lightStrength2Loc, light_2_strength);
    glUniform3f(ambientStrengthLoc, gAmbientStrength.r, gAmbientStrength.g, gAmbientStrength.b);
    glUniform3f(diffuseStrengthLoc, gDiffuseStrength.r, gAmbientStrength.g, gAmbientStrength.b);
    glUniform1f(specularIntensityLoc, gSpecularIntensity);;
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    // tell fragment shader there is not multiple textures
    GLuint multipleTexturesLoc = glGetUniformLocation(gProgramId, "multipleTextures");
    glUniform1i(multipleTexturesLoc, false);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[0]);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureCup);

    // Draws the cup cylinder
    glDrawElements(GL_TRIANGLES, cylinder1.getIndexCount(), GL_UNSIGNED_INT, NULL);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);


    //---------------------- CUP HANDLE - CUBE 1 ----------------------

    // set up cup handle buffers
    setupHandleBuffers(gMesh);

    // Change model view before drawing handle
    scale = glm::mat4(1.0f);
    scale = glm::scale(scale, glm::vec3(0.3f, 0.1f, 1.1f));
    rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, 0.0f, glm::vec3(1.0, 0.0f, 0.0f));
    translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(0.0f, 1.5f, 1.55f));
    model = translation * rotation * scale;

    // specify color of cube
    myColor[0] = 0.7f;
    myColor[1] = 0.7f;
    myColor[2] = 0.7f;

    // Set new model matrix and color in shader's uniform variables
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    // tell fragment shader there is not multiple textures
    glUniform1i(multipleTexturesLoc, false);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[2]);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureCup);

    // Draw cup handle piece #1
    glDrawArrays(GL_TRIANGLES, 0, cube1.verts.size() / 8);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);


    //---------------------- CUP HANDLE - CUBE 2 ----------------------

    scale = glm::mat4(1.0f);
    scale = glm::scale(scale, glm::vec3(0.3f, 0.1f, 1.7f));
    rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, -0.785398f, glm::vec3(1.0, 0.0f, 0.0f));
    translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(0.0f, 0.83f, 1.5f));
    model = translation * rotation * scale;

    // Set new model matrix and color in shader's uniform variables
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    // tell fragment shader there is not multiple textures
    glUniform1i(multipleTexturesLoc, false);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[2]);

    // Draw cup handle piece #2
    glDrawArrays(GL_TRIANGLES, 0, cube1.verts.size() / 8);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);



    //---------------------- TABLE ----------------------

    // Change model view before drawing plane
    scale = glm::mat4(1.0f);
    scale = glm::scale(scale, glm::vec3(16.0f, 1.0f, 16.0f));
    rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, 1.5708f, glm::vec3(0.0f, 1.0f, 0.0f));
    translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(-1.0f, -0.57f, -2.0f));
    model = translation * rotation * scale;

    // Set new model matrix and specular intensity in shader's uniform variables
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    // change lighting components for this object
    glUniform3f(ambientStrengthLoc, 0.0001f, 0.0001f, 0.0001f);
    glUniform1f(specularIntensityLoc, 1.0f);

    // tell fragment shader there is not multiple textures
    glUniform1i(multipleTexturesLoc, false);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[1]);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureTable);

    // Draw the plane
    glDrawArrays(GL_TRIANGLES, 0, plane1.verts.size() / 8);

    // set lighting components back to normal
    glUniform3f(ambientStrengthLoc, gAmbientStrength.r, gAmbientStrength.g, gAmbientStrength.b);
    glUniform3f(diffuseStrengthLoc, gDiffuseStrength.r, gAmbientStrength.g, gAmbientStrength.b);
    glUniform1f(specularIntensityLoc, gSpecularIntensity);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);



    //---------------------- CLOTH ----------------------

    // Change model view before drawing plane
    scale = glm::mat4(1.0f);
    scale = glm::scale(scale, glm::vec3(3.0f, 1.0f, 3.0f));
    rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, -0.8f, glm::vec3(0.0f, 1.0f, 0.0f));
    translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(0.0f, -0.56f, 0.0f));
    model = translation * rotation * scale;

    // Set new model matrix and specular intensity in shader's uniform variables
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    // change lighting components for this object
    glUniform3f(ambientStrengthLoc, 0.00001f, 0.00001f, 0.00001f);
    glUniform1f(specularIntensityLoc, 0.0f);

    // tell fragment shader there is not multiple textures
    glUniform1i(multipleTexturesLoc, false);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[1]);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureCloth);

    // Draw the plane
    glDrawArrays(GL_TRIANGLES, 0, plane1.verts.size() / 8);

    // set lighting components back to normal
    glUniform3f(ambientStrengthLoc, gAmbientStrength.r, gAmbientStrength.g, gAmbientStrength.b);
    glUniform1f(specularIntensityLoc, gSpecularIntensity);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);



    //---------------------- TEA ----------------------

    scale = glm::mat4(1.0f);
    rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, -1.5708f, glm::vec3(1.0, 0.0f, 0.0f));
    translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(0.0f, 1.951f, 0.0f));
    model = translation * rotation * scale;

    // Set new model matrix and color in shader's uniform variables
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

    // tell fragment shader there is multiple textures
    glUniform1i(multipleTexturesLoc, true);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[3]);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureTea);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gTextureLemon);

    // Draw the tea cylinder
    glDrawElements(GL_TRIANGLES, cylinder2.getIndexCount(), GL_UNSIGNED_INT, NULL);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);



    //---------------------- PLATE ----------------------

    scale = glm::mat4(1.0f);
    rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, -1.5708f, glm::vec3(0.0, 1.0f, 0.0f));
    rotation = glm::rotate(rotation, -1.5708f, glm::vec3(1.0, 0.0f, 0.0f));
    translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(-3.9f, 0.08f, -1.6f));
    model = translation * rotation * scale;

    // Set new model matrix and color in shader's uniform variables
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    // change lighting components for this object
    glUniform3f(ambientStrengthLoc, 0.08f, 0.08f, 0.08f);
    glUniform1f(specularIntensityLoc, 0.5f);

    // tell fragment shader there is multiple textures
    glUniform1i(multipleTexturesLoc, false);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[5]);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTexturePlate);

    // Draw the tea cylinder
    glDrawElements(GL_TRIANGLES, cylinder3.getIndexCount(), GL_UNSIGNED_INT, NULL);

    // set lighting components back to normal
    glUniform3f(ambientStrengthLoc, gAmbientStrength.r, gAmbientStrength.g, gAmbientStrength.b);
    glUniform1f(specularIntensityLoc, gSpecularIntensity);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);




    //---------------------- ORANGE ----------------------

    scale = glm::mat4(1.0f);
    rotation = glm::mat4(1.0f);
    rotation = glm::rotate(rotation, 1.0f, glm::vec3(0.0, 1.0f, 0.0f));
    translation = glm::mat4(1.0f);
    translation = glm::translate(translation, glm::vec3(-3.5f, 0.98f, -1.3f));
    model = translation * rotation * scale;

    // Set new model matrix and color in shader's uniform variables
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    // change lighting components for this object
    glUniform3f(ambientStrengthLoc, 0.08f, 0.08f, 0.08f);
    glUniform1f(specularIntensityLoc, 0.3f);

    // tell fragment shader there is multiple textures
    glUniform1i(multipleTexturesLoc, false);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao[4]);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureOrange);

    // Draw the tea cylinder
    glDrawElements(GL_TRIANGLES, sphere1.getIndexCount(), GL_UNSIGNED_INT, NULL);

    // set lighting components back to normal
    glUniform3f(ambientStrengthLoc, gAmbientStrength.r, gAmbientStrength.g, gAmbientStrength.b);
    glUniform1f(specularIntensityLoc, gSpecularIntensity);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);


    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}



/* ------------------- Set up GPU buffers for cup cylinder -------------------*/
void setupCupBuffers(GLMesh& mesh)
{
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormals = 3;
    const GLuint floatsPerUV = 2;
    const GLuint floatsInEachStride = 8;

    glBindVertexArray(mesh.vao[0]); // activate vertex array object

    // Activates the first buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]);
    // Sends vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, cylinder1.getInterleavedVertexSize(), cylinder1.getInterleavedVertices(), GL_STATIC_DRAW);

    // activate second buffer for index array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    // stores indices[] array on GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinder1.getIndexSize(), cylinder1.getIndices(), GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = cylinder1.getInterleavedStride();

    // Create Vertex Attribute Pointers
    // position attribute -- instructs GPU how to handle vertex position data
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0); // enables the vertex attribute array numbered 0
    // normals attribute -- instructs GPU how to handle normals data
    glVertexAttribPointer(1, floatsPerNormals, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1); // enables the vertex attribute array numbered 0
    // texture coordinate attribute -- instructs GPU how to handle texture coordinates
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2); // enables the vertex attribute array numbered 0
}



/* ------------------- Set up GPU buffers for plane -------------------*/
void setupPlaneBuffers(GLMesh& mesh) {
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormals = 3;
    const GLuint floatsPerUV = 2;
    const GLuint floatsInEachStride = 8;

    glBindVertexArray(mesh.vao[1]); // activate vertex array object

    // Activates the first buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[2]);
    // Sends vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, plane1.verts.size() * sizeof(float), plane1.verts.data(), GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsInEachStride);// The number of floats before each

    // Create Vertex Attribute Pointers
    // instructs GPU how to handle vertex buffer object data
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0); // enables the vertex attribute array numbered 0
    // normals attribute -- instructs GPU how to handle normals data
    glVertexAttribPointer(1, floatsPerNormals, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1); // enables the vertex attribute array numbered 0
    // texture coordinate attribute -- instructs GPU how to handle texture coordinates
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormals)));
    glEnableVertexAttribArray(2); // enables the vertex attribute array numbered 0
}


/* ------------------- Set up GPU buffers for cup handle cubes -------------------*/
void setupHandleBuffers(GLMesh& mesh) {

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormals = 3;
    const GLuint floatsPerUV = 2;
    const GLuint floatsInEachStride = 8;

    glBindVertexArray(mesh.vao[2]); // activate vertex array object

    // Activates the first buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[3]);
    // Sends vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, cube1.verts.size() * sizeof(float), cube1.verts.data(), GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsInEachStride);// The number of floats before each

    // Create Vertex Attribute Pointers
    // instructs GPU how to handle vertex buffer object data
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0); // enables the vertex attribute array numbered 0
    // normals attribute -- instructs GPU how to handle normals data
    glVertexAttribPointer(1, floatsPerNormals, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1); // enables the vertex attribute array numbered 0
    // texture coordinate attribute -- instructs GPU how to handle texture coordinates
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormals)));
    glEnableVertexAttribArray(2); // enables the vertex attribute array numbered 2
}


/* ------------------- Set up GPU buffers for tea cylinder -------------------*/
void setupTeaBuffers(GLMesh& mesh)
{
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormals = 3;
    const GLuint floatsPerUV = 2;
    const GLuint floatsInEachStride = 8;

    glBindVertexArray(mesh.vao[3]); // activate vertex array object

    // Activates the first buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[4]);
    // Sends vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, cylinder2.getInterleavedVertexSize(), cylinder2.getInterleavedVertices(), GL_STATIC_DRAW);

    // activate second buffer for index array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[5]);
    // stores indices[] array on GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinder2.getIndexSize(), cylinder2.getIndices(), GL_STATIC_DRAW);

    // The number of floats that make up a block of vertex data. Should be 32 bytes
    GLint stride = cylinder2.getInterleavedStride();

    // Create Vertex Attribute Pointers
    // position attribute -- instructs GPU how to handle vertex position data
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0); // enables the vertex attribute array numbered 0
    // normals attribute -- instructs GPU how to handle normals data
    glVertexAttribPointer(1, floatsPerNormals, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1); // enables the vertex attribute array numbered 0
    // texture coordinate attribute -- instructs GPU how to handle texture coordinates
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2); // enables the vertex attribute array numbered 0

}


/* ------------------- Set up GPU buffers for orange sphere -------------------*/
void setupSphereBuffers(GLMesh& mesh)
{
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormals = 3;
    const GLuint floatsPerUV = 2;
    const GLuint floatsInEachStride = 8;

    glBindVertexArray(mesh.vao[4]); // activate vertex array object

    // Activates the first buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[6]);
    // Sends vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, sphere1.getInterleavedVertexSize(), sphere1.getInterleavedVertices(), GL_STATIC_DRAW);

    // activate second buffer for index array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[7]);
    // stores indices[] array on GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere1.getIndexSize(), sphere1.getIndices(), GL_STATIC_DRAW);

    // Strides between vertex coordinates
    GLint stride = sphere1.getInterleavedStride();

    // Create Vertex Attribute Pointers
    // position attribute -- instructs GPU how to handle vertex position data
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0); // enables the vertex attribute array numbered 0
    // normals attribute -- instructs GPU how to handle normals data
    glVertexAttribPointer(1, floatsPerNormals, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1); // enables the vertex attribute array numbered 0
    // texture coordinate attribute -- instructs GPU how to handle texture coordinates
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2); // enables the vertex attribute array numbered 0
}


/* ------------------- Set up GPU buffers for plate cylinder -------------------*/
void setupPlateBuffers(GLMesh& mesh)
{
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormals = 3;
    const GLuint floatsPerUV = 2;
    const GLuint floatsInEachStride = 8;

    glBindVertexArray(mesh.vao[5]); // activate vertex array object

    // Activates the first buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[8]);
    // Sends vertex data to the GPU
    glBufferData(GL_ARRAY_BUFFER, cylinder3.getInterleavedVertexSize(), cylinder3.getInterleavedVertices(), GL_STATIC_DRAW);

    // activate second buffer for index array
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[9]);
    // stores indices[] array on GPU
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinder3.getIndexSize(), cylinder3.getIndices(), GL_STATIC_DRAW);

    // The number of floats that make up a block of vertex data. Should be 32 bytes
    GLint stride = cylinder3.getInterleavedStride();

    // Create Vertex Attribute Pointers
    // position attribute -- instructs GPU how to handle vertex position data
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0); // enables the vertex attribute array numbered 0
    // normals attribute -- instructs GPU how to handle normals data
    glVertexAttribPointer(1, floatsPerNormals, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1); // enables the vertex attribute array numbered 0
    // texture coordinate attribute -- instructs GPU how to handle texture coordinates
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2); // enables the vertex attribute array numbered 0

}



/* ------------------- Create plane mesh -------------------*/
void createPlaneMesh() {
    // Position data  for plane
    vector<float> verts = {

        // position           // normal           // tex coordinate
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    // populate plane1 struct with this mesh data
    verts.swap(plane1.verts);
}



/* ------------------- Create cube mesh -------------------*/
void createCubeMesh() {

    vector<float> verts = {

        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    // populate cube1 struct with this mesh data
    verts.swap(cube1.verts);
}



/* ------------------- Destroy VAOs and VBOs -------------------*/
void UDestroyMesh(GLMesh& mesh)
{
    // delete the VAOs
    glDeleteVertexArrays(6, mesh.vao);
    // delete the VBOs
    glDeleteBuffers(8, mesh.vbos);
}



/* ------------------- Generate and load the texture -------------------*/
bool UCreateTexture(const char* filename, GLuint& textureId, int textureUnit)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        switch (textureUnit)
        {
        case 0:
        {
            glActiveTexture(GL_TEXTURE0);
        }
        break;

        case 1:
        {
            glActiveTexture(GL_TEXTURE1);
        }
        break;

        default:
            glActiveTexture(GL_TEXTURE0);
            break;
        }

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


/* ------------------- Deletes a texture -------------------*/
void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


/* ------------------- Flip image vertically -------------------*/
// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


/* ------------------- Create the shader program from the vertex and fragment shader sources -------------------*/
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512]; // create character string of length 512 for the error log

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}



/* ------------------- Destroy shader program -------------------*/
void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId); // delete the shader program
}



// FOR DEBUGGING
void APIENTRY glDebugOutput(GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}