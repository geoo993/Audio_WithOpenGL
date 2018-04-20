/*

	Copyright 2011 Etay Meiri

	// Modified by Dr Greg Slabaugh to work with OpenGL template

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Texture.h"
#include "Transform.h"
#include "Shaders.h"
#include "Camera.h"

#define INVALID_OGL_VALUE 0xFFFFFFFF
#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }


struct Vertex
{
    glm::vec3 m_pos;
    glm::vec2 m_tex;
    glm::vec3 m_normal;
    glm::vec3 m_tangent;
    glm::vec3 m_bitangent;

    Vertex() {}

    Vertex(const glm::vec3& pos, const glm::vec2& tex, const glm::vec3& normal, const glm::vec3& tangent, const glm::vec3& bitangent)
    {
        m_pos    = pos;
        m_tex    = tex;
        m_normal = normal;
        m_tangent = tangent;
        m_bitangent = bitangent;
    }
};


struct MeshEntry {

    MeshEntry();

    ~MeshEntry();

    void Init(const std::vector<Vertex>& Vertices,
              const std::vector<GLuint>& Indices,
              const std::vector<glm::mat4> &mMatrices);
    GLuint vbo;
    GLuint mbo;
    GLuint ibo;
    GLuint m_NumIndices;
    GLuint m_MaterialIndex;
};

class COpenAssetImportMesh
{
public:
    COpenAssetImportMesh();
    ~COpenAssetImportMesh();
    bool Load(const std::string& Filename);
    bool LoadWithInstances(const std::string& Filename,
                           const bool &withIntances = false,
                           const GLuint &instanceCount = 0,
                           const glm::vec3 &center = glm::vec3(0.0f),
                           const float &scale = 0.0f);
    void Render(CShaderProgram *pProgram = nullptr, CCamera *pCamera = nullptr,
                GLfloat rotation = 0, GLint helicopterRotor = 0);

    CTransform transform;

private:
    bool InitFromScene(const aiScene* pScene, const std::string& Filename);
    void InitMesh(unsigned int Index, const aiMesh* paiMesh);
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void Clear();
	

#define INVALID_MATERIAL 0xFFFFFFFF


private:
    vector<MeshEntry> m_Meshes; // meshes of the current model
    vector<CTexture*> m_Textures; // mesh textures of the current model
    vector<glm::mat4> m_ModelMatrixInstances; // number of instances of this model
    GLuint m_vao;

    CTexture* LoadMaterialTextures(const aiMaterial *pMaterial,
                                   const aiTextureType &type,
                                   const std::string &directory);
    CTexture* CreateColorTexture(const char* pMatKey,
                                 GLuint pType,
                                 GLuint pIndex,
                                 const aiMaterial* pMaterial);

    bool m_UseInstances;
    GLuint m_InstanceCount;
    glm::vec3 m_Center;
    float m_Scale;
    vector<glm::mat4> GetModelMatrixInstancesData();
};



