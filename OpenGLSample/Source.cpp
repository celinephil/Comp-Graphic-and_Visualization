#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#include "camera.h"         // Camera class
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define SCREEN_WIDTH 640    // Defining the screen width
#define SCREEN_HEIGHT 480   // Defining the screen height

using namespace std;        // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Celine Philachanh: Final Project"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 850;
    const int WINDOW_HEIGHT = 900;

    // glm functions for the various matrices
    glm::mat4 rotation;     // Rotation matrice
    glm::mat4 translation;  // Translation matice
    glm::mat4 model;        // Model matrice
    glm::mat4 view;         // View matrice
    glm::mat4 projection;   // Projection matrice
    glm::mat4 perspective;  // Perspective matrice
    glm::mat4 ortho;        // Orthographic view matrice

    // Handles the orthographic change between perspective and ortho
    bool isPerspective = false;

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;

    // Shader programs
    GLuint gProgramId;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;          // Handle for the vertex array object
        GLuint vbos[2];      // Handle for the vertex buffer object
        GLuint nIndices;     // Number of indices of the mesh
    };

    // stores coordinates for points
    struct GLCoord
    {
        GLfloat x;
        GLfloat y;
        GLfloat z;
    };

    // Triangle mesh data
    GLMesh upperCandlestickMesh;
    GLMesh lowerCandlestickMesh; 
    GLMesh candleMesh;
    GLMesh candleWickMesh;
    GLMesh tableMesh;
    GLMesh napkinMesh;
    GLMesh knifeMesh;
    GLMesh knifeTipMesh;

    // VBO and VAO objects
    unsigned int VBOplane, VBOcandle, VBOupperCandlestickMesh, VBOlowerCandlestickMesh, VBOcandleWick, VBOnapkin, VBOknife, VBOknifeTip;
    unsigned int VAOplane, VAOcandle, VAOupperCandlestickMesh, VAOlowerCandlestickMesh, VAOcandleWick, VAOnapkin, VAOknife, VAOknifeTip;

    //Camera variables 
    glm::vec3 gCameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 gCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 gCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 gCameraRight = glm::normalize(glm::cross(gCameraFront, gCameraUp));

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 7.0f));  // camera angle on the x, y, and z axis
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f;    // time between current frame
    float gLastFrame = 0.0f;    // time between last frame

    GLfloat halfScreenWidth = SCREEN_WIDTH / 2;
    GLfloat halfScreenHeight = SCREEN_HEIGHT / 2;

    //Texture variables
    unsigned int upperCandlestickTexture, candleTexture, napkinTexture, knifeTexture, knifeTipTexture, tableTexture, lowerCandlestickTexture, candleWickTexture;

    // Keylight position, scale, and color
    glm::vec3 gLightPosition(-3.5f, 2.0f, 3.0f);    // This is the light's position based on the x, y, z axis
    glm::vec3 gLightScale(0.3f);                    // Scaling the light size
    glm::vec3 gLightColor(1.0f, 0.2f, 0.75f);       // The key light color is set to pink

    // fill light position, scale, and color
    glm::vec3 gLight2Position(4.0f, 0.0f, 1.0f);    // This is the fill light's position based on the x, y, z axis
    glm::vec3 gLight2Scale(0.3f);                   // Scaling the fill light size 
    glm::vec3 gLight2Color(1.0f, 0.0f, 0.0f);       // The key light color is set to red

}

