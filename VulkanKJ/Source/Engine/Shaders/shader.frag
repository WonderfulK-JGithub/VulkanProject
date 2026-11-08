#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require 

#include "defines.glsl"

#pragma region Dependencies
#include "FragmentInputs/RegularFragmentIn.glsl"
#include "PipelineLayouts/DefaultPipelineLayout.glsl"
#pragma endregion

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform Constants {
	float aRandomValue2;
	TextureIndex baseTextureIndex;
	ColorValue colorValue;
}push;

const vec4 sunColor = vec4(1.0f);
const vec3 sunDirection = normalize(vec3(0.0f,0.0f,-1.0f));

void main() {
	vec2 texCoords = fragTexCoord;
	texCoords.x += push.aRandomValue2 * 0.1f;
	outColor = sunColor * max(0.0f,dot(fragNormal,-sunDirection)) * vec4(fragColor, 1.0) * texture(textures[push.baseTextureIndex],texCoords);
	outColor.rgb *= push.colorValue.rgb;
}