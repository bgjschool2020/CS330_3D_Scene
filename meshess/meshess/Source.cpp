///////////////////////////////////////////////////////////////////////////////
// Source.cpp  *** Adapted by Bryce Jensen ***
// ==========
// CS-330: 6-5 Milestone
// Basically runs everything. Contains the scene and all objects in the URender function.
// 
// meshes Credit goes to:
// AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
// Created for CS-330-Computational Graphics and Visualization, Nov. 7th, 2022
///////////////////////////////////////////////////////////////////////////////


#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb/stb_image.h"

#include <Windows.h>		// sleep function
// GLM Math Header inclusions
#include <glm/glm.hpp>
//#include <glad/glad.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "meshes.h"

// Camera Class
#include "meshess/camera.h"


using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
	const char* const WINDOW_TITLE = "Bryce Jensen"; // Macro for window title

	// Variables for window width and height
	const int WINDOW_WIDTH = 1920;
	const int WINDOW_HEIGHT = 1080;

	struct gLampMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbo;         // Handle for the vertex buffer object
		GLuint nVertices;    // Number of indices of the mesh
	};

	gLampMesh lamp1Mesh;
	gLampMesh lamp2Mesh;

	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao;         // Handle for the vertex array object
		GLuint vbos[2];     // Handles for the vertex buffer objects
		GLuint nIndices;    // Number of indices of the mesh
	};

	// Main GLFW window
	GLFWwindow* gWindow = nullptr;
	// Triangle mesh data
	//GLMesh gMesh;
	
	// Texture
	GLuint gTextureId_Screen;
	GLuint gTextureId_Keyboard;
	GLuint gTextureId_Plastic;
	GLuint gTextureId_Aluminum;
	GLuint gTextureId_Wood;
	GLuint gTextureId_BlackPlastic;
	GLuint gTextureId_LightWood;
	GLuint gTextureId_BlackFabric;
	GLuint gTextureId_Screen2;
	GLuint gTextureId_BlueFabric;
	GLuint gTextureId_BluePlastic;
	GLuint gTextureId_JBL;
	glm::vec2 gUVScale(1.0f, 2.0f);
	glm::vec2 gUVScale1(1.0f, 1.0f);

	GLint gTexWrapMode = GL_REPEAT;
	
	// Shader program
	GLuint gProgramId;
	GLuint gLampProgramId;

	//Shape Meshes from Professor Brian
	Meshes meshes;

	// camera
	Camera gCamera(glm::vec3(0.0f, 2.0f, -8.0f));
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// timing
	float gDeltaTime = 0.0f; // time between current frame and last frame
	float gLastFrame = 0.0f;

	// Lighting
	// Cube and light color
	//m::vec3 gObjectColor(0.6f, 0.5f, 0.75f);
	glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
	glm::vec3 gLightColor(1.f, 0.8f, 0.5f);
	glm::vec3 gLightColor2(0.8f, 0.8f, 1.0f);

	// Light position and scale
	glm::vec3 gLightPosition(-8.f, 8.f, -8.f);
	glm::vec3 gLightPosition2(-30.f, 36.f, 36.f);

	glm::vec3 gLightScale(0.5f);

}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char*[], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
//void UCreateMesh(GLMesh &mesh);
//void UDestroyMesh(GLMesh &mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint &programId);
void UDestroyShaderProgram(GLuint programId);
void importTexture(const char* filename, GLuint &texName);
void UCreateLampMesh(gLampMesh& lampMesh);