// User-defined Function prototypes
bool UInitialize(int, char* [], GLFWwindow** window);
void generateTextures();
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UProcessInput(GLFWwindow* window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void URender();
void UCreatePlaneMesh(GLMesh& mesh, GLCoord topRight, GLCoord topLeft, GLCoord bottomLeft, GLCoord bottomRight);
void UCreateCandleMesh(GLMesh& mesh);
void UCreateNapkinMesh(GLMesh& mesh);
void UCreateKnifeMesh(GLMesh& mesh);
void UCreateKnifeTipMesh(GLMesh& mesh);
void UCreateCandleWickMesh(GLMesh& mesh);
void UCreateUpperCandlestickMesh(GLMesh& mesh);
void UCreateLowerCandlestickMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
    layout(location = 1) in vec3 normal;   // VAP position 1 for normals
    layout(location = 2) in vec2 texture; // VAP position 1 for texture coordinates

    out vec3 vertexNormal;      // For outgoing normals to fragment shader
    out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
    out vec2 TextureCoord;  // For outgoing texture coordinates to fragment shader

    //Uniform 
    uniform mat4 model;       // Global variable for the model transform matrices
    uniform mat4 view;        // Global variable for the view transform matrices
    uniform mat4 projection;  // Global variable for the projection transform matrices

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
        vertexFragmentPos = vec3(model * vec4(position, 1.0f));         // Gets fragment / pixel position in world space only (exclude view and projection)
        vertexNormal = mat3(transpose(inverse(model))) * normal;        // get normal vectors in world space only and exclude normal translation properties
        TextureCoord = texture; //references incoming texture data
    }
);

/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec3 vertexNormal;              // For incoming normals
    in vec3 vertexFragmentPos;         // For incoming fragment position
    in vec2 TextureCoord; //Variable to hold incoming texture data from vertex shader
    
    out vec4 fragmentColor;            // For outgoing pyramid color to the GPU

    // Uniform 
    uniform vec3 lightColor;           // Global variable for key light color
    uniform vec3 light2Color;          // Global variable for fill light color
    uniform vec3 lightPos;             // Global variable for key light position
    uniform vec3 light2Pos;            // Global variable for fill position
    uniform vec3 viewPosition;         // Global variable for view position
    uniform sampler2D ourTexture;      // Useful when working with multiple textures
    uniform vec2 uvScale;           

    void main()
    {
        // Phong lighting model calculations to generate ambient, diffuse, and specular components

        //Calculate Ambient lighting
        float ambientStrength = 1.0f;                   // Set ambient or global lighting strength for key light
        float ambient2Strength = 0.1f;                  // Set ambient or global lighting strength for fill light

        vec3 ambient = ambientStrength * lightColor;    // Generate ambient light color for key light
        vec3 ambient2 = ambient2Strength * light2Color; // Generate ambient light color for fill light

        //Calculate Diffuse lighting
        vec3 norm = normalize(vertexNormal);                             // Normalize vectors to 1 unit
        vec3 lightDirection = normalize(lightPos - vertexFragmentPos);   // Calculate distance (light direction) between light source and fragments/pixels on the pyramid
        vec3 light2Direction = normalize(light2Pos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on the pyramid

        float impact = max(dot(norm, lightDirection), 0.0);              // Calculate diffuse impact by generating dot product of normal and light
        float impact2 = max(dot(norm, light2Direction), 0.0);            // Calculate diffuse impact by generating dot product of normal and light

        vec3 diffuse = impact * lightColor;          // Generate diffuse light color for key light
        vec3 diffuse2 = impact2 * light2Color;       // Generate diffuse light color for fill light

        //Calculate Specular lighting
        float specularIntensity = 0.8f;         // Set specular light strength
        float highlightSize = 16.0f;            // Set specular highlight size
        vec3 viewDir = normalize(viewPosition - vertexFragmentPos);  // Calculate view direction
        vec3 reflectDir = reflect(-lightDirection, norm);            // Calculate reflection vector
        //Calculate specular component
        float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
        vec3 specular = specularIntensity * specularComponent * lightColor;   // Generate specular intensity, component, and color for key light
        vec3 specular2 = specularIntensity * specularComponent * light2Color; // Generate specular intensity, component, and color for fill light

        // Texture holds the color to be used for all three components
        vec4 textureColor = texture(ourTexture, TextureCoord * uvScale);

        // Calculate phong result
        vec3 phong = (ambient + ambient2 + diffuse + specular) * textureColor.xyz;

        fragmentColor = texture(ourTexture, TextureCoord); // Send lighting results to GPU
    }
);

// Lamp Shader Source Code
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

    // Uniform 
    uniform mat4 model;          // Global variable for the model transform matrices
    uniform mat4 view;           // Global variable for the view transform matrices
    uniform mat4 projection;     // Global variable for the projection transform matrices

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    }
);

