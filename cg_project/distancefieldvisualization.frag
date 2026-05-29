#version 430 core

uniform sampler3D distanceField;
in vec3 pixelPos;
in vec3 texCoord;
in vec3 pixelNorm;

out vec4 color;

void main() {
    float v = texture(distanceField,texCoord).r;
    color = vec4(vec3(v), 0.01);
}