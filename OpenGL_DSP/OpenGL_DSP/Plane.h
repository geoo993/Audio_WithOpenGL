#pragma once

#include "Transform.h"
#include "Texture.h"
#include "VertexBufferObject.h"

// Class for generating a xz plane of a given size
class CPlane
{
public:
	CPlane();
	~CPlane();
	void Create(string sDirectory, string sFilename, float fSize, float fTextureRepeat);
	void Render(const bool &useTexture = true);
	void Release();

    CTransform transform;
    
private:
    GLuint m_vao;
    CVertexBufferObject m_vbo;
    CTexture m_texture;
    string m_directory;
    string m_filename;
    GLfloat m_size;
};
