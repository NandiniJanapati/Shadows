#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>

#include "Program.h"
#include "Shape.h"



#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define NUM_LIGHTS 1
#define SHADOW_WIDTH 1024
#define SHADOW_HEIGHT 1024

GLFWwindow *window;

std::vector<Program> prog;

GLuint depthbuffer = 0;
GLuint shadowMap = 0;
bool isShadowMap = false;

std::vector<Shape> mainObjs;
Shape ground;

glm::vec3 eye(0.0f, 5.0f, 5.0f);

struct lightStruct {
	glm::vec3 position;
	glm::vec3 color;
} lights[NUM_LIGHTS];


void Display()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	// To make sure the code runs when window is minimized
	if (width == 0 || height == 0)
		width = WINDOW_WIDTH, height = WINDOW_HEIGHT;


	glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), float(width) / float(height), 0.1f, 100.0f);
	glm::mat4 viewMatrix = glm::lookAt(eye, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));


	if (!isShadowMap)
	{
		//******************************************************//
		//********* Planar Shadow Implementation ***************//
		//******************************************************//

		glViewport(0, 0, width, height);
		prog[0].Bind();

		prog[0].SendUniformData(viewMatrix, "view");
		prog[0].SendUniformData(projectionMatrix, "projection");

		//---nandini
		prog[0].SendUniformData(lights[0].position, "lights[0].position");
		prog[0].SendUniformData(lights[0].color, "lights[0].color");
		//--nandini

		for (int i = 0; i < mainObjs.size(); i++)
		{
			prog[0].SendUniformData(mainObjs[i].GetModelMatrix(), "model");
			//---nandini
			prog[0].SendUniformData(mainObjs[i].ka, "ka");
			prog[0].SendUniformData(mainObjs[i].kd, "kd");
			prog[0].SendUniformData(mainObjs[i].ks, "ks");
			prog[0].SendUniformData(mainObjs[i].s, "s");
			//--nandini
			mainObjs[i].Draw(prog[0]);
		}

		

		/// Commented out so you can see the red Bunny and Teapot. 
		/// Otherwise, the whole screen will be rendered red because of the ground.
		/// Uncomment once you start implementing the approach.

		prog[0].SendUniformData(ground.GetModelMatrix(), "model");
		//---nandini
		prog[0].SendUniformData(ground.ka, "ka");
		prog[0].SendUniformData(ground.kd, "kd");
		prog[0].SendUniformData(ground.ks, "ks");
		prog[0].SendUniformData(ground.s, "s");
		//--nandini
		ground.Draw(prog[0]);

		prog[0].Unbind();



		//---nandini
		// draw the planar shadow

		prog[1].Bind();

		prog[1].SendUniformData(viewMatrix, "view");
		prog[1].SendUniformData(projectionMatrix, "projection");
		prog[1].SendUniformData(lights[0].position, "lightpos");
		glm::vec3 gndpoint = { 0.0, 0.0, 0.0 };
		glm::vec3 gndnormal = { 0.0, 1.0, 0.0 };
		prog[1].SendUniformData(gndpoint, "gndPoint");
		prog[1].SendUniformData(gndnormal, "gndNormal");
		prog[1].SendUniformData(ground.ka, "ka");
		
		for (int i = 0; i < mainObjs.size(); i++) {
			prog[1].SendUniformData(mainObjs[i].GetModelMatrix(), "model");
			mainObjs[i].Draw(prog[1]);
		}
		
		prog[1].Unbind();

		//--nandini
	}
	else
	{
		//******************************************************//
		//********* Shadow Map Implementation ******************//
		//******************************************************//

		// 1. Render to depth map
		glBindFramebuffer(GL_FRAMEBUFFER, depthbuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);


		prog[2].Bind();

		//glm::vec3 camTranslation = lights[0].position - eye;//there is no physical camera to move/orient. just make new projection and view matrices
		glm::mat4 lightProjection = glm::perspective(glm::radians(60.0f), float(SHADOW_WIDTH) / float(SHADOW_HEIGHT), 1.0f, 15.0f);
			//im assuming that SHADOW_WIDTH and SHADOW_HEIGHT is the size of the shadow map/ size of the texture I'm building
		glm::mat4 lightView = glm::lookAt(lights[0].position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		//prog[2].SendUniformData(viewMatrix, "view");
		prog[2].SendUniformData(lightView, "view");
		//prog[2].SendUniformData(projectionMatrix, "projection");
		prog[2].SendUniformData(lightProjection, "projection");

		for (int i = 0; i < mainObjs.size(); i++)
		{
			prog[2].SendUniformData(mainObjs[i].GetModelMatrix(), "model");
			mainObjs[i].Draw(prog[2]);
		}

		prog[2].SendUniformData(ground.GetModelMatrix(), "model");
		ground.Draw(prog[2]);

		prog[2].Unbind();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 2. Render scene as normal with shadow mapping (using depth map)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, width, height);


		prog[3].Bind();



		/// Passing the shadowMap to the shader
		int unit = 0;
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glUniform1i(glGetUniformLocation(prog[3].GetID(), "shadowMap"), unit);



		prog[3].SendUniformData(viewMatrix, "view");
		prog[3].SendUniformData(projectionMatrix, "projection");

		prog[3].SendUniformData(lightView, "lightView");
		prog[3].SendUniformData(lightProjection, "lightProjection");

		prog[3].SendUniformData(lights[0].position, "lights[0].position");
		prog[3].SendUniformData(lights[0].color, "lights[0].color");


		for (int i = 0; i < mainObjs.size(); ++i)
		{
			prog[3].SendUniformData(mainObjs[i].GetModelMatrix(), "model");
			prog[3].SendUniformData(mainObjs[i].ka, "ka");
			prog[3].SendUniformData(mainObjs[i].kd, "kd");
			prog[3].SendUniformData(mainObjs[i].ks, "ks");
			prog[3].SendUniformData(mainObjs[i].s, "s");
			mainObjs[i].Draw(prog[3]);
		}

		/// Commented out so you can see the red Bunny and Teapot. 
		/// Otherwise, the whole screen will be rendered red because of the ground.
		/// Uncomment once you start implementing the approach.

		prog[3].SendUniformData(ground.GetModelMatrix(), "model");
		prog[3].SendUniformData(ground.ka, "ka");
		prog[3].SendUniformData(ground.kd, "kd");
		prog[3].SendUniformData(ground.ks, "ks");
		prog[3].SendUniformData(ground.s, "s");
		ground.Draw(prog[3]);

		prog[3].Unbind();


		/// 3. Debugging tool to make sure the depth map is obtained properly.
		/// Will be shown at the bottom left corner of the display window.
		/// DO NOT TOUCH THIS PART!!
		glDisable(GL_DEPTH_TEST);
		glUseProgram(0);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, 1, 0, 1, 1, 3);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glColor4f(1, 1, 1, 1);
		glTranslated(0, 0, -2.0);
		glActiveTextureARB(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glEnable(GL_TEXTURE_2D);

		glBegin(GL_QUADS);
		float sy = 0.25;
		float sx = sy / (float(SHADOW_WIDTH) / float(SHADOW_HEIGHT));
		glTexCoord2d(0, 0); glVertex3f(0, 0, 0);
		glTexCoord2d(1, 0); glVertex3f(sx, 0, 0);
		glTexCoord2d(1, 1); glVertex3f(sx, sy, 0);
		glTexCoord2d(0, 1); glVertex3f(0, sy, 0);
		glEnd();

		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
	}
	
	glFlush();
}

