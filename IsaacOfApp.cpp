#include "ofApp.h"
#include "CameraMatrices.h"
#include "GLFW/glfw3.h"
#include "buildTerrainMesh.h"

using namespace glm;

void buildPlaneMesh(float width, float depth, float height, ofMesh& planeMesh)
{
    // Northwest corner
    planeMesh.addVertex(vec3(0, height, 0));
    planeMesh.addTexCoord(vec2(0, 0));
    planeMesh.addNormal(vec3(0, 1, 0));

    // Northeast corner
    planeMesh.addVertex(vec3(0, height, depth));
    planeMesh.addTexCoord(vec2(1, 0));
    planeMesh.addNormal(vec3(0, 1, 0));

    // Southwest corner
    planeMesh.addVertex(vec3(width, height, 0));
    planeMesh.addTexCoord(vec2(0, 1));
    planeMesh.addNormal(vec3(0, 1, 0));

    // Southeast corner
    planeMesh.addVertex(vec3(width, height, depth));
    planeMesh.addTexCoord(vec2(1, 1));
    planeMesh.addNormal(vec3(0, 1, 0));

    // NW - NE - SW triangle
    planeMesh.addIndex(0);
    planeMesh.addIndex(1);
    planeMesh.addIndex(2);

    // SW - NE - SE triangle
    planeMesh.addIndex(2);
    planeMesh.addIndex(1);
    planeMesh.addIndex(3);
}

void ofApp::reloadShaders()
{
    terrainShader.load("shaders/mesh.vert", "shaders/directionalLight.frag");
    waterShader.load("shaders/mesh.vert", "shaders/water.frag");
    shader.load("shaders/my.vert", "shaders/my.frag");
    skyboxShader.load("shaders/skybox.vert", "shaders/skybox.frag");

    // Setup terrain shader uniform variables
    terrainShader.begin();
    terrainShader.setUniform3f("lightDir", normalize(vec3(1, 1, -1)));
    terrainShader.setUniform3f("lightColor", vec3(1, 1, 0.5));
    terrainShader.setUniform3f("ambientColor", vec3(0.15, 0.15, 0.3));
    terrainShader.setUniform1f("gammaInv", 1.0f / 2.2f);
    terrainShader.setUniform3f("meshColor", vec3(0.25, 0.5, 0.25));
    terrainShader.setUniformMatrix3f("normalMatrix", mat3());
    terrainShader.end();

    needsReload = false;
}

