
#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform sampler2D source;
uniform sampler2D fg;
varying vec2 qt_TexCoord0;
uniform float qt_Opacity;

#ifndef BLEND
#define BLEND(bg, fg) (bg * bg * fg.a)
#endif

void main(void) {
    vec4 bgColor = texture2D(source, qt_TexCoord0.st);
    vec4 fgColor = texture2D(fg, qt_TexCoord0.st);
    gl_FragColor = (BLEND(bgColor, fgColor)) * qt_Opacity;
}
