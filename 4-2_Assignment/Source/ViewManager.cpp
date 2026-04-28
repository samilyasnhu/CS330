///////////////////////////////////////////////////////////////////////////////
// viewmanager.cpp
// ============
// manage the viewing of 3D objects within the viewport
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// declaration of the global variables and defines
namespace
{
	const int WINDOW_WIDTH = 1000;
	const int WINDOW_HEIGHT = 800;
	const char* g_ViewName = "view";
	const char* g_ProjectionName = "projection";

	// camera object used for viewing and interacting with the 3D scene
	Camera* g_pCamera = nullptr;

	// variables used for mouse-movement processing
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// time between current frame and last frame
	float gDeltaTime = 0.0f;
	float gLastFrame = 0.0f;

	// movement speed multiplier controlled by mouse scroll
	float gCameraSpeedMultiplier = 1.0f;

	// if orthographic projection is on, this value will be true
	bool bOrthographicProjection = false;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();

	// default camera view parameters
	g_pCamera->Position = glm::vec3(0.5f, 5.5f, 10.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 80;
}

/***********************************************************
 *  ~ViewManager()
 *
 *  The destructor for the class
 ***********************************************************/
ViewManager::~ViewManager()
{
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

	// callback for mouse movement to orbit / look around the scene
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	// callback for mouse wheel to adjust camera movement speed
	glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Callback);

	// tell GLFW to capture all mouse events if desired
	// glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// enable blending for supporting transparent rendering
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_pWindow = window;

	return(window);
}

/***********************************************************
 *  Mouse_Position_Callback()
 *
 *  Called automatically by GLFW whenever the mouse moves.
 *  This rotates the camera so the user can look up/down
 *  and left/right around the 3D world.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	if (gFirstMouse)
	{
		gLastX = static_cast<float>(xMousePos);
		gLastY = static_cast<float>(yMousePos);
		gFirstMouse = false;
	}

	float xOffset = static_cast<float>(xMousePos) - gLastX;
	float yOffset = gLastY - static_cast<float>(yMousePos);

	gLastX = static_cast<float>(xMousePos);
	gLastY = static_cast<float>(yMousePos);

	if (NULL != g_pCamera)
	{
		g_pCamera->ProcessMouseMovement(xOffset, yOffset);
	}
}

/***********************************************************
 *  Mouse_Scroll_Callback()
 *
 *  Called automatically by GLFW whenever the mouse wheel
 *  is scrolled. This adjusts how fast the camera moves.
 ***********************************************************/
void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset)
{
	// increase or decrease movement speed in small steps
	gCameraSpeedMultiplier += static_cast<float>(yOffset) * 0.1f;

	// clamp the value so movement never becomes too slow or too fast
	if (gCameraSpeedMultiplier < 0.2f)
	{
		gCameraSpeedMultiplier = 0.2f;
	}
	if (gCameraSpeedMultiplier > 4.0f)
	{
		gCameraSpeedMultiplier = 4.0f;
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
	// close the window if escape has been pressed
	if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(m_pWindow, true);
	}

	if (NULL == g_pCamera)
	{
		return;
	}

	// apply the scroll-based movement multiplier
	float adjustedDeltaTime = gDeltaTime * gCameraSpeedMultiplier;

	// move forward and backward
	if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(FORWARD, adjustedDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(BACKWARD, adjustedDeltaTime);
	}

	// pan left and right
	if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(LEFT, adjustedDeltaTime);
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
	{
		g_pCamera->ProcessKeyboard(RIGHT, adjustedDeltaTime);
	}

	// move upward and downward
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		g_pCamera->Position += g_pCamera->Up * adjustedDeltaTime * 5.0f;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		g_pCamera->Position -= g_pCamera->Up * adjustedDeltaTime * 5.0f;
	}
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  Prepares the camera view and projection for rendering.
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	ProcessKeyboardEvents();

	view = g_pCamera->GetViewMatrix();

	projection = glm::perspective(
		glm::radians(g_pCamera->Zoom),
		(GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT,
		0.1f,
		100.0f);

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ViewName, view);
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}