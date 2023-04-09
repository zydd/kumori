#version 140

uniform sampler2D source;
varying vec2 qt_TexCoord0;

float lin1(float c) {
    return c <= 0.04045 ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
}
float flin1(float c) {
    return pow(c, 2.2);
}

float srgb1(float c) {
    return c < 0.0031308 ? c * 12.92 : 1.055 * pow(c, 0.41667) - 0.055;
}
float fsrgb1(float c){
    return pow(c, 0.45455);
}

vec3 lin3(vec3 c) {
    return vec3(lin1(c.r), lin1(c.g), lin1(c.b));
}
vec3 flin3(vec3 c) {
    return vec3(flin1(c.r), flin1(c.g), flin1(c.b));
}

vec3 srgb3(vec3 c){
    return vec3(srgb1(c.r), srgb1(c.g), srgb1(c.b));
}
vec3 fsrgb3(vec3 c){
    return vec3(fsrgb1(c.r), fsrgb1(c.g), fsrgb1(c.b));
}

vec3 scanline(vec3 src, float y) {
    y = abs((int(y - 1) % 3) - 1);
    float y_i = 2 - y;
    return src * y + clamp(src * y_i - 1, 0, 0.8);
}

vec3 rgbmask(vec2 pos) {
    vec3 mask = vec3(1.0);

    pos.x = fract(pos.x / 3);
    if (pos.x < 0.33333)
        mask.b = 0.5;
    else
        mask.b = 1.25;

    return mask;
}

void main(void) {
    vec4 src = texture2D(source, qt_TexCoord0.st).rgba;
    src.rgb = flin3(src.rgb);
    src.rgb = scanline(src.rgb, gl_FragCoord.y) * rgbmask(gl_FragCoord.xy);
    src.rgb = fsrgb3(src.rgb);
    gl_FragColor = src;
}
