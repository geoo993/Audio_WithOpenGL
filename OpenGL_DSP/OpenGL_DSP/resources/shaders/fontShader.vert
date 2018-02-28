#version 400 core


// Structure for matrices
uniform struct Matrices
{
    mat4 projMatrix;
} matrices;

// Layout of vertex attributes in VBO
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>

out vec2 vTexCoord;

void main()
{
    // Pass through the texture coord
    vTexCoord = vertex.zw;

    // Transform the point
    gl_Position = matrices.projMatrix * vec4(vertex.xy, 0.0, 1.0);
}

