//
//  CircularBuffer.cpp
//  OpenGL_DSP
//
//  Created by GEORGE QUENTIN on 15/04/2018.
//  Copyright © 2018 Geo Games. All rights reserved.
//

#include "CircularBuffer.h"

CCircularBuffer::CCircularBuffer(const GLuint &size)
{
    m_size = size;
    m_tail = 0;
    m_buffer = new float[size];
    m_bufferValues = *new list<GLfloat>();
}

CCircularBuffer::~CCircularBuffer()
{
    delete m_buffer;
    m_bufferValues.clear();
}

void CCircularBuffer::Put(const GLfloat &value) {
    m_buffer[m_tail % m_size] = value;
    m_bufferValues.push_back(value);
    m_tail++;
}

GLfloat CCircularBuffer::AtPosition(const GLuint &position) {
    if (position > m_tail || position < 0) {
        return -1;
    }
    return m_buffer[position % m_size];
}

GLuint CCircularBuffer::GetTail() {
    return m_tail;
}

GLuint CCircularBuffer::GetSize() {
    return m_tail % m_size;
}
