#version 330 core
#define maxLights 10

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal_modelspace;
layout(location = 3) in vec3 vertexTangent_modelspace;
layout(location = 4) in vec3 vertexBitangent_modelspace;

uniform mat4 MVP;
uniform mat4 MV;
uniform mat4 V;

out vec2 UV;
out vec3 fragPosView; // view-space
out vec3 viewDir;     // tangent-space
out vec3 tangentSpaceLightPos[maxLights];
out vec3 tangentSpaceLightDir[maxLights];

uniform int numLights;
uniform vec3 lightPositions[maxLights];  // view-space
uniform vec3 lightDirections[maxLights]; // view-space

void main()
{
    gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);
    UV = vertexUV;

    vec3 T = normalize(mat3(MV) * vertexTangent_modelspace);
    vec3 B = normalize(mat3(MV) * vertexBitangent_modelspace);
    vec3 N = normalize(mat3(MV) * vertexNormal_modelspace);
    mat3 TBN = transpose(mat3(T, B, N)); // view-space to tangent-space

    fragPosView = vec3(MV * vec4(vertexPosition_modelspace, 1.0));
    viewDir = TBN * normalize(-fragPosView); // camera is at origin in view-space

    for (int i = 0; i < numLights; ++i) {
        vec3 lightPosView = lightPositions[i];
        vec3 lightDirView = lightDirections[i];

        tangentSpaceLightPos[i] = TBN * (lightPosView - fragPosView);
        tangentSpaceLightDir[i] = TBN * lightDirView;
    }
}
