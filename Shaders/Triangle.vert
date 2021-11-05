#version 450

void main() {
    // Const array of positions for the triangle
    const vec3 positions[3] = vec3[3](
        vec3(.5f,.5f, 0.0f),
        vec3(-.5f,.5f, 0.0f),
        vec3(0.f,-.5f, 0.0f)
    );

    // Output the position of each vertex
    gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
}
