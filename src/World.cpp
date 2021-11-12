#include "World.h"
#include "buildTerrainMesh.h"

using namespace glm;

void World::buildMeshForTerrainCell(ofMesh& terrainMesh, uvec2 startPos, uvec2 size) const
{
    if (startPos.x < heightmap->getWidth() && startPos.y < heightmap->getHeight())
    {
        // Clamp the size to the bounds of the heightmap
        size = min(size, uvec2(heightmap->getWidth(), heightmap->getHeight()) - startPos - 1u);

        // Use buildTerrainMesh() to initialize or re-initialize the mesh.
        // The scale parameter taken by buildTerrainMesh needs to be relative to the dimensions of the heightmap
        buildTerrainMesh(terrainMesh, *heightmap, startPos.x, startPos.y, startPos.x + size.x, startPos.y + size.y, 
            dimensions / vec3(heightmap->getWidth() - 1, 1, heightmap->getHeight() - 1));
    }
}

float World::getTerrainHeightAtPosition(const glm::vec3& position) const
{
    if (!heightmap)
    {
        return 0.0f;
    }
    else
    {
        // After unscaling, should range between (0, 0, 0) and (1, 1, 1)
        vec3 unscaledPosition { position / dimensions };

        // Remap to the resolution of the heightmap
        vec2 pixelScaledPosition { vec2(unscaledPosition.x, unscaledPosition.z) * vec2(heightmap->getWidth() - 1, heightmap->getHeight() - 1) };

        // Round down and clamp to get pixel indices
        ivec2 baseIndices { clamp(ivec2(floor(pixelScaledPosition)), ivec2(0), ivec2(heightmap->getWidth() - 2, heightmap->getHeight() - 2)) };

        // Calculate linear interpolation weights
        float height00 { static_cast<float>(heightmap->getColor(baseIndices[0],     baseIndices[1])[0]) };
        float height01 { static_cast<float>(heightmap->getColor(baseIndices[0],     baseIndices[1] + 1)[0]) };
        float height10 { static_cast<float>(heightmap->getColor(baseIndices[0] + 1, baseIndices[1])[0]) };
        float height11 { static_cast<float>(heightmap->getColor(baseIndices[0] + 1, baseIndices[1] + 1)[0]) };
        vec2 st { pixelScaledPosition - vec2(baseIndices) };

        // Linearly interpolate and apply the correct scale to the height being returned.
        return (mix(mix(height00, height01, st[1]), mix(height10, height11, st[1]), st[0]) / USHRT_MAX) * dimensions.y; 
    }
}