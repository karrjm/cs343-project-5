#include "calcTangents.h"

// Tangent calculation from Halladay text
void calcTangents(ofMesh& mesh)
{
    using namespace glm;
    std::vector<vec4> tangents;
    tangents.resize(mesh.getNumVertices());

    uint indexCount = mesh.getNumIndices();

    const vec3* vertices = mesh.getVerticesPointer();
    const vec2* uvs = mesh.getTexCoordsPointer();
    const uint* indices = mesh.getIndexPointer();

    for (uint i = 0; i < indexCount - 2; i += 3)
    {
        const vec3& v0 = vertices[indices[i]];
        const vec3& v1 = vertices[indices[i + 1]];
        const vec3& v2 = vertices[indices[i + 2]];
        const vec2& uv0 = uvs[indices[i]];
        const vec2& uv1 = uvs[indices[i + 1]];
        const vec2& uv2 = uvs[indices[i + 2]];

        vec3 edge1 = v1 - v0;
        vec3 edge2 = v2 - v0;
        vec2 dUV1 = uv1 - uv0;
        vec2 dUV2 = uv2 - uv0;

        float f = 1.0f / (dUV1.x * dUV2.y - dUV2.x * dUV1.y);

        vec4 tan;
        tan.x = f * (dUV2.y * edge1.x - dUV1.y * edge2.x);
        tan.y = f * (dUV2.y * edge1.y - dUV1.y * edge2.y);
        tan.z = f * (dUV2.y * edge1.z - dUV1.y * edge2.z);
        tan.w = 0;
        tan = normalize(tan);

        tangents[indices[i]] += (tan);
        tangents[indices[i + 1]] += (tan);
        tangents[indices[i + 2]] += (tan);
    }

    for (int i = 0; i < tangents.size(); ++i)
    {
        vec3 t = normalize(tangents[i]);

        if (i < mesh.getNumColors())
        {
            mesh.setColor(i, ofFloatColor(t.x, t.y, t.z, 0.0));
        }
        else
        {
            mesh.addColor(ofFloatColor(t.x, t.y, t.z, 0.0));
        }
    }
}
