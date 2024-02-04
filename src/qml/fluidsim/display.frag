/**************************************************************************
 *  display.frag
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
