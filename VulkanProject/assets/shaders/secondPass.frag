#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(set = 0, binding = 0) uniform sampler texSampler;
layout(set = 0, binding = 1) uniform texture2D textures[1];

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(sampler2D(textures[0], texSampler), fragTexCoord);
    //outColor = texture(previousFrame, fragTexCoord);
}
