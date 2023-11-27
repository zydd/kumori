#version 300 es

#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform sampler2D dom;
uniform sampler2D den;

in highp vec2 uv;

layout(location = 0) out highp vec4 outcolor;

void main(void) {
    vec4 _dom = texture(dom, uv);
    vec4 _den = texture(den, uv);
    outcolor = vec4(1.0, 0.0, 0.0, 1.0);
}
