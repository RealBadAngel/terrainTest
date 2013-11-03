#ifndef _WATERSCENENODE_H
#define _WATERSCENENODE_H

#include <irrlicht.h>

class CShaderMaterial;

namespace irr
{
namespace scene
{

	// water properties (shader constants)
	struct SWater
	{
		SWater()
		{
			// set default water properties
			WaterColor = video::SColorf(video::SColor(255,10,50,60));
			DeepColor = video::SColorf(video::SColor(255,5,5,10));
			WaveHeight = 0.07f;
			WaveLength = 50.0f;
			WindForce = core::vector2d<f32>(5.f, 5.f);
			FresnelExponent = 1.0f;
			DepthBias = 60.0f;
			DepthExponent = 3.0f;
			SpecExponent = 2.0f;
			SpecAlpha = 0.0f;
			BlendFactor = 0.4f;
			WaterTransparency = 100.0f;
		}
		// water color
		video::SColorf WaterColor;
		// depth color of the water
		video::SColorf DeepColor;
		// direction an speed of waves
		core::vector2d<f32> WindForce;
		// wave height (strength of pertubation by normal map)
		f32 WaveHeight;
		// wave lentgh
		f32 WaveLength;
		// fesnel exponent (mixing reflection and refraction)
		f32 FresnelExponent;
		// poition of the sun (specular effect)
		core::vector3d<f32> SunPos;
		// power of the specular effect
		f32 SpecExponent;
		// alpha of the specular effect
		f32 SpecAlpha;
		// blend factor (mixing of water color, refraction and reflection)
		f32 BlendFactor;
		// depth bias (higher bias makes the water intransparent)
		f32 DepthBias;
		// depth exponent (higher exponent makes the shore line sharper)
		f32 DepthExponent;
		// the transparency of the water (lower values make the water clearer)
		f32 WaterTransparency;
	};

	class CWaterSceneNode : public scene::ISceneNode
	{
	public:
		// constructor
		CWaterSceneNode(scene::ISceneNode* parent, IrrlichtDevice* device, scene::ISceneManager* mgr, s32 id = -1,
			core::dimension2d<u32> rttSize = core::dimension2d<u32>(512, 512),
			core::dimension2d<f32> waterSize = core::dimension2d<f32>(10000,10000));

		// destructor
		~CWaterSceneNode();
	private:
		// irrlicht device
		IrrlichtDevice* Device;
	protected:
		// Bounding box
		core::aabbox3d<f32> BBox;

		// material of the node
		CShaderMaterial* WaterMaterial;
		SWater WaterData;


		// offsets of the clip planes used for reflection and refraction drawing
		f32 ReflectionClipPlaneOffet;
		f32 RefractionClipPlaneOffet;

		// is active camera under the water surface
		bool UnderWater;

		// water mesh scene node
		IMeshSceneNode* WaterMeshSceneNode;

		// camera for creating reflection map
		scene::ICameraSceneNode* Camera;

		// updates the refraction render target texture
		void updateRefractionMap(f32 clip);

		// updates the refraction render target texture
		void updateReflectionMap();

	public:
		// refraction, reflection render target texture
		video::ITexture* RefractionMap;
		video::ITexture* ReflectionMap;

		// get Under water state
		const bool getUnderWater() { return UnderWater; }

		// scene node registration
		virtual void OnRegisterSceneNode();

		// animates the node
		virtual void OnAnimate(u32 timeMs);

		// return the water propertie
		const SWater& getWaterData() { return WaterData; }

		// updates the water
		// (must be called before rendering the scene)
		virtual void updateWater();

		// renders the node
		virtual void render();

		// returns the bounding box
		virtual const core::aabbox3d<f32>& getBoundingBox() const;

		// returns the node type
		virtual ESCENE_NODE_TYPE getType() const;

		// returns the material count
		virtual u32 getMaterialCount() const;

		// returns the material
		virtual video::SMaterial& getMaterial(u32 i);

		// clones the node
		virtual ISceneNode* clone(ISceneNode* newParent = 0, ISceneManager* newManager = 0);

		// Writes attributes of the scene node.
		virtual void serializeAttributes(io::IAttributes* out, io::SAttributeReadWriteOptions* options = 0) const;

		// Reads attributes of the scene node.
		virtual void deserializeAttributes(io::IAttributes* in,	io::SAttributeReadWriteOptions* options = 0);
	};

} // end namespace scene
} // end namespace irr

#endif
