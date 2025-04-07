#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(set = 0, binding = 1) uniform sampler texSampler;
layout(set = 0, binding = 2) uniform texture2D colorTex;
layout(set = 0, binding = 3) uniform texture2D normalTex;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 colorTexValue = texture(sampler2D(colorTex, texSampler), fragTexCoord).rgb;
    vec3 normalTexValue = texture(sampler2D(normalTex, texSampler), fragTexCoord).rgb;
    outColor = vec4(colorTexValue * normalTexValue, 1.0f);
}
