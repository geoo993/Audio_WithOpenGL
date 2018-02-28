#pragma once

#include "Texture.h"
#include "Shaders.h"
#include "VertexBufferObject.h"


// This class is a wrapper for FreeType fonts and their usage with OpenGL
class CFreeTypeFont
{
public:
	CFreeTypeFont();
	~CFreeTypeFont();

	bool LoadFont(string file, int pixelSize);
	bool LoadSystemFont(string file, int pixelSize);

    int GetTextWidth(string text, int pixelSize);

    void Print(CShaderProgram* program, string text, int x, int y, int pixelSize = -1);
    void Render(CShaderProgram* program, int x, int y, int pixelSize, char* text, ...);
    void Render(CShaderProgram* program, string text, GLfloat x, GLfloat y, GLfloat scale);
    void ReleaseFont();


private:

	void CreateChar(int index);

	CTexture m_charTextures[256];
	int m_advX[256], m_advY[256];
	int m_bearingX[256], m_bearingY[256];
	int m_charWidth[256], m_charHeight[256];
	int m_loadedPixelSize, m_newLine;

    bool m_isLoaded;
	GLuint m_vao;
	CVertexBufferObject m_vbo;

	FT_Library m_ftLib;
	FT_Face m_ftFace;



    // Second way of loading font
    struct Character {
        GLuint   TextureID;    // ID handle of the glyph texture
        glm::ivec2 Size;       // Size of glyph
        glm::ivec2 Bearing;    // Offset from baseline to left/top of glyph
        GLuint     Advance;    // Offset to advance to next glyph
    };

    std::map<GLchar, Character> Characters;
};
