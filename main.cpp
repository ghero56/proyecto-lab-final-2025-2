/*---------------------------------------------------------*/
/*-------- Código fuente de Luis Sergio Valencia ----------*/
/*---------------------------------------------------------*/
/*-------- Edición inicial de Fernando Arciga G. ----------*/
/*--------  - Bibliotecas implementadas: ------------------*/
/*--------     - Fmod				  ---------------------*/
/*--------     - DearImGUI			  ---------------------*/
/*--------     - Yaml			      ---------------------*/
/*-------- Edición de Oscar Manuel Suaznavar Arvizu -------*/
/*-------- Edición de........................... ----------*/
/*-------- Edición de........................... ----------*/
/*--------- Proyecto Final Computación Gráfica ------------*/
/*---------- e Interacción Humano Computadora -------------*/
/*---------------------------------------------------------*/

#include <Windows.h>

// para dearImGUI
#include "imgui.h"
#include <glad/glad.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// antes de la OpenGL
#include "GLFW/glfw3.h"						//main

#include <stdlib.h>		
#include <glm/glm.hpp>					//camera y model
#include <glm/gtc/matrix_transform.hpp>	//camera y model
#include <glm/gtc/type_ptr.hpp>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>					//Texture

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

#include <shader_m.h>
#include <camera.h>
#include <modelAnim.h>
#include <model.h>
#include <Skybox.h>
#include <iostream>
#include <mmsystem.h>

// extras para la lectura de archivos
#include <fstream>
#include <string>
#include <filesystem>

using namespace std;

#include "Scene.h"

// para la de/serialización
#include <yaml-cpp/yaml.h>

// para el audio
#include <fmod.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void my_input(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseInput_Callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void animate(void);

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWmonitor* monitors;

GLuint VBO[3], VAO[3], EBO[3];

//Camera
Camera camera(glm::vec3(0.0f, 10.0f, 3.0f));
float MovementSpeed = 0.1f;
GLfloat lastX = SCR_WIDTH / 2.0f,
		lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool movementMode = false;
bool showMouse = false;

//Timing
const int FPS = 60;
const int LOOP_TIME = 1000 / FPS; // = 16 milisec // 1000 millisec == 1 sec
double	deltaTime = 0.0f,
lastFrame = 0.0f;

void getResolution(void);
void myData(void);							// De la practica 4
void LoadTextures(void);					// De la pr�ctica 6
unsigned int generateTextures(char*, bool, bool);	// De la pr�ctica 6

//For Keyboard
float	movX = 0.0f,
movY = 0.0f,
movZ = -5.0f,
rotX = 0.0f;

//Texture
unsigned int	t_smile,
t_toalla,
t_unam,
t_white,
t_ladrillos;

//Lighting
glm::vec3 lightPosition(0.0f, 4.0f, -10.0f);
glm::vec3 lightDirection(0.0f, -1.0f, -1.0f);

//// Light
glm::vec3 lightColor = glm::vec3(0.7f);
glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
glm::vec3 ambientColor = diffuseColor * glm::vec3(0.75f);

// posiciones
float	movAuto_x = 0.0f,
movAuto_z = 0.0f,
orienta = 90.0f;
bool	animacion = false,
recorrido1 = true,
recorrido2 = false,
recorrido3 = false,
recorrido4 = false;

//Keyframes (Manipulaci�n y dibujo)
float	posX = 0.0f,
		posY = 0.0f,
		posZ = 0.0f,
		rotRodIzq = 0.0f,
		giroMonito = 0.0f;
float	incX = 0.0f,
		incY = 0.0f,
		incZ = 0.0f,
		rotRodIzqInc = 0.0f,
		giroMonitoInc = 0.0f;

#define MAX_FRAMES 9
int i_max_steps = 60;
int i_curr_steps = 0;
typedef struct _frame
{
	//Variables para GUARDAR Key Frames
	float posX;		//Variable para PosicionX
	float posY;		//Variable para PosicionY
	float posZ;		//Variable para PosicionZ
	float rotRodIzq;
	float giroMonito;

}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 0;			//introducir n�mero en caso de tener Key guardados
bool play = false;
int playIndex = 0;

