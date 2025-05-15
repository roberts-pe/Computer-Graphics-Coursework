#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "model.hpp"
#include "stb_image.hpp"

Model::Model(const char* path)
{
    bool res = loadObj(path, vertices, uvs, normals);
    if (res) {
        setupBuffers();
    }
}

void Model::draw(unsigned int& shaderID)
{
    glUniform1f(glGetUniformLocation(shaderID, "ka"), ka);
    glUniform1f(glGetUniformLocation(shaderID, "kd"), kd);
    glUniform1f(glGetUniformLocation(shaderID, "ks"), ks);
    glUniform1f(glGetUniformLocation(shaderID, "Ns"), Ns);

    for (unsigned int i = 0; i < textures.size(); i++) {
        std::string name = textures[i].type;
        glActiveTexture(GL_TEXTURE0 + i);
        glUniform1i(glGetUniformLocation(shaderID, (name + "Map").c_str()), i);
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<unsigned int>(vertices.size()));
    glBindVertexArray(0);
}

void Model::setupBuffers()
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

    glGenBuffers(1, &tangentBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
    glBufferData(GL_ARRAY_BUFFER, tangents.size() * sizeof(glm::vec3), &tangents[0], GL_STATIC_DRAW);

    glGenBuffers(1, &bitangentBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, bitangentBuffer);
    glBufferData(GL_ARRAY_BUFFER, bitangents.size() * sizeof(glm::vec3), &bitangents[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, tangentBuffer);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, bitangentBuffer);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);
}

void Model::deleteBuffers()
{
    glDeleteBuffers(1, &vertexBuffer);
    glDeleteBuffers(1, &uvBuffer);
    glDeleteBuffers(1, &normalBuffer);
    glDeleteBuffers(1, &tangentBuffer);
    glDeleteBuffers(1, &bitangentBuffer);
    glDeleteVertexArrays(1, &VAO);
}

bool Model::loadObj(const char* path,
    std::vector<glm::vec3>& outVertices,
    std::vector<glm::vec2>& outUVs,
    std::vector<glm::vec3>& outNormals)
{
    printf("Loading file %s\n", path);

    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> tempVertices;
    std::vector<glm::vec2> tempUVs;
    std::vector<glm::vec3> tempNormals;

    FILE* file = fopen(path, "r");
    if (file == NULL)
    {
        printf("Impossible to open the file. Check paths and directories.\n");
        return false;
    }

    while (true)
    {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF) break;

        if (strcmp(lineHeader, "v") == 0)
        {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            tempVertices.push_back(vertex);
        }
        else if (strcmp(lineHeader, "vt") == 0)
        {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            tempUVs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0)
        {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            tempNormals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0)
        {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                &vertexIndex[2], &uvIndex[2], &normalIndex[2]);

            if (matches != 9)
            {
                printf("File can't be read by loadObj().\n");
                fclose(file);
                return false;
            }

            for (int i = 0; i < 3; ++i) {
                outVertices.push_back(tempVertices[vertexIndex[i] - 1]);
                outUVs.push_back(tempUVs[uvIndex[i] - 1]);
                outNormals.push_back(tempNormals[normalIndex[i] - 1]);
            }
        }
        else
        {
            char commentBuffer[1000];
            fgets(commentBuffer, 1000, file);
        }
    }
    fclose(file);

    tangents.resize(outVertices.size());
    bitangents.resize(outVertices.size());

    for (size_t i = 0; i < outVertices.size(); i += 3)
    {
        const glm::vec3& v0 = outVertices[i + 0];
        const glm::vec3& v1 = outVertices[i + 1];
        const glm::vec3& v2 = outVertices[i + 2];

        const glm::vec2& uv0 = outUVs[i + 0];
        const glm::vec2& uv1 = outUVs[i + 1];
        const glm::vec2& uv2 = outUVs[i + 2];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        float denom = deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x;
        if (fabs(denom) < 1e-6f) denom = 1.0f;
        float f = 1.0f / denom;

        glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
        glm::vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);

        for (int j = 0; j < 3; ++j) {
            tangents[i + j] = tangent;
            bitangents[i + j] = bitangent;
        }
    }
    return true;
}

void Model::addTexture(const char* path, const std::string type)
{
    Texture texture;
    texture.id = loadTexture(path, type);
    texture.type = type;
    textures.push_back(texture);
}

unsigned int Model::loadTexture(const char* path, const std::string& type)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, numComponents;
    unsigned char* data = stbi_load(path, &width, &height, &numComponents, 0);
    if (data)
    {
        GLenum format;
        if (numComponents == 1)
            format = GL_RED;
        else if (numComponents == 3)
            format = GL_RGB;
        else if (numComponents == 4)
            format = GL_RGBA;

        GLenum internalFormat = format;
        if (type == "diffuse") {
            if (format == GL_RGB) internalFormat = GL_SRGB;
            else if (format == GL_RGBA) internalFormat = GL_SRGB_ALPHA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture " << path << " failed to load." << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
