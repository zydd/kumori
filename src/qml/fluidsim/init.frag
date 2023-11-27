#version 300 es

#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

in highp vec2 uv;

layout(location = 0) out highp vec4 dom_out;
layout(location = 1) out highp vec4 den_out;

void main(void) {
    dom_out = vec4(0.0, 0.0, 1.0, 1.0);
    den_out = vec4(0.0, 0.0, 1.0, 1.0);
}