void saveFrame(void)
{
	//printf("frameindex %d\n", FrameIndex);
	std::cout << "Frame Index = " << FrameIndex << std::endl;

	KeyFrame[FrameIndex].posX = posX;
	KeyFrame[FrameIndex].posY = posY;
	KeyFrame[FrameIndex].posZ = posZ;

	KeyFrame[FrameIndex].rotRodIzq = rotRodIzq;
	KeyFrame[FrameIndex].giroMonito = giroMonito;

	FrameIndex++;
}

void resetElements(void)
{
	posX = KeyFrame[0].posX;
	posY = KeyFrame[0].posY;
	posZ = KeyFrame[0].posZ;

	rotRodIzq = KeyFrame[0].rotRodIzq;
	giroMonito = KeyFrame[0].giroMonito;
}

void interpolation(void)
{
	incX = (KeyFrame[playIndex + 1].posX - KeyFrame[playIndex].posX) / i_max_steps;
	incY = (KeyFrame[playIndex + 1].posY - KeyFrame[playIndex].posY) / i_max_steps;
	incZ = (KeyFrame[playIndex + 1].posZ - KeyFrame[playIndex].posZ) / i_max_steps;

	rotRodIzqInc = (KeyFrame[playIndex + 1].rotRodIzq - KeyFrame[playIndex].rotRodIzq) / i_max_steps;
	giroMonitoInc = (KeyFrame[playIndex + 1].giroMonito - KeyFrame[playIndex].giroMonito) / i_max_steps;

}

unsigned int generateTextures(const char* filename, bool alfa, bool isPrimitive)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	
	if(isPrimitive)
		stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	else
		stbi_set_flip_vertically_on_load(false); // tell stb_image.h to flip loaded texture's on the y-axis.


	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
	if (data)
	{
		if (alfa)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		return textureID;
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
		return 100;
	}

	stbi_image_free(data);
}

void LoadTextures()
{
	t_smile = generateTextures("Assets/Texturas/awesomeface.png", 1, true);
	t_toalla = generateTextures("Assets/Texturas/toalla.tga", 0, true);
	t_unam = generateTextures("Assets/Texturas/escudo_unam.jpg", 0, true);
	t_ladrillos = generateTextures("Assets/Texturas/bricks.jpg", 0, true);
	//This must be the last
	t_white = generateTextures("Assets/Texturas/white.jpg", 0, false);
}

void animate(void) 
{
	if (play)
	{
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)	//end of total animation?
			{
				std::cout << "Animation ended" << std::endl;
				//printf("termina anim\n");
				playIndex = 0;
				play = false;
			}
			else //Next frame interpolations
			{
				i_curr_steps = 0; //Reset counter
				//Interpolation
				interpolation();
			}
		}
		else
		{
			//Draw animation
			posX += incX;
			posY += incY;
			posZ += incZ;

			rotRodIzq += rotRodIzqInc;
			giroMonito += giroMonitoInc;

			i_curr_steps++;
		}
	}

	//Veh�culo
	if (animacion)
	{
		movAuto_x += 3.0f;
	}
}

void getResolution() {
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	SCR_WIDTH = mode->width;
	SCR_HEIGHT = (mode->height) - 80;
}

