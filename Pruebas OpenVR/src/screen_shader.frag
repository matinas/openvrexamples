#version 150

// It was expressed that some drivers required this next line to function properly
precision highp float;

in vec4 ex_Color;
in vec2 tex_Coords;

out vec4 fragColor;

uniform sampler2D frame_buffer_tex;

bool invertColor = false;
bool greyscale = false;
bool blur = false;
bool sobel = false;
bool outline = false;

void main(void) {

	// If outline is enabled we draw a little outline at frontier pixels
	if (outline)
		if ((tex_Coords.x < 0.005f) || (tex_Coords.y < 0.005f) || (tex_Coords.x > 0.995f) || (tex_Coords.y > 0.995f)) {
			fragColor = vec4(0.0f,0.0f,0.0f,1.0f);
			return;
		}

	if (invertColor)
	{
		fragColor = vec4(1.0,1.0,1.0,1.0) - texture(frame_buffer_tex, tex_Coords); // If we enable alpha blending this doesn't work because also affects the alpha channel (both in 1.0)
	}
	else if (greyscale)
	{
		vec4 tex_color = texture(frame_buffer_tex, tex_Coords);
		// float average = (tex_color.r + tex_color.g + tex_color.b) / 3.0f; // This works fine, but humans are the most sensitive to green and the least to blue, so a better conversion would work with weighed channels
		float average = (0.2126*tex_color.r + 0.7152*tex_color.g + 0.0722*tex_color.b) / 3.0f;
		average*= 3.f; // Increase a little the overall intensity so it isn't much dark
		fragColor = vec4(average,average,average,1.0);
	}
	else if (blur)
	{
		// There are two well known blur techniques: box blur and Gaussian blur. The latter results in a higher quality result, but the former is easier to
		// implement and still approximates Gaussian blur fairly well. Blurring is done by sampling pixels around a pixel and calculating the average color

		const float blurSizeH = 1.0/800; // The blurSize variables determine the distance between each sample
		const float blurSizeV = 1.0/600;
		vec4 sum = vec4(0.0);
		for (int x = -4; x <= 4; x++)
			for (int y = -4; y <= 4; y++)
				sum += texture(frame_buffer_tex, vec2(tex_Coords.x + x * blurSizeH, tex_Coords.y + y * blurSizeV)) / 81.0;

		fragColor = sum;
	}
	else if (sobel)
	{
		vec4 top         = texture(frame_buffer_tex, vec2(tex_Coords.x, tex_Coords.y + 1.0 / 200.0));
		vec4 bottom      = texture(frame_buffer_tex, vec2(tex_Coords.x, tex_Coords.y - 1.0 / 200.0));
		vec4 left        = texture(frame_buffer_tex, vec2(tex_Coords.x - 1.0 / 300.0, tex_Coords.y));
		vec4 right       = texture(frame_buffer_tex, vec2(tex_Coords.x + 1.0 / 300.0, tex_Coords.y));
		vec4 topLeft     = texture(frame_buffer_tex, vec2(tex_Coords.x - 1.0 / 300.0, tex_Coords.y + 1.0 / 200.0));
		vec4 topRight    = texture(frame_buffer_tex, vec2(tex_Coords.x + 1.0 / 300.0, tex_Coords.y + 1.0 / 200.0));
		vec4 bottomLeft  = texture(frame_buffer_tex, vec2(tex_Coords.x - 1.0 / 300.0, tex_Coords.y - 1.0 / 200.0));
		vec4 bottomRight = texture(frame_buffer_tex, vec2(tex_Coords.x + 1.0 / 300.0, tex_Coords.y - 1.0 / 200.0));
		vec4 sx = -topLeft - 2 * left - bottomLeft + topRight   + 2 * right  + bottomRight;
		vec4 sy = -topLeft - 2 * top  - topRight   + bottomLeft + 2 * bottom + bottomRight;
		vec4 sobel = sqrt(sx * sx + sy * sy);
		fragColor = sobel;
	}
	else
		fragColor = texture(frame_buffer_tex, tex_Coords);
}