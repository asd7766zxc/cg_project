#version 430 core

uniform sampler3D distanceField;
in vec3 pixelPos;
in vec3 texCoord;
in vec3 pixelNorm;

out vec4 color;
uniform float slice;

void main() {
    float v = texture(distanceField,texCoord + vec3(0,0,slice)).r;
    if(v >= 0){
        color = vec4(vec3(pow(1.0 - v,10)), 1.0f);
    }else{
         color = mix(vec4(pow(1.0 + v,10),1.0f,0.0f,1.0f),vec4(vec3(pow(1.0 + v,10)),1.0f),-v);
    }
}