// Lamp Fragment Shader Source Code
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor;         // For outgoing lamp color (smaller pyramid) to the GPU

    void main()
    {
        fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
    }
);

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

// Main
int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    //Coordinates for planes
    struct GLCoord topLeft = { -5.0f, -0.3f, -5.0f };
    struct GLCoord topRight = { 5.0f, -0.3f, -5.0f };
    struct GLCoord bottomLeft = { -5.0f, -0.3f, 5.0f };
    struct GLCoord bottomRight = { 5.0f, -0.3f, 5.0f };
    UCreatePlaneMesh(tableMesh, topRight, topLeft, bottomLeft, bottomRight);
    UCreateUpperCandlestickMesh(upperCandlestickMesh);
    UCreateLowerCandlestickMesh(lowerCandlestickMesh);
    UCreateCandleMesh(candleMesh);
    UCreateCandleWickMesh(candleWickMesh);
    UCreateNapkinMesh(napkinMesh);
    UCreateKnifeMesh(knifeMesh);
    UCreateKnifeTipMesh(knifeTipMesh);

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    //generate the textures
    generateTextures();

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        //set perspective and ortho projections and enables nuanced camera controls such as in the mouse scroll and mouse cursor
        perspective = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
        ortho = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 0.1f, 100.0f);

        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        rotation = glm::rotate(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        rotation = glm::rotate(glm::radians(0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
        translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

        // Model matrix: transformations are applied right-to-left order
        model = translation * rotation;

        view = gCamera.GetViewMatrix();

        // Retrieves and passes transform matrices to the Shader program
        GLint modelLoc = glGetUniformLocation(gProgramId, "model");
        GLint viewLoc = glGetUniformLocation(gProgramId, "view");
        GLint projLoc = glGetUniformLocation(gProgramId, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // input
        // -----
        UProcessInput(gWindow);

        if (isPerspective)
            projection = perspective;
        else
            projection = ortho;

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release shader program
    UDestroyShaderProgram(gProgramId);

    // Release mesh program
    UDestroyMesh(candleMesh);
    UDestroyMesh(candleWickMesh);
    UDestroyMesh(tableMesh);
    UDestroyMesh(upperCandlestickMesh);
    UDestroyMesh(lowerCandlestickMesh);
    UDestroyMesh(napkinMesh);
    UDestroyMesh(knifeMesh);
    UDestroyMesh(knifeTipMesh);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;
    float cameraOffset = cameraSpeed * gDeltaTime;

    // ESC key: Used to exit the application
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    // WSAD keys: Used to control forward, backward, right, and left movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    // QE keys: Used to control the upward and downward movement
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    // P key: Used to change the view of the scene between 2D and 3D views at will
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        isPerspective = true;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        isPerspective = false;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
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

// Functioned called to render a frame
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(1.0f, 0.0784314f, 0.576471f, 1.0f); // Color set to deep pink
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set the shader to be used
    glUseProgram(gProgramId);

    // Table
    glBindTexture(GL_TEXTURE_2D, tableTexture);
    glBindVertexArray(tableMesh.vao);
    glDrawElements(GL_TRIANGLES, tableMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle
    glBindVertexArray(0);   // Deactivate Vertex Array Object

    // Candle
    glBindTexture(GL_TEXTURE_2D, candleTexture);
    glBindVertexArray(candleMesh.vao);
    glDrawElements(GL_TRIANGLES, candleMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle
    glBindVertexArray(0);    // Deactivate Vertex Array Object

    // Candle Wick
    glBindTexture(GL_TEXTURE_2D, candleWickTexture);
    glBindVertexArray(candleWickMesh.vao);
    glDrawElements(GL_TRIANGLES, candleWickMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle
    glBindVertexArray(0);    // Deactivate Vertex Array Object

    // Upper Candlestick
    glBindTexture(GL_TEXTURE_2D, upperCandlestickTexture);
    glBindVertexArray(upperCandlestickMesh.vao);
    glDrawElements(GL_TRIANGLES, upperCandlestickMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle
    glBindVertexArray(0);    // Deactivate Vertex Array Object

    // Lower Candlestick
    glBindTexture(GL_TEXTURE_2D, lowerCandlestickTexture);
    glBindVertexArray(lowerCandlestickMesh.vao);
    glDrawElements(GL_TRIANGLES, lowerCandlestickMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle
    glBindVertexArray(0);    // Deactivate Vertex Array Object

    // Napkin
    glBindTexture(GL_TEXTURE_2D, napkinTexture);
    glBindVertexArray(napkinMesh.vao);
    glDrawElements(GL_TRIANGLES, napkinMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle
    glBindVertexArray(0);    // Deactivate Vertex Array Object

    // Butter knife handle
    glBindTexture(GL_TEXTURE_2D, knifeTexture);
    glBindVertexArray(knifeMesh.vao);
    glDrawElements(GL_TRIANGLES, knifeMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle
    glBindVertexArray(0);    // Deactivate Vertex Array Object

    // Butter knife tip
    glBindTexture(GL_TEXTURE_2D, knifeTipTexture);
    glBindVertexArray(knifeTipMesh.vao);
    glDrawElements(GL_TRIANGLES, knifeTipMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle
    glBindVertexArray(0);    // Deactivate Vertex Array Object

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}

// Implements the UCreatePlaneMesh function to create the plane to represent the table
void UCreatePlaneMesh(GLMesh& mesh, GLCoord topRight, GLCoord topLeft, GLCoord bottomLeft, GLCoord bottomRight) {

    GLfloat verts[] = {
        //x, y, z                                     //rgb               //texture
        topLeft.x, topLeft.y, topLeft.z,              1.0f, 0.0f, 0.0f,      0.0f, 1.0f,  //top left
        topRight.x, topRight.y, topRight.z,           1.0f, 0.0f, 0.0f,      1.0f, 1.0f,  //top right
        bottomRight.x, bottomRight.y,  bottomRight.z, 1.0f, 0.0f, 0.0f,      1.0f, 0.0f,  //bottom right
        bottomLeft.x, bottomLeft.y,  bottomLeft.z,    1.0f, 0.0f, 0.0f,      0.0f, 0.0f   //bottom left
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 3,  //first triangle
        1, 2, 3   //second triangle

    };
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, s, t). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);
}

// Implements the UCreateNapkinMesh function to create the napkin
void UCreateNapkinMesh(GLMesh& mesh) {

    GLfloat verts[] = {
        //x, y, z                    //rgb                  //texture
        2.0f, -0.25f, 1.0f,           1.0f, 0.0f, 0.0f,      0.0f, 1.0f,  //top left
        3.0f, -0.25f, 1.0f,           1.0f, 0.0f, 0.0f,      1.0f, 1.0f,  //top right
        3.0f, -0.25f, 3.0f,           1.0f, 0.0f, 0.0f,      1.0f, 0.0f,  //bottom right
        2.0f, -0.25f, 3.0f,           1.0f, 0.0f, 0.0f,      0.0f, 0.0f   //bottom left
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 3,  //first triangle
        1, 2, 3   //second triangle

    };
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 3;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, s, t). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);
}

// Implements the UCreateUpperCandlestickMesh function to create the top half of the candle holder
void UCreateUpperCandlestickMesh(GLMesh& mesh) {
    // Position and Color data
    GLfloat verts[] = {
        // Vertex Positions    // Colors
         0.2f, 0.5f, -0.2f,   1.0f, 0.0f, 1.0f, 1.0f, // Top Right: Vertex 0
         0.2f, 0.5f,  0.2f,   1.0f, 0.0f, 0.0f, 1.0f, // Bottom Right: Vertex 1
        -0.2f, 0.5f,  0.2f,   0.0f, 1.0f, 1.0f, 1.0f, // Bottom Left: Vertex 2
        -0.2f, 0.5f, -0.2f,   0.2f, 0.2f, 0.5f, 1.0f, // Top Left: Vertex 3

         0.0f, 0.1f, 0.0f,    0.5f, 0.5f, 1.0f, 1.0f, // Top: Vertex 4
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 2,  // Triangle 1
        0, 3, 2,   // Triangle 2
        0, 1, 4,  // Triangle 3
        1, 2, 4,  // Triangle 4
        2, 3, 4, // Triangle 5
        3, 0, 4,  // Triangle 6
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;
    const GLuint floatsPerUV = 2;

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);
}

// Implements the UCreateCandleMesh function to create the candle
void UCreateCandleMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        // Vertex Positions    // Colors (r,g,b,a)
         0.05f, 1.5f, 0.05f,   1.0f, 0.0f, 0.0f, 1.0f, // Top Right Vertex 0
         0.05f, 0.5f, 0.05f,   0.0f, 1.0f, 0.0f, 1.0f, // Bottom Right Vertex 1
        -0.05f, 0.5f, 0.05f,   0.0f, 0.0f, 1.0f, 1.0f, // Bottom Left Vertex 2
        -0.05f, 1.5f, 0.05f,   1.0f, 0.0f, 1.0f, 1.0f, // Top Left Vertex 3

         0.05f, 0.5f, -0.05f,  0.5f, 0.5f, 1.0f, 1.0f, // 4 br  right
         0.05f, 1.5f, -0.05f,  1.0f, 1.0f, 0.5f, 1.0f, //  5 tl  right
        -0.05f, 1.5f, -0.05f,  0.2f, 0.2f, 0.5f, 1.0f, //  6 tl  top
        -0.05f, 0.5f, -0.05f,  1.0f, 0.0f, 1.0f, 1.0f  //  7 bl back
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 3,  // Triangle 1
        1, 2, 3,   // Triangle 2
        0, 1, 4,  // Triangle 3
        0, 4, 5,  // Triangle 4
        0, 5, 6, // Triangle 5
        0, 3, 6,  // Triangle 6
        4, 5, 6, // Triangle 7
        4, 6, 7, // Triangle 8
        2, 3, 6, // Triangle 9
        2, 6, 7, // Triangle 10
        1, 4, 7, // Triangle 11
        1, 2, 7 // Triangle 12
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);
}

// Implements the UCreateCandleWickMesh function to create the candle wick
void UCreateCandleWickMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        // Vertex Positions    // Colors (r,g,b,a)
         0.02f, 1.7f, 0.02f,   1.0f, 0.0f, 0.0f, 1.0f, // Top Right Vertex 0
         0.02f, 1.5f, 0.02f,   0.0f, 1.0f, 0.0f, 1.0f, // Bottom Right Vertex 1
        -0.02f, 1.5f, 0.02f,   0.0f, 0.0f, 1.0f, 1.0f, // Bottom Left Vertex 2
        -0.02f, 1.7f, 0.02f,   1.0f, 0.0f, 1.0f, 1.0f, // Top Left Vertex 3

         0.02f, 1.5f, -0.02f,  0.5f, 0.5f, 1.0f, 1.0f, // 4 br  right
         0.02f, 1.7f, -0.02f,  1.0f, 1.0f, 0.5f, 1.0f, //  5 tl  right
        -0.02f, 1.7f, -0.02f,  0.2f, 0.2f, 0.5f, 1.0f, //  6 tl  top
        -0.02f, 1.5f, -0.02f,  1.0f, 0.0f, 1.0f, 1.0f  //  7 bl back
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 3,  // Triangle 1
        1, 2, 3,   // Triangle 2
        0, 1, 4,  // Triangle 3
        0, 4, 5,  // Triangle 4
        0, 5, 6, // Triangle 5
        0, 3, 6,  // Triangle 6
        4, 5, 6, // Triangle 7
        4, 6, 7, // Triangle 8
        2, 3, 6, // Triangle 9
        2, 6, 7, // Triangle 10
        1, 4, 7, // Triangle 11
        1, 2, 7 // Triangle 12
    };
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);
   
    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);
}

// Implements the UCreateCandleMesh function to create the lower half of the candle holder
void UCreateLowerCandlestickMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        // Vertex Positions    // Colors (r,g,b,a)
         0.2f, -0.3f, -0.2f,   1.0f, 0.0f, 1.0f, 1.0f, // Top Right: Vertex 0
         0.2f, -0.3f,  0.2f,   1.0f, 0.0f, 0.0f, 1.0f, // Bottom Right: Vertex 1
        -0.2f, -0.3f,  0.2f,   0.0f, 1.0f, 1.0f, 1.0f, // Bottom Left: Vertex 2
        -0.2f, -0.3f, -0.2f,   0.2f, 0.2f, 0.5f, 1.0f, // Top Left: Vertex 3

         0.0f, 0.2f, 0.0f,    0.5f, 0.5f, 1.0f, 1.0f, // Top: Vertex 4
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 2,  // Triangle 1
        0, 3, 2,   // Triangle 2
        0, 1, 4,  // Triangle 3
        1, 2, 4,  // Triangle 4
        2, 3, 4, // Triangle 5
        3, 0, 4,  // Triangle 6
 };
    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;
    const GLuint floatsPerUV = 2;

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);
}

