///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
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
SceneManager::SceneManager(ShaderManager* pShaderManager)
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

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
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

	modelView = translation * rotationZ * rotationY * rotationX * scale;

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
 *  LoadSceneTextures()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	bReturn = CreateGLTexture(
		"textures/marble.jpg",
		"marble");

	bReturn = CreateGLTexture(
		"textures/gold.jpg",
		"gold");

	bReturn = CreateGLTexture(
		"textures/versace.jpg",
		"versace");

	bReturn = CreateGLTexture(
		"textures/blue_glass.jpg",
		"blue_glass");

	bReturn = CreateGLTexture(
		"textures/perfume.jpg",
		"perfume");

	bReturn = CreateGLTexture(
		"textures/gray_felt.jpg",
		"gray_felt");

	bReturn = CreateGLTexture(
		"textures/white_leather.jpg",
		"white_leather");

	bReturn = CreateGLTexture(
		"textures/brown_leather.jpg",
		"brown_leather");

	bReturn = CreateGLTexture(
		"textures/black_felt.jpg",
		"black_felt");

	bReturn = CreateGLTexture(
		"textures/green_felt.jpg",
		"green_felt");
	// after the texture image data is loaded into memory, the 
	// loaded textures need to be bound to texture slots - there
	// are a total of 16 available slots for scene textures
	BindGLTextures();
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
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// load the textures for the 3D scene
	LoadSceneTextures();

	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadPrismMesh();
	m_basicMeshes->LoadPyramid4Mesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	m_basicMeshes->LoadTorusMesh();

}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	RenderTable();
	RenderCologneBottle();
	RenderPerfumeBottle();
	RenderItinerary();
	RenderNecklaceBox();
	RenderRingBox();
	RenderWhiteVowBook();
	RenderBrownVowBook();
}



/***********************************************************
 *  RenderTable()
 *
 *  This method is called to render the shapes for the scene
 *  backdrop object.
 ***********************************************************/
void SceneManager::RenderTable()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(35.0f, 1.0f, 30.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//SetShaderColor(0.4f, 0.7f, 1.0f, 1.0f); // Light blue color
	SetShaderTexture("marble");

	// draw the mesh with transformation values
	m_basicMeshes->DrawPlaneMesh();
	/****************************************************************/
}


/***********************************************************
 *  RenderCologneBottle()
 *
 *  This method is called to render the shapes for the scene
 *  backdrop object.
 ***********************************************************/
void SceneManager::RenderCologneBottle()
{
	// Declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

#pragma region CologneBody
	// --- Blue Box for the Cologne Body ---
	scaleXYZ = glm::vec3(5.25f, 7.5f, 2.25f);  // Scaled up by 1.5
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-15.0f, 0.75f, -15.0f);  // Base position remains the same
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("blue_glass");
	m_basicMeshes->DrawBoxMesh();

	// --- Gold Sphere (Center of the Blue Box) ---
	scaleXYZ = glm::vec3(0.75f, 0.75f, 0.3f);  // Scaled up by 1.5
	XrotationDegrees = -90.0f;  // Rotate the half-sphere along with the body
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-15.0f, 1.875f, -15.0f);  // Adjusted position (0.75 * 1.5 = 1.125; 0.75 + 1.125 = 1.875)
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("versace");
	m_basicMeshes->DrawSphereMesh();
#pragma endregion

#pragma region CologneCap
	// --- Smaller Cylinder (Base of the Cap) ---
	scaleXYZ = glm::vec3(1.05f, 1.2f, 1.05f);  // Scaled up by 1.5
	XrotationDegrees = 90.0f;  // Rotate the cylinder to match the body's orientation
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-15.0f, 0.75f, -19.95f);  // Adjusted position (-3.3 * 1.5 = -4.95; -15.0 + (-4.95) = -19.95)
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gold");
	m_basicMeshes->DrawCylinderMesh();

	// --- Larger Cylinder (Top of the Cap) ---
	scaleXYZ = glm::vec3(1.5f, 1.5f, 1.5f);  // Scaled up by 1.5
	XrotationDegrees = -90.0f;  // Rotate the larger cylinder to match the orientation
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	positionXYZ = glm::vec3(-15.0f, 0.75f, -19.95f);  // Same adjusted position as the smaller cylinder
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gold");
	m_basicMeshes->DrawCylinderMesh(false, true, true);
	SetShaderTexture("versace");
	m_basicMeshes->DrawCylinderMesh(true, false, false);  // Using different texture for the top of the cylinder
#pragma endregion

}


/***********************************************************
 *  RenderPerfumeBottle()
 *
 *  This method is called to render the shapes for the scene
 *  backdrop object.
 ***********************************************************/
