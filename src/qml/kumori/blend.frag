/**************************************************************************
 *  blend.frag
 *
 *  Copyright 2024 Gabriel Machado
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 **************************************************************************/


#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform sampler2D source;
uniform sampler2D foregroundSource;
varying vec2 qt_TexCoord0;
uniform float qt_Opacity;

#ifndef BLEND
#define BLEND(bg, fg) (bg * bg * fg.a)
#endif

void main(void) {
    vec4 bgColor = texture2D(source, qt_TexCoord0.st);
    vec4 fgColor = texture2D(foregroundSource, qt_TexCoord0.st);
    gl_FragColor = (BLEND(bgColor, fgColor)) * qt_Opacity;
}
