#include "FreeTypeFont.h"

CFreeTypeFont::CFreeTypeFont()
{
	m_isLoaded = false;
}
CFreeTypeFont::~CFreeTypeFont()
{}

/*-----------------------------------------------

Name:	createChar

Params:	iIndex - character index in Unicode.

Result:	Creates one single character (its
		texture).

/*---------------------------------------------*/

inline int next_p2(int n){int res = 1; while(res < n)res <<= 1; return res;}

void CFreeTypeFont::CreateChar(int index)
{

	FT_Load_Glyph(m_ftFace, FT_Get_Char_Index(m_ftFace, index), FT_LOAD_DEFAULT);

	FT_Render_Glyph(m_ftFace->glyph, FT_RENDER_MODE_NORMAL);
	FT_Bitmap* pBitmap = &m_ftFace->glyph->bitmap;

	int iW = pBitmap->width, iH = pBitmap->rows;
	int iTW = next_p2(iW), iTH = next_p2(iH);

	GLubyte* bData = new GLubyte[iTW*iTH];
	// Copy glyph data and add dark pixels elsewhere
	for (int ch = 0; ch < iTH; ch++) 
		for (int cw = 0; cw < iTW; cw++)
			bData[ch*iTW+cw] = (ch >= iH || cw >= iW) ? 0 : pBitmap->buffer[(iH-ch-1)*iW+cw];
 
	// And create a texture from it

	m_charTextures[index].CreateFromData(bData, iTW, iTH, 8, GL_DEPTH_COMPONENT, false);
	m_charTextures[index].SetSamplerObjectParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	m_charTextures[index].SetSamplerObjectParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	m_charTextures[index].SetSamplerObjectParameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	m_charTextures[index].SetSamplerObjectParameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Calculate glyph data
	m_advX[index] = m_ftFace->glyph->advance.x>>6;
	m_bearingX[index] = m_ftFace->glyph->metrics.horiBearingX>>6;
	m_charWidth[index] = m_ftFace->glyph->metrics.width>>6;

	m_advY[index] = (m_ftFace->glyph->metrics.height - m_ftFace->glyph->metrics.horiBearingY)>>6;
	m_bearingY[index] = m_ftFace->glyph->metrics.horiBearingY>>6;
	m_charHeight[index] = m_ftFace->glyph->metrics.height>>6;

	m_newLine = max(m_newLine, int(m_ftFace->glyph->metrics.height >> 6));

	// Rendering data, texture coordinates are always the same, so now we waste a little memory
	glm::vec2 vQuad[] =
	{
		glm::vec2(0.0f, float(-m_advY[index]+iTH)),
		glm::vec2(0.0f, float(-m_advY[index])),
		glm::vec2(float(iTW), float(-m_advY[index]+iTH)),
		glm::vec2(float(iTW), float(-m_advY[index]))
	};
	glm::vec2 vTexQuad[] = {glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f)};

	// Add this char to VBO
	for (int i = 0; i < 4; i++) {
		m_vbo.AddData(&vQuad[i], sizeof(glm::vec2));
		m_vbo.AddData(&vTexQuad[i], sizeof(glm::vec2));
	}
	delete[] bData;
}


// Loads an entire font with the given path sFile and pixel size iPXSize
bool CFreeTypeFont::LoadFont(string file, int ipixelSize)
{

	BOOL bError = FT_Init_FreeType(&m_ftLib);
	
	bError = FT_New_Face(m_ftLib, file.c_str(), 0, &m_ftFace);
	if(bError) {
		char message[1024];
		sprintf(message, "ERROR::FREETYPE: Failed to load font\n%s\n", file.c_str());
		return false;
	}
	FT_Set_Pixel_Sizes(m_ftFace, ipixelSize, ipixelSize);
	m_loadedPixelSize = ipixelSize;

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);
	m_vbo.Create();
	m_vbo.Bind();

	for (int i = 0; i < 128; i++)
		CreateChar(i);
	m_isLoaded = true;

	FT_Done_Face(m_ftFace);
	FT_Done_FreeType(m_ftLib);
	
	m_vbo.UploadDataToGPU(GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2)*2, 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2)*2, (void*)(sizeof(glm::vec2)));
	return true;
}