void SceneManager::RenderPerfumeBottle()
{
	// Declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

#pragma region PerfumeBody
	// --- Gold Box for the Perfume Body ---
	scaleXYZ = glm::vec3(3.5f, 7.0f, 3.5f);  // Scaled up by 2
	XrotationDegrees = 90.0f;  // Rotate the body 90 degrees around the X-axis
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-21.0f, 0.875f, 2.0f);  // Base position remains the same
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("perfume");
	m_basicMeshes->DrawBoxMesh();
#pragma endregion

#pragma region RedLabel
	// --- Red Label ---
	scaleXYZ = glm::vec3(1.3f, 2.0f, 2.5f);  // Scaled up by 2
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	// Calculate the new Y position to align with the scaled body
	positionXYZ = glm::vec3(-21.0f, 2.725f, 2.0f);  // Adjusted Y position
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1.0f, 0.0f, 0.0f, 1.0f);  // Red color
	m_basicMeshes->DrawPlaneMesh();
#pragma endregion

#pragma region PerfumeCapBase
	// --- Smaller Cylinder (Base of the Cap) ---
	scaleXYZ = glm::vec3(1.3f, 1.5f, 1.3f);  // Scaled up by 2
	XrotationDegrees = 90.0f;  // Rotate the cylinder to match the body's orientation
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	// Calculate the new Z position to align with the scaled body
	positionXYZ = glm::vec3(-21.0f, 0.875f, -3.0f);  // Adjusted Z position
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gold");
	m_basicMeshes->DrawCylinderMesh();
#pragma endregion

#pragma region PerfumeCap
	// --- Perfume Cap ---
	scaleXYZ = glm::vec3(3.0f, 1.5f, 3.0f);  // Scaled up by 2
	XrotationDegrees = -90.0f;  // Rotate the cap to match the orientation
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 180.0f;
	// Calculate the new Z position to align with the cap base
	positionXYZ = glm::vec3(-21.0f, 0.875f, -3.5f);  // Adjusted Z position
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gold");
	// Draw the sides of the cap
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::bottom);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::right);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::left);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::back);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::front);

	// Draw the top with a different texture
	SetShaderTexture("versace");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::top);  // Different texture for top of cap
#pragma endregion


}

/***********************************************************
 *  RenderItinerary()
 *
 *  This method is called to render the shapes for the scene
 *  backdrop object.
 ***********************************************************/
void SceneManager::RenderItinerary()
{
	// Declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// --- Itinerary ---
	scaleXYZ = glm::vec3(22.0f, 0.1f, 11.0f);
	XrotationDegrees = 0.0f;  // Rotate the body 90 degrees around the X-axis
	YrotationDegrees = -60.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-19.5f, 0.1f, -10.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1.0f, 1.0f, 1.0f, 1.0f);  // white color
	//SetShaderTexture("");
	m_basicMeshes->DrawBoxMesh();

	// --- Green Torus ---
	scaleXYZ = glm::vec3(1.5f, 1.5f, 0.75f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-23.25f, 0.3f, -17.25f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.12f, 0.21f, 0.18f, 1.0f);  // Dark green color
	//SetShaderTexture("");
	m_basicMeshes->DrawTorusMesh();

	// --- leaf motif ---
	scaleXYZ = glm::vec3(1.6f, 0.3f, 1.6f);
	XrotationDegrees = 0.0f;  // Rotate the half-sphere along with the body
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-23.25f, 0.0f, -17.25f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(0.12f, 0.21f, 0.18f, 1.0f);  // Dark green color
	//SetShaderTexture("leaf");
	m_basicMeshes->DrawHalfSphereMesh();

}

/***********************************************************
 *  RenderNecklace()
 *
 *  This method is called to render the shapes for the scene
 *  backdrop object.
 ***********************************************************/
void SceneManager::RenderNecklaceBox()
{
	// Declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// --- Ring Box Bottom---
	scaleXYZ = glm::vec3(6.0f, 2.0f, 6.0f);  
	XrotationDegrees = 0.0f;  
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-5.0f, 1.0f, -15.0f);  
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("green_felt");
	// Draw the sides of the cap
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::bottom);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::right);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::left);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::back);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::front);

	// Draw the top with a different texture
	SetShaderTexture("black_felt");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::top);

	// --- Necklace Platform ---
	scaleXYZ = glm::vec3(5.0f, 0.2f, 5.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-5.0f, 2.1f, -15.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("black_felt");
	m_basicMeshes->DrawBoxMesh();


	// --- Ring Box Top ---
	scaleXYZ = glm::vec3(6.0f, 2.0f, 6.0f);
	XrotationDegrees = 70.0f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-6.3f, 4.5f, -19.8f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("green_felt");
	m_basicMeshes->DrawBoxMesh();

	// --- Necklace Platform ---
	scaleXYZ = glm::vec3(5.0f, 0.2f, 5.0f);
	XrotationDegrees = 70.0f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-6.0f, 4.75f, -18.75f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("black_felt");
	m_basicMeshes->DrawBoxMesh();
}

