#version 150

in vec2 tex_Coords;

uniform sampler2D tex;

out vec4 fragColor;

void main(void) {
    fragColor = texture(tex, tex_Coords);
}