#version 430 core

uniform sampler3D distanceField;
in vec3 pixelPos;
in vec3 texCoord;
in vec3 pixelNorm;

out vec4 color;

void main() {
    float v = texture(distanceField,texCoord + vec3(0,0,-0.5)).r;
    color = vec4(vec3(pow(1.0 - v,10)), 1);
}