#version 150

// Each version of OpenGL has its own version of the shader language with availability of a certain feature set and we will be using GLSL 1.50. This version number
// may seem a bit off when we're using OpenGL 3.2, but that's because shaders were only introduced in OpenGL 2.0 as GLSL 1.10. Starting from OpenGL 3.3, this problem
// was solved and the GLSL version is the same as the OpenGL version

// It was expressed that some drivers required this next line to function properly
precision highp float;

in vec4 ex_Color;
in vec2 tex_Coords;

uniform sampler2D tex;

out vec4 fragColor;

void main(void) {
    fragColor = texture(tex, tex_Coords) * ex_Color;
}