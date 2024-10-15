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
		"textures/black_felt.jpg",
		"black_felt");

	bReturn = CreateGLTexture(
		"textures/green_felt.jpg",
		"green_felt");

	bReturn = CreateGLTexture(
		"textures/peach_felt.jpg",
		"peach_felt");

	bReturn = CreateGLTexture(
		"textures/white_leather.jpg",
		"white_leather");

	bReturn = CreateGLTexture(
		"textures/brown_leather.jpg",
		"brown_leather");
	
	
	
	bReturn = CreateGLTexture(
		"textures/chain.jpg",
		"gold_chain");
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
 *  DefineObjectMaterials()
 *
 *  This method is used for configuring the various material
 *  settings for all of the objects within the 3D scene.
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	OBJECT_MATERIAL goldMaterial;
	goldMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
	goldMaterial.specularColor = glm::vec3(0.7f, 0.7f, 0.6f);
	goldMaterial.shininess = 85.0;
	goldMaterial.tag = "metal";

	m_objectMaterials.push_back(goldMaterial);

	OBJECT_MATERIAL glassMaterial;
	glassMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
	glassMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glassMaterial.shininess = 95.0;
	glassMaterial.tag = "glass";

	m_objectMaterials.push_back(glassMaterial);

	OBJECT_MATERIAL marbleMaterial;
	marbleMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
	marbleMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	marbleMaterial.shininess = 30.0;
	marbleMaterial.tag = "marble";

	m_objectMaterials.push_back(marbleMaterial);

	OBJECT_MATERIAL feltMaterial;
	feltMaterial.diffuseColor = glm::vec3(0.1f, 0.1f, 0.1f);
	feltMaterial.specularColor = glm::vec3(0.0f, 0.0f, 0.f);
	feltMaterial.shininess = 1.0;
	feltMaterial.tag = "felt";

	m_objectMaterials.push_back(feltMaterial);

	OBJECT_MATERIAL leatherMaterial;
	leatherMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
	leatherMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	leatherMaterial.shininess = 30.0;
	leatherMaterial.tag = "leather";

	m_objectMaterials.push_back(leatherMaterial);

	
}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	// this line of code is NEEDED for telling the shaders to render 
	// the 3D scene with custom lighting, if no light sources have
	// been added then the display window will be black - to use the 
	// default OpenGL lighting then comment out the following line
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	/*** STUDENTS - add the code BELOW for setting up light sources ***/
	/*** Up to four light sources can be defined. Refer to the code ***/
	/*** in the OpenGL Sample for help                              ***/

	// Directional Light - simulates sunlight coming from the right side
	//m_pShaderManager->setVec3Value("directionalLight.direction", 0.0f, -1.0f, 0.0f); 
	//m_pShaderManager->setVec3Value("directionalLight.ambient", 0.2f, 0.2f, 0.2f);  // Adjusted ambient for a base light
	//m_pShaderManager->setVec3Value("directionalLight.diffuse", .8f, .8f, .8f);  // Stronger diffuse light for brighter highlights
	//m_pShaderManager->setVec3Value("directionalLight.specular", 1.0f, 1.0f, 1.0f); // Strong specular for reflections
	//m_pShaderManager->setBoolValue("directionalLight.bActive", true);

	// Point Light - simulates a nearby light source, like a window or a lamp
	m_pShaderManager->setVec3Value("pointLights.position", 0.0f, 20.0f, 0.0f); // Positioned slightly above and to the side
	m_pShaderManager->setVec3Value("pointLights[0].ambient", .55f, 0.5f, 0.5f);  // Adjusted ambient for more shadow contrast
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", .75f, .7f, .7f);  // Stronger diffuse for more intense lighting
	m_pShaderManager->setVec3Value("pointLights[0].specular", 1.0f, 0.9f, 0.9);  // Strong specular highlights for shinier surfaces
	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);

	// Set the position of the spotlight to emulate the sun's position
	m_pShaderManager->setVec3Value("spotLight.position", -18.0f, 10.0f, 55.0f); // Place the sun high and behind the objects
	m_pShaderManager->setVec3Value("spotLight.direction", 1.0f, -0.5f, -1.0f); // Adjust this to match the sunlight direction from the image
	m_pShaderManager->setVec3Value("spotLight.ambient", 6.0f, 6.0f, 6.0f); // Increase ambient for more overall light
	m_pShaderManager->setVec3Value("spotLight.diffuse", 15.0f, 15.0f, 15.0f); // Increase diffuse light for more brightness
	m_pShaderManager->setVec3Value("spotLight.specular", 10.0f, 10.0f, 10.0f); // Brighten specular highlights
	m_pShaderManager->setFloatValue("spotLight.constant", 1.0f);   // Minimal distance attenuation
	m_pShaderManager->setFloatValue("spotLight.linear", 0.01f);    // Further reduced attenuation over distance
	m_pShaderManager->setFloatValue("spotLight.quadratic", 0.005f); // Further reduced quadratic term for greater reach
	m_pShaderManager->setFloatValue("spotLight.cutOff", glm::cos(glm::radians(110.0f))); // Wide inner angle for full scene coverage
	m_pShaderManager->setFloatValue("spotLight.outerCutOff", glm::cos(glm::radians(130.0f))); // Even wider outer angle for soft edges
	m_pShaderManager->setBoolValue("spotLight.bActive", true);


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
	// load the textures for the 3D scene
	LoadSceneTextures();
	// define the materials for objects in the scene
	DefineObjectMaterials();
	// add and define the light sources for the scene
	SetupSceneLights();

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
	m_basicMeshes->LoadHexagonMesh();

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
	RenderEarrings();
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
	positionXYZ = glm::vec3(0.0f, 0.0f, -6.50f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//SetShaderColor(0.4f, 0.7f, 1.0f, 1.0f); // Light blue color
	SetShaderTexture("marble");
	SetShaderMaterial("marble");

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
	SetShaderMaterial("glass");

	m_basicMeshes->DrawBoxMesh();

	// --- Gold Sphere (Center of the Blue Box) ---
	scaleXYZ = glm::vec3(0.75f, 0.75f, 0.3f);  // Scaled up by 1.5
	XrotationDegrees = -90.0f;  // Rotate the half-sphere along with the body
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-15.0f, 1.875f, -15.0f);  // Adjusted position (0.75 * 1.5 = 1.125; 0.75 + 1.125 = 1.875)
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("versace");
	SetShaderMaterial("metal");
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
	SetShaderMaterial("metal");

	m_basicMeshes->DrawCylinderMesh();

	// --- Larger Cylinder (Top of the Cap) ---
	scaleXYZ = glm::vec3(1.5f, 1.5f, 1.5f);  // Scaled up by 1.5
	XrotationDegrees = -90.0f;  // Rotate the larger cylinder to match the orientation
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	positionXYZ = glm::vec3(-15.0f, 0.75f, -19.95f);  // Same adjusted position as the smaller cylinder
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gold");
	SetShaderMaterial("metal");
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

	// --- Gold Box for the Perfume Body ---
	scaleXYZ = glm::vec3(3.5f, 7.0f, 3.5f);  
	XrotationDegrees = 90.0f;  
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-21.0f, 0.875f, 2.0f);  
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("perfume");
	SetShaderMaterial("glass");
	m_basicMeshes->DrawBoxMesh();

	// --- Red Label ---
	scaleXYZ = glm::vec3(1.3f, 2.0f, 2.5f);  
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-21.0f, 2.725f, 2.0f);  
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1.0f, 0.0f, 0.0f, 1.0f);  // Red color
	m_basicMeshes->DrawPlaneMesh();

	// --- Smaller Cylinder (Base of the Cap) ---
	scaleXYZ = glm::vec3(1.3f, 1.5f, 1.3f);  
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-21.0f, 0.875f, -3.0f);  
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gold");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawCylinderMesh();

	// --- Perfume Cap ---
	scaleXYZ = glm::vec3(3.0f, 1.5f, 3.0f);  
	XrotationDegrees = -90.0f;  
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 180.0f;
	positionXYZ = glm::vec3(-21.0f, 0.875f, -3.5f);  
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gold");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::bottom);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::right);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::left);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::back);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::front);

	// Draw the top with a different texture
	SetShaderMaterial("metal");
	SetShaderTexture("versace");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::top);  // Different texture for top of cap


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

	// --- Necklace Box Bottom---
	scaleXYZ = glm::vec3(6.0f, 2.0f, 6.0f);  
	XrotationDegrees = 0.0f;  
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-5.0f, 1.0f, -15.0f);  
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("green_felt");
	SetShaderMaterial("felt");

	// Draw the sides of the cap
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::bottom);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::right);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::left);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::back);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::front);

	// Draw the top with a different texture
	SetShaderMaterial("felt");
	SetShaderTexture("black_felt");
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::BoxSide::top);

	// --- Necklace Platform ---
	scaleXYZ = glm::vec3(4.3f, 0.2f, 4.3f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-5.0f, 2.1f, -15.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("black_felt");
	SetShaderMaterial("felt");
	m_basicMeshes->DrawBoxMesh();

	// --- Necklace left ---
	scaleXYZ = glm::vec3(.50f, 0.15f, .50f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-5.30f, 2.3f, -15.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gold");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawCylinderMesh();

	// --- Necklace Right ---
	scaleXYZ = glm::vec3(.50f, 0.15f, .50f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-4.5f, 2.3f, -15.2f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gold");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawCylinderMesh();

	// --- Necklace Top ---
	scaleXYZ = glm::vec3(.50f, 0.15f, .50f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-5.0f, 2.3f, -15.47f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gold");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawCylinderMesh();

	// --- Necklace Bottom ---
	scaleXYZ = glm::vec3(.50f, 0.15f, .50f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-4.8f, 2.3f, -14.75f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gold");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawCylinderMesh();

	// --- Necklace Center ---
	scaleXYZ = glm::vec3(.15f, 0.15f, .15f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-4.9f, 2.45f, -15.2f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gold");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawSphereMesh();

	// --- Necklace Chain Left ---
	scaleXYZ = glm::vec3(1.75f, 0.2f, .2f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = -25.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-6.0f, 2.3f, -16.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("gold_chain");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawBoxMesh();

	// --- Necklace Chain Left ---
	scaleXYZ = glm::vec3(3.95f, 0.2f, .2f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 105.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-6.3f, 2.25f, -14.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(2.25f, 1.0f);
	SetShaderTexture("gold_chain");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawBoxMesh();

	// --- Necklace Chain Left Down ---
	scaleXYZ = glm::vec3(0.5f, 0.2f, .2f);
	XrotationDegrees = 105.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	positionXYZ = glm::vec3(-5.8f, 2.09f, -12.6f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(0.5f, .5f);
	SetShaderTexture("gold_chain");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawBoxMesh();

	// --- Necklace Chain Right ---
	scaleXYZ = glm::vec3(1.75f, 0.2f, .2f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 55.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-4.5f, 2.3f, -16.3f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("gold_chain");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawBoxMesh();

	// --- Necklace Chain Right ---
	scaleXYZ = glm::vec3(3.95f, 0.2f, .2f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 107.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-3.4f, 2.25f, -15.25f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(2.25f, 1.0f);
	SetShaderTexture("gold_chain");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawBoxMesh();

	// --- Necklace Chain Right Down ---
	scaleXYZ = glm::vec3(0.5f, 0.2f, .2f);
	XrotationDegrees = 105.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 90.0f;
	positionXYZ = glm::vec3(-2.8f, 2.09f, -13.35f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(0.5f, .5f);
	SetShaderTexture("gold_chain");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawBoxMesh();

	// --- Necklace Box Top ---
	scaleXYZ = glm::vec3(6.0f, 2.0f, 6.0f);
	XrotationDegrees = 70.0f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-6.3f, 4.5f, -19.8f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetTextureUVScale(1.0f, 1.0f);
	SetShaderTexture("green_felt");
	SetShaderMaterial("felt");
	m_basicMeshes->DrawBoxMesh();

	// --- Black felt Top Of Necklace Box ---
	scaleXYZ = glm::vec3(5.0f, 0.2f, 5.0f);
	XrotationDegrees = 70.0f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-6.0f, 4.75f, -18.75f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("black_felt");
	SetShaderMaterial("felt");
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

	// --- Ring Box 1 --- // 
	scaleXYZ = glm::vec3(7.0f, 7.f, 2.f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = -20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(10.0f, 1.f, -13.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("peach_felt");
	SetShaderMaterial("felt");
	m_basicMeshes->DrawHexagonMesh();

	// --- Ring Box Lip 1 --- // 
	scaleXYZ = glm::vec3(5.75f, 5.75f, .4f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = -20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(10.0f, 2.2f, -13.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("peach_felt");
	SetShaderMaterial("felt");
	m_basicMeshes->DrawHexagonMesh();

	// --- Ring Box Top 2 --- // 
	scaleXYZ = glm::vec3(7.0f, 7.f, 2.f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = -20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(16.0f, 1.f, -16.50f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("peach_felt");
	SetShaderMaterial("felt");
	m_basicMeshes->DrawHexagonMesh();

	// --- Ring Box Top Lip 2 --- // 
	scaleXYZ = glm::vec3(5.75f, 5.75f, .4f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = -20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(16.0f, 2.2f, -16.50f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("peach_felt");
	SetShaderMaterial("felt");
	m_basicMeshes->DrawHexagonMesh();

	// --- Groom Wedding Ban --- // 
	scaleXYZ = glm::vec3(1.35f, 1.f, 1.35f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = -20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(10.60f, 2.2f, -14.40f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.4f, 0.7f, 1.0f, 1.0f); // Light blue color
	SetShaderTexture("gold");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawCylinderMesh();

	// --- Groom Wedding Ban --- // 
	scaleXYZ = glm::vec3(.4f, .75f, .1f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = -20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(10.450f, 3.55f, -13.950f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.4f, 0.7f, 1.0f, 1.0f); // Light blue color
	SetShaderTexture("blue_glass");
	SetShaderMaterial("glass");
	m_basicMeshes->DrawBoxMesh();

	// --- Bride Engagement Ring --- // 
	scaleXYZ = glm::vec3(.8f, 1.f, .8f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = -20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(10.f, 2.2f, -12.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.4f, 0.7f, 1.0f, 1.0f); // Light blue color
	SetShaderTexture("gold");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawTorusMesh();

	// --- Bride Engagement Ring Hidden Halo --- // 
	scaleXYZ = glm::vec3(.3f, .5f, .2f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = -20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(10.f, 3.65f, -12.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.4f, 0.7f, 1.0f, 1.0f); // Light blue color
	SetShaderTexture("gold");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawTorusMesh();

	// --- Bride Engagement Ring Top Of Diamond --- // 
	scaleXYZ = glm::vec3(.3f, .2f, .5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = -20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(10.f, 3.65f, -12.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.4f, 0.7f, 1.0f, 1.0f); // Light blue color
	SetShaderTexture("marble");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawHalfSphereMesh();

	// --- Bride Engagement Ring Bottom Of Diamond --- // 
	scaleXYZ = glm::vec3(.5f, .4f, .5f);
	XrotationDegrees = 180.0f;
	YrotationDegrees = -20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(10.f, 3.45f, -12.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.4f, 0.7f, 1.0f, 1.0f); // Light blue color
	SetShaderTexture("marble");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawPyramid4Mesh();

	// --- Bride Wedding Ban --- // 
	scaleXYZ = glm::vec3(.8f, 1.f, .8f);
	XrotationDegrees = 15.0f;
	YrotationDegrees = -25.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(9.6f, 2.1f, -12.5f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	//SetShaderColor(0.4f, 0.7f, 1.0f, 1.0f); // Light blue color
	SetShaderTexture("gold");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawTorusMesh();
		
}

/***********************************************************
 *  RenderEarrings()
 *
 *  This method is called to render the shapes for the scene
 *  backdrop object.
 ***********************************************************/
void SceneManager::RenderEarrings()
{
	// Declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	// --- Black Felt Sleeve ---
	scaleXYZ = glm::vec3(4.50f, 0.5f, 6.50f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = -15.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(3.0f, 0.25f, -15.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("black_felt");
	SetShaderMaterial("felt");
	m_basicMeshes->DrawBoxMesh();

	// --- 1st Earring Pearl ---
	scaleXYZ = glm::vec3(.40f, .4f, .5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = -15.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(2.50f, 0.75f, -14.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("marble");
	SetShaderMaterial("marble");
	m_basicMeshes->DrawSphereMesh();

	// --- 2nd Earring Pearl ---
	scaleXYZ = glm::vec3(.40f, .4f, .5f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 20.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(3.15f, 0.75f, -14.50f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("marble");
	SetShaderMaterial("marble");
	m_basicMeshes->DrawSphereMesh();

	// --- 1st Earring Loop ---
	scaleXYZ = glm::vec3(.40f, .6f, .2f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = -25.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(2.8f, 0.65f, -15.10f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gold");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawTorusMesh();

	// --- 2nd Earring Loop ---
	scaleXYZ = glm::vec3(.40f, .6f, .2f);
	XrotationDegrees = 90.0f;
	YrotationDegrees = -45.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(3.2f, 0.65f, -15.40f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gold");
	SetShaderMaterial("metal");
	m_basicMeshes->DrawTorusMesh();

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
	SetShaderMaterial("felt");
	m_basicMeshes->DrawBoxMesh();

	// --- Bottom Vow Cover ---
	scaleXYZ = glm::vec3(10.0f, 0.2f, 14.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-3.0f, .6f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("white_leather");
	SetShaderMaterial("leather");
	m_basicMeshes->DrawBoxMesh();

	// --- Top Vow Cover ---
	scaleXYZ = glm::vec3(10.0f, 0.2f, 14.0f);
	XrotationDegrees = 2.0f;
	YrotationDegrees = 15.0f;
	ZrotationDegrees = 5.0f;
	positionXYZ = glm::vec3(-3.0f, 1.0f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("white_leather");
	SetShaderMaterial("leather");
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
	positionXYZ = glm::vec3(12.0f, 0.25f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("gray_felt");
	SetShaderMaterial("felt");
	m_basicMeshes->DrawBoxMesh();

	// --- Bottom Vow Cover ---
	scaleXYZ = glm::vec3(10.0f, 0.2f, 14.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(12.0f, 0.6f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("brown_leather");
	SetShaderMaterial("leather");
	m_basicMeshes->DrawBoxMesh();

	// --- Top Vow Cover ---
	scaleXYZ = glm::vec3(10.0f, 0.2f, 14.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 5.0f;
	positionXYZ = glm::vec3(12.0f, 1.0f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderTexture("brown_leather");
	SetShaderMaterial("leather");
	m_basicMeshes->DrawBoxMesh();

	// --- Paper Inside Vow ---
	scaleXYZ = glm::vec3(8.0, 0.05f, 14.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 4.0f;
	positionXYZ = glm::vec3(12.75f, 0.8f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1, 1, 1, 1);
	m_basicMeshes->DrawBoxMesh();

	// --- Paper Inside Vow ---
	scaleXYZ = glm::vec3(8.0, 0.05f, 14.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 3.0f;
	positionXYZ = glm::vec3(12.75f, .8f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1, 1, 1, 1);
	m_basicMeshes->DrawBoxMesh();

	// --- Paper Inside Vow ---
	scaleXYZ = glm::vec3(8.0, 0.05f, 14.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 2.0f;
	positionXYZ = glm::vec3(12.75f, .8f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1, 1, 1, 1);
	m_basicMeshes->DrawBoxMesh();

	// --- Paper Inside Vow ---
	scaleXYZ = glm::vec3(8.0, 0.05f, 14.0f);
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 1.0f;
	positionXYZ = glm::vec3(12.75f, .8f, 1.0f);
	SetTransformations(scaleXYZ, XrotationDegrees, YrotationDegrees, ZrotationDegrees, positionXYZ);
	SetShaderColor(1, 1, 1, 1);
	m_basicMeshes->DrawBoxMesh();
}
