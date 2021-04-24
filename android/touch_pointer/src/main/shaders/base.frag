#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    float len = length(gl_PointCoord - vec2(0.5f));
    if (len > 0.5) {
        discard;
    }
    outColor = vec4(fragColor, 1.0);
}