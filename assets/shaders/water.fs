#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform vec3 skyColorTop;
uniform float fogDensity;
uniform vec3 fogColor;

void main() {
    vec3 waterColor = vec3(0.1, 0.4, 0.7);
    
    // Simple Fresnel Approximation
    vec3 viewDir = normalize(viewPos - FragPos);
    float fresnel = pow(1.0 - max(dot(vec3(0,1,0), viewDir), 0.0), 3.0);
    
    vec3 result = mix(waterColor, skyColorTop, fresnel * 0.5);
    
    // Fog
    float dist = length(viewPos - FragPos);
    float fogFactor = exp(-dist * fogDensity);
    result = mix(fogColor, result, clamp(fogFactor, 0.0, 1.0));

    FragColor = vec4(result, 0.8);
}
