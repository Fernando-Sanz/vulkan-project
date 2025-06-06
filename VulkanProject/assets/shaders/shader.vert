#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 modelView;
    mat4 invTrans_modelView;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragTexCoord;

void main() {
    vec4 positionTemp = ubo.modelView * vec4(inPosition, 1.0);
    fragPosition = positionTemp.xyz;
    fragNormal = (ubo.invTrans_modelView * vec4(inNormal, 0.0f)).xyz;
    fragTexCoord = inTexCoord;
    gl_Position = ubo.proj * positionTemp;
}
