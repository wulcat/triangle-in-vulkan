#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) in vec3 fragColor;

// we have to specify the output variable for each framebuffer. (location = 0) specifies the index of buffer
layout(location = 0) out vec4 outColor;


// this is called for every fragment
void main() {
	outColor = vec4(fragColor, 1.0);

}