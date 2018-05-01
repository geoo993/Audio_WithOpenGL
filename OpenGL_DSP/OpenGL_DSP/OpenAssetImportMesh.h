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
#include "MeshEntry.h"

#define INVALID_OGL_VALUE 0xFFFFFFFF
#define SAFE_DELETE(p) if (p) { delete p; p = NULL; }

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
    void Render(CShaderProgram *pProgram = nullptr,
                CCamera *pCamera = nullptr,
                GLfloat helicopterRotationSpeed = 0,
                glm::vec3 helicopterPosition = glm::vec3(0.0f),
                glm::mat4 helicopterOrientation = glm::mat4(1.0f),
                GLint helicopterRotor = 0);
    
    CTransform transform;

private:
    bool InitFromScene(const aiScene* pScene, const std::string& Filename);
    void InitMesh(unsigned int Index, const aiMesh* paiMesh);
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void Clear();


#define INVALID_MATERIAL 0xFFFFFFFF

private:
    vector<MeshEntry> m_Meshes; // All the polygon meshes within this object.
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