/***********************************************************
 *  RenderRingBox()
 *
 *  This method is called to render the shapes for the scene
 *  backdrop object.
 ***********************************************************/
void SceneManager::RenderRingBox()
{
	// Declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;
}


/***********************************************************
 *  RenderWhiteVowBook()
 *
 *  This method is called to render the shapes for the scene
 *  backdrop object.
 ***********************************************************/
void SceneManager::RenderWhiteVowBook()
{
	// Declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// --- Gray Felt Sleeve ---
	scaleXYZ = glm::vec3(13.0f, 0.5f, 17.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-3.0f, 0.25f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gray_felt");
	m_basicMeshes->DrawBoxMesh();

	// --- Bottom Vow Cover ---
	scaleXYZ = glm::vec3(10.0f, 0.2f, 14.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-3.0f, .6f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("white_leather");
	m_basicMeshes->DrawBoxMesh();

	// --- Top Vow Cover ---
	scaleXYZ = glm::vec3(10.0f, 0.2f, 14.0f);
	XrotationDegrees = 2.0f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 5.0f;
	positionXYZ = glm::vec3(-3.0f, 1.0f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("white_leather");
	m_basicMeshes->DrawBoxMesh();

	// --- Paper Inside Vow ---
	scaleXYZ = glm::vec3(8.0, 0.05f, 14.0f);
	XrotationDegrees = 0.75f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 3.5f;
	positionXYZ = glm::vec3(-2.25f, .8f, .75f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1, 1, 1, 1);
	m_basicMeshes->DrawBoxMesh();

	// --- Paper Inside Vow ---
	scaleXYZ = glm::vec3(8.0, 0.05f, 14.0f);
	XrotationDegrees = 0.75f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 2.5f;
	positionXYZ = glm::vec3(-2.25f, .8f, .75f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1, 1, 1, 1);
	m_basicMeshes->DrawBoxMesh();

	// --- Paper Inside Vow ---
	scaleXYZ = glm::vec3(8.0, 0.05f, 14.0f);
	XrotationDegrees = 0.75f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 1.5f;
	positionXYZ = glm::vec3(-2.25f, .8f, .75f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1, 1, 1, 1);
	m_basicMeshes->DrawBoxMesh();

	// --- Paper Inside Vow ---
	scaleXYZ = glm::vec3(8.0, 0.05f, 14.0f);
	XrotationDegrees = 0.75f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.5f;
	positionXYZ = glm::vec3(-2.25f, .8f, .75f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1, 1, 1, 1);  
	m_basicMeshes->DrawBoxMesh();

}

/***********************************************************
 *  RenderBrownVowBook()
 *
 *  This method is called to render the shapes for the scene
 *  backdrop object.
 ***********************************************************/
void SceneManager::RenderBrownVowBook()
{
	// Declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// --- Gray Felt Sleeve ---
	scaleXYZ = glm::vec3(13.0f, 0.5f, 17.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(15.0f, 0.25f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gray_felt");
	m_basicMeshes->DrawBoxMesh();

	// --- Bottom Vow Cover ---
	scaleXYZ = glm::vec3(10.0f, 0.2f, 14.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(15.0f, 0.6f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("brown_leather");
	m_basicMeshes->DrawBoxMesh();

	// --- Top Vow Cover ---
	scaleXYZ = glm::vec3(10.0f, 0.2f, 14.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 5.0f;
	positionXYZ = glm::vec3(15.0f, 1.0f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("brown_leather");
	m_basicMeshes->DrawBoxMesh();

	// --- Paper Inside Vow ---
	scaleXYZ = glm::vec3(8.0, 0.05f, 14.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 4.0f;
	positionXYZ = glm::vec3(15.75f, 0.8f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1, 1, 1, 1);
	m_basicMeshes->DrawBoxMesh();

	// --- Paper Inside Vow ---
	scaleXYZ = glm::vec3(8.0, 0.05f, 14.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 3.0f;
	positionXYZ = glm::vec3(15.75f, .8f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1, 1, 1, 1);
	m_basicMeshes->DrawBoxMesh();

	// --- Paper Inside Vow ---
	scaleXYZ = glm::vec3(8.0, 0.05f, 14.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 2.0f;
	positionXYZ = glm::vec3(15.75f, .8f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1, 1, 1, 1);
	m_basicMeshes->DrawBoxMesh();

	// --- Paper Inside Vow ---
	scaleXYZ = glm::vec3(8.0, 0.05f, 14.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 1.0f;
	positionXYZ = glm::vec3(15.75f, .8f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1, 1, 1, 1);
	m_basicMeshes->DrawBoxMesh();

}