//--------------------------------------------------------------
void ofApp::setup()
{
    // Disable legacy "ArbTex"
    ofDisableArbTex();

    // Enable depth test, alpha blending, and face culling.
    ofEnableDepthTest();
    ofEnableAlphaBlending();
    glEnable(GL_CULL_FACE);

    // Set sky color.
    // ofSetBackgroundColor(186, 186, 255);

    auto window{ ofGetCurrentWindow() };

    // Uncomment the following line to let GLFW take control of the cursor so we have unlimited cursor space
    glfwSetInputMode(dynamic_pointer_cast<ofAppGLFWWindow>(window)->getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load the shaders for the first time.
    reloadShaders();

    // Initialize the camera.
    headAngle = radians(180.0f);
    fpCamera.fov = radians(90.0f);
    fpCamera.rotation = rotate(headAngle, vec3(0, 1, 0)); // Rotate 180 degrees around the y-axis.

    cout << "Loading heightmap..." << endl;

    // Load heightmap
    heightmap.setUseTexture(false);
    heightmap.load("TamrielBeta_10_2016_01.png");
    assert(heightmap.getWidth() != 0 && heightmap.getHeight() != 0);

    // Set initial camera position.
    fpCamera.position = vec3((heightmap.getWidth() - 1) * 0.5f, 0, (heightmap.getHeight() - 1) * 0.5f);

    // Build a single terrain mesh.  Uncomment the following line if not using a cell manager.
    // buildTerrainMesh(staticTerrain, heightmap, fpCamera.position.x - 384, fpCamera.position.z - 384, fpCamera.position.x + 384, fpCamera.position.z + 384, vec3(1, heightmapScale, 1));

    // Setup the world parameters.
    world.heightmap = &heightmap.getPixels();
    world.dimensions = vec3((heightmap.getWidth() - 1), heightmapScale, heightmap.getHeight() - 1);
    world.gravity = -world.dimensions.y * 0.05f;
    world.waterHeight = 0.4375f * world.dimensions.y;

    cout << "Downscaling heightmap for far LOD..." << endl;

    float heightmapAspect = static_cast<float>(heightmap.getWidth() - 1) / static_cast<float>(heightmap.getHeight() - 1);

    // Make a copy of the heightmap to resize for distant land.
    heightmapFarLOD = heightmap;
    heightmapFarLOD.resize(static_cast<int>(round(heightmapAspect * FAR_LOD_RESOLUTION)), FAR_LOD_RESOLUTION);

    // Make a copy of the world the uses the low-resolution heightmap.
    farLODWorld = world;
    farLODWorld.heightmap = &heightmapFarLOD.getPixels();

    cout << "Building far LOD terrain meshes..." << endl;
    farLODCellManager.initializeForPosition(fpCamera.position);

    cout << "Building terrain meshes..." << endl;
    cellManager.initializeForPosition(fpCamera.position);

    cout << "DONE!" << endl;

    // Create the water plane
    buildPlaneMesh(heightmap.getWidth() - 1, heightmap.getHeight() - 1, world.waterHeight, waterPlane);

    // Define character height relative to gravity
    float charHeight = -world.gravity * 0.1685f;
    character.setCharacterHeight(charHeight);

    // Set initial character position.
    character.setPosition(fpCamera.position);

    // Set character movement parameters
    characterWalkSpeed = 10 * charHeight; // much faster than realism for efficiently moving around the map
    characterJumpSpeed = 10 * charHeight; // much higher than realism for efficiently moving around the map

    // load sword model
    swordMesh.load("models/sword.ply");

    /*swordMesh.flatNormals();
    for (size_t i{ 0 }; i < swordMesh.getNumNormals(); i++)
    {
        swordMesh.setNormal(i, -swordMesh.getNormal(i));
    }*/

    // load sword texture
    swordTex.load("textures/sword_metallic.png");
    swordTex.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
    swordTex.getTexture().generateMipmap(); // create the mipmaps
    swordTex.getTexture().setTextureMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

    // load skybox mesh
    cubeMesh.load("models/cube.ply");

    // load cubemap images
    cubemap.load("textures/skybox_front.png", "textures/skybox_back.png",
        "textures/skybox_right.png", "textures/skybox_left.png",
        "textures/skybox_top.png", "textures/skybox_bottom.png");
}

void ofApp::updateFPCamera(float dx, float dy)
{
    // Turn left/right.  Use quadratic response curve.
    float dxQuad = dx * abs(dx);

    // Calculate rotation around the y-axis.
    headAngle += dxQuad;
    if (headAngle > 2 * pi<float>())
    {
        headAngle -= 2 * pi<float>();
    }
    else if (headAngle < 0)
    {
        headAngle += 2 * pi<float>();
    }

    // Look up/down.  Use quadratic response curve.
    pitchAngle = clamp(pitchAngle + dy * abs(dy), -pi<float>() / 2, pi<float>() / 2);

    // Update character's rotation: rotation around the x-axis.
    fpCamera.rotation = rotate(headAngle, vec3(0, 1, 0)) * rotate(pitchAngle, vec3(1, 0, 0));
}

//--------------------------------------------------------------
void ofApp::update()
{
    mat3 headRotationMatrix{ rotate(headAngle, vec3(0, 1, 0)) };

    // Mouse / keyboard controls: set character velocity from WASD
    character.setDesiredVelocity(headRotationMatrix * vec3(wasdVelocity.x, 0, -wasdVelocity.y));

    if (prevMouseX != 0 && prevMouseY != 0) // Skip if the cursor position was uninitialized previously.
    {
        // Update camera direction from mouse.
        updateFPCamera(-camSensitivity * (ofGetMouseX() - prevMouseX), -camSensitivity * (ofGetMouseY() - prevMouseY));
    }

    prevMouseX = ofGetMouseX();
    prevMouseY = ofGetMouseY();

    if (needsReload)
    {
        // Reload shaders if the hotkey was pressed.
        reloadShaders();
    }

    // Advance character physics.
    character.update(ofGetLastFrameTime());

    // Use new character position as the camera position.
    fpCamera.position = character.getPosition();

    // Load new cells if necessary:
    cellManager.optimizeForPosition(fpCamera.position);
    cellManager.processLoadQueue();
}

//--------------------------------------------------------------
void ofApp::draw()
{
    float aspect{ static_cast<float>(ofGetViewportWidth()) / static_cast<float>(ofGetViewportHeight()) };

    // Calculate an appropriate distance for a plane conceptually dividing the high level-of-detail close terrain and the lower level-of-detail distant terrain (or fog with no distant terrain).
    float midLODPlane{ length(vec2(
        0.5f * NEAR_LOD_SIZE * NEAR_LOD_RANGE,
        fpCamera.position.y - world.getTerrainHeightAtPosition(fpCamera.position))) };

    // Calculate an appropriate far plane for the distant terrain.
    float farPlaneDistant{ length(vec2(
        FAR_LOD_SIZE * FAR_LOD_RANGE * heightmap.getHeight() / FAR_LOD_RESOLUTION,
        fpCamera.position.y - world.getTerrainHeightAtPosition(fpCamera.position))) };

    // Calculate view and projection matrices for the distant terrain.
    // Set the clipping plane to be 25% of the dividing plane to allow for sufficient overlap for a smooth transition.
    CameraMatrices camFarMatrices{ fpCamera, aspect, midLODPlane * 0.25f, farPlaneDistant };

    float nearPlane = -world.gravity * 0.01f;

    CameraMatrices camNearMatrices{ fpCamera, aspect, nearPlane, midLODPlane };
    mat4 modelView{ camNearMatrices.getView() };
    mat4 mvp{ camNearMatrices.getProj() * camNearMatrices.getView() };
    // Disable depth clamping for distant terrain
    glDisable(GL_DEPTH_CLAMP);

    drawCube(camFarMatrices);


    // Distant terrain
    terrainShader.begin();
    terrainShader.setUniform1f("startFade", farPlaneDistant * 0.95f);
    terrainShader.setUniform1f("endFade", farPlaneDistant * 1.0f);
    terrainShader.setUniformMatrix4f("modelView", camFarMatrices.getView());
    terrainShader.setUniformMatrix4f("mvp", camFarMatrices.getProj() * camFarMatrices.getView());

    // Draw the distant terrain cells.
    farLODCellManager.drawActiveCells(fpCamera.position, farPlaneDistant);

    terrainShader.end();

    // Enable depth clamping for water to cover up distant terrain regardless of depth values.
    // It seems that depth clamping can be left on for near terrain without any undesired effects.
    glEnable(GL_DEPTH_CLAMP);

    // Distant water
    waterShader.begin();
    waterShader.setUniform3f("meshColor", vec3(0.64, 0.73, 0.81));
    waterShader.setUniform1f("startFade", farPlaneDistant * 0.95f);
    waterShader.setUniform1f("endFade", farPlaneDistant * 1.0f);
    waterShader.setUniformMatrix4f("modelView", camFarMatrices.getView());
    waterShader.setUniformMatrix4f("mvp", camFarMatrices.getProj() * camFarMatrices.getView());

    // Draw the water plane.
    waterPlane.draw();

    waterShader.end();

    // Clear depth buffer as our clipping planes have changed
    glClear(GL_DEPTH_BUFFER_BIT);

    ; // Define near plane relative to world gravity (which is also proportional to player character height)

    // Calculate view and projection matrices for the close terrain.

    //// Debugging matrices:
    //midLODPlane = 100000; // to push back fog
    //mat4 proj { glm::perspective(radians(100.0f), aspect, 1.0f, 100000.0f) };
    //mat4 view { glm::rotate(radians(60.0f), vec3(1, 0, 0)) *
    //    glm::translate(vec3(-10240, -8000, -16000)) };
    //mat4 modelView { view };
    //mat4 mvp { proj * view };



    // Near terrain
    terrainShader.begin();
    terrainShader.setUniform1f("startFade", midLODPlane * 0.75f);
    terrainShader.setUniform1f("endFade", midLODPlane);
    terrainShader.setUniformMatrix4f("modelView", modelView);
    terrainShader.setUniformMatrix4f("mvp", mvp);

    // Draw the high level-of-detail cells.
    cellManager.drawActiveCells(fpCamera.position, midLODPlane);

    // Alternatively, draw the static terrain mesh if not using a cell manager.
    //staticTerrain.draw();

    terrainShader.end();

    // Near water
    waterShader.begin();
    waterShader.setUniform3f("meshColor", vec3(0.64, 0.73, 0.81));
    waterShader.setUniform1f("startFade", midLODPlane * 0.75f);
    waterShader.setUniform1f("endFade", midLODPlane);
    waterShader.setUniformMatrix4f("modelView", modelView);
    waterShader.setUniformMatrix4f("mvp", mvp);

    // Draw the water plane.
    waterPlane.draw();

    waterShader.end();

    shader.begin();
    swordMesh.draw();
    shader.end();


}

void ofApp::drawCube(const CameraMatrices& camMatrices)
{
    mat4 model{ translate(camMatrices.getCamera().position) };

    glDisable(GL_CULL_FACE);
    skyboxShader.begin();
    glDepthFunc(GL_LEQUAL);// pass depth cest at far clipping plane
    skyboxShader.setUniformMatrix4f("mvp",
        camMatrices.getProj() * mat4(mat3(camMatrices.getView())));
    skyboxShader.setUniformTexture("cubemap", cubemap.getTexture(), 0);
    cubeMesh.draw();
    skyboxShader.end();
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
}

void ofApp::exit()
{
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    // Update the local character velocity when a WASD key is pressed.
    if (key == 'a')
    {
        wasdVelocity.x = -characterWalkSpeed;
    }
    else if (key == 'd')
    {
        wasdVelocity.x = characterWalkSpeed;
    }
    else if (key == 'w')
    {
        wasdVelocity.y = characterWalkSpeed;
    }
    else if (key == 's')
    {
        wasdVelocity.y = -characterWalkSpeed;
    }
    else if (key == ' ')
    {
        character.jump(characterJumpSpeed);
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
    if (key == '`')
    {
        // Reload shaders
        needsReload = true;
    }
    else if (key == 'a') // Update the local character velocity when a WASD key is pressed.
    {
        wasdVelocity.x = 0.0f;
    }
    else if (key == 'd')
    {
        wasdVelocity.x = 0.0f;
    }
    else if (key == 'w')
    {
        wasdVelocity.y = 0.0f;
    }
    else if (key == 's')
    {
        wasdVelocity.y = 0.0f;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y)
{
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y)
{
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{

}
