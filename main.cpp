#include <irrlicht.h>

#include "TerrainTexturing.h"
#include "PostProcessManager.h"
#include "ShaderPostProcess.h"
#include "WaterSceneNode.h"

using namespace irr;

#define MAP_CHUNK_SIZE 256
#define MAP_SCALE 10
#define MAP_SCALE_Y 0.08 * MAP_SCALE

int main(int argc, char** argv)
{
    //Set up device and receiver
    IrrlichtDevice *device = createDevice(video::EDT_OPENGL, core::dimension2d<u32>(640, 480), 32, false, false, false);

    video::IVideoDriver *driver = device->getVideoDriver();
    scene::ISceneManager *smgr = device->getSceneManager();

    //Create the camera
    SKeyMap keyMap[8];
    keyMap[0].Action = EKA_MOVE_FORWARD;
    keyMap[0].KeyCode = KEY_UP;
    keyMap[1].Action = EKA_MOVE_FORWARD;
    keyMap[1].KeyCode = KEY_KEY_W;

    keyMap[2].Action = EKA_MOVE_BACKWARD;
    keyMap[2].KeyCode = KEY_DOWN;
    keyMap[3].Action = EKA_MOVE_BACKWARD;
    keyMap[3].KeyCode = KEY_KEY_S;

    keyMap[4].Action = EKA_STRAFE_LEFT;
    keyMap[4].KeyCode = KEY_LEFT;
    keyMap[5].Action = EKA_STRAFE_LEFT;
    keyMap[5].KeyCode = KEY_KEY_A;

    keyMap[6].Action = EKA_STRAFE_RIGHT;
    keyMap[6].KeyCode = KEY_RIGHT;
    keyMap[7].Action = EKA_STRAFE_RIGHT;
    keyMap[7].KeyCode = KEY_KEY_D;

    scene::ICameraSceneNode *camera = smgr->addCameraSceneNodeFPS(0, 100, 0.6, -1, keyMap, 8);
    camera->setPosition(core::vector3df(1300, 250, 0));
    camera->setTarget(core::vector3df(600, 0, 600));
    camera->setNearValue(0.01);

	scene::ISceneNode* skydome=smgr->addSkyDomeSceneNode(driver->getTexture("Data/textures/skydome.jpg"), 16, 8, 0.95f, 2.0f);
    skydome->setRotation(core::vector3df(0, -100, 0));
    TerrainTexturing *texture_manager = new TerrainTexturing(device, smgr);

    CPostProcessManager           *postProcessManager = new CPostProcessManager(device);
    irr::scene::CWaterSceneNode   *waterNode;

    //Create a terrain like in the terrain sample
    scene::ITerrainSceneNode *terrain = smgr->addTerrainSceneNode("Data/textures/splatting_test.png",
                                                                  0,					                                              // parent node
                                                                  -1,					                                              // node id
                                                                  core::vector3df(00.f, -(MAP_SCALE_Y * MAP_CHUNK_SIZE) / 2, 00.f),   // position
                                                                  core::vector3df(0.f, 0.f, 0.f),		                              // rotation
                                                                  core::vector3df(MAP_SCALE, MAP_SCALE_Y, MAP_SCALE),	              // scale
                                                                  video::SColor(255, 255, 255, 255),	                              // vertexColor
                                                                  5,					                                              // maxLOD
                                                                  scene::ETPS_17,				                                      // patchSize
                                                                  3);					                                              // smoothFactor

    terrain->setMaterialFlag(video::EMF_LIGHTING, false);
	terrain->scaleTexture(1.0f, 20.0f);
    terrain->setScale(core::vector3df(5, 2, 5));

    //Add it to the manager
    texture_manager->addNode(terrain);

    //Set the passes
    //First Pass, with Sand, Gras and Rock
    texture_manager->addTextures(terrain,
                                 driver->getTexture("Data/textures/splat_1.tga"),
                                 driver->getTexture("Data/textures/Gras.jpg"),
                                 driver->getTexture("Data/textures/Sand.jpg"),
                                 driver->getTexture("Data/textures/Rock.jpg"));

    waterNode = new scene::CWaterSceneNode(smgr->getRootSceneNode(),
                                           device,
                                           smgr,
                                           -1,
                                           core::dimension2d<u32>(256, 256),
                                           core::dimension2d<f32>((MAP_SCALE * MAP_CHUNK_SIZE) * 3, (MAP_SCALE * MAP_CHUNK_SIZE) * 3));

    waterNode->setPosition(core::vector3df((MAP_SCALE * MAP_CHUNK_SIZE) / 2, 0, (MAP_SCALE * MAP_CHUNK_SIZE) / 2));
    waterNode->drop();

    postProcessManager->addNodeToDepthPass(terrain);

    //FPS measurement
    int lastFPS = -1;

    while (device->run()) {
        //Draw the scene
        driver->beginScene(true, true, video::SColor(50, 50, 50, 50));

        postProcessManager->renderDepth(); //create the depth information of the scene
        waterNode->updateWater();          //update the water node (prepare reflection and refraction rtt)
        postProcessManager->prepare();     //prepare scene for post processing (render into the PostProcess rtt)

        smgr->drawAll();
        texture_manager->drawAll();

        //apply postprocessing
        if (waterNode->getUnderWater()) {
            //set the underwater vars
            CShaderPostProcess* underWaterPass = (CShaderPostProcess*)postProcessManager->getEffectChain(EPPE_UNDER_WATER)[0];
            underWaterPass->getShaderMaterial()->setPixelShaderConstant("WaterColor", &waterNode->getWaterData().WaterColor.r, 4);
            underWaterPass->getShaderMaterial()->setPixelShaderConstant("BlendFactor", &waterNode->getWaterData().BlendFactor, 1);
            postProcessManager->render(EPPE_UNDER_WATER);
        }

        //finalize post processing
        //(render to the framebuffer)
        postProcessManager->update();

        driver->endScene();

        //Display the FPS
        int fps = driver->getFPS();

        if (lastFPS != fps) {
            core::stringw str = L"Texture Splatting with water - Move with Mouse and WASD/Arrow Keys - FPS:";
            str += fps;

            device->setWindowCaption(str.c_str());
            lastFPS = fps;
        }
    }

    //Cleanup
    device->drop();
    delete waterNode;
    delete postProcessManager;
    delete texture_manager;

    return 0;
}