// Implements the UCreateKnifeMesh function to create the butter knife handle
void UCreateKnifeMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        // Vertex Positions    // Colors (r,g,b,a)
        2.6f, -0.20f, 2.0f,   1.0f, 0.0f, 0.0f, 1.0f, // Top Right Vertex 0
        2.6f, -0.15f, 3.5f,   0.0f, 1.0f, 0.0f, 1.0f, // Bottom Right Vertex 1
        2.5f, -0.15f, 3.5f,   0.0f, 0.0f, 1.0f, 1.0f, // Bottom Left Vertex 2
        2.5f, -0.20f, 2.0f,   1.0f, 0.0f, 1.0f, 1.0f, // Top Left Vertex 3

        2.6f, -0.24f, 3.5f,  0.5f, 0.5f, 1.0f, 1.0f, // 4 br  right
        2.6f, -0.24f, 2.0f,  1.0f, 1.0f, 0.5f, 1.0f, //  5 tl  right
        2.5f, -0.24f, 2.0f,  0.2f, 0.2f, 0.5f, 1.0f, //  6 tl  top
        2.5f, -0.24f, 3.5f,  1.0f, 0.0f, 1.0f, 1.0f  //  7 bl back
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 3,  // Triangle 1
        1, 2, 3,   // Triangle 2
        0, 1, 4,  // Triangle 3
        0, 4, 5,  // Triangle 4
        0, 5, 6, // Triangle 5
        0, 3, 6,  // Triangle 6
        4, 5, 6, // Triangle 7
        4, 6, 7, // Triangle 8
        2, 3, 6, // Triangle 9
        2, 6, 7, // Triangle 10
        1, 4, 7, // Triangle 11
        1, 2, 7 // Triangle 12  
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);
}

