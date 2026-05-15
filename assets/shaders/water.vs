#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform float waveSpeed;
uniform float waveStrength;

void main() {
    vec3 pos = aPos;
    // Simple Sine Wave Displacement
    pos.y += sin(pos.x * 0.5 + time * waveSpeed) * waveStrength;
    pos.y += cos(pos.z * 0.5 + time * waveSpeed) * waveStrength;

    FragPos = vec3(model * vec4(pos, 1.0));
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
