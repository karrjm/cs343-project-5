#pragma once
#include "ofMain.h"
#include "Camera.h"

// Class for bundling a perspective camera with its associated matrices
class CameraMatrices
{
public:
    // Construct this object using an existing camera 
    // and specified aspect ratio, near clipping, and far clipping plane
    CameraMatrices(const Camera& camera, float aspect, 
        float nearPlane = 0.01f, float farPlane = 100.0f);

    // Get the camera associated with the matrices
    const Camera& getCamera() const;

    // Get the view matrix derived from the camera
    const glm::mat4& getView() const;

    // Get the projection matrix derived from the camera
    const glm::mat4& getProj() const;

private:
    Camera camera;
    glm::mat4 view;
    glm::mat4 proj;
};
