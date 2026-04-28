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
	// Variables for window width and height
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

	// camera speed multiplier controlled by mouse scroll
	float gCameraSpeedMultiplier = 1.0f;

	// current projection mode
	bool bOrthographicProjection = false;

	// used to prevent repeated toggling while a key is held down
	bool gProjectionTogglePressed = false;
}

/***********************************************************
 *  ViewManager()
 *
 *  The constructor for the class
 ***********************************************************/
ViewManager::ViewManager(ShaderManager* pShaderManager)
{
	// initialize the member variables
	m_pShaderManager = pShaderManager;
	m_pWindow = NULL;
	g_pCamera = new Camera();

	// default camera view parameters for perspective mode
	g_pCamera->Position = glm::vec3(0.0f, 5.0f, 28.0f);
	g_pCamera->Front = glm::vec3(0.0f, -0.08f, -1.0f);
	g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
	g_pCamera->Zoom = 32.0f;
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

	// mouse movement changes camera orientation
	glfwSetCursorPosCallback(window, &ViewManager::Mouse_Position_Callback);

	// mouse wheel changes the camera movement speed
	glfwSetScrollCallback(window, &ViewManager::Mouse_Scroll_Callback);

	// uncomment if you want the cursor locked to the window
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
 *  This method is automatically called from GLFW whenever
 *  the mouse is moved within the active GLFW display window.
 *  It updates the camera orientation so the user can look
 *  up/down and left/right around the scene.
 ***********************************************************/
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xMousePos, double yMousePos)
{
	// store the first mouse position so future offsets are correct
	if (gFirstMouse)
	{
		gLastX = static_cast<float>(xMousePos);
		gLastY = static_cast<float>(yMousePos);
		gFirstMouse = false;
	}

	// calculate change in mouse position
	float xOffset = static_cast<float>(xMousePos) - gLastX;
	float yOffset = gLastY - static_cast<float>(yMousePos);

	// save current values for next frame
	gLastX = static_cast<float>(xMousePos);
	gLastY = static_cast<float>(yMousePos);

	// update camera yaw and pitch
	if (NULL != g_pCamera)
	{
		g_pCamera->ProcessMouseMovement(xOffset, yOffset);
	}
}

/***********************************************************
 *  Mouse_Scroll_Callback()
 *
 *  This method is automatically called from GLFW whenever
 *  the mouse wheel is scrolled. It changes how quickly the
 *  camera moves through the scene.
 ***********************************************************/
void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xOffset, double yOffset)
{
	gCameraSpeedMultiplier += static_cast<float>(yOffset) * 0.1f;

	// clamp speed multiplier to a safe range
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

	// camera travel speed uses frame time and scroll multiplier
	float adjustedDeltaTime = gDeltaTime * gCameraSpeedMultiplier;

	// perspective mode movement:
	// W/S = forward/backward, A/D = left/right
	if (!bOrthographicProjection)
	{
		if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(FORWARD, adjustedDeltaTime);
		}
		if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(BACKWARD, adjustedDeltaTime);
		}
		if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(LEFT, adjustedDeltaTime);
		}
		if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
		{
			g_pCamera->ProcessKeyboard(RIGHT, adjustedDeltaTime);
		}
	}

	// Q/E vertical movement works in both views
	if (glfwGetKey(m_pWindow, GLFW_KEY_Q) == GLFW_PRESS)
	{
		g_pCamera->Position += g_pCamera->Up * adjustedDeltaTime * 5.0f;
	}
	if (glfwGetKey(m_pWindow, GLFW_KEY_E) == GLFW_PRESS)
	{
		g_pCamera->Position -= g_pCamera->Up * adjustedDeltaTime * 5.0f;
	}

	// switch to perspective view with P
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_PRESS && !gProjectionTogglePressed)
	{
		bOrthographicProjection = false;
		gProjectionTogglePressed = true;

		// restore perspective camera settings
		g_pCamera->Position = glm::vec3(0.0f, 4.5f, 22.0f);
		g_pCamera->Front = glm::vec3(0.0f, -0.10f, -1.0f);
		g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
		g_pCamera->Zoom = 45.0f;
	}

	// switch to orthographic view with O
	if (glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_PRESS && !gProjectionTogglePressed)
	{
		bOrthographicProjection = true;
		gProjectionTogglePressed = true;

		// top-down orthographic camera so the bottom plane is not visible
		g_pCamera->Position = glm::vec3(0.0f, 12.0f, 0.0f);
		g_pCamera->Front = glm::vec3(0.0f, -1.0f, 0.0f);
		g_pCamera->Up = glm::vec3(0.0f, 0.0f, -1.0f);
		g_pCamera->Zoom = 45.0f;
	}

	// reset one-time toggle when keys are released
	if (glfwGetKey(m_pWindow, GLFW_KEY_P) == GLFW_RELEASE &&
		glfwGetKey(m_pWindow, GLFW_KEY_O) == GLFW_RELEASE)
	{
		gProjectionTogglePressed = false;
	}
}

/***********************************************************
 *  PrepareSceneView()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes and setting the camera and projection state.
 ***********************************************************/
void ViewManager::PrepareSceneView()
{
	glm::mat4 view;
	glm::mat4 projection;

	// per-frame timing
	float currentFrame = glfwGetTime();
	gDeltaTime = currentFrame - gLastFrame;
	gLastFrame = currentFrame;

	// process any keyboard events that may be waiting
	ProcessKeyboardEvents();

	// get the current view matrix from the camera
	view = g_pCamera->GetViewMatrix();

	// select the active projection mode
	if (bOrthographicProjection)
	{
		// 2D-style orthographic view
		projection = glm::ortho(-16.0f, 16.0f, -10.0f, 10.0f, 0.1f, 100.0f);
	}
	else
	{
		// standard 3D perspective view
		projection = glm::perspective(
			glm::radians(g_pCamera->Zoom),
			(GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT,
			0.1f,
			100.0f);
	}

	// if the shader manager object is valid
	if (NULL != m_pShaderManager)
	{
		// pass camera matrices and camera position to the shader
		m_pShaderManager->setMat4Value(g_ViewName, view);
		m_pShaderManager->setMat4Value(g_ProjectionName, projection);
		m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
	}
}