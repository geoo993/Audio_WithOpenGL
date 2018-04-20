//
//  DSPHelper.cpp
//  OpenGL_DSP
//
//  Created by GEORGE QUENTIN on 20/04/2018.
//  Copyright © 2018 Geo Games. All rights reserved.
//

#pragma once
#include "Common.h"

public static void ToFMODVector(glm::vec3 &glVec3, FMOD_VECTOR *fmodVec)
{
    fmodVec->x = glVec3.x;
    fmodVec->y = glVec3.y;
    fmodVec->z = glVec3.z;
}
