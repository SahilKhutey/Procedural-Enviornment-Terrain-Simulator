#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

uniform vec3 lightDir;
uniform vec3 viewPos;
uniform vec3 ambientColor;
uniform vec3 sunColor;
uniform float fogDensity;
uniform vec3 fogColor;

void main() {
    // Ambient
    vec3 ambient = ambientColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(-lightDir);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * sunColor;
    
    // Result
    vec3 result = (ambient + diffuse) * Color;
    
    // Fog calculation
    float dist = length(viewPos - FragPos);
    float fogFactor = exp(-dist * fogDensity);
    result = mix(fogColor, result, clamp(fogFactor, 0.0, 1.0));

    FragColor = vec4(result, 1.0);
}
