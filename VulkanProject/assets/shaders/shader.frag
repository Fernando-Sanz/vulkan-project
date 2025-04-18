#version 450

//================================
// INPUT

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

//================================
// UNIFORM

layout(set = 0, binding = 1) uniform sampler texSampler;
layout(set = 0, binding = 2) uniform texture2D textures[2];

struct Light{
    vec3 pos;
    vec3 color;
    vec3 direction;
};

#define LIGHT_COUNT 2
layout(set = 0, binding = 3) uniform LightUBO {
    Light lights[LIGHT_COUNT];
} lightUBO;

//================================
// OUTPUT

layout(location = 0) out vec4 outColor;


vec3 AMBIENT_COLOR = vec3(0.15);
float SHININESS = 20.0f;

void main() {

    vec3 color = texture(sampler2D(textures[0], texSampler), fragTexCoord).rgb;
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(-fragPosition); // cameraPos - pos -> cameraPos = center of the space

    // ambient
    vec3 ambient = AMBIENT_COLOR * color;

    vec3 result = color * ambient;
    for(int i = 0; i < LIGHT_COUNT; i++){
        Light light = lightUBO.lights[i];

        vec3 lightDir = normalize(light.pos - fragPosition);
    
        // diffuse
        float diff = max(dot(lightDir, normal), 0.0);
        vec3 diffuse = diff * light.color;

        // specular
        vec3 reflectDir = reflect(-lightDir, normal);

        vec3 halfwayDir = normalize(lightDir + viewDir);

        float spec = pow(max(dot(normal, halfwayDir), 0.0), SHININESS);
        vec3 specular = light.color * spec;

        result += (color * diffuse) + specular;
    }
    outColor = vec4(result, 1.0);
}
