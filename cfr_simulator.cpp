#include <filesystem>
#include <vector>
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>

#include "Camera.h"
#include "Shader.h"
#include "Model.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace fs = std::filesystem;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
unsigned int loadCubemap(std::vector<std::string> faces);
glm::vec3 moveTrain(float& X, float& Y, float& Z, float& DegreesY, float& DegreesZ);
glm::vec3 moveTrainBack(float& X, float& Y, float& Z, float& DegreesY, float& DegreesZ);

// settings
constexpr unsigned int SCR_WIDTH = 1920;
constexpr unsigned int SCR_HEIGHT = 1080;
float speed = 1.0f;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = static_cast<float>(SCR_WIDTH) / 2.0;
float lastY = static_cast<float>(SCR_HEIGHT) / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//light
enum LightAction
{
	Sunrise,
	Sunset
};

int main()
{
	std::cout << "<ENTER> Start the train movement\n"
		"<BACKSPACE> Stop the train movement\n"
		"<BACKSLASH> Move train back\n"
		"<1> Driver Camera\n"
		"<2> Outside Camera\n"
		"<3> Free Camera\n"
		"<4> Day Mode\n"
		"<5> Night Mode\n"
		"<+> Increase train speed\n"
		"<-> Decrease train speed\n";


	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	float rotY = 0, rotZ = 0;

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Bucuresti-Brasov", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	float skyboxVertices[] = {
		// positions          
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f
	};

	glm::vec3 lightPos(-10.f, 25.f, 10.0f);

	// build and compile shaders
	// -------------------------
	Shader skyboxShader("skybox.vs", "skybox.fs");

	Shader trainShader("model.vs", "model.fs");
	Shader terrainShader("model.vs", "model.fs");

	Shader bucurestiMapShader("model.vs", "model.fs");
	Shader ploiestiMapShader("model.vs", "model.fs");
	Shader bucegiShader("model.vs", "model.fs");
	Shader brasovShader("model.vs", "model.fs");
	
	Shader lightingShader("PhongLight.vs", "PhongLight.fs");
	Shader lightCubeShader("Lamp.vs", "Lamp.fs");

	Shader shadowShader("ShadowMapping.vs", "ShadowMapping.fs");
	Shader simpleDepthShader("ShadowMappingDepth.vs", "ShadowMappingDepth.fs");

	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));

	// load textures
	// -------------
	fs::path localPath = fs::current_path();
	std::string textureFolder = localPath.string() + "/Resources/Textures";

	Model driverWagon(localPath.string() + "/Resources/train/train-new.obj");
	Model terrain(localPath.string() + "/Resources/terrain/terrain.obj");

	Model bucuresti(localPath.string() + "/Resources/station/bucurestiMap/bucuresti.obj");
	Model ploiesti(localPath.string() + "/Resources/station/ploiestiMap/ploiesti.obj");
	Model bucegi(localPath.string() + "/Resources/bucegi/bucegi.obj");
	Model brasov(localPath.string() + "/Resources/station/brasovMap/brasov.obj");

	// configure depth map FBO
	// -----------------------
	constexpr constexpr unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
	             nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = {1.0, 1.0, 1.0, 1.0};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// shader configuration
	// --------------------
	shadowShader.use();
	shadowShader.setInt("diffuseTexture", 0);
	shadowShader.setInt("shadowMap", 1);

	std::vector<std::string> faces
	{
		textureFolder + "/right.jpg",
		textureFolder + "/left.jpg",
		textureFolder + "/top.jpg",
		textureFolder + "/bottom.jpg",
		textureFolder + "/front.jpg",
		textureFolder + "/back.jpg"
	};

	std::vector<std::string> faces2
	{
		textureFolder + "/right2.jpg",
		textureFolder + "/left2.jpg",
		textureFolder + "/top2.jpg",
		textureFolder + "/bottom2.jpg",
		textureFolder + "/front2.jpg",
		textureFolder + "/back2.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	float startX = -265.0f;
	float startY = -16.0f;
	float startZ = 250.0f;

	float moveX = -10.0f;
	float moveY = 0.0f;
	float moveZ = 0.0f;

	float degreesY = 0.0f;
	float degreesZ = 0.0f;

	bool start = false;


	enum class CameraType
	{
		Free,
		ThirdPerson,
		Driver
	};

	CameraType cameraType = CameraType::Free;

	unsigned int lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), static_cast<void*>(nullptr));
	glEnableVertexAttribArray(0);

	//lights
	LightAction light_action = Sunrise;
	float ambient_intensity = 0.7f;
	float diffuse = 0.2f;
	bool day = true;

	//shadows
	glGenFramebuffers(1, &depthMapFBO);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		lightingShader.use();
		lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		lightingShader.setVec3("lightPos", lightPos);
		lightingShader.setVec3("viewPos", camera.Position);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
		                                        static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f,
		                                        3000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);

		// world transformation
		auto model = glm::mat4(1.0f);
		lightingShader.setMat4("model", model);

		// also draw the lamp object
		lightCubeShader.use();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = translate(model, lightPos);
		model = scale(model, glm::vec3(10.0f));
		lightCubeShader.setMat4("model", model);

		glBindVertexArray(lightCubeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw scene as normal
		bucurestiMapShader.use();
		trainShader.use();
		terrainShader.use();

		trainShader.setMat4("projection", projection);
		trainShader.setMat4("view", view);
		terrainShader.setMat4("projection", projection);
		terrainShader.setMat4("view", view);
		bucurestiMapShader.setMat4("projection", projection);
		bucurestiMapShader.setMat4("view", view);

		// render the loaded model
		auto train = glm::mat4(1.0f);
		auto _terrain = glm::mat4(1.0f);
		auto _bucuresti = glm::mat4(1.0f);
		auto _ploiesti = glm::mat4(1.0f);
		auto _bucegi = glm::mat4(1.0f);
		auto _brasov = glm::mat4(1.0f);

		// train
		if (!start)
			train = translate(train, glm::vec3(startX, startY, startZ));
		else
		{
			train = translate(train, moveTrain(startX, startY, startZ, rotY, rotZ));
		}

		train = scale(train, glm::vec3(0.3f, 0.3f, 0.3f));						   
		train = glm::rotate(train, glm::radians(rotY), glm::vec3(0, 1, 0));
		train = glm::rotate(train, glm::radians(0.0f + rotZ), glm::vec3(0, 0, 1));
		trainShader.setMat4("model", train);
		driverWagon.Draw(trainShader);

		// terrain

		_terrain = translate(_terrain, glm::vec3(650.0f, -38.0f, -750.0f));
		_terrain = scale(_terrain, glm::vec3(2500.0f, 2500.0f, 2500.0f));
		terrainShader.setMat4("model", _terrain);
		terrain.Draw(terrainShader);

		// bucuresti
		_bucuresti = translate(_bucuresti, glm::vec3(-289.2f, -16.0f, 250.0f));
		_bucuresti = scale(_bucuresti, glm::vec3(0.45f, 0.45f, 0.45f));
		_bucuresti = glm::rotate(_bucuresti, glm::radians(313.1f), glm::vec3(0, 1, 0));
		bucurestiMapShader.setMat4("model", _bucuresti);
		bucuresti.Draw(bucurestiMapShader);

		// ploiesti
		_ploiesti = translate(_ploiesti, glm::vec3(347.75f, 12.0f, -450.0f));
		_ploiesti = scale(_ploiesti, glm::vec3(0.45f, 0.45f, 0.45f));
		_ploiesti = glm::rotate(_ploiesti, glm::radians(288.0f), glm::vec3(0, 1, 0));
		_ploiesti = glm::rotate(_ploiesti, glm::radians(2.5f), glm::vec3(1, 0, 0));
		ploiestiMapShader.setMat4("model", _ploiesti);
		ploiesti.Draw(ploiestiMapShader);

		// bucegi
		_bucegi = translate(_bucegi, glm::vec3(250.0f, 40.0f, -850.0f));
		_bucegi = scale(_bucegi, glm::vec3(0.15f, 0.15f, 0.15f));
		_bucegi = glm::rotate(_bucegi, glm::radians(265.0f), glm::vec3(0, 1, 0));
		bucegiShader.setMat4("model", _bucegi);
		bucegi.Draw(bucegiShader);

		// brasov
		_brasov = translate(_brasov, glm::vec3(-90.0f, 25.0f, -1840.0f));
		_brasov = scale(_brasov, glm::vec3(0.45f, 0.45f, 0.45f));
		_brasov = glm::rotate(_brasov, glm::radians(14.0f), glm::vec3(0, 1, 0));
		_brasov = glm::rotate(_brasov, glm::radians(-1.5f), glm::vec3(1, 0, 0));
		_brasov = glm::rotate(_brasov, glm::radians(-5.0f), glm::vec3(0, 0, 1));
		brasovShader.setMat4("model", _brasov);
		brasov.Draw(brasovShader);

		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) // driver camera
		{
			cameraType = CameraType::Driver;
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) // 3rd person camera
		{
			cameraType = CameraType::ThirdPerson;
		}
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) // free camera
		{
			cameraType = CameraType::Free;
		}
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) // day
		{
			cubemapTexture = loadCubemap(faces);
		}
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) // night
		{
			cubemapTexture = loadCubemap(faces2);
		}
		if (glfwGetKey(window, GLFW_KEY_BACKSLASH) == GLFW_PRESS) // move train back
		{
			train = glm::translate(moveTrainBack(startX, startY, startZ, rotY, rotZ));			
		}
		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) // start train
		{
			start = true;
		}
		if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) // stop train
		{
			start = false;
		}

		switch (cameraType)
		{
		case CameraType::Free:
			break;
		case CameraType::ThirdPerson:
			camera.setViewMatrix(glm::vec3(startX - 15, startY + 50, startZ + 100));
			break;
		case CameraType::Driver:
			camera.setViewMatrix(glm::vec3(startX, startY + 2, startZ - 9.5));
			break;
		default: ;
		}

		// draw skybox as last
		glDepthFunc(GL_LEQUAL);
		// change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyboxVAO);
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); // set depth function back to default

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		camera.printPosition();
	if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
	{
		if (speed <= 4.5)
			speed += 0.5;
	}
	if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
	{
		if (speed > 1.5)
			speed -= 0.5;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(const char* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// skybox
// -------------------------------------------------------
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
			             data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

glm::vec3 moveTrain(float& X, float& Y, float& Z, float& DegreesY, float& DegreesZ)
{
	if (X == -265 && Z > 114)
	{
		Z -= 0.1 * speed;
	}
	else if (X < -248 && Z > -40)
	{
		if (DegreesY > -12.5)
			DegreesY -= 0.2;

		X += 0.02 * speed;
		Z -= 0.1 * speed;
		Y += 0.004 * speed;
	}
	else if (X < -214 && Z > -28)
	{
		if (DegreesY > -27)
			DegreesY -= 0.2;

		X += 0.05 * speed;
		Z -= 0.082 * speed;
		Y += 0.002 * speed;
	}
	else if (X < -170 && Z > -82)
	{
		if (DegreesY > -40)
			DegreesY -= 0.3;

		X += 0.06 * speed;
		Z -= 0.09 * speed;
		Y += 0.002 * speed;
	}
	else if (X < -111 && Z > -139)
	{
		if (DegreesY > -52)
			DegreesY -= 0.3;

		X += 0.07 * speed;
		Z -= 0.07 * speed;
		Y += 0.002 * speed;
	}
	else if (X < -54 && Z > -174)
	{
		if (DegreesY > -60)
			DegreesY -= 0.3;

		X += 0.09 * speed;
		Z -= 0.055 * speed;
		Y += 0.002 * speed;
	}
	else if (X < 159 && Z > -248)
	{
		if (DegreesY > -73)
			DegreesY -= 0.3;

		X += 0.09 * speed;
		Z -= 0.028 * speed;
		Y += 0.003 * speed;
	}
	else if (X < 270 && Z > -353)
	{
		if (DegreesY < -50)
			DegreesY += 0.3;

		X += 0.07 * speed;
		Z -= 0.07 * speed;
		Y += 0.003 * speed;
	}
	else if (X < 303 && Z > -419)
	{
		if (DegreesY < -25)
			DegreesY += 0.3;

		X += 0.04 * speed;
		Z -= 0.07 * speed;
		Y += 0.004 * speed;
	}
	else if (X < 329 && Z > -492)
	{
		if (DegreesY < -17)
			DegreesY += 0.3;

		X += 0.04 * speed;
		Z -= 0.11 * speed;
		Y += 0.006 * speed;
	}
	else if (X < 340 && Z > -566)
	{
		if (DegreesY < 0)
			DegreesY += 0.3;
		if (DegreesZ < 5)
			DegreesZ += 0.3;

		X += 0.02 * speed;
		Z -= 0.09 * speed;
		Y += 0.01 * speed;
	}
	else if (X < 349 && Z > -657)
	{
		X += 0.008 * speed;
		Z -= 0.1 * speed;
		Y += 0.012 * speed;
	}
	else if (X < 359 && Z > -766)
	{
		X += 0.014 * speed;
		Z -= 0.13 * speed;
		Y += 0.012 * speed;
	}
	else if (X < 365 && Z > -838)
	{
		X += 0.01 * speed;
		Z -= 0.1 * speed;
		Y += 0.006 * speed;
	}
	else if (X < 369 && Z > -901)
	{
		X += 0.006 * speed;
		Z -= 0.1 * speed;
		Y += 0.013 * speed;
	}
	else if (X < 371 && Z > -965)
	{
		X += 0.003 * speed;
		Z -= 0.1 * speed;
		Y += 0.017 * speed;
	}
	else if (X < 372 && Z > -1059)
	{
		Z -= 0.12 * speed;
		Y += 0.016 * speed;
	}
	else if (X < 372 && Z > -1137)
	{
		Z -= 0.12 * speed;
		Y += 0.025 * speed;
	}
	else if (X < 376 && Z > -1240)
	{
		if (DegreesZ > -10)
			DegreesZ -= 0.3;
		if (X > 390)
			Y -= 0.015;

		X += 0.003 * speed;
		Z -= 0.12 * speed;
		Y -= 0.024 * speed;
	}
	else if (X < 404 && Z > -1435)
	{
		if (DegreesZ > -15)
			DegreesZ -= 0.3;
		if (DegreesY < -5)
			DegreesY += 0.3;

		X += 0.02 * speed;
		Z -= 0.13 * speed;
		Y -= 0.03 * speed;
	}
	else if (X < 406 && Z > -1507)
	{
		if (DegreesZ < 0)
			DegreesZ += 0.3;

		X += 0.003 * speed;
		Z -= 0.13 * speed;
		Y -= 0.01 * speed;
	}
	else if (X > 390 && Z > -1582)
	{
		if (DegreesY < 15)
			DegreesY += 0.3;

		X -= 0.01 * speed;
		Z -= 0.1 * speed;
		Y -= 0.01 * speed;
	}
	else if (X > 351 && Z > -1648)
	{
		if (DegreesY < 30)
			DegreesY += 0.3;

		X -= 0.064 * speed;
		Z -= 0.1 * speed;
		Y -= 0.004 * speed;
	}
	else if (X > 294 && Z > -1705)
	{
		if (DegreesY < 50)
			DegreesY += 0.3;

		X -= 0.1 * speed;
		Z -= 0.1 * speed;
	}
	else if (X > 240 && Z > -1735)
	{
		if (DegreesY < 60)
			DegreesY += 0.3;

		X -= 0.12 * speed;
		Z -= 0.08 * speed;
	}
	else if (X > 167 && Z > -1761)
	{
		if (DegreesY < 70)
			DegreesY += 0.3;

		X -= 0.15 * speed;
		Z -= 0.06 * speed;
		Y -= 0.004 * speed;
	}
	else if (X > -128 && Z > -1763)
	{
		if (DegreesY < 90)
			DegreesY += 0.3;

		X -= 0.16 * speed;
		Z -= 0.01 * speed;
		Y += 0.007;
	}
	else if (X > -136 && Z > -1765)
	{
		X -= 0.13 * speed;
		Z -= 0.0013 * speed;
		Y += 0.007 * speed;
	}

	return glm::vec3(X, Y, Z);
}

//brasov-bucuresti
glm::vec3 moveTrainBack(float& X, float& Y, float& Z, float& DegreesY, float& DegreesZ)
{
	if (X == -265 && Z > 114)
	{
		Z += 0.1 * speed;
	}
	else if (X < -248 && Z > -40)
	{
		if (DegreesY > -12.5)
			DegreesY -= 0.2;

		X -= 0.02 * speed;
		Z += 0.1 * speed;
		Y -= 0.004 * speed;
	}
	else if (X < -214 && Z > -28)
	{
		if (DegreesY > -27)
			DegreesY -= 0.2;

		X -= 0.05 * speed;
		Z += 0.082 * speed;
		Y -= 0.002 * speed;
	}
	else if (X < -170 && Z > -82)
	{
		if (DegreesY > -40)
			DegreesY -= 0.3;

		X -= 0.06 * speed;
		Z += 0.09 * speed;
		Y -= 0.002 * speed;
	}
	else if (X < -111 && Z > -139)
	{
		if (DegreesY > -52)
			DegreesY -= 0.3;

		X -= 0.07 * speed;
		Z += 0.07 * speed;
		Y -= 0.002 * speed;
	}
	else if (X < -54 && Z > -174)
	{
		if (DegreesY > -60)
			DegreesY -= 0.3;

		X -= 0.09 * speed;
		Z += 0.055 * speed;
		Y -= 0.002 * speed;
	}
	else if (X < 159 && Z > -248)
	{
		if (DegreesY > -73)
			DegreesY -= 0.3;

		X -= 0.09 * speed;
		Z += 0.028 * speed;
		Y -= 0.003 * speed;
	}
	else if (X < 270 && Z > -353)
	{
		if (DegreesY < -50)
			DegreesY += 0.3;

		X -= 0.07 * speed;
		Z += 0.07 * speed;
		Y -= 0.003 * speed;
	}
	else if (X < 303 && Z > -419)
	{
		if (DegreesY < -25)
			DegreesY += 0.3;

		X -= 0.04 * speed;
		Z += 0.07 * speed;
		Y -= 0.004 * speed;
	}
	else if (X < 329 && Z > -492)
	{
		if (DegreesY < -17)
			DegreesY += 0.3;

		X -= 0.04 * speed;
		Z += 0.11 * speed;
		Y -= 0.006 * speed;
	}
	else if (X < 340 && Z > -566)
	{
		if (DegreesY < 0)
			DegreesY += 0.3;
		if (DegreesZ < 5)
			DegreesZ += 0.3;

		X -= 0.02 * speed;
		Z += 0.09 * speed;
		Y -= 0.01 * speed;
	}
	else if (X < 349 && Z > -657)
	{
		X -= 0.008 * speed;
		Z += 0.1 * speed;
		Y -= 0.012 * speed;
	}
	else if (X < 359 && Z > -766)
	{
		X -= 0.014 * speed;
		Z += 0.13 * speed;
		Y -= 0.012 * speed;
	}
	else if (X < 365 && Z > -838)
	{
		X -= 0.01 * speed;
		Z += 0.1 * speed;
		Y -= 0.006 * speed;
	}
	else if (X < 369 && Z > -901)
	{
		X -= 0.006 * speed;
		Z += 0.1 * speed;
		Y -= 0.013 * speed;
	}
	else if (X < 371 && Z > -965)
	{
		X -= 0.003 * speed;
		Z += 0.1 * speed;
		Y -= 0.017 * speed;
	}
	else if (X < 372 && Z > -1059)
	{
		Z += 0.12 * speed;
		Y -= 0.016 * speed;
	}
	else if (X < 372 && Z > -1137)
	{
		Z += 0.12 * speed;
		Y -= 0.025 * speed;
	}
	else if (X < 376 && Z > -1240)
	{
		if (DegreesZ > -10)
			DegreesZ -= 0.3;
		if (X > 390)
			Y -= 0.015;

		X -= 0.003 * speed;
		Z += 0.12 * speed;
		Y += 0.024 * speed;
	}
	else if (X < 404 && Z > -1435)
	{
		if (DegreesZ > -15)
			DegreesZ -= 0.3;
		if (DegreesY < -5)
			DegreesY += 0.3;

		X -= 0.02 * speed;
		Z += 0.13 * speed;
		Y += 0.03 * speed;
	}
	else if (X < 406 && Z > -1507)
	{
		if (DegreesZ < 0)
			DegreesZ += 0.3;

		X -= 0.003 * speed;
		Z += 0.13 * speed;
		Y += 0.01 * speed;
	}
	else if (X > 390 && Z > -1582)
	{
		if (DegreesY < 15)
			DegreesY += 0.3;

		X += 0.01 * speed;
		Z += 0.1 * speed;
		Y += 0.01 * speed;
	}
	else if (X > 351 && Z > -1648)
	{
		if (DegreesY < 30)
			DegreesY += 0.3;

		X += 0.064 * speed;
		Z += 0.1 * speed;
		Y += 0.004 * speed;
	}
	else if (X > 294 && Z > -1705)
	{
		if (DegreesY < 50)
			DegreesY += 0.3;

		X += 0.1 * speed;
		Z += 0.1 * speed;
	}
	else if (X > 240 && Z > -1735)
	{
		if (DegreesY < 60)
			DegreesY += 0.3;

		X += 0.12 * speed;
		Z += 0.08 * speed;
	}
	else if (X > 167 && Z > -1761)
	{
		if (DegreesY < 70)
			DegreesY -= 0.3;

		X += 0.15 * speed;
		Z += 0.06 * speed;
		Y += 0.004 * speed;
	}
	else if (X > -128 && Z > -1763)
	{
		if (DegreesY < 90)
			DegreesY -= 0.3;

		X += 0.16 * speed;
		Z += 0.01 * speed;
		Y -= 0.007;
	}
	else if (X > -136 && Z > -1765)
	{
		X += 0.13 * speed;
		Z += 0.0013 * speed;
		Y -= 0.007 * speed;
	}

	return glm::vec3(X, Y, Z);
}
