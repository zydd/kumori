uniform sampler2D source;
uniform sampler2D bg;
varying vec2 qt_TexCoord0;
uniform float qt_Opacity;

float lum(vec3 c) {
    return dot(vec3(0.212655, 0.715158, 0.072187), c);
}

float con(float c) {
    float a = 9.0;
    return a * (c - 0.5) - 0.5;
}

float step(float c) {
    float d = 0.9;

    float r = c * (1.0 - d);
    if (c > 0.5)
        r += d;
    return r;
}

void main(void) {
    float alpha = texture2D(source, qt_TexCoord0.st).a;
    vec3 col = texture2D(bg, qt_TexCoord0.st).rgb;

    float l = lum(col);

    l = 1.0 - step(l);

    gl_FragColor = vec4(vec3(l), 1.0) * alpha * qt_Opacity;
}
