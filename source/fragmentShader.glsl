#version 330 core
#define maxLights 10

in vec2 UV;
in vec3 viewDir;
in vec3 tangentSpaceLightPos[maxLights];
in vec3 tangentSpaceLightDir[maxLights];

out vec4 outColor;

// Textures
uniform sampler2D baseColorMap;
uniform sampler2D normalMap;
uniform sampler2D specularMap;
uniform sampler2D glossinessMap;

// Material values
uniform float ka, kd, ks, Ns;
uniform int numLights;
uniform vec3 lightColours[maxLights];

// Debug toggle
uniform int debugMode;

void main()
{
    vec2 uv = UV;

    vec3 normal = texture(normalMap, uv).rgb;
    //normal.g = 1.0 - normal.g; // Flip Y for DirectX -> OpenGL if needed
    normal = normalize(normal * 2.0 - 1.0);

    vec3 albedo = texture(baseColorMap, uv).rgb;
    vec3 specularColor = texture(specularMap, uv).rgb;
    float gloss = texture(glossinessMap, uv).r;

    float shininess = mix(32.0, 128.0, gloss);
    vec3 finalColor = vec3(0.0);

    for (int i = 0; i < numLights; ++i) {
        vec3 L = normalize(tangentSpaceLightPos[i]);
        vec3 H = normalize(L + viewDir);

        float diff = max(dot(normal, L), 0.0);
        diff = pow(diff, 0.6); // exaggerate light falloff
        float spec = pow(max(dot(normal, H), 0.0), shininess);

        vec3 diffuse = kd * diff * albedo * lightColours[i];
        vec3 specular = ks * spec * specularColor * lightColours[i];

        finalColor += ka * albedo + diffuse + specular;
    }

    // Rim light for subtle glow effect
    float rim = 0.8 - max(dot(normalize(viewDir), normal), 0.0);
    vec3 rimLight = pow(rim, 2.0) * vec3(1.0, 0.7, 0.9);
    finalColor += rimLight * 0.2;

    finalColor = pow(finalColor, vec3(1.0 / 2.4));
    finalColor = clamp(finalColor, 0.0, 1.0);

    // Debug Modes
    if (debugMode == 1) outColor = vec4(normal * 0.5 + 0.5, 1.0);
    else if (debugMode == 2) outColor = vec4(albedo, 1.0);
    else if (debugMode == 3) outColor = vec4(specularColor, 1.0);
    else if (debugMode == 4) outColor = vec4(vec3(gloss), 1.0);
    else if (debugMode == 5) outColor = vec4(normalize(tangentSpaceLightPos[0]) * 0.5 + 0.5, 1.0);
    else if (debugMode == 6) outColor = vec4(viewDir * 0.5 + 0.5, 1.0);
    else if (debugMode == 7) {
        float d = max(dot(normal, normalize(tangentSpaceLightPos[0])), 0.0);
        outColor = vec4(vec3(d), 1.0);
    }
    else if (debugMode == 8) {
        vec3 L = normalize(tangentSpaceLightDir[0]);
        outColor = vec4(L * 0.5 + 0.5, 1.0);
    }
    else if (debugMode == 9) {
        vec3 N = normalize(vec3(0.0, 0.0, 1.0));
        vec3 L = normalize(vec3(0.0, 0.0, 1.0));
        float d = max(dot(N, L), 0.0);
        outColor = vec4(vec3(d), 1.0);
    }
    else outColor = vec4(finalColor, 1.0);
}