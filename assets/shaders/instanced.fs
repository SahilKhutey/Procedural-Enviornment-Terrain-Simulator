#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;

uniform vec3 lightDir;
uniform vec3 sunColor;
uniform vec3 ambientColor;
uniform float fogDensity;
uniform vec3 fogColor;
uniform vec3 viewPos;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(-lightDir);
    float diff = max(dot(norm, lightDirection), 0.0);
    
    vec3 result = (ambientColor + diff * sunColor) * Color;
    
    // Fog
    float dist = length(viewPos - FragPos);
    float fogFactor = exp(-dist * fogDensity);
    result = mix(fogColor, result, clamp(fogFactor, 0.0, 1.0));

    FragColor = vec4(result, 1.0);
}
