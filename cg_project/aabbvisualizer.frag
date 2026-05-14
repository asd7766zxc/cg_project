#version 430 core

in vec3 pixelPos;
in vec3 pixelNorm;
out vec4 color;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.0, 0.5));
    float diff = max(dot(normalize(pixelNorm), lightDir), 0.3); // simple shading

    // semi-transparent green
    color = vec4(vec3(0.1, 0.8, 0.2) * diff, 0.8);
}