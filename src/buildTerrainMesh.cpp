#include "buildTerrainMesh.h"
using namespace glm;

void buildTerrainMesh(ofMesh& terrainMesh, const ofShortPixels& heightmap,
    unsigned int xStart, unsigned int yStart, unsigned int xEnd, unsigned int yEnd, vec3 scale)
{
    // Initialize vertex positions
    for (unsigned int x { xStart }; x <= xEnd; x++)
    {
        for (unsigned int y { yStart }; y <= yEnd; y++)
        {
            terrainMesh.addVertex(scale * (vec3(x, static_cast<float>(heightmap.getColor(x, y).r) / static_cast<float>(USHRT_MAX), y)));
        }
    }

    // Initialize index buffer
    int k { 0 }; // k stores the index of the first corner of the quad.
    for (unsigned int x { xStart }; x < xEnd; x++)
    {
        for (unsigned int y { yStart }; y < yEnd; y++)
        {
            // Triangle 1
            terrainMesh.addIndex(k);                           // SW
            terrainMesh.addIndex(k + 1);                       // NW
            terrainMesh.addIndex(k + (1 + yEnd - yStart));     // SE

            // Triangle 2
            terrainMesh.addIndex(k + (1 + yEnd - yStart));     // SE
            terrainMesh.addIndex(k + 1);                       // NW
            terrainMesh.addIndex(k + (1 + yEnd - yStart) + 1); // NE

            k++; // Advance k to the next vertex in the "column."
        }

        // We've reached the end of the column, advance k an extra time past the final vertex of the column, to the start of the next column.
        k++;
    }

    // Generate normals
    terrainMesh.flatNormals();

    // Flip normals if necessary.
    for (vec3& normal : terrainMesh.getNormals())
    {
        if (normal.y < 0)
        {
            normal = -normal;
        }
    }
}