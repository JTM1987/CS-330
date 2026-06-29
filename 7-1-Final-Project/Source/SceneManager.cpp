///////////////////////////////////////////////////////////////////////////////
// shadermanager.cpp
// ============
// manage the loading and rendering of 3D scenes
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
//  Contributor: Jesse Martin, SNHU Student, Jun 2026
//  Module 3-3: mug from my Milestone One desk photo (tapered cylinder, torus handle, coaster).
//  Module 4-3: desk plane + mug serve as the navigable scene base for camera controls.
//  Module 5-3: texture mapping on the multi-part mug + tiled desk surface.
//  Module 6-3: Milestone Five — Phong lighting on textured desk plane + multi-part mug.
//  Module 7-1: Final Project — Module One Image 5 desk (iMac-style monitor + workspace props).
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

	// Legacy flat-color values — kept for reference; Module 5-3 draws with textures instead.
	const float kDeskSurfaceR = 0.88f;
	const float kDeskSurfaceG = 0.84f;
	const float kDeskSurfaceB = 0.78f;
	const float kDeskWallR = 0.72f;
	const float kDeskWallG = 0.68f;
	const float kDeskWallB = 0.62f;
	const float kMugBodyR = 0.96f;
	const float kMugBodyG = 0.94f;
	const float kMugBodyB = 0.90f;
	const float kMugHandleR = 0.52f;
	const float kMugHandleG = 0.60f;
	const float kMugHandleB = 0.70f;
	const float kCoasterR = 0.55f;
	const float kCoasterG = 0.40f;
	const float kCoasterB = 0.24f;
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

	// same init pattern as 5-2_Assignment — up to 16 texture slots per scene
	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].tag = "";
		m_textureIDs[i].ID = 0;
	}
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	DestroyGLTextures();
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	size_t index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.ambientColor = m_objectMaterials[index].ambientColor;
			material.ambientStrength = m_objectMaterials[index].ambientStrength;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(bFound);
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

	modelView = translation * rotationX * rotationY * rotationZ * scale;

	if (NULL != m_pShaderManager)
	{
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
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}

/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.ambientColor", material.ambientColor);
			m_pShaderManager->setFloatValue("material.ambientStrength", material.ambientStrength);
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/


