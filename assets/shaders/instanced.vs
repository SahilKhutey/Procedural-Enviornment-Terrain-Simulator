#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoords;
layout (location = 4) in mat4 aInstanceMatrix; // Takes locations 4, 5, 6, 7

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;

uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform float windStrength;
uniform float windSpeed;

void main() {
    vec3 pos = aPos;

    // --- Wind Animation (Vertex Displacement) ---
    // Higher vertices sway more (simple pivot at base)
    if (pos.y > 0.1) {
        // Spatial hash for unique phase per instance
        float phase = aInstanceMatrix[3][0] * 0.5 + aInstanceMatrix[3][2] * 0.5;
        float sway = sin(time * windSpeed + phase) * windStrength;
        
        // Multi-frequency wind for more organic look
        sway += sin(time * windSpeed * 2.1 + phase * 1.5) * windStrength * 0.3;
        
        pos.x += sway * (pos.y * 0.4);
        pos.z += sway * (pos.y * 0.4);
    }

    FragPos = vec3(aInstanceMatrix * vec4(pos, 1.0));
    Normal = mat3(transpose(inverse(aInstanceMatrix))) * aNormal;
    Color = aColor;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