// Implements the UCreateKnifeTipMesh function to create the butter knife tip
void UCreateKnifeTipMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {
        // Vertex Positions    // Colors (r,g,b,a)
        2.3f, -0.24f, 0.8f,   1.0f, 0.0f, 0.0f, 1.0f, // 0 back top right Vertex 0
        2.6f, -0.24f, 0.5f,   0.0f, 1.0f, 0.0f, 1.0f, // 1 front top right Vertex 
        2.3f, -0.20f, 0.8f,   0.0f, 0.0f, 1.0f, 1.0f, // 2 front top left Vertex 
        2.3f, -0.24f, 2.0f,   1.0f, 0.0f, 1.0f, 1.0f, // 3 Back Top Left Vertex 

        2.6f, -0.20f, 2.0f,  0.5f, 0.5f, 1.0f, 1.0f, //  4 front bottom right 
        2.6f, -0.24f, 2.0f,  1.0f, 1.0f, 0.5f, 1.0f, //  5 bottom back right
        2.6f, -0.24f, 0.5f,  0.2f, 0.2f, 0.5f, 1.0f, //  6 bottom back left 
        2.3f, -0.20f, 2.0f,  1.0f, 0.0f, 1.0f, 1.0f  //  7 bottom front left 
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 3,  // Triangle 1
        1, 2, 3,   // Triangle 2
        0, 1, 4,  // Triangle 3
        0, 4, 5,  // Triangle 4
        0, 5, 6, // Triangle 5
        0, 3, 6,  // Triangle 6
        4, 5, 6, // Triangle 7
        4, 6, 7, // Triangle 8
        2, 3, 6, // Triangle 9
        2, 6, 7, // Triangle 10
        1, 4, 7, // Triangle 11
        1, 2, 7 // Triangle 12
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;
    const GLuint floatsPerUV = 2;

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);
}

