///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	// matrix math for calculating the final model matrix
	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
		// pass the model matrix into the shader
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		// pass the color values into the shader
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadBoxMesh();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/******************* FLOOR *******************/
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderColor(0.2f, 0.3f, 0.8f, 1); // blue
	m_basicMeshes->DrawPlaneMesh();

	/******************* BACK WALL *******************/
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	positionXYZ = glm::vec3(0.0f, 9.0f, -10.0f);

	SetTransformations(scaleXYZ, 90, 0, 0, positionXYZ);
	SetShaderColor(0.3f, 0.5f, 0.9f, 1);
	m_basicMeshes->DrawPlaneMesh();

	/******************* LEFT CYLINDER *******************/
	scaleXYZ = glm::vec3(2.0f, 1.0f, 2.0f);
	positionXYZ = glm::vec3(-5.0f, 1.0f, 0.0f);

	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderColor(0.3f, 0.4f, 0.9f, 1);
	m_basicMeshes->DrawCylinderMesh();

	/******************* LEFT SPHERE *******************/
	scaleXYZ = glm::vec3(1.5f, 1.5f, 1.5f);
	positionXYZ = glm::vec3(-5.0f, 3.5f, 0.0f);

	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderColor(0.6f, 0.3f, 0.8f, 1);
	m_basicMeshes->DrawSphereMesh();

	/******************* CENTER CYLINDER *******************/
	scaleXYZ = glm::vec3(2.5f, 2.5f, 2.5f);
	positionXYZ = glm::vec3(0.0f, 2.0f, 0.0f);

	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderColor(0.3f, 0.4f, 0.9f, 1);
	m_basicMeshes->DrawCylinderMesh();

	/******************* CONE *******************/
	scaleXYZ = glm::vec3(1.8f, 3.5f, 1.8f);
	positionXYZ = glm::vec3(0.0f, 4.0f, 0.0f);

	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderColor(1.0f, 1.0f, 0.2f, 1);
	m_basicMeshes->DrawConeMesh();

	/******************* RIGHT CYLINDER *******************/
	scaleXYZ = glm::vec3(2.0f, 1.5f, 2.0f);
	positionXYZ = glm::vec3(5.0f, 1.5f, 0.0f);

	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderColor(0.3f, 0.4f, 0.9f, 1);
	m_basicMeshes->DrawCylinderMesh();

	/******************* BOX *******************/
	scaleXYZ = glm::vec3(1.8f, 1.8f, 1.8f);
	positionXYZ = glm::vec3(5.0f, 4.0f, 0.0f);

	SetTransformations(scaleXYZ, 0, 0, 0, positionXYZ);
	SetShaderColor(1.0f, 0.3f, 0.3f, 1);
	m_basicMeshes->DrawBoxMesh();
}