/***********************************************************
 *  LoadSceneTextures()
 *
 *  Module 5-3: load square course JPGs for the desk and each mug part.
 *  Different image per primitive so the handle, cup, and coaster read as one object.
 ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	// --- Mug (complex object): three shapes, three textures ---
	bReturn = CreateGLTexture(
		"../../Utilities/textures/rusticwood.jpg",
		"mugCoaster");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/drywall.jpg",
		"mugBody");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/knife_handle.jpg",
		"mugHandle");

	// --- Desk: cream drywall tile — closer to my light flat-lay photo than dark wood planks ---
	bReturn = CreateGLTexture(
		"../../Utilities/textures/drywall.jpg",
		"deskFloor");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/backdrop.jpg",
		"deskWall");

	// --- Keyboard: one thin box with a tiled key-grid texture ---
	bReturn = CreateGLTexture(
		"../../Utilities/textures/tilesf2.jpg",
		"keyboardKeys");

	bReturn = CreateGLTexture(
		"../../Utilities/textures/stainless.jpg",
		"monitorSilver");

	BindGLTextures();
}

/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// textures first — same order as 5-2_Assignment / 1-2_OpenGLSample
	LoadSceneTextures();

	// I only load meshes I actually draw — no point keeping the whole shape library in memory.
	// Materials and lights get configured once here; RenderScene() stays about transforms.

	DefineObjectMaterials();
	SetupSceneLights();

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	m_basicMeshes->LoadTorusMesh(0.14f); // 0.14 tube thickness reads like a slim mug handle
}

/***********************************************************
 *  DefineObjectMaterials()
 *
 *  Phong materials for the mug and desk. I kept ambient low on purpose —
 *  too much fill washes the white cup out against the desk (same issue I hit in 3-2).
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	// Dark charcoal mug body — matches Image 5 matte ceramic cup.
	OBJECT_MATERIAL mugCeramic;
	mugCeramic.ambientColor = glm::vec3(0.06f, 0.06f, 0.07f);
	mugCeramic.ambientStrength = 0.14f;
	mugCeramic.diffuseColor = glm::vec3(0.18f, 0.18f, 0.20f);
	mugCeramic.specularColor = glm::vec3(0.06f, 0.06f, 0.07f);
	mugCeramic.shininess = 8.0f;
	mugCeramic.tag = "mugCeramic";
	m_objectMaterials.push_back(mugCeramic);

	OBJECT_MATERIAL mugHandle;
	mugHandle.ambientColor = glm::vec3(0.05f, 0.05f, 0.06f);
	mugHandle.ambientStrength = 0.16f;
	mugHandle.diffuseColor = glm::vec3(0.18f, 0.18f, 0.20f);
	mugHandle.specularColor = glm::vec3(0.05f, 0.05f, 0.06f);
	mugHandle.shininess = 6.0f;
	mugHandle.tag = "mugHandle";
	m_objectMaterials.push_back(mugHandle);

	// Desk floor — kept matte; ambient kept low because the shader stacks ambient per light.
	OBJECT_MATERIAL deskMatte;
	deskMatte.ambientColor = glm::vec3(0.08f, 0.08f, 0.08f);
	deskMatte.ambientStrength = 0.14f;
	deskMatte.diffuseColor = glm::vec3(0.68f, 0.66f, 0.63f);
	deskMatte.specularColor = glm::vec3(0.04f, 0.04f, 0.04f);
	deskMatte.shininess = 3.0f;
	deskMatte.tag = "deskMatte";
	m_objectMaterials.push_back(deskMatte);

	// Back wall, slightly darker than the floor so the white mug has something to contrast against.
	OBJECT_MATERIAL deskWall;
	deskWall.ambientColor = glm::vec3(0.08f, 0.07f, 0.07f);
	deskWall.ambientStrength = 0.22f;
	deskWall.diffuseColor = glm::vec3(0.48f, 0.44f, 0.40f);
	deskWall.specularColor = glm::vec3(0.02f, 0.02f, 0.02f);
	deskWall.shininess = 1.5f;
	deskWall.tag = "deskWall";
	m_objectMaterials.push_back(deskWall);

	// Keyboard slab — low gloss so the key texture reads under the lights.
	OBJECT_MATERIAL keyboardMatte;
	keyboardMatte.ambientColor = glm::vec3(0.07f, 0.07f, 0.07f);
	keyboardMatte.ambientStrength = 0.12f;
	keyboardMatte.diffuseColor = glm::vec3(0.58f, 0.58f, 0.60f);
	keyboardMatte.specularColor = glm::vec3(0.10f, 0.10f, 0.12f);
	keyboardMatte.shininess = 14.0f;
	keyboardMatte.tag = "keyboardMatte";
	m_objectMaterials.push_back(keyboardMatte);

	OBJECT_MATERIAL notebookPaper;
	notebookPaper.ambientColor = glm::vec3(0.08f, 0.08f, 0.07f);
	notebookPaper.ambientStrength = 0.12f;
	notebookPaper.diffuseColor = glm::vec3(0.78f, 0.76f, 0.72f);
	notebookPaper.specularColor = glm::vec3(0.05f, 0.05f, 0.05f);
	notebookPaper.shininess = 4.0f;
	notebookPaper.tag = "notebookPaper";
	m_objectMaterials.push_back(notebookPaper);

	OBJECT_MATERIAL notebookDark;
	notebookDark.ambientColor = glm::vec3(0.05f, 0.05f, 0.05f);
	notebookDark.ambientStrength = 0.18f;
	notebookDark.diffuseColor = glm::vec3(0.16f, 0.16f, 0.18f);
	notebookDark.specularColor = glm::vec3(0.08f, 0.08f, 0.10f);
	notebookDark.shininess = 6.0f;
	notebookDark.tag = "notebookDark";
	m_objectMaterials.push_back(notebookDark);

	OBJECT_MATERIAL monitorScreen;
	monitorScreen.ambientColor = glm::vec3(0.06f, 0.06f, 0.06f);
	monitorScreen.ambientStrength = 0.10f;
	monitorScreen.diffuseColor = glm::vec3(0.78f, 0.78f, 0.78f);
	monitorScreen.specularColor = glm::vec3(0.06f, 0.06f, 0.06f);
	monitorScreen.shininess = 8.0f;
	monitorScreen.tag = "monitorScreen";
	m_objectMaterials.push_back(monitorScreen);

	OBJECT_MATERIAL monitorBezel;
	monitorBezel.ambientColor = glm::vec3(0.04f, 0.04f, 0.04f);
	monitorBezel.ambientStrength = 0.16f;
	monitorBezel.diffuseColor = glm::vec3(0.08f, 0.08f, 0.09f);
	monitorBezel.specularColor = glm::vec3(0.20f, 0.20f, 0.22f);
	monitorBezel.shininess = 40.0f;
	monitorBezel.tag = "monitorBezel";
	m_objectMaterials.push_back(monitorBezel);

	OBJECT_MATERIAL monitorSilver;
	monitorSilver.ambientColor = glm::vec3(0.07f, 0.07f, 0.07f);
	monitorSilver.ambientStrength = 0.12f;
	monitorSilver.diffuseColor = glm::vec3(0.58f, 0.58f, 0.60f);
	monitorSilver.specularColor = glm::vec3(0.28f, 0.28f, 0.30f);
	monitorSilver.shininess = 32.0f;
	monitorSilver.tag = "monitorSilver";
	m_objectMaterials.push_back(monitorSilver);

	OBJECT_MATERIAL pencilHolderMat;
	pencilHolderMat.ambientColor = glm::vec3(0.07f, 0.07f, 0.07f);
	pencilHolderMat.ambientStrength = 0.12f;
	pencilHolderMat.diffuseColor = glm::vec3(0.72f, 0.72f, 0.72f);
	pencilHolderMat.specularColor = glm::vec3(0.05f, 0.05f, 0.05f);
	pencilHolderMat.shininess = 6.0f;
	pencilHolderMat.tag = "pencilHolderMat";
	m_objectMaterials.push_back(pencilHolderMat);

	// Mouse shell — slightly glossy plastic dome.
	OBJECT_MATERIAL mouseShell;
	mouseShell.ambientColor = glm::vec3(0.06f, 0.06f, 0.07f);
	mouseShell.ambientStrength = 0.12f;
	mouseShell.diffuseColor = glm::vec3(0.62f, 0.62f, 0.64f);
	mouseShell.specularColor = glm::vec3(0.18f, 0.18f, 0.20f);
	mouseShell.shininess = 24.0f;
	mouseShell.tag = "mouseShell";
	m_objectMaterials.push_back(mouseShell);

	// Pencil in the cup — black shaft + graphite cone tip.
	OBJECT_MATERIAL pencilShaft;
	pencilShaft.ambientColor = glm::vec3(0.04f, 0.04f, 0.04f);
	pencilShaft.ambientStrength = 0.16f;
	pencilShaft.diffuseColor = glm::vec3(0.12f, 0.12f, 0.14f);
	pencilShaft.specularColor = glm::vec3(0.10f, 0.10f, 0.12f);
	pencilShaft.shininess = 14.0f;
	pencilShaft.tag = "pencilShaft";
	m_objectMaterials.push_back(pencilShaft);

	OBJECT_MATERIAL pencilTip;
	pencilTip.ambientColor = glm::vec3(0.06f, 0.05f, 0.04f);
	pencilTip.ambientStrength = 0.14f;
	pencilTip.diffuseColor = glm::vec3(0.28f, 0.24f, 0.20f);
	pencilTip.specularColor = glm::vec3(0.08f, 0.07f, 0.06f);
	pencilTip.shininess = 8.0f;
	pencilTip.tag = "pencilTip";
	m_objectMaterials.push_back(pencilTip);
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  Image 5 lighting — two point lights only. The course shader adds ambient
 *  and diffuse per light, so three lights was blowing out the white desk/screen.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// Key point light — upper-left window light; low ambient to avoid wash-out.
	m_pShaderManager->setVec3Value("lightSources[0].position", -3.2f, 4.5f, 4.8f);
	m_pShaderManager->setVec3Value("lightSources[0].ambientColor", 0.025f, 0.025f, 0.025f);
	m_pShaderManager->setVec3Value("lightSources[0].diffuseColor", 0.45f, 0.45f, 0.45f);
	m_pShaderManager->setVec3Value("lightSources[0].specularColor", 0.20f, 0.20f, 0.20f);
	m_pShaderManager->setFloatValue("lightSources[0].focalStrength", 18.0f);
	m_pShaderManager->setFloatValue("lightSources[0].specularIntensity", 0.22f);

	// Warm fill point light — colored ambient tint on shadow sides (right/back).
	m_pShaderManager->setVec3Value("lightSources[1].position", 3.5f, 3.8f, 3.2f);
	m_pShaderManager->setVec3Value("lightSources[1].ambientColor", 0.045f, 0.035f, 0.025f);
	m_pShaderManager->setVec3Value("lightSources[1].diffuseColor", 0.15f, 0.12f, 0.09f);
	m_pShaderManager->setVec3Value("lightSources[1].specularColor", 0.05f, 0.04f, 0.03f);
	m_pShaderManager->setFloatValue("lightSources[1].focalStrength", 14.0f);
	m_pShaderManager->setFloatValue("lightSources[1].specularIntensity", 0.06f);

	// Light 2 off — a third pass was stacking too much diffuse on the desk.
	m_pShaderManager->setVec3Value("lightSources[2].position", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[2].ambientColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[2].diffuseColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[2].specularColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setFloatValue("lightSources[2].focalStrength", 1.0f);
	m_pShaderManager->setFloatValue("lightSources[2].specularIntensity", 0.0f);

	// Slot 3 unused — zero it so the shader loop does not add stray light.
	m_pShaderManager->setVec3Value("lightSources[3].position", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[3].ambientColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[3].diffuseColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setVec3Value("lightSources[3].specularColor", 0.0f, 0.0f, 0.0f);
	m_pShaderManager->setFloatValue("lightSources[3].focalStrength", 1.0f);
	m_pShaderManager->setFloatValue("lightSources[3].specularIntensity", 0.0f);
}

/***********************************************************
 *  RenderDeskSurface()
 *
 *  Milestone One desk plane (4-3 scene base) plus a back wall.
 *  Module 5-3: tiled pavers on the floor; backdrop on the wall (texture + Phong lighting).
 ***********************************************************/