// Keyboard character callback function
void CharacterCallback(GLFWwindow* lWindow, unsigned int key)
{
	switch (key) {

	case 's':
		isShadowMap = !isShadowMap;
		break;
	case 'x':
		lights[0].position.x += 0.05;
		break;
	case 'y':
		lights[0].position.y += 0.05;
		break;
	case 'z':
		lights[0].position.z += 0.05;
		break;
	case 'X':
		lights[0].position.x -= 0.05;
		break;
	case 'Y':						
		lights[0].position.y -= 0.05;
		break;
	case 'Z':						
		lights[0].position.z -= 0.05;
		break;
	case 'q':
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		break;
	default:
		break;
	}
}

void Init()
{
	glfwInit();
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_FALSE);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Assignment4 - <Your Name>", NULL, NULL);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glfwSetCharCallback(window, CharacterCallback);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	
	
	/// Loading the objects
	mainObjs.push_back(Shape());
	mainObjs.push_back(Shape());

	mainObjs[0].LoadModel("../obj/teapot.obj");
	mainObjs[0].Init();
	mainObjs[0].SetModel(glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f)));
	//---nandini
	mainObjs[0].ka = glm::vec3(0.2, 0.2, 0.2);
	mainObjs[0].kd = glm::vec3(0.8, 0.7, 0.7);
	mainObjs[0].ks = glm::vec3(1.0, 1.0, 1.0);
	mainObjs[0].s = 10.0;
	//---nandini

	mainObjs[1].LoadModel("../obj/bunny.obj");
	mainObjs[1].Init();
	mainObjs[1].SetModel(glm::translate(glm::mat4(1.0f), glm::vec3(-0.2f, 0.0f, 1.3f)));
	//---nandini
	mainObjs[1].ka = glm::vec3(0.2, 0.2, 0.2);
	mainObjs[1].kd = glm::vec3(0.8, 0.7, 0.7);
	mainObjs[1].ks = glm::vec3(1.0, 1.0, 1.0);
	mainObjs[1].s = 10.0;
	//---nandini
	
	ground.LoadModel("../obj/square.obj");
	ground.Init();
	ground.SetModel(glm::scale(glm::mat4(1.0f), glm::vec3(20.0f)) * 
					 glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
	//---nandini
	ground.ka = glm::vec3(0.2, 0.2, 0.2);
	ground.kd = glm::vec3(0.8, 0.1, 0.1);
	ground.ks = glm::vec3(1.0, 1.0, 1.0);
	ground.s = 100.0;
	//---nandini

	lights[0].color = { 0.5, 0.5, 0.5 };
	lights[0].position = { 0.0, 5.0, 1.0 };


	/// Setting up the shader programs
	prog.push_back(Program());
	prog[0].SetShadersFileName("../shaders/planar_render_vert.glsl", "../shaders/planar_render_frag.glsl");
	prog[0].Init();

	prog.push_back(Program());
	prog[1].SetShadersFileName("../shaders/planar_shadow_vert.glsl", "../shaders/planar_shadow_frag.glsl");
	prog[1].Init();

	prog.push_back(Program());
	prog[2].SetShadersFileName("../shaders/map_pass1_vert.glsl", "../shaders/map_pass1_frag.glsl");
	prog[2].Init();

	prog.push_back(Program());
	prog[3].SetShadersFileName("../shaders/map_pass2_vert.glsl", "../shaders/map_pass2_frag.glsl");
	prog[3].Init();

	// Initialize frame buffers	
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &depthbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, depthbuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
}


int main()
{	
	Init();
	while ( glfwWindowShouldClose(window) == 0) 
	{
		Display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}