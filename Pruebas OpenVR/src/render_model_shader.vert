#version 150

// in_Position was bound to attribute index 0, in_Normals was bound to attribute index 1 and in_Texcoords was bound to attribute index 2
attribute vec4 in_Position;
attribute vec3 in_Normals;
attribute vec2 in_Texcoord;
 
uniform mat4 mvp;

// We output the tex_Coords variable to the next shader in the chain
out vec2 tex_Coords;

void main(void) {

    gl_Position = mvp * vec4(in_Position.xyz, 1.0f);
 
    // Pass the normals and texture coordinates on to the fragment shader
	tex_Coords = in_Texcoord;
}