#version 460

#extension GL_ARB_shading_language_include : require

layout(location = 0) in vec3 localPosition;
layout(location = 1) in vec2 localCoordinate;

layout(location = 0) out vec2 localUV;

void main()
{
	localUV = localCoordinate;

	gl_Position = vec4(localPosition.xy * 2.0, 0.0, 1.0);
}