// Destroy the mesh
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, mesh.vbos);
}

// build and create the textures used
void generateTextures() {
    // Table texture
    glGenTextures(1, &tableTexture);
    glBindTexture(GL_TEXTURE_2D, tableTexture);
    //set texture wrapping params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //set texture filtering params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //load image, create texture and generate mipmaps
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true); //flip texture on y-axis
    unsigned char* image = stbi_load("../resources/textures/pinktable.png", &width, &height, &channels, 0);
    if (image) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "failed to load texture ../resources/textures/pinktable.png" << std::endl;
    }
    stbi_image_free(image);

    // Candle texture
    glGenTextures(1, &candleTexture);
    glBindTexture(GL_TEXTURE_2D, candleTexture);
    //set texture wrapping params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //set texture filtering params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); //flip texture on y-axis
    unsigned char* image2 = stbi_load("../resources/textures/candle.png", &width, &height, &channels, 0);
    if (image2) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image2);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "failed to load texture ../resources/textures/candle.png" << std::endl;
    }
    stbi_image_free(image2);

    // Upper candlestick holder texture
    glGenTextures(1, &upperCandlestickTexture);
    glBindTexture(GL_TEXTURE_2D, upperCandlestickTexture);
    //set texture wrapping params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //set texture filtering params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); //flip texture on y-axis
    unsigned char* image3 = stbi_load("../resources/textures/silver1.jpg", &width, &height, &channels, 0);
    if (image3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image3);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "failed to load texture ../resources/textures/silver1.jpg" << std::endl;
    }
    stbi_image_free(image3);

    // Lower candlestick holder texture
    glGenTextures(1, &lowerCandlestickTexture);
    glBindTexture(GL_TEXTURE_2D, lowerCandlestickTexture);
    //set texture wrapping params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //set texture filtering params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); //flip texture on y-axis
    unsigned char* image4 = stbi_load("../resources/textures/silver1.jpg", &width, &height, &channels, 0);
    if (image4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image4);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "failed to load texture ../resources/textures/silver1.jpg" << std::endl;
    }
    stbi_image_free(image4);

    // Candle Wick texture
    glGenTextures(1, &candleWickTexture);
    glBindTexture(GL_TEXTURE_2D, candleWickTexture);
    //set texture wrapping params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //set texture filtering params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); //flip texture on y-axis
    unsigned char* image5 = stbi_load("../resources/textures/wickflame.png", &width, &height, &channels, 0);
    if (image5) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image5);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "failed to load texture ../resources/textures/wickflame.png" << std::endl;
    }
    stbi_image_free(image5);

    // Napkin
    glGenTextures(1, &napkinTexture);
    glBindTexture(GL_TEXTURE_2D, napkinTexture);
    //set texture wrapping params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //set texture filtering params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); //flip texture on y-axis
    unsigned char* image6 = stbi_load("../resources/textures/napkin.png", &width, &height, &channels, 0);
    if (image6) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image6);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "failed to load texture ../resources/textures/napkin.png" << std::endl;
    }
    stbi_image_free(image6);

    // Butter Knife Handle
    glGenTextures(1, &knifeTexture);
    glBindTexture(GL_TEXTURE_2D, knifeTexture);
    //set texture wrapping params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //set texture filtering params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); //flip texture on y-axis
    unsigned char* image7 = stbi_load("../resources/textures/butterknife.jpg", &width, &height, &channels, 0);
    if (image7) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image7);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "failed to load texture ../resources/textures/butterknife.jpg" << std::endl;
    }
    stbi_image_free(image7);

    // Butter Knife tip
    glGenTextures(1, &knifeTipTexture);
    glBindTexture(GL_TEXTURE_2D, knifeTipTexture);
    //set texture wrapping params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //set texture filtering params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(true); //flip texture on y-axis
    unsigned char* image8 = stbi_load("../resources/textures/butterknife.jpg", &width, &height, &channels, 0);
    if (image8) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image8);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "failed to load texture ../resources/textures/butterknife.jpg" << std::endl;
    }
    stbi_image_free(image8);
}

// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

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

void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