void myData() {
	float vertices[] = {
		// positions          // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
		 0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	float verticesPiso[] = {
		// positions          // texture coords
		 10.5f,  10.5f, 0.0f,   4.0f, 4.0f, // top right
		 10.5f, -10.5f, 0.0f,   4.0f, 0.0f, // bottom right
		-10.5f, -10.5f, 0.0f,   0.0f, 0.0f, // bottom left
		-10.5f,  10.5f, 0.0f,   0.0f, 4.0f  // top left 
	};
	unsigned int indicesPiso[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	GLfloat verticesCubo[] = {
		//Position				//texture coords
		-0.5f, -0.5f, 0.5f,		0.0f, 0.0f,	//V0 - Frontal
		0.5f, -0.5f, 0.5f,		1.0f, 0.0f,	//V1
		0.5f, 0.5f, 0.5f,		1.0f, 1.0f,	//V5
		-0.5f, -0.5f, 0.5f,		0.0f, 0.0f,	//V0
		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f,	//V4
		0.5f, 0.5f, 0.5f,		1.0f, 1.0f,	//V5

		0.5f, -0.5f, -0.5f,		0.0f, 0.0f,	//V2 - Trasera
		-0.5f, -0.5f, -0.5f,	1.0f, 0.0f,	//V3
		-0.5f, 0.5f, -0.5f,		1.0f, 1.0f,	//V7
		0.5f, -0.5f, -0.5f,		0.0f, 0.0f,	//V2
		0.5f, 0.5f, -0.5f,		0.0f, 1.0f,	//V6
		-0.5f, 0.5f, -0.5f,		1.0f, 1.0f,	//V7

		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f,	//V4 - Izq
		-0.5f, 0.5f, -0.5f,		0.0f, 1.0f,	//V7
		-0.5f, -0.5f, -0.5f,	0.0f, 1.0f,	//V3
		-0.5f, -0.5f, -0.5f,	0.0f, 1.0f,	//V3
		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f,	//V4
		-0.5f, -0.5f, 0.5f,		0.0f, 1.0f,	//V0

		0.5f, 0.5f, 0.5f,		1.0f, 0.0f,	//V5 - Der
		0.5f, -0.5f, 0.5f,		1.0f, 0.0f,	//V1
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f,	//V2
		0.5f, 0.5f, 0.5f,		1.0f, 0.0f,	//V5
		0.5f, 0.5f, -0.5f,		1.0f, 0.0f,	//V6
		0.5f, -0.5f, -0.5f,		1.0f, 0.0f,	//V2

		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f,	//V4 - Sup
		0.5f, 0.5f, 0.5f,		0.0f, 1.0f,	//V5
		0.5f, 0.5f, -0.5f,		0.0f, 1.0f,	//V6
		-0.5f, 0.5f, 0.5f,		0.0f, 1.0f,	//V4
		-0.5f, 0.5f, -0.5f,		0.0f, 1.0f,	//V7
		0.5f, 0.5f, -0.5f,		0.0f, 1.0f,	//V6

		-0.5f, -0.5f, 0.5f,		1.0f, 1.0f,	//V0 - Inf
		-0.5f, -0.5f, -0.5f,	1.0f, 1.0f,	//V3
		0.5f, -0.5f, -0.5f,		1.0f, 1.0f,	//V2
		-0.5f, -0.5f, 0.5f,		1.0f, 1.0f,	//V0
		0.5f, -0.5f, -0.5f,		1.0f, 1.0f,	//V2
		0.5f, -0.5f, 0.5f,		1.0f, 1.0f,	//V1
	};

	glGenVertexArrays(3, VAO);
	glGenBuffers(3, VBO);
	glGenBuffers(3, EBO);

	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//Para Piso
	glBindVertexArray(VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesPiso), verticesPiso, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesPiso), indicesPiso, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	//PARA CUBO
	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCubo), verticesCubo, GL_STATIC_DRAW);

	/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

string LastScene;

enum ShaderIndex {
	cielo = 0,
	estatico = 1,
	animado = 2,
	custom = 3
};

int GetShaderIndexFromString(const std::string& shaderName) {
	if (shaderName == "skybox") return cielo;
	if (shaderName == "static") return estatico;
	if (shaderName == "animation") return animado;
	if (shaderName == "myshader") return custom;
	return -1; // o algún índice de shader por defecto
}

struct GameObjectData {
	std::string name;
	glm::vec3 position, rotation, scale;
	std::vector<std::string> objFiles;
	Shader* shader;
	Model* model = nullptr;
	ModelAnim* model_a = nullptr;
};

std::vector<GameObjectData> gameObjects;

