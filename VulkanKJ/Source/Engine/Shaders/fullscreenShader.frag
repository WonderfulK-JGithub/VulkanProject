#version 450

#extension GL_EXT_nonuniform_qualifier : require 

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(set = 1,binding = 0) uniform sampler2D meshTextures[];

layout(push_constant) uniform Constants {
	uint baseTextureIndex;
}push;

void main() {
	outColor = texture(meshTextures[push.baseTextureIndex],uv);
}