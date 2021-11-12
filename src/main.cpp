#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main() {
	ofGLWindowSettings glSettings;
	glSettings.setSize(720, 480);
	glSettings.windowMode = OF_WINDOW;
	glSettings.setGLVersion(4, 1);
	ofCreateWindow(glSettings);

	/*auto window{ ofGetCurrentWindow() };
	glfwSetInputMode(dynamic_pointer_cast<ofAppGLFWWindow>(window)->getGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	window->setFullscreen(true);*/

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}