// Prints text at the specified location (x, y) with the given pixel size (iPXSize)
void CFreeTypeFont::Print(CShaderProgram* program, string text, int x, int y, int pixelSize)
{
	if(!m_isLoaded)
		return;

	glBindVertexArray(m_vao);
	program->SetUniform("textSampler", 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int iCurX = x, iCurY = y;
	if (pixelSize == -1)
		pixelSize = m_loadedPixelSize;
	float fScale = float(pixelSize) / float(m_loadedPixelSize);
	for (int i = 0; i < (int) text.size(); i++) {
		if (text[i] == '\n')
		{
			iCurX = x;
			iCurY -= m_newLine*pixelSize / m_loadedPixelSize;
			continue;
		}
		int iIndex = int(text[i]);
		iCurX += m_bearingX[iIndex] * pixelSize / m_loadedPixelSize;
		if(text[i] != ' ')
		{
			m_charTextures[iIndex].BindTexture2D();
			glm::mat4 mModelView = glm::translate(glm::mat4(1.0f), glm::vec3(float(iCurX), float(iCurY), 0.0f));
			mModelView = glm::scale(mModelView, glm::vec3(fScale));
			program->SetUniform("matrices.modelViewMatrix", mModelView);
			// Draw character
			glDrawArrays(GL_TRIANGLE_STRIP, iIndex*4, 4);
		}

		iCurX += (m_advX[iIndex] - m_bearingX[iIndex])*pixelSize / m_loadedPixelSize;
	}
	glDisable(GL_BLEND);
}


// Print formatted text at the location (x, y) with specified pixel size (iPXSize)
void CFreeTypeFont::Render(CShaderProgram* program, int x, int y, int pixelSize, char* text, ...)
{
    char buf[512];
    va_list ap;
    va_start(ap, text);
    vsprintf(buf, text, ap);
    va_end(ap);
    Print(program, buf, x, y, pixelSize);
}

// Deletes all font textures
void CFreeTypeFont::ReleaseFont()
{
    for (int i = 0; i < 128; i++)
        m_charTextures[i].Release();
    m_vbo.Release();
    glDeleteVertexArrays(1, &m_vao);
}

// Gets the width of text
int CFreeTypeFont::GetTextWidth(string sText, int iPixelSize)
{
    int iResult = 0;
    for (int i = 0; i < (int)sText.size(); i++)
        iResult += m_advX[sText[i]];
    return iResult*iPixelSize / m_loadedPixelSize;
}


// Loads a system font with given name (sName) and pixel size (iPXSize)
bool CFreeTypeFont::LoadSystemFont(string file, int ipixelSize)
{
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&m_ftLib)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return false;
    }

    // Load font as face
    if (FT_New_Face(m_ftLib, file.c_str(), 0, &m_ftFace)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return false;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(m_ftFace, 0, ipixelSize);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(m_ftFace, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }

        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                     GL_TEXTURE_2D,
                     0,
                     GL_RED,
                     m_ftFace->glyph->bitmap.width,
                     m_ftFace->glyph->bitmap.rows,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     m_ftFace->glyph->bitmap.buffer
                     );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(m_ftFace->glyph->bitmap.width, m_ftFace->glyph->bitmap.rows),
            glm::ivec2(m_ftFace->glyph->bitmap_left, m_ftFace->glyph->bitmap_top),
            static_cast<GLuint>(m_ftFace->glyph->advance.x)
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Destroy FreeType once we're finished
    FT_Done_Face(m_ftFace);
    FT_Done_FreeType(m_ftLib);

    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    m_vbo.Create();
    m_vbo.Bind();

    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}


void CFreeTypeFont::Render(CShaderProgram* program,string text,GLfloat x,GLfloat y,GLfloat scale)
{
    // Activate corresponding render state
    program->SetUniform("textSampler", 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glUniform3f(glGetUniformLocation(Program, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_vao);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }
        };
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        m_vbo.Bind();
        //glBindBuffer(GL_ARRAY_BUFFER, m_vboo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable(GL_BLEND);
}

