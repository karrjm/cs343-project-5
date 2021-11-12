#pragma once
#include "ofMain.h" // Pull in openFrameworks API

// Struct for representing a perspective camera
struct Camera
{
public:
    // Position of the camera
    glm::vec3 position {};

    // Camera rotation (camera-space orientation to world-space orientation)
    // Inverse of the rotation part of view transformation
    glm::mat3 rotation {};

    // Field of view, in radians (default is 90 degrees)
    float fov { glm::radians(90.0f) };
};