/* Cube Vertex Shader Source Code*/
const GLchar* cubeVertexShaderSource = GLSL(440,

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
const GLchar* cubeFragmentShaderSource = GLSL(440,

	in vec3 vertexNormal; // For incoming normals
	in vec3 vertexFragmentPos; // For incoming fragment position
	in vec2 vertexTextureCoordinate;

	out vec4 fragmentColor; // For outgoing cube color to the GPU

	// Uniform / Global variables for object color, light color, light position, and camera/view position
	uniform vec3 objectColor;
	uniform vec3 lightColor;
	uniform vec3 lightColor2;
	uniform vec3 lightPos;
	uniform vec3 lightPos2;
	uniform vec3 viewPosition;

	uniform sampler2D uTexture; // Useful when working with multiple textures
	uniform vec2 uvScale;

	void main()
	{
		/*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

		/*Calculate Ambient lighting*/
		float light1Strength = 0.2f; // Set ambient or global lighting strength
		float light2Strength = 0.4f; // Set ambient or global lighting strength
		vec3 ambient = light1Strength * lightColor; // Generate ambient light color
		vec3 ambient2 = light2Strength * lightColor2; // Generate ambient light color


		/*Calculate Diffuse lighting*/
		vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit

		// Light 1
		vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
		float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
		vec3 diffuse = impact * lightColor; // Generate diffuse light color

		// Light 2
		vec3 lightDirection2 = normalize(lightPos2 - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
		float impact2 = max(dot(norm, lightDirection2), 0.0);// Calculate diffuse impact by generating dot product of normal and light
		vec3 diffuse2 = impact2 * lightColor2; // Generate diffuse light color

		/*Calculate Specular lighting*/
		// Light 1
		float specularIntensity = 0.4f; // Set specular light strength
		float highlightSize = 16.0f; // Set specular highlight size

		//// Light 2
		//float specularIntensity2 = 3.f; // Set specular light strength
		//float highlightSize2 = 16.0f; // Set specular highlight size

		vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
		vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector for light 1
		//vec3 reflectDir2 = reflect(-lightDirection2, norm);// Calculate reflection vector for light 2

		/*Calculate specular component*/
		// Light 1
		float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
		vec3 specular = specularIntensity * specularComponent * lightColor;
		vec3 specular2 = specularIntensity * specularComponent * lightColor2;

		//// Light 2
		//float specularComponent2 = pow(max(dot(viewDir, reflectDir2), 0.0), highlightSize2);
		//vec3 specular2 = specularIntensity2 * specularComponent2 * lightColor2;

		// Texture holds the color to be used for all three components
		vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

		/*Calculate phong result*/
		vec3 phong = (ambient + ambient2 + diffuse + diffuse2 + specular) * textureColor.xyz;  //Light 1
		//vec3 phong2 = (ambient + diffuse2 + specular2) * textureColor.xyz;  //Light 2

		fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
	}
);


/*Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

	layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

	//Uniform / Global variables for the  transform matrices
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;

	void main()
	{
		gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
	}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

	out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

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

int main(int argc, char* argv[]) {
	if (!UInitialize(argc, argv, &gWindow))
		return EXIT_FAILURE;
 
	// Create the mesh
	//UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object
	meshes.CreateMeshes();

	// Create the shader program
	//if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
	//	return EXIT_FAILURE;

	// Create the shader programs
	if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gProgramId))
		return EXIT_FAILURE;

	if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
		return EXIT_FAILURE;

	UCreateLampMesh(lamp1Mesh);
	UCreateLampMesh(lamp2Mesh);


	/* Load textures */ // All other textures, not specifically credited, were sourced from ambientcg.com
	importTexture("resources/textures/sun_cropped.png", gTextureId_Screen);  // Credit: u/ajamesmccarthy : reddit
	importTexture("resources/textures/Keyboard.jpg", gTextureId_Keyboard);  // Credit: Bryce Jensen
	importTexture("resources/textures/Plastic004_2K-PNG/Plastic004_2K-PNG_Color.png", gTextureId_Plastic);
	importTexture("resources/textures/Metal032_2K-PNG_Color.png", gTextureId_Aluminum);
	importTexture("resources/textures/Wood060_4K-PNG_Color.png", gTextureId_Wood);
	importTexture("resources/textures/Plastic006_2K-PNG/Plastic006_2K-PNG_Color.png", gTextureId_BlackPlastic);
	importTexture("resources/textures/WoodFloor007_2k-PNG/WoodFloor007_2K-PNG_Color.png", gTextureId_LightWood);
	importTexture("resources/textures/Fabric042_2K-PNG/Fabric042_2K-PNG_Color.png", gTextureId_BlackFabric);
	importTexture("resources/textures/moon.png", gTextureId_Screen2);  // Credit: u/ajamesmccarthy : reddit
	importTexture("resources/textures/Fabric022_2K-PNG/Fabric022_2K-PNG_Color.png", gTextureId_BlueFabric);
	importTexture("resources/textures/Plastic008_2K-PNG/Plastic008_2K-PNG_Color.png", gTextureId_BluePlastic);
	importTexture("resources/textures/JBL.png", gTextureId_JBL);  // Credit:Bryce Jensen




	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	glUseProgram(gProgramId);
	// We set the texture as texture unit 0
	glUniform1i(glGetUniformLocation(gProgramId, "Sun"), 0);
	// We set the texture as texture unit 1
	glUniform1i(glGetUniformLocation(gProgramId, "Keyboard"), 1);
	// We set the texture as texture unit 2
	glUniform1i(glGetUniformLocation(gProgramId, "Plastic"), 2);
	// We set the texture as texture unit 3
	glUniform1i(glGetUniformLocation(gProgramId, "Aluminum"), 3);
	// We set the texture as texture unit 4
	glUniform1i(glGetUniformLocation(gProgramId, "Wood"), 4);
	// We set the texture as texture unit 5
	glUniform1i(glGetUniformLocation(gProgramId, "BlackPlastic"), 5);
	// We set the texture as texture unit 6
	glUniform1i(glGetUniformLocation(gProgramId, "LightWood"), 6);
	// We set the texture as texture unit 7
	glUniform1i(glGetUniformLocation(gProgramId, "BlackFabric"), 7);
	// We set the texture as texture unit 8
	glUniform1i(glGetUniformLocation(gProgramId, "Moon"), 8);
	// We set the texture as texture unit 9
	glUniform1i(glGetUniformLocation(gProgramId, "BlueFabric"), 9);
	// We set the texture as texture unit 10
	glUniform1i(glGetUniformLocation(gProgramId, "BluePlastic"), 10);
	// We set the texture as texture unit 11
	glUniform1i(glGetUniformLocation(gProgramId, "JBL"), 11);

	// Sets the background color of the window (it will be implicitely used by glClear)
	glClearColor(1.f, 0.3f, 0.3f, 1.0f);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(gWindow)) {
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
	//UDestroyMesh(gMesh);
	meshes.DestroyMeshes();

	// Release textures
	UDestroyTexture(gTextureId_Screen);
	UDestroyTexture(gTextureId_Keyboard);
	UDestroyTexture(gTextureId_Plastic);
	UDestroyTexture(gTextureId_Aluminum);
	UDestroyTexture(gTextureId_Wood);
	UDestroyTexture(gTextureId_BlackPlastic);
	UDestroyTexture(gTextureId_LightWood);
	UDestroyTexture(gTextureId_BlackFabric);
	UDestroyTexture(gTextureId_Screen2);
	UDestroyTexture(gTextureId_BlueFabric);
	UDestroyTexture(gTextureId_BluePlastic);
	UDestroyTexture(gTextureId_JBL);

	// Release shader program
	UDestroyShaderProgram(gProgramId);
	UDestroyShaderProgram(gLampProgramId);
	
	exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window) {
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
	*window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
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


	//using a mouse
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
void UProcessInput(GLFWwindow* window) {

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W))
		gCamera.ProcessKeyboard(FORWARD, gDeltaTime);

	if (glfwGetKey(window, GLFW_KEY_A))
		gCamera.ProcessKeyboard(LEFT, gDeltaTime);

	if (glfwGetKey(window, GLFW_KEY_S))
		gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);

	if (glfwGetKey(window, GLFW_KEY_D))
		gCamera.ProcessKeyboard(RIGHT, gDeltaTime);

	if (glfwGetKey(window, GLFW_KEY_E))
		gCamera.ProcessKeyboard(UPWARD, gDeltaTime);

	if (glfwGetKey(window, GLFW_KEY_Q))
		gCamera.ProcessKeyboard(DOWNWARD, gDeltaTime);

	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
		gCamera.SetIsPerspective(!gCamera.IsPerspective);
		Sleep(250);
	}
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
	// resets back to initial perspective view
	case GLFW_MOUSE_BUTTON_LEFT:
	{
		if (action == GLFW_PRESS) {
			cout << "Left mouse button pressed" << endl;
			gCamera.IsPerspective = true;
			gCamera.Position = glm::vec3(0.0f, 2.0f, -8.0f);
			gCamera.Yaw = 90.f;
		}
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
	glm::mat4 scale;
	glm::mat4 rotation;
	glm::mat4 rotation2;
	glm::mat4 translation;
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	GLint modelLoc;
	GLint viewLoc;
	GLint projLoc;
	GLint objectColorLoc;

	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the frame and z buffers
	glClearColor(0.6f, 0.48f, 0.37f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glfwSwapInterval(1); // Set framerate to screen refresh. 0 (default) is unlimited, was doing 2000 fps and eating 100% of the GPU. https://stackoverflow.com/a/76840008

	//***********************************************************************************************************//
	// CAMERA SETTINGS
	
	view = gCamera.GetViewMatrix();

	projection = gCamera.GetProjectionMatrix(WINDOW_WIDTH, WINDOW_HEIGHT, gCamera.Zoom);

	// Perspective
	///////////////////////////////////////
	// Activate these two lines for a perspective view from in front, a birds eye view if you will
	//					where the camera is, where it is looking, which direction is up for the camera
	//view = glm::lookAt(glm::vec3(0.f, 2.f, -4.f), glm::vec3(0.0f, 1.f, 0.f), glm::vec3(0, 1, 0));
	//projection = glm::perspective(glm::radians(90.0f), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	
	// view from the left at the laptop
	//view = glm::lookAt(glm::vec3(3.8f, 1.5f, -1.5f), glm::vec3(1.55f, 0.6f, 0.095f), glm::vec3(0, 1, 0));
	
	// view from the right at the laptop
	//view = glm::lookAt(glm::vec3(0.f, 2.f, -1.f), glm::vec3(1.45f, 0.73f, 0.085f), glm::vec3(0, 1, 0));
	///////////////////////////////////////

	//Orthographic
	///////////////////////////////////////
	// Activate these two lines for an orthographic view from straight above
	//view = glm::lookAt(glm::vec3(0.f, 3.f, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0, 0, 1));  // above for ortho
	//projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
	///////////////////////////////////////

	///////////////////////////////////////
	// These are the defaults that came with the program
	// Transforms the camera
	//view = glm::translate(glm::vec3(0.0f, -4.0f, -12.0f));

	// Creates a orthographic projection
	//projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);
	//projection = glm::perspective(glm::radians(60.0f), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	///////////////////////////////////////

	//***********************************************************************************************************//

	

	// Set the shader to be used
	glUseProgram(gProgramId);

	// Retrieves and passes transform matrices to the Shader program
	modelLoc = glGetUniformLocation(gProgramId, "model");
	viewLoc = glGetUniformLocation(gProgramId, "view");
	projLoc = glGetUniformLocation(gProgramId, "projection");
	objectColorLoc = glGetUniformLocation(gProgramId, "uObjectColor");

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);



	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PLANE 
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPlaneMesh.vao);
	
	// 1. Scales the object
	scale = glm::scale(glm::vec3(4.0f, 1.0f, 2.0f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gProgramId, objectColorLoc, 0.8f, 0.8f, 0.8f, 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_Plastic);
	
	// Scales the Texture by gUVScale
	GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Scales texture back to 1
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale1));


	// Deactivate the Texture
	glBindTexture(GL_TEXTURE_2D, 0);
	//glBindTexture(GL_TEXTURE_2D, 0);



	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	

	// OBJECT 1: Plasma Ball
	//***********************************************************************************************************//
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//// TAPERED CYLINDER
	//// Activate the VBOs contained within the mesh's VAO
	//glBindVertexArray(meshes.gTaperedCylinderMesh.vao);

	//// 1. Scales the object
	//scale = glm::scale(glm::vec3(0.33f, 0.33f, 0.33f));  // 4 inches across
	//// 2. Rotate the object
	//rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	//// 3. Position the object
	//translation = glm::translate(glm::vec3(-0.5f, 0.f, 0.f));
	//// Model matrix: transformations are applied right-to-left order
	//model = translation * rotation * scale;
	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 0.2f, 0.2f, 0.2f, 1.0f);

	//// bind textures on corresponding texture units
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, gTextureId_BlackPlastic);
	//
	//// Draws the triangles
	//glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	//glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	//glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	//// Destroy the Texture
	//glBindTexture(GL_TEXTURE_2D, 0);

	//// Deactivate the Vertex Array Object
	//glBindVertexArray(0);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//// SPHERE
	//// Activate the VBOs contained within the mesh's VAO
	//glBindVertexArray(meshes.gSphereMesh.vao);

	//// 1. Scales the object
	//scale = glm::scale(glm::vec3(0.25f, 0.25f, 0.25f));  // 3 inches across
	//// 2. Rotate the object
	//rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	//// 3. Position the object
	//translation = glm::translate(glm::vec3(-0.5f, 0.5f, 0.f));
	//// Model matrix: transformations are applied right-to-left order
	//model = translation * rotation * scale;
	//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 1.0f, 1.0f, 0.1f);

	//// Draws the triangles
	//glDrawElements(GL_TRIANGLES, meshes.gSphereMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	//// Deactivate the Vertex Array Object
	//glBindVertexArray(0);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//***********************************************************************************************************//
	

	// OBJECT 2: Monitor
	//***********************************************************************************************************//
	

	// Currently appears floating in the air, a primitive "crate" will be underneath it eventually
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// BOX - Base scale 2x
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.5f, 0.04f, 1.4f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.4f, 0.627f, 1.2f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gProgramId, objectColorLoc, 0.5f, 0.5f, 0.5f, 1.0f);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_Aluminum);

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Destroy the Texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CYLINDER scale 1x
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.06f, 1.25f, 0.08f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.5, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.4f, 0.625f, 1.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gProgramId, objectColorLoc, 0.5f, 0.5f, 0.5f, 1.0f);
	
	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_Aluminum);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Destroy the Texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// BOX - Screen Frame scale 2x
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(4.0f, 2.5f, 0.125f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.4f, 2.025f, 1.4f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gProgramId, objectColorLoc, 0.2f, 0.2f, 0.2f, 1.0f);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_BlackPlastic);

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Destroy the Texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// BOX - Screen scale 2x
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(3.8f, 2.3f, 0.12f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.4f, 2.025f, 1.39f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// Sets uniform color of the object
	glProgramUniform4f(gProgramId, objectColorLoc, 0.2f, 0.2f, 0.6f, 1.0f);


	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_Screen);

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);
	
	// Destroy the Texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//***********************************************************************************************************//
	



	// Primitive 1 - Crate - scale 2x
	//***********************************************************************************************************//
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(2.5f, 0.627f, 1.66f));
	// 2. Rotate the object
	rotation = glm::rotate(0.0f, glm::vec3(1.0, 1.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.4f, 0.313f, 1.1f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gProgramId, objectColorLoc, 0.5f, 0.5f, 0.5f, 1.0f);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_LightWood);
	
	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Destroy the Texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);

	//***********************************************************************************************************//




	// OBJECT 3: Laptop and Stand
	//***********************************************************************************************************//
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// BOX - Base
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.5f, 0.04f, 1.4f));
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(45.f), glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(2.8f, 0.02f, 1.f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gProgramId, objectColorLoc, 0.5f, 0.5f, 0.5f, 1.0f);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_BlackPlastic);

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CYLINDER - stand support scale 1x
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.12f, 1.72f, 0.16f));
	// 2. Rotate the object
	// Note to self: Look into quaternions, it may make this weird double rotation thing work better.
	rotation = glm::rotate(glm::radians(-45.0f), glm::vec3(1.0, 0.0f, 0.0f));
	rotation2 = glm::rotate(glm::radians(45.0f), glm::vec3(0.0, 0.0f, 1.0f));

	// 3. Position the object
	translation = glm::translate(glm::vec3(2.7f, -0.08f, 0.9f));
	// Model matrix: transformations are applied right-to-left order
	model = translation  * rotation * rotation2 * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gProgramId, objectColorLoc, 1.f, 1.f, 1.f, 1.0f);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Destroy the Texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// BOX - Screen frame scale 2x
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(2.66f, 1.66f, 0.05f));
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(45.f), glm::vec3(0.f, 1.0f, 0.f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(2.f, 2.025f, 0.5f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gProgramId, objectColorLoc, 0.2f, 0.2f, 0.2f, 1.0f);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_BlackPlastic);

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Destroy the Texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// BOX - Keyboard scale 2x
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(2.66f, 1.66f, 0.1f));
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(45.f), glm::vec3(0.f, 1.f, 0.f));
	rotation2 = glm::rotate(glm::radians(60.f), glm::vec3(1.f, 0.f, 0.f));
	
	// 3. Position the object
	translation = glm::translate(glm::vec3(1.48f, 0.77f, -0.05f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * rotation2 * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_BlackPlastic);

	glProgramUniform4f(gProgramId, objectColorLoc, 0.3f, 0.3f, 0.3f, 1.0f);

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// BOX - Screen scale 2x
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gBoxMesh.vao);

	// 1. Scales the object
	scale = glm::scale( glm::vec3(2.55f, 1.4f, 0.03f));
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(45.f), glm::vec3(0.f, 1.0f, 0.f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(1.99f, 2.105f, 0.49f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gProgramId, objectColorLoc, 0.2f, 0.2f, 0.6f, 1.0f);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_Screen2);

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gBoxMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Destroy the Texture
	glBindTexture(GL_TEXTURE_2D, 0);
	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////


	// PLANE - Keyboard Tex
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPlaneMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(1.33f, 1.0f, 0.83f ));
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(225.f), glm::vec3(0.f, 1.f, 0.f));
	rotation2 = glm::rotate(glm::radians(30.f), glm::vec3(1.f, 0.f, 0.f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(1.45f, 0.81f, -0.07f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * rotation2 * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gProgramId, objectColorLoc, 0.8f, 0.8f, 0.8f, 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_Keyboard);

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	//***********************************************************************************************************//



	//Primitive 2 - Homepod - scale 1x
	//***********************************************************************************************************//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// SPHERE
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gSphereMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.33f, 0.33f, 0.33f));  // 3 inches across
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(180.f), glm::vec3(0.0, 1.0f, 0.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-2.4f, 0.93f, 0.6f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_BlackFabric);

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gSphereMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Destroy the Texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//***********************************************************************************************************//


	// Primitive 3 - JBL Speaker - scale 1x
	//***********************************************************************************************************//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// CYLINDER
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.25f, 0.92f, 0.25f));
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(90.f), glm::vec3(0.0, 0.0f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-0.6f, 0.87f, 0.38f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gProgramId, objectColorLoc, 1.0f, 1.0f, 0.0f, 1.0f);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_BlueFabric);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Destroy the Texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 
	// 
	// Added some things to it.
	// 
	// Left of speaker
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TAPERED CYLINDER
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gTaperedCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.25f, 0.05f, 0.25f));  // 3 inches across
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(-90.f), glm::vec3(0.f, 0.f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-0.6f, 0.87f, 0.38f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gProgramId, objectColorLoc, 0.2f, 0.2f, 0.2f, 1.0f);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_BluePlastic);
	
	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Destroy the Texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//Right of speaker
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// TAPERED CYLINDER
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gTaperedCylinderMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.25f, 0.05f, 0.25f));  // 3 inches across
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(90.f), glm::vec3(0.f, 0.f, 1.0f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.52f, 0.87f, 0.38f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gProgramId, objectColorLoc, 0.2f, 0.2f, 0.2f, 1.0f);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_BluePlastic);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLE_FAN, 0, 36);		//bottom
	glDrawArrays(GL_TRIANGLE_FAN, 36, 36);		//top
	glDrawArrays(GL_TRIANGLE_STRIP, 72, 146);	//sides

	// Destroy the Texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// JBL logo
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// PLANE 
	// Activate the VBOs contained within the mesh's VAO
	glBindVertexArray(meshes.gPlaneMesh.vao);

	// 1. Scales the object
	scale = glm::scale(glm::vec3(0.083f, 1.0f, 0.07f));
	// 2. Rotate the object
	rotation = glm::rotate(glm::radians(90.f), glm::vec3(1.0, 0.f, 0.f));
	rotation2 = glm::rotate(glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f));
	// 3. Position the object
	translation = glm::translate(glm::vec3(-1.045f, 0.87f, 0.13f));
	// Model matrix: transformations are applied right-to-left order
	model = translation * rotation * rotation2 * scale;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glProgramUniform4f(gProgramId, objectColorLoc, 0.8f, 0.8f, 0.8f, 1.0f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId_JBL);

	// Draws the triangles
	glDrawElements(GL_TRIANGLES, meshes.gPlaneMesh.nIndices, GL_UNSIGNED_INT, (void*)0);

	// Deactivate the Texture
	glBindTexture(GL_TEXTURE_2D, 0);
	//glBindTexture(GL_TEXTURE_2D, 0);



	// Deactivate the Vertex Array Object
	glBindVertexArray(0);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//***********************************************************************************************************//



	//***********************************************************************************************************//
	// LIGHTS
	
	// Reference matrix uniforms from the shape shader program for the shape color, light color, light position, and camera position
	objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");

	// Spotlight
	GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
	GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");

	// Key light
	GLint lightColorLoc2 = glGetUniformLocation(gProgramId, "lightColor2");
	GLint lightPositionLoc2 = glGetUniformLocation(gProgramId, "lightPos2");

	// Camera view
	GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

	// Pass color, light, and camera data to the shape shader 
	glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);

	// Light 1
	glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
	glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);

	// Light 2
	glUniform3f(lightColorLoc2, gLightColor2.r, gLightColor2.g, gLightColor2.b);
	glUniform3f(lightPositionLoc2, gLightPosition2.x, gLightPosition2.y, gLightPosition2.z);

	// --------------------
	// Draw Lamp 1
	glUseProgram(gLampProgramId);
	glBindVertexArray(lamp1Mesh.vao);

	// Light location and Scale
	model = glm::translate(gLightPosition) * glm::scale(gLightScale);

	// Matrix uniforms from the Light Shader program
	modelLoc = glGetUniformLocation(gLampProgramId, "model");
	viewLoc = glGetUniformLocation(gLampProgramId, "view");
	projLoc = glGetUniformLocation(gLampProgramId, "projection");

	// Matrix data
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Draw the lamp
	glDrawArrays(GL_TRIANGLES, 0, lamp1Mesh.nVertices);
	// --------------------


	// --------------------
	// Draw Lamp 2
	glUseProgram(gLampProgramId);
	glBindVertexArray(lamp2Mesh.vao);

	// Light location and Scale
	model = glm::translate(gLightPosition2) * glm::scale(gLightScale);

	// Matrix uniforms from the Light Shader program
	modelLoc = glGetUniformLocation(gLampProgramId, "model");
	viewLoc = glGetUniformLocation(gLampProgramId, "view");
	projLoc = glGetUniformLocation(gLampProgramId, "projection");

	// Matrix data
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Draw the lamp
	glDrawArrays(GL_TRIANGLES, 0, lamp2Mesh.nVertices);
	// --------------------

	//***********************************************************************************************************//

	
	glUseProgram(0);

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
	if (image)
	{
		flipImageVertically(image, width, height, channels);

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


void UDestroyTexture(GLuint textureId)
{
	glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint &programId)
{
	// Compilation and linkage error reporting
	int success = 0;
	char infoLog[512];

	// Create a Shader program object.
	programId = glCreateProgram();

	// Create the vertex and fragment shader objects
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrieve the shader source
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

void importTexture(const char* filename, GLuint& texName) {
	const char* texFilename = filename;
	if (!UCreateTexture(filename, texName)) {
		std::cout << "Failed to load texture " << filename << std::endl;
	}
}

// Template for creating a cube light
void UCreateLampMesh(gLampMesh& lampMesh) {
	// Position and Color data
	GLfloat verts[] = {
		//Positions          //Normals
		// ------------------------------------------------------
		//Back Face          //Negative Z Normal  Texture Coords.
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		//Front Face         //Positive Z Normal
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

		//Left Face          //Negative X Normal
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		//Right Face         //Positive X Normal
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		//Bottom Face        //Negative Y Normal
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		//Top Face           //Positive Y Normal
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerNormal = 3;
	const GLuint floatsPerUV = 2;

	lampMesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

	glGenVertexArrays(1, &lampMesh.vao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(lampMesh.vao);

	// Create 2 buffers: first one for the vertex data; second one for the indices
	glGenBuffers(1, &lampMesh.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, lampMesh.vbo); // Activates the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

	// Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);
}