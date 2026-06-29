///////////////////////////////////////////////////////////////////////////////

// viewmanager.h

// ============

// manage the viewing of 3D objects within the viewport

//

//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science

//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023

//  Contributor: Jesse Martin, SNHU Student, Jun 2026

//  Module 4-3: desk-scene camera controls + O/P perspective/orthographic toggle
//  Module 7-1: Final Project — wider framing for the full flat-lay desk layout

///////////////////////////////////////////////////////////////////////////////



#include "ViewManager.h"



// GLM Math Header inclusions

#include <glm/glm.hpp>

#include <glm/gtx/transform.hpp>

#include <glm/gtc/type_ptr.hpp>    



// declaration of the global variables and defines

namespace

{

	// Variables for window width and height

	const int WINDOW_WIDTH = 1000;

	const int WINDOW_HEIGHT = 800;

	const char* g_ViewName = "view";

	const char* g_ProjectionName = "projection";



	// camera object used for viewing and interacting with

	// the 3D scene

	Camera* g_pCamera = nullptr;



	// these variables are used for mouse movement processing

	float gLastX = WINDOW_WIDTH / 2.0f;

	float gLastY = WINDOW_HEIGHT / 2.0f;

	bool gFirstMouse = true;



	// time between current frame and last frame

	float gDeltaTime = 0.0f; 

	float gLastFrame = 0.0f;



	// false = perspective (3D), true = orthographic (2D) — toggled with P / O keys

	bool bOrthographicProjection = false;



	// my default perspective framing from 3-3 — P key snaps back to this view

	// Eye-level front view — straight-on like Module One Image 5.
	const glm::vec3 kPerspectivePosition = glm::vec3(0.0f, 1.35f, 5.6f);

	const glm::vec3 kPerspectiveFront = glm::vec3(0.0f, -0.04f, -0.999f);

	const glm::vec3 kPerspectiveUp = glm::vec3(0.0f, 1.0f, 0.0f);

	const float kPerspectiveZoom = 38.0f;



	const glm::vec3 kOrthographicPosition = glm::vec3(0.0f, 1.35f, 5.6f);

	const glm::vec3 kOrthographicFront = glm::vec3(0.0f, -0.04f, -0.999f);

	const glm::vec3 kOrthographicUp = glm::vec3(0.0f, 1.0f, 0.0f);

}



/***********************************************************

 *  ViewManager()

 *

 *  The constructor for the class

 ***********************************************************/

ViewManager::ViewManager(

	ShaderManager *pShaderManager)

{

	// initialize the member variables

	m_pShaderManager = pShaderManager;

	m_pWindow = NULL;

	g_pCamera = new Camera();

	// I nudged the camera closer and lowered FOV so the mug fills the frame and the

	// handle reads in a 3/4 view — closer to how I see it on my desk, not straight top-down.

	g_pCamera->Position = kPerspectivePosition;

	g_pCamera->Front = kPerspectiveFront;

	g_pCamera->Up = kPerspectiveUp;

	g_pCamera->Zoom = kPerspectiveZoom;

}



/***********************************************************

 *  ~ViewManager()

 *

 *  The destructor for the class

 ***********************************************************/

ViewManager::~ViewManager()

{

	// free up allocated memory

	m_pShaderManager = NULL;

	m_pWindow = NULL;

	if (NULL != g_pCamera)

	{

		delete g_pCamera;

		g_pCamera = NULL;

	}

}



/***********************************************************

 *  CreateDisplayWindow()

 *

 *  This method is used to create the main display window.

 ***********************************************************/

GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)

{

	GLFWwindow* window = nullptr;



	// try to create the displayed OpenGL window

	window = glfwCreateWindow(

		WINDOW_WIDTH,

		WINDOW_HEIGHT,

		windowTitle,

		NULL, NULL);

	if (window == NULL)

	{

		std::cout << "Failed to create GLFW window" << std::endl;

		glfwTerminate();

		return NULL;

	}

	glfwMakeContextCurrent(window);



	// mouse move rotates the view; scroll adjusts WASD/QE travel speed (same as 4-2)

	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Callback);



	// cursor stays visible — easier to click away from the window while testing

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



	// enable blending for supporting tranparent rendering

	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



	m_pWindow = window;



	return(window);

}



/***********************************************************

 *  Mouse_Position_Callback()

 *

 *  This method is automatically called from GLFW whenever

 *  the mouse is moved within the active GLFW display window.

 ***********************************************************/

void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)

{

	if (NULL == g_pCamera)

	{

		return;

	}



	// when the first mouse move event is received, this needs to be recorded so that

	// all subsequent mouse moves can correctly calculate the X position offset and Y

	// position offset for proper operation

	if (gFirstMouse)

	{

		gLastX = (float)xMousePos;

		gLastY = (float)yMousePos;

		gFirstMouse = false;

	}



	// calculate the X offset and Y offset values for moving the 3D camera accordingly

	float xOffset = (float)xMousePos - gLastX;

	float yOffset = gLastY - (float)yMousePos; // reversed since y-coordinates go from bottom to top



	// set the current positions into the last position variables

	gLastX = (float)xMousePos;

	gLastY = (float)yMousePos;



	// move the 3D camera according to the calculated offsets

	g_pCamera->ProcessMouseMovement(xOffset, yOffset);

}



