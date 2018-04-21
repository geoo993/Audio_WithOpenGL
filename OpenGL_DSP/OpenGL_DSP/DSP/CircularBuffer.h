//
//  CircularBuffer.hpp
//  OpenGL_DSP
//
//  Created by GEORGE QUENTIN on 15/04/2018.
//  Copyright © 2018 Geo Games. All rights reserved.
//

#pragma once
#include "Common.h"

#ifndef CircularBuffer_h
#define CircularBuffer_h

class CCircularBuffer
{
public:
    CCircularBuffer(const GLuint &size);
    ~CCircularBuffer();
    void Put(const GLfloat &value);
    void PutAt(const GLint &index, const GLfloat &value);
    GLfloat AtPosition(const GLuint &position);
    GLuint GetTail();
    GLuint GetSize();


private:
    GLuint m_size;
    GLfloat *m_buffer;
    GLuint m_tail;
    list<GLfloat> m_bufferValues;
};

#endif /* CircularBuffer_h */