void LoadScene(
	const std::string& filepath,
	std::vector<Shader*>& shaders,
	std::vector<std::string>& faces
) {
	YAML::Node scene = YAML::LoadFile(filepath);
	const YAML::Node& sceneNode = scene["Scene"];

	if (sceneNode["Skybox"]) {
		const YAML::Node& skyboxNode = sceneNode["Skybox"]["files"];
		for (const auto& fileNode : skyboxNode) {
			faces.push_back(fileNode.as<std::string>());
		}
	}

	const YAML::Node& objects = sceneNode["GameObjects"];
	for (const auto& node : objects) {
		GameObjectData obj;
		obj.name = node["name"].as<std::string>();

		auto pos = node["transform"]["position"];
		auto rot = node["transform"]["rotation"];
		auto scl = node["transform"]["scale"];
		obj.position = glm::vec3(pos[0].as<float>(), pos[1].as<float>(), pos[2].as<float>());
		obj.rotation = glm::vec3(rot[0].as<float>(), rot[1].as<float>(), rot[2].as<float>());
		obj.scale = glm::vec3(scl[0].as<float>(), scl[1].as<float>(), scl[2].as<float>());

		std::string shaderName = node["shader"].as<std::string>();
		int shaderIndex = GetShaderIndexFromString(shaderName);
		if (shaderIndex >= 0 && shaderIndex < shaders.size()) {
			obj.shader = shaders[shaderIndex]; // ya es puntero
		}
		else {
			obj.shader = shaders[0]; // fallback
		}

		for (const auto& file : node["model"]["obj_files"]) {
			std::cout << file.as<std::string>() << std::endl;
			obj.objFiles.push_back(file.as<std::string>());
		}
		
		if (obj.objFiles.empty()) {
			std::cerr << "No OBJ files found for object: " << obj.name << std::endl;
			continue; // Skip this object if no files are found
		}
		else {
            if (obj.shader == shaders[estatico]) {  
				obj.model = new Model(obj.objFiles[0]);
            } else if (obj.shader == shaders[animado]) {  
				obj.model_a = new ModelAnim(obj.objFiles[0]);
				obj.model_a->initShaders(obj.shader->ID);
            } else {  
				continue;
            }
		}
			

		// ModelAnim animacionPersonaje("Assets/objects/Personaje1/Arm.dae");
		// animacionPersonaje.initShaders(shaders[2].ID);

		gameObjects.push_back(obj);
	}
}

void deserialize(string filename) {

}

void serialize() {

}

void setAnimationShader(std::vector<Shader*>& shaders, const glm::mat4& projection, const glm::mat4& view) {
	shaders[animado]->use();
	shaders[animado]->setMat4("projection", projection);
	shaders[animado]->setMat4("view", view);
	shaders[animado]->setVec3("material.specular", glm::vec3(0.5f));
	shaders[animado]->setFloat("material.shininess", 32.0f);
	shaders[animado]->setVec3("light.ambient", ambientColor);
	shaders[animado]->setVec3("light.diffuse", diffuseColor);
	shaders[animado]->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
	shaders[animado]->setVec3("light.direction", lightDirection);
	shaders[animado]->setVec3("viewPos", camera.Position);
}