/***********************************************************

 *  Mouse_Scroll_Callback()

 *

 *  Called when the mouse scroll wheel moves. Adjusts camera

 *  travel speed so the user can fine-tune WASD/QE motion.

 ***********************************************************/

void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset)

{

	if (NULL != g_pCamera)

	{

		g_pCamera->ProcessMouseScroll((float)yOffset);

	}

}



/***********************************************************

 *  ProcessKeyboardEvents()

 *

 *  This method is called to process any keyboard events

 *  that may be waiting in the event queue.

 ***********************************************************/

void ViewManager::ProcessKeyboardEvents()

{

	// edge-trigger flags so O/P fire once per tap, not every frame while held

	static bool oKeyWasPressed = false;

	static bool pKeyWasPressed = false;



	// close the window if the escape key has been pressed

	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)

	{

		glfwSetWindowShouldClose(m_pWindow, true);

	}



	// if the camera object is null, then exit this method

	if (NULL == g_pCamera)

	{

		return;

	}



	// process camera zooming in and out

	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)

	{

		g_pCamera->ProcessKeyboard(FORWARD, gDeltaTime);

	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)

	{

		g_pCamera->ProcessKeyboard(BACKWARD, gDeltaTime);

	}



	// process camera panning left and right

	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)

	{

		g_pCamera->ProcessKeyboard(LEFT, gDeltaTime);

	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)

	{

		g_pCamera->ProcessKeyboard(RIGHT, gDeltaTime);

	}



	// Q/E move the camera up and down (world-space vertical pan)

	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)

	{

		g_pCamera->ProcessKeyboard(UP, gDeltaTime);

	}

	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)

	{

		g_pCamera->ProcessKeyboard(DOWN, gDeltaTime);

	}



	// O — orthographic front view of the mug (rubric: bottom plane should stay out of frame)

	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS)

	{

		if (!oKeyWasPressed)

		{

			bOrthographicProjection = true;

			g_pCamera->Position = kOrthographicPosition;

			g_pCamera->Front = kOrthographicFront;

			g_pCamera->Up = kOrthographicUp;

			oKeyWasPressed = true;

		}

	}

	else

	{

		oKeyWasPressed = false;

	}



	// P — back to the 3/4 perspective desk view I set up in 3-3

	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS)

	{

		if (!pKeyWasPressed)

		{

			bOrthographicProjection = false;

			g_pCamera->Position = kPerspectivePosition;

			g_pCamera->Front = kPerspectiveFront;

			g_pCamera->Up = kPerspectiveUp;

			g_pCamera->Zoom = kPerspectiveZoom;

			pKeyWasPressed = true;

		}

	}

	else

	{

		pKeyWasPressed = false;

	}

}



/***********************************************************

 *  PrepareSceneView()

 *

 *  This method is used for preparing the 3D scene by loading

 *  the shapes, textures in memory to support the 3D scene 

 *  rendering

 ***********************************************************/

void ViewManager::PrepareSceneView()

{

	glm::mat4 view;

	glm::mat4 projection;



	// per-frame timing

	float currentFrame = static_cast<float>(glfwGetTime());

	gDeltaTime = currentFrame - gLastFrame;

	gLastFrame = currentFrame;



	// process any keyboard events that may be waiting in the 

	// event queue

	ProcessKeyboardEvents();



	// keep the full window as the draw target (course hint: glViewport)

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);



	// get the current view matrix from the camera

	view = g_pCamera->GetViewMatrix();



	// pick perspective or orthographic projection — same pattern as 1-2_OpenGLSample

	if (bOrthographicProjection == false)

	{

		projection = glm::perspective(

			glm::radians(g_pCamera->Zoom),

			(GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT,

			0.1f,

			100.0f);

	}

	else

	{

		// tighter vertical bounds than the sample so the desk floor stays below the mug

		const float orthoHalfWidth = 3.2f;

		const float orthoHalfHeight = 1.6f;

		double scale = 0.0;



		if (WINDOW_WIDTH > WINDOW_HEIGHT)

		{

			scale = (double)WINDOW_HEIGHT / (double)WINDOW_WIDTH;

			projection = glm::ortho(

				-orthoHalfWidth,

				orthoHalfWidth,

				-orthoHalfHeight * (float)scale,

				orthoHalfHeight * (float)scale,

				0.1f,

				100.0f);

		}

		else if (WINDOW_WIDTH < WINDOW_HEIGHT)

		{

			scale = (double)WINDOW_WIDTH / (double)WINDOW_HEIGHT;

			projection = glm::ortho(

				-orthoHalfWidth * (float)scale,

				orthoHalfWidth * (float)scale,

				-orthoHalfHeight,

				orthoHalfHeight,

				0.1f,

				100.0f);

		}

		else

		{

			projection = glm::ortho(

				-orthoHalfWidth,

				orthoHalfWidth,

				-orthoHalfHeight,

				orthoHalfHeight,

				0.1f,

				100.0f);

		}

	}



	// if the shader manager object is valid

	if (NULL != m_pShaderManager)

	{

		// set the view matrix into the shader for proper rendering

		m_pShaderManager->setMat4Value(g_ViewName, view);

		// set the view matrix into the shader for proper rendering

		m_pShaderManager->setMat4Value(g_ProjectionName, projection);

		// set the view position of the camera into the shader for proper rendering

		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);

	}

}


