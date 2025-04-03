#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(binding = 0) uniform sampler2D previousFrame;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(previousFrame, fragTexCoord);
}
