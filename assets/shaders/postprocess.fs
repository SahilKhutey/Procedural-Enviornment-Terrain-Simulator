#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float exposure;
uniform float bloomStrength;
uniform vec3 tint;

// ACES Tone Mapping Approximation
vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main() {
    vec3 sceneColor = texture(screenTexture, TexCoords).rgb;
    
    // 1. Exposure Scaling
    sceneColor *= exposure;

    // 2. Simple Bloom (Cheap version: Glow bright areas)
    float brightness = dot(sceneColor, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0) {
        sceneColor += sceneColor * bloomStrength;
    }

    // 3. ACES Tone Mapping
    vec3 result = ACESFilm(sceneColor);

    // 4. Color Grading (Tint)
    result *= tint;

    // 5. Vignette Effect
    vec2 uv = TexCoords * (1.0 - TexCoords.yx);
    float vign = uv.x * uv.y * 15.0;
    vign = pow(vign, 0.25);
    result *= vign;

    // 6. Gamma Correction
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}
