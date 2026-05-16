#version 430 core

in vec3 pixelPos;
in vec3 pixelNorm;
out vec4 color;

uniform vec4 solid_color;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.0, 0.5));
    float diff = max(dot(normalize(pixelNorm), lightDir), 0.3); // simple shading

    color = vec4(solid_color.xyz * diff,  solid_color.w);
}