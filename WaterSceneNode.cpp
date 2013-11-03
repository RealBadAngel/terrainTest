#include "WaterSceneNode.h"
//#include "ECustomSceneNodeTypes.h"
#include "ShaderMaterial.h"
namespace irr
{
namespace scene
{

CWaterSceneNode::CWaterSceneNode(scene::ISceneNode* parent, IrrlichtDevice* device, scene::ISceneManager* mgr, s32 id,
								 core::dimension2d<u32> rttSize, core::dimension2d<f32> waterSize) :
	scene::ISceneNode(parent, mgr, id), Device(device)
{
	#ifdef _DEBUG
	setDebugName("CWaterSceneNode");
	#endif

	// create the shader material name
	// use the pointer to create a unique name
	core::stringw name("water_");
	name += (long)this;

	// create a plane mesh
	scene::IMesh* mesh = mgr->getGeometryCreator()->createPlaneMesh(waterSize);

	// create the water mesh scene node
	WaterMeshSceneNode = mgr->addMeshSceneNode(mesh, this);

	// create new camera
	Camera=SceneManager->addCameraSceneNode(
		0,
		core::vector3d<f32>(0,0,0),
		core::vector3d<f32>(0,0,100),
		-1,
		false);

	// create the refraction render target texture
	core::stringw rttRefraction(name);
	rttRefraction +=  "_rtt_refraction";
	RefractionMap = mgr->getVideoDriver()->addRenderTargetTexture(rttSize, rttRefraction);

	// create the reflection render target texture
	core::stringw rttReflection(name);
	rttReflection += "_rtt_reflection";
	ReflectionMap = mgr->getVideoDriver()->addRenderTargetTexture(rttSize, rttReflection);

	// create the shader material
	WaterMaterial = new CShaderMaterial(Device, name, "material/water.vsh", "main", video::EVST_VS_2_a, "material/water.psh", "main", video::EPST_PS_3_0, video::EMT_SOLID);
	WaterMaterial->getMaterial().TextureLayer[0].Texture = mgr->getVideoDriver()->getTexture("./Data/textures/water/water.png");
	WaterMaterial->getMaterial().TextureLayer[1].Texture = ReflectionMap;
	WaterMaterial->getMaterial().TextureLayer[2].Texture = RefractionMap;
	WaterMaterial->getMaterial().TextureLayer[3].Texture = mgr->getVideoDriver()->getTexture("rttDepth");

	// set the texture properties for the material
	for (u32 i=1; i<video::MATERIAL_MAX_TEXTURES; ++i)
	{
		WaterMaterial->getMaterial().TextureLayer[i].TextureWrapU = video::ETC_MIRROR;
		WaterMaterial->getMaterial().TextureLayer[i].TextureWrapV = video::ETC_MIRROR;
	}

	// set the vertex shader constants
	WaterMaterial->setVertexShaderFlag(ESC_WORLD, true);
	WaterMaterial->setVertexShaderFlag(ESC_VIEW, true);
	WaterMaterial->setVertexShaderFlag(ESC_PROJECTION, true);
	WaterMaterial->setVertexShaderFlag(ESC_TIME, true);
	WaterMaterial->setVertexShaderConstant("ReflectedViewMatrix", Camera->getViewMatrix().pointer(), 16);
	WaterMaterial->setVertexShaderConstant("WaveLength", &WaterData.WaveLength, 1);
	WaterMaterial->setVertexShaderConstant("WindForce", &WaterData.WindForce.X, 2);

	// set the pixel shader constants
	WaterMaterial->setPixelShaderFlag(ESC_CAMERAPOSITION, true);
	WaterMaterial->setPixelShaderConstant("WaterColor", &WaterData.WaterColor.r, 4);
	WaterMaterial->setPixelShaderConstant("DeepColor", &WaterData.DeepColor.r, 4);
	WaterMaterial->setPixelShaderConstant("WaveHeight", &WaterData.WaveHeight, 1);
	WaterMaterial->setPixelShaderConstant("BlendFactor", &WaterData.BlendFactor, 1);
	WaterMaterial->setPixelShaderConstant("DepthBias", &WaterData.DepthBias, 1);
	WaterMaterial->setPixelShaderConstant("DepthExponent", &WaterData.DepthExponent, 1);
	WaterMaterial->setPixelShaderConstant("SpecExponent", &WaterData.SpecExponent, 1);
	WaterMaterial->setPixelShaderConstant("SpecAlpha", &WaterData.SpecAlpha, 1);
	WaterMaterial->setPixelShaderConstant("FresnelExponent", &WaterData.FresnelExponent, 1);
	WaterMaterial->setPixelShaderConstant("WaterTransparency", &WaterData.WaterTransparency, 1);

	// setclip plane offsets
	ReflectionClipPlaneOffet = 2.0f;
	RefractionClipPlaneOffet = 2.0f;

	// set the material
	WaterMeshSceneNode->getMaterial(0) = WaterMaterial->getMaterial();
	WaterMeshSceneNode->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);

