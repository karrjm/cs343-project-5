#pragma once

#include "ofMain.h"
#include "World.h"
#include "CellManager.h"
#include "Camera.h"
#include "CharacterPhysics.h"
// #include "ofxXboxController.h"

class ofApp : public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

private:
    // The number of quads in each row and column of a single terrain cell for the close, high level-of-detail terrain.
    const static unsigned int NEAR_LOD_SIZE { 64 };

    // The number of cells beyond the current cell that the cell manager loads for the close, high level-of-detail terrain.
    const static unsigned int NEAR_LOD_RANGE { 6 };

    // The vertical scale of the heightmap.  Used to convert the pixel values in the heightmap to physical height units.
    float heightmapScale { 1640.0f };

    // Non-GPU image containing the heightmap.
    ofShortImage heightmap {};

    // Plane mesh for rendering water.
    ofMesh waterPlane {};

    // Shader for rendering terrain.
    ofShader terrainShader {};

    // Shader for rendering water.
    ofShader waterShader {};

    // The main game "world" that uses the heightmap.
    World world {};

    // A cell manager for the high level-of-detail close terrain.
    CellManager<NEAR_LOD_RANGE + 1> cellManager { world, NEAR_LOD_SIZE };

    // The height (north-south) of the low-resolution heightmap used for generating the distant terrain.
    const static unsigned int FAR_LOD_RESOLUTION { 1024 };

    // The number of quads in each row and column of a single terrain cell for the far, low level-of-detail terrain.
    const static unsigned int FAR_LOD_SIZE { 32 };

    // The number of cells beyond the current cell that the cell manager loads for the far, low level-of-detail terrain.
    const static unsigned int FAR_LOD_RANGE { 6 };

    // Non-GPU image containing a lower-resolution heightmap for distant land.
    ofShortImage heightmapFarLOD;

    // A secondary "world" instance that uses a lower-resolution heightmap for distant land.
    World farLODWorld {};

    // A cell manager for the lower level-of-detail distant terrain.
    CellManager<FAR_LOD_RANGE + 1> farLODCellManager { farLODWorld, FAR_LOD_SIZE };

    // A single terrain mesh. Uncomment the following line if not using a cell manager.
    //ofMesh staticTerrain {};

    // Set to true when the shader reload hotkey is pressed.
    bool needsReload { true };

    // The first-person camera.
    Camera fpCamera {};

    // The helper object for physics calculations related to the first person character.
    CharacterPhysics character { world };

    // The camera's "look" sensitivity when using the mouse.
    float camSensitivity { 0.01f };

    // The camera's "look" sensitivity when using an Xbox controller.
    float xboxCamSensitivity { 10.0f };

    // The character's walk speed (will be calculated based on the world's gravity).
    float characterWalkSpeed;

    // The character's jump speed (will be calculated based on the world's gravity).
    float characterJumpSpeed;

    // The first-person camera's head rotation.
    float headAngle { 0 };

    // The first-person camera's pitch angle.
    float pitchAngle { 0 };

    // The mouse's previous x-coordinate; used for mouse camera controls.
    int prevMouseX { 0 };

    // The mouse's previous y-coordinate; used for mouse camera controls.
    int prevMouseY { 0 };

    // The local character velocity based on which of the WASD keys are pressed; needs to be transformed from local space to world space.
    glm::vec2 wasdVelocity { 0 };

    // The Xbox controller handle.
    // ofxXboxController controller;

    // Reloads the shaders while the application is running.
    void reloadShaders();

    // Updates the first-person camera bsed on some 2D input (from a mouse or Xbox controller).
    void updateFPCamera(float dx, float dy);
};