void setStaticShader(vector<Shader*>& shaders, const glm::mat4& projection, const glm::mat4& view) {
	
	shaders[1]->use();
	
	//Setup Advanced Lights
	shaders[1]->setVec3("viewPos", camera.Position);
	shaders[1]->setVec3("dirLight.direction", lightDirection);
	shaders[1]->setVec3("dirLight.ambient", ambientColor);
	shaders[1]->setVec3("dirLight.diffuse", diffuseColor);
	shaders[1]->setVec3("dirLight.specular", glm::vec3(0.6f, 0.6f, 0.6f));

	shaders[1]->setVec3("pointLight[0].position", lightPosition);
	shaders[1]->setVec3("pointLight[0].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
	shaders[1]->setVec3("pointLight[0].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
	shaders[1]->setVec3("pointLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
	shaders[1]->setFloat("pointLight[0].constant", 0.08f);
	shaders[1]->setFloat("pointLight[0].linear", 0.009f);
	shaders[1]->setFloat("pointLight[0].quadratic", 0.032f);

	shaders[1]->setVec3("pointLight[1].position", glm::vec3(-80.0, 0.0f, 0.0f));
	shaders[1]->setVec3("pointLight[1].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
	shaders[1]->setVec3("pointLight[1].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
	shaders[1]->setVec3("pointLight[1].specular", glm::vec3(0.0f, 0.0f, 0.0f));
	shaders[1]->setFloat("pointLight[1].constant", 1.0f);
	shaders[1]->setFloat("pointLight[1].linear", 0.009f);
	shaders[1]->setFloat("pointLight[1].quadratic", 0.032f);

	shaders[1]->setVec3("spotLight[0].position", glm::vec3(0.0f, 20.0f, 10.0f));
	shaders[1]->setVec3("spotLight[0].direction", glm::vec3(0.0f, -1.0f, 0.0f));
	shaders[1]->setVec3("spotLight[0].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
	shaders[1]->setVec3("spotLight[0].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
	shaders[1]->setVec3("spotLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
	shaders[1]->setFloat("spotLight[0].cutOff", glm::cos(glm::radians(10.0f)));
	shaders[1]->setFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(60.0f)));
	shaders[1]->setFloat("spotLight[0].constant", 1.0f);
	shaders[1]->setFloat("spotLight[0].linear", 0.0009f);
	shaders[1]->setFloat("spotLight[0].quadratic", 0.0005f);

	shaders[1]->setFloat("material_shininess", 32.0f);
}

int main() {
	// FMOD initialization
	FMOD_RESULT result;
	FMOD::System* system = nullptr;
	result = FMOD::System_Create(&system);
	system->init(512, FMOD_INIT_NORMAL, 0);

	FMOD::Sound* sound = nullptr;
	system->createSound("Assets/Sounds/Howling Abyss.mp3", FMOD_DEFAULT, 0, &sound);
	FMOD::Channel* channel = nullptr;
	result = system->playSound(sound, nullptr, false, &channel);

	// glfw: initialize and configure
	glfwInit();

	// usando opengl 4.6
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// glfw window creation
	monitors = glfwGetPrimaryMonitor();
	getResolution();

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto Final 2025-2", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetWindowPos(window, 0, 30);
	glfwMakeContextCurrent(window);
	// callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, my_input);
	glfwSetMouseButtonCallback(window,(GLFWmousebuttonfun) mouseInput_Callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	//Mis funciones
	//Datos a utilizar
	LoadTextures();
	myData();
	glEnable(GL_DEPTH_TEST);
	
	// Dear ImGui posterior al glEnable
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.IniFilename = "Assets/imgui.ini"; // para guardar el .ini en otro lado

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	// build and compile shaders
	// -------------------------
	Shader myShader("Assets/Shaders/shader_texture_color.vs", "Assets/Shaders/shader_texture_color.fs"); //To use with primitives
	Shader staticShader("Assets/Shaders/shader_Lights.vs", "Assets/Shaders/shader_Lights_mod.fs");	//To use with static models
	Shader skyboxShader("Assets/Shaders/skybox.vs", "Assets/Shaders/skybox.fs");	//To use with skybox
	Shader animShader("Assets/Shaders/anim.vs", "Assets/Shaders/anim.fs");	//To use with animated models 

	vector<Shader*> shaders = { &skyboxShader, &staticShader, &animShader, &myShader };

	// Get yaml file data
	vector<std::string> faces{
		//"Assets/skybox/right.jpg",
		//"Assets/skybox/left.jpg",
		//"Assets/skybox/top.jpg",
		//"Assets/skybox/bottom.jpg",
		//"Assets/skybox/front.jpg",
		//"Assets/skybox/back.jpg"
	};

	LastScene = "scene.yaml";

	LoadScene("Assets/" + LastScene, shaders, faces);
	
	Skybox skybox = Skybox(faces);

	// Shader configuration
	// --------------------
	shaders[cielo]->use();
	shaders[cielo]->setInt("skybox", 0);

	// load models
	// -----------
	// Model piso("Assets/objects/piso/piso.obj");
	// Model edificio("Assets/objects/edificio Q.obj");
	// Model carro("Assets/objects/lambo/carroceria.obj");
	// Model llanta("Assets/objects/lambo/Wheel.obj");
	// Model casaVieja("Assets/objects/casa/OldHouse.obj");
	// Model cubo("Assets/objects/cubo/cube02.obj");
	// Model casaDoll("Assets/objects/casa/DollHouse.obj");

	// Modelos Animados
	// ModelAnim animacionPersonaje("Assets/objects/Personaje1/Arm.dae");
	// animacionPersonaje.initShaders(shaders[2].ID);

	//Inicializaci�n de KeyFrames
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		KeyFrame[i].posX = 0;
		KeyFrame[i].posY = 0;
		KeyFrame[i].posZ = 0;
		KeyFrame[i].rotRodIzq = 0;
		KeyFrame[i].giroMonito = 0;
	}

	// create transformations and Projection
	glm::mat4 modelOp = glm::mat4(1.0f);		// initialize Matrix, Use this matrix for individual models
	glm::mat4 viewOp = glm::mat4(1.0f);		//Use this matrix for ALL models
	glm::mat4 projectionOp = glm::mat4(1.0f);	//This matrix is for Projection

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// ImGui::ShowDemoWindow();
		// brief tutorial window
		ImGui::Begin("Instrucciones de uso");
		// size and position
		ImGui::SetWindowSize(ImVec2(400, 200), ImGuiCond_Always);
		// font size
		ImGui::Text("Presiona 'Esc' para salir");
		ImGui::Text("Muevete con 'click derecho + WASD' en el modo editor");
		ImGui::Text("Muevete con 'WASD' en el modo juego");
		ImGui::Text("Presiona 'ctrl + S' para guardar la escena");
		ImGui::End();

		shaders[0]->setInt("skybox", 0);

		// per-frame time logic
		// --------------------
		lastFrame = SDL_GetTicks();

		// input
		// -----
		//my_input(window);
		animate();

		// render
		// ------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 tmp = glm::mat4(1.0f);
		// view/projection transformations
		//glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		projectionOp = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		viewOp = camera.GetViewMatrix();
		shaders[1]->setMat4("projection", projectionOp);
		shaders[1]->setMat4("view", viewOp);


		setAnimationShader(shaders, projectionOp, viewOp);
		setStaticShader(shaders, projectionOp, viewOp);

		//Setup shader for primitives
		myShader.use();
		// view/projection transformations
		//projectionOp = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 400.0f);
		viewOp = camera.GetViewMatrix();
		// pass them to the shaders
		//myShader.setMat4("model", modelOp);
		myShader.setMat4("view", viewOp);
		// note: currently we set the projection matrix each frame, but since the projection matrix rarely changes it's often best practice to set it outside the main loop only once.
		myShader.setMat4("projection", projectionOp);
		/**********/

		// -------------------------------------------------------------------------------------------------------------------------
		// Escenario Primitivas
		// -------------------------------------------------------------------------------------------------------------------------
		myShader.use();

		//Tener Piso como referencia
		glBindVertexArray(VAO[2]);
		//Colocar c�digo aqu�
		modelOp = glm::scale(glm::mat4(1.0f), glm::vec3(40.0f, 2.0f, 40.0f));
		modelOp = glm::translate(modelOp, glm::vec3(0.0f, -1.0f, 0.0f));
		modelOp = glm::rotate(modelOp, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		myShader.setMat4("model", modelOp);
		myShader.setVec3("aColor", 1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, t_ladrillos);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindVertexArray(VAO[0]);
		//Colocar c�digo aqu�
		modelOp = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 0.0f));
		modelOp = glm::scale(modelOp, glm::vec3(5.0f, 5.0f, 1.0f));
		myShader.setMat4("model", modelOp);
		myShader.setVec3("aColor", 1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, t_unam);
		//glDrawArrays(GL_TRIANGLES, 0, 36); //A lonely cube :(
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		// ------------------------------------------------------------------------------------------------------------------------
		// Termina Escenario Primitivas
		// -------------------------------------------------------------------------------------------------------------------------


		for (const GameObjectData& gameObject : gameObjects)
		{
			cout << gameObject.name << endl;

			gameObject.shader->use();
			if (gameObject.shader == shaders[estatico]) {
				setStaticShader(shaders, projectionOp, viewOp);
			}
			else if (gameObject.shader == shaders[animado]) {
				setAnimationShader(shaders, projectionOp, viewOp);
			}

			if (gameObject.model != nullptr) {

				// set the view and projection matrices
				gameObject.shader->setMat4("projection", projectionOp);
				gameObject.shader->setMat4("view", viewOp);

				modelOp = glm::translate(glm::mat4(1.0f), gameObject.position);
				modelOp = glm::rotate(modelOp, glm::radians(gameObject.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
				modelOp = glm::rotate(modelOp, glm::radians(gameObject.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
				modelOp = glm::rotate(modelOp, glm::radians(gameObject.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
				modelOp = glm::scale(modelOp, gameObject.scale);
				gameObject.shader->setMat4("model", modelOp);

				if (gameObject.model != nullptr) {

					gameObject.model->Draw(*gameObject.shader);
					cout << "- inicializado de manera correcta" << endl;
				}
				else {
					std::cerr << "Error: El modelo de " << gameObject.name << " es nulo." << std::endl;
				}

			}
			else if (gameObject.model_a != nullptr) {
				// set the view and projection matrices
				gameObject.shader->setMat4("projection", projectionOp);
				gameObject.shader->setMat4("view", viewOp);

				modelOp = glm::translate(glm::mat4(1.0f), gameObject.position);
				modelOp = glm::rotate(modelOp, glm::radians(gameObject.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
				modelOp = glm::rotate(modelOp, glm::radians(gameObject.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
				modelOp = glm::rotate(modelOp, glm::radians(gameObject.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
				modelOp = glm::scale(modelOp, gameObject.scale);
				gameObject.shader->setMat4("model", modelOp);

				gameObject.model_a->Draw(*gameObject.shader);
			}
		}

		shaders[0]->use();
		skybox.Draw(*shaders[0], viewOp, projectionOp, camera);

		// Limitar el framerate a 60
		deltaTime = SDL_GetTicks() - lastFrame; // time for full 1 loop
		if (deltaTime < LOOP_TIME)
		{
			SDL_Delay((int)(LOOP_TIME - deltaTime));
		}


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glDeleteVertexArrays(2, VAO);
	glDeleteBuffers(2, VBO);
	//skybox.Terminate();
	
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void my_input(GLFWwindow* window, int key, int scancode, int action, int mode) 
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		movementMode = true;
	}
	
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		movementMode = false;
	}

	if (movementMode) {
		if (glfwGetKey(window, GLFW_KEY_W))
			camera.ProcessKeyboard(FORWARD, (float)deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S))
			camera.ProcessKeyboard(BACKWARD, (float)deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A))
			camera.ProcessKeyboard(LEFT, (float)deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D))
			camera.ProcessKeyboard(RIGHT, (float)deltaTime);
		if (glfwGetKey(window, GLFW_KEY_E))
			camera.ProcessKeyboard(UP, (float)deltaTime);
		if (glfwGetKey(window, GLFW_KEY_Q))
			camera.ProcessKeyboard(DOWN, (float)deltaTime);
	}
	else {
		if (glfwGetKey(window, GLFW_KEY_W))
			gameObjects[1].position.z -= 0.01f;
		if (glfwGetKey(window, GLFW_KEY_S))
			gameObjects[1].position.z += 0.01f;
		if (glfwGetKey(window, GLFW_KEY_A))
			gameObjects[1].position.x -= 0.01f;
		if (glfwGetKey(window, GLFW_KEY_D))
			gameObjects[1].position.x += 0.01f;
	}
	

	//To Configure Model
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
		posZ++;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		posZ--;
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		posX--;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		posX++;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		rotRodIzq--;
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
		rotRodIzq++;
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
		giroMonito--;
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		giroMonito++;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		lightPosition.x++;
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		lightPosition.x--;

	//Car animation
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		animacion ^= true;

	//To play KeyFrame animation 
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		if (play == false && (FrameIndex > 1))
		{
			std::cout << "Play animation" << std::endl;
			resetElements();
			//First Interpolation				
			interpolation();

			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
			std::cout << "Not enough Key Frames" << std::endl;
		}
	}

	//To Save a KeyFrame
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		if (FrameIndex < MAX_FRAMES)
		{
			saveFrame();
		}
	}

}

void mouseInput_Callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		movementMode = true;
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		movementMode = false;
	}
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos) 
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	if(movementMode)
		camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}