void SceneManager::RenderDeskSurface()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// --- Floor: repeat wood grain so the large plane matches my light desk photo ---
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("deskFloor");
	SetTextureUVScale(8.0f, 8.0f);
	SetShaderMaterial("deskMatte");
	m_basicMeshes->DrawPlaneMesh();

	// --- Back wall: pushed back so the default flat-lay camera mostly sees the desk ---
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(0.0f, 5.5f, -8.0f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("deskWall");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("deskWall");
	m_basicMeshes->DrawPlaneMesh();
}

/***********************************************************
 *  RenderMonitor()
 *
 *  Image 5 centerpiece — iMac-style monitor built from boxes:
 *  bezel, screen, chin, stand neck, and stand base.
 ***********************************************************/
void SceneManager::RenderMonitor()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	const float monitorZ = -2.0f;
	const float screenCenterY = 1.55f;

	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// Black bezel sits just behind the white screen panel.
	scaleXYZ = glm::vec3(3.05f, 1.78f, 0.08f);
	positionXYZ = glm::vec3(0.0f, screenCenterY, monitorZ - 0.05f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	SetShaderMaterial("monitorBezel");
	m_basicMeshes->DrawBoxMesh();

	// Bright screen panel.
	scaleXYZ = glm::vec3(2.80f, 1.58f, 0.05f);
	positionXYZ = glm::vec3(0.0f, screenCenterY, monitorZ);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	SetShaderMaterial("monitorScreen");
	m_basicMeshes->DrawBoxMesh();

	// Silver chin below the screen.
	scaleXYZ = glm::vec3(2.35f, 0.12f, 0.10f);
	positionXYZ = glm::vec3(0.0f, 0.60f, monitorZ + 0.02f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("monitorSilver");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("monitorSilver");
	m_basicMeshes->DrawBoxMesh();

	// Stand neck.
	scaleXYZ = glm::vec3(0.16f, 0.52f, 0.10f);
	positionXYZ = glm::vec3(0.0f, 0.92f, monitorZ + 0.04f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("monitorSilver");
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderMaterial("monitorSilver");
	m_basicMeshes->DrawBoxMesh();

	// Stand base on the desk.
	scaleXYZ = glm::vec3(1.05f, 0.04f, 0.72f);
	positionXYZ = glm::vec3(0.0f, 0.02f, monitorZ + 0.18f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("monitorSilver");
	SetTextureUVScale(1.2f, 0.8f);
	SetShaderMaterial("monitorSilver");
	m_basicMeshes->DrawBoxMesh();
}

/***********************************************************
 *  RenderDeskMug()
 *
 *  Image 5 dark mug — tapered cylinder + torus handle (no coaster in the reference).
 ***********************************************************/
void SceneManager::RenderDeskMug()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// Left of the monitor, mid-depth on the desk.
	const float mugCenterX = -2.0f;
	const float mugCenterZ = -0.35f;
	const float mugYawDegrees = 55.0f;

	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	const float mugBodyHeight = 0.82f;
	const float mugBodyRadius = 0.40f;

	scaleXYZ = glm::vec3(mugBodyRadius, mugBodyHeight, mugBodyRadius);
	YrotationDegrees = mugYawDegrees;
	positionXYZ = glm::vec3(mugCenterX, 0.0f, mugCenterZ);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	SetShaderMaterial("mugCeramic");
	m_basicMeshes->DrawTaperedCylinderMesh();

	const float handleHeightFraction = 0.55f;
	const float mugRadiusAtHandle = mugBodyRadius * (1.0f - handleHeightFraction * 0.5f);
	const float handleLoopScale = 0.24f;
	const float handleCenterY = mugBodyHeight * handleHeightFraction;
	const float handleTubeInset = handleLoopScale * 0.14f;
	const float handleOutwardOffset = mugRadiusAtHandle + handleLoopScale * 0.10f - handleTubeInset * 0.5f;
	const float mugYawRadians = glm::radians(mugYawDegrees);
	const float handleCenterX = mugCenterX + handleOutwardOffset * cos(mugYawRadians);
	const float handleCenterZ = mugCenterZ + handleOutwardOffset * sin(mugYawRadians);

	scaleXYZ = glm::vec3(handleLoopScale, handleLoopScale * 1.35f, handleLoopScale);
	YrotationDegrees = mugYawDegrees + 90.0f;
	ZrotationDegrees = 180.0f;
	positionXYZ = glm::vec3(handleCenterX, handleCenterY, handleCenterZ);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	SetShaderMaterial("mugHandle");
	m_basicMeshes->DrawHalfTorusMesh();
}

/***********************************************************
 *  RenderDeskProps()
 *
 *  Image 5 workspace props — keyboard, mouse, book stack, pencil cup.
 ***********************************************************/
void SceneManager::RenderDeskProps()
{
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// Keyboard centered in the foreground.
	const float keyboardHeight = 0.05f;
	scaleXYZ = glm::vec3(2.55f, keyboardHeight, 0.92f);
	positionXYZ = glm::vec3(0.0f, keyboardHeight * 0.5f, 1.55f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderTexture("keyboardKeys");
	SetTextureUVScale(2.2f, 1.0f);
	SetShaderMaterial("keyboardMatte");
	m_basicMeshes->DrawBoxMesh();

	// Mouse to the right of the keyboard.
	const float mouseHeight = 0.14f;
	scaleXYZ = glm::vec3(0.48f, mouseHeight, 0.72f);
	positionXYZ = glm::vec3(1.35f, mouseHeight * 0.5f, 1.50f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	SetShaderMaterial("mouseShell");
	m_basicMeshes->DrawSphereMesh();

	// Stack of notebooks on the far left — alternating dark and light covers.
	const float bookCenterX = -3.05f;
	const float bookCenterZ = -0.55f;
	const float bookThickness = 0.035f;

	for (int i = 0; i < 4; i++)
	{
		const float bookY = bookThickness * 0.5f + (float)i * bookThickness;
		const float offsetX = (float)i * 0.015f;
		const float offsetZ = (float)i * 0.012f;

		scaleXYZ = glm::vec3(0.95f - (float)i * 0.02f, bookThickness, 1.15f - (float)i * 0.03f);
		positionXYZ = glm::vec3(bookCenterX + offsetX, bookY, bookCenterZ + offsetZ);

		SetTransformations(
			scaleXYZ,
			XrotationDegrees,
			YrotationDegrees,
			ZrotationDegrees,
			positionXYZ);
		SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
		if (i == 0 || i == 2)
		{
			SetShaderMaterial("notebookDark");
		}
		else
		{
			SetShaderMaterial("notebookPaper");
		}
		m_basicMeshes->DrawBoxMesh();
	}

	// White pencil cup on the right side of the desk.
	const float cupX = 2.65f;
	const float cupZ = -0.45f;
	const float cupHeight = 0.52f;

	scaleXYZ = glm::vec3(0.42f, cupHeight, 0.42f);
	positionXYZ = glm::vec3(cupX, cupHeight * 0.5f, cupZ);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
	SetShaderMaterial("pencilHolderMat");
	m_basicMeshes->DrawBoxMesh();

	// Pencils leaning inside the cup.
	const float pencilRadius = 0.018f;
	const float pencilLength = 0.42f;
	const float pencilBaseY = cupHeight * 0.35f;

	for (int i = 0; i < 4; i++)
	{
		const float yaw = -18.0f + (float)i * 12.0f;
		const float tiltX = 8.0f + (float)i * 4.0f;
		const float offsetX = -0.08f + (float)i * 0.05f;
		const float offsetZ = -0.04f + (float)i * 0.03f;

		scaleXYZ = glm::vec3(pencilRadius, pencilLength, pencilRadius);
		XrotationDegrees = tiltX;
		YrotationDegrees = yaw;
		ZrotationDegrees = 0.0f;
		positionXYZ = glm::vec3(cupX + offsetX, pencilBaseY, cupZ + offsetZ);

		SetTransformations(
			scaleXYZ,
			XrotationDegrees,
			YrotationDegrees,
			ZrotationDegrees,
			positionXYZ);
		SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
		SetShaderMaterial("pencilShaft");
		m_basicMeshes->DrawCylinderMesh();

		scaleXYZ = glm::vec3(pencilRadius * 1.5f, 0.05f, pencilRadius * 1.5f);
		positionXYZ = glm::vec3(
			cupX + offsetX,
			pencilBaseY + pencilLength * 0.48f,
			cupZ + offsetZ);

		SetTransformations(
			scaleXYZ,
			XrotationDegrees,
			YrotationDegrees,
			ZrotationDegrees,
			positionXYZ);
		SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);
		SetShaderMaterial("pencilTip");
		m_basicMeshes->DrawConeMesh();
	}
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// Image 5 layout — desk, monitor centerpiece, then surrounding props.
	RenderDeskSurface();
	RenderMonitor();
	RenderDeskMug();
	RenderDeskProps();
}
