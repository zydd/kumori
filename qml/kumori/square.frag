uniform sampler2D source;
uniform sampler2D bg;
varying vec2 qt_TexCoord0;
uniform float qt_Opacity;

void main(void) {
    float src0 = texture2D(source, qt_TexCoord0.st).a;
    vec3 col = texture2D(bg, qt_TexCoord0.st).rgb;
    gl_FragColor = vec4(col * col, 1.0) * src0 * qt_Opacity;
}
