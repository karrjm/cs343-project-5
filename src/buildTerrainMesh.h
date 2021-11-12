#pragma once
#include "ofMain.h"

// A function that should contain the logic of assembling the terrain mesh.
// "terrainMesh" is a reference to the mesh that needs to be initialized.
// "heightmap" is a reference to the heightmap pixels that define the shape of the mesh.
// "xStart", "yStart", "xEnd", and "yEnd" define the rectangle within the heightmap that will be included in the mesh
// "scale" is a scaling factor by which the vertex positions should be multiplied after extracting them from the heightmap.
// If the indices of a pixel are (x, y) -- integers ranging from (xStart, yStart) to (xEnd, yEnd) --
// and the value stored in the heightmap at that location is h -- an integer ranging from 0 to USHRT_MAX (2^16 - 1)
// -- then the position of the vertex in object space should be (x, h / USHRT_MAX (using floating-point division), y) * scale.
void buildTerrainMesh(ofMesh& terrainMesh, const ofShortPixels& heightmap,
    unsigned int xStart, unsigned int yStart, unsigned int xEnd, unsigned int yEnd, glm::vec3 scale);
