#version 330 core
out vec4 FragColor;
in vec3 TexCoords;

uniform vec3 skyColorTop;
uniform vec3 skyColorBottom;
uniform vec3 sunDir;

void main() {
    float height = normalize(TexCoords).y;
    float t = clamp(height * 0.5 + 0.5, 0.0, 1.0);
    vec3 skyColor = mix(skyColorBottom, skyColorTop, t);
    
    // Simple Sun Glow
    float sunGlow = pow(max(dot(normalize(TexCoords), sunDir), 0.0), 32.0);
    skyColor += sunGlow * vec3(1.0, 0.8, 0.4);
    
    FragColor = vec4(skyColor, 1.0);
}