	// set noce visible
	setVisible(true);
}

CWaterSceneNode::~CWaterSceneNode()
{
	// drop the water material
	if (WaterMaterial != NULL) {
		delete WaterMaterial;
		WaterMaterial = NULL;
	}
}

void CWaterSceneNode::OnRegisterSceneNode()
{
	if (IsVisible)
		SceneManager->registerNodeForRendering(this, scene::ESNRP_TRANSPARENT_EFFECT);

	ISceneNode::OnRegisterSceneNode();
}

void CWaterSceneNode::OnAnimate(u32 timeMs)
{
	// call base OnAnimate
	ISceneNode::OnAnimate(timeMs);
}

void CWaterSceneNode::updateRefractionMap(f32 clip)
{
	//get current camera
	scene::ICameraSceneNode* currentCamera = SceneManager->getActiveCamera();
	float distanceToSurface = abs(currentCamera->getPosition().Y - RelativeTranslation.Y);

	// get the video driver
	video::IVideoDriver* videoDriver = SceneManager->getVideoDriver();

	// render to refraction
	videoDriver->setRenderTarget(RefractionMap, true, true, video::SColor(0,0,0,0));

	// enable refraction clip plane
	core::plane3d<f32> refractionClipPlane(
		0, RelativeTranslation.Y-(RefractionClipPlaneOffet+distanceToSurface*0.1f)*clip, 0,
		0, clip, 0);
	videoDriver->setClipPlane(0, refractionClipPlane, true);

	// draw the scene
	SceneManager->drawAll();

	// disable refraction clip plane
	videoDriver->enableClipPlane(0, false);
}

void CWaterSceneNode::updateReflectionMap()
{
	//get current camera
	scene::ICameraSceneNode* currentCamera = SceneManager->getActiveCamera();

	// get the video driver
	video::IVideoDriver* videoDriver = SceneManager->getVideoDriver();

	// render to reflection
	videoDriver->setRenderTarget(ReflectionMap, true, true, video::SColor(0,0,0,0));

	// set the FOV and far value
    Camera->setNearValue(currentCamera->getNearValue());
    Camera->setFarValue(currentCamera->getFarValue());
    Camera->setFOV(currentCamera->getFOV());

	// set the position of the water camera
	core::vector3df position = currentCamera->getAbsolutePosition();
	position.Y = -position.Y + 2.0f*RelativeTranslation.Y;
	Camera->setPosition(position);

	// set the target of the water camera
	core::vector3df target = currentCamera->getTarget();
	target.Y = -target.Y+2.0f*RelativeTranslation.Y;
	Camera->setTarget(target);

	// set the reflection camera as active camera
	SceneManager->setActiveCamera(Camera);

	// enable reflection clip plane
	core::plane3d<f32> reflectionClipPlane(
		0, RelativeTranslation.Y-ReflectionClipPlaneOffet, 0,
		0, 1, 0);
	videoDriver->setClipPlane(0, reflectionClipPlane, true);

	// draw the scene
	SceneManager->drawAll();

	// disable reflection clip plane
	videoDriver->enableClipPlane(0, false);

	// reset active camera
	SceneManager->setActiveCamera(currentCamera);
}

void CWaterSceneNode::updateWater()
{
	//get current camera
	scene::ICameraSceneNode* currentCamera=SceneManager->getActiveCamera();

	// get the video driver
	video::IVideoDriver* videoDriver = SceneManager->getVideoDriver();

	if (IsVisible && currentCamera && videoDriver)
	{
		// set under water
		if (currentCamera->getAbsolutePosition().Y < RelativeTranslation.Y)
			UnderWater = true;
		else
			UnderWater = false;

		// hide the water
		setVisible(false);

		// update the render target textures
		if (UnderWater)
		{
			updateRefractionMap(1.0f);
		}
		else
		{
			updateReflectionMap();
			updateRefractionMap(-1.0f);
		}

		// set MaxDistance pixel shader constant
		WaterMaterial->setPixelShaderConstant("MaxDistance", currentCamera->getFarValue());

		//show the node again
		setVisible(true);
	}
}

void CWaterSceneNode::render()
{
}

const core::aabbox3d<f32>& CWaterSceneNode::getBoundingBox() const
{
	return WaterMeshSceneNode->getBoundingBox();
}

ESCENE_NODE_TYPE CWaterSceneNode::getType() const
{
	return scene::ESNT_UNKNOWN;//(ESCENE_NODE_TYPE)ECSNT_WATER;
}

u32 CWaterSceneNode::getMaterialCount() const
{
	return WaterMeshSceneNode->getMaterialCount();
}

video::SMaterial& CWaterSceneNode::getMaterial(u32 i)
{
	return WaterMeshSceneNode->getMaterial(i);
}

// Writes attributes of the scene node.
void CWaterSceneNode::serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options) const
{
	ISceneNode::serializeAttributes(out, options);
}

// Reads attributes of the scene node.
void CWaterSceneNode::deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options)
{
	ISceneNode::deserializeAttributes(in, options);
}

ISceneNode* CWaterSceneNode::clone(ISceneNode* newParent, ISceneManager* newManager)
{
	if (!newParent)
		newParent = Parent;
	if (!newManager)
		newManager = SceneManager;

	CWaterSceneNode* nb = new CWaterSceneNode(newParent, Device, newManager, ID);

	nb->cloneMembers(this, newManager);

	delete nb;
	return NULL;
}

} // end namespace scene
} // end namespace irr
