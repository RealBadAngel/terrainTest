#ifndef TERRAINTEXTURING_H
#define TERRAINTEXTURING_H

#include <irrlicht.h>
#include <SMaterial.h>

using namespace irr;

class TerrainTexturing: public video::IShaderConstantSetCallBack
{
    public:
        struct STextureSplat {
            video::ITexture *pAlphaTexture;
            video::ITexture *pRedTexture;
            video::ITexture *pGreenTexture;
            video::ITexture *pBlueTexture;
        };

    private:
        struct SMultiTextureNode {
            scene::ISceneNode *pNode;
            STextureSplat* pAlpha;
        };

    public:
        TerrainTexturing(IrrlichtDevice *pDevice, scene::ISceneManager* pSceneMgr);
        ~TerrainTexturing();

        void drawAll();
        virtual void OnSetConstants(video::IMaterialRendererServices* pServices,s32 userData);
        bool addNode(scene::ISceneNode *pNode);
        STextureSplat *addTextures(scene::ISceneNode *pNode, video::ITexture *pAlpha, video::ITexture *pRed, video::ITexture *pGreen, video::ITexture *pBlue);

    private:
        scene::ISceneManager *m_pSceneMgr;
        video::IVideoDriver  *m_pDriver;
        scene::ISceneNode    *m_pNode;
        SMultiTextureNode    *m_pMultitextureNode;
        STextureSplat        *m_pTextureSplat;
        int                   m_nShaderMaterial;

};

#endif
