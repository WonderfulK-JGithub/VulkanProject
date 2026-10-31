#version 450

// vulkan NDC:	x: -1(left), 1(right)
//				y: -1(top), 1(bottom)

layout(set = 0,binding = 0) uniform CameraVectors
{
	vec4 right;
	vec4 up;
	vec4 forward;
} cameraData;

layout(location = 0) out vec2 uv;

const vec2 screen_corners[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2(-1.0,  1.0),
	vec2( 1.0,  1.0),
	vec2( 1.0,  1.0),
	vec2( 1.0, -1.0),
	vec2(-1.0, -1.0)
);

void main() {
	vec2 pos = screen_corners[gl_VertexIndex];
	gl_Position = vec4(pos, 0.0, 1.0);
	uv.x = (pos.x + 1.f) / 2.f;
	uv.y = (pos.g + 1.f) / 2.f;
}