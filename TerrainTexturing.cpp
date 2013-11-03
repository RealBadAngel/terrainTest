#include "TerrainTexturing.h"

TerrainTexturing::TerrainTexturing(IrrlichtDevice *pDevice, scene::ISceneManager* pSceneMgr)
{
    //Assign all parameters
    m_pSceneMgr = pSceneMgr;
    m_pDriver = pSceneMgr->getVideoDriver();

    io::path vertShader;
	io::path fragShader;
	io::path vsFile = "terrain.vsh";
	io::path psFile = "terrain.psh";

    // log action
	core::stringc msg = core::stringc("compiling material terrainsplat");
	pDevice->getLogger()->log(msg.c_str(), ELL_INFORMATION);

	// create destination path for (driver specific) shader files
	if (m_pDriver->getDriverType() == video::EDT_OPENGL) {
		vertShader = core::stringc("./shaders/glsl/") + vsFile;
		fragShader = core::stringc("./shaders/glsl/") + psFile;
	}

    //Create the shader material
    m_nShaderMaterial = m_pDriver->getGPUProgrammingServices()->addHighLevelShaderMaterialFromFiles(vertShader, "main", video::EVST_VS_1_1,
                                                                                                    fragShader, "main", video::EPST_PS_1_1,
                                                                                                    this, video::EMT_TRANSPARENT_ALPHA_CHANNEL);
    m_pTextureSplat = new STextureSplat();
    m_pMultitextureNode = new SMultiTextureNode;
}

TerrainTexturing::~TerrainTexturing()
{
    if (m_pMultitextureNode != NULL) {
        delete m_pMultitextureNode;
        m_pMultitextureNode = NULL;
    }
    if (m_pTextureSplat != NULL) {
        delete m_pTextureSplat;
        m_pTextureSplat = NULL;
    }
}

bool TerrainTexturing::addNode(scene::ISceneNode *pNode)
{
    if (m_pMultitextureNode->pNode == pNode) {
        return false;
    } else {
        m_pMultitextureNode->pNode = pNode;
    }

    return true;
}

TerrainTexturing::STextureSplat *TerrainTexturing::addTextures(scene::ISceneNode *pNode, video::ITexture *pAlpha, video::ITexture *pRed, video::ITexture *pGreen, video::ITexture *pBlue)
{
    if (m_pMultitextureNode->pNode == pNode) {
        m_pTextureSplat->pAlphaTexture = pAlpha;
        m_pTextureSplat->pRedTexture = pRed;
        m_pTextureSplat->pGreenTexture = pGreen;
        m_pTextureSplat->pBlueTexture = pBlue;
        return m_pTextureSplat;
    }
    return NULL;
}

void TerrainTexturing::drawAll()
{
    //meshbuffer trick, only draw non culled nodes
    if (!m_pSceneMgr->isCulled(m_pMultitextureNode->pNode)) {
        m_pMultitextureNode->pNode->OnRegisterSceneNode();
        m_pMultitextureNode->pNode->updateAbsolutePosition();

        //Reset the transformation
        if (m_pMultitextureNode->pNode->getType() == scene::ESNT_TERRAIN) {
            m_pDriver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
        } else {
            m_pDriver->setTransform(video::ETS_WORLD, m_pMultitextureNode->pNode->getAbsoluteTransformation());
        }

        //set the splatting textures
        m_pMultitextureNode->pNode->setMaterialTexture(0, m_pTextureSplat->pAlphaTexture);
        m_pMultitextureNode->pNode->setMaterialTexture(1, m_pTextureSplat->pRedTexture);
        m_pMultitextureNode->pNode->setMaterialTexture(2, m_pTextureSplat->pGreenTexture);
        m_pMultitextureNode->pNode->setMaterialTexture(3, m_pTextureSplat->pBlueTexture);

        //pass the shader material to the terrain node
        if (m_pMultitextureNode->pNode->getType() == scene::ESNT_TERRAIN) {
            m_pMultitextureNode->pNode->setMaterialType((video::E_MATERIAL_TYPE)m_nShaderMaterial);
        }
    }
}

void TerrainTexturing::OnSetConstants(video::IMaterialRendererServices* services, s32 userData)
{
    int layer0 = 0;
    int layer1 = 1;
    int layer2 = 2;
    int layer3 = 3;

    services->setPixelShaderConstant("alphaMap1", (float*)&layer0, 1);
    services->setPixelShaderConstant("layer_red", (float*)&layer1, 1);
    services->setPixelShaderConstant("layer_green", (float*)&layer2, 1);
    services->setPixelShaderConstant("layer_blue", (float*)&layer3, 1);
}
