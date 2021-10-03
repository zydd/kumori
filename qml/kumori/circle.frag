#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

#define PI 3.14159265358979

uniform float qt_Opacity;
varying vec2 qt_TexCoord0;
uniform float ratio;
uniform float radius;
uniform vec2 dir;

//void main(void)
//{
////    gl_FragColor = texture2D(source, qt_TexCoord0.st);
//    gl_FragColor = vec4(0.0, sin(qt_TexCoord0.st + t)/2.0 + 0.5, qt_Opacity);
//}

vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

float rgb2v(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    return q.x;
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

// https://www.shadertoy.com/view/4djSRW
float hash11(float p) {
    p = fract(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

float noise(float t) {
    float i = floor(t);
    float f = fract(t);

    return mix(hash11(i) * f, hash11(i+1.0) * (f - 1.0), f);
}

vec3 sstep(float thr, vec3 x) {
    return sin(clamp((x - thr) * 20.0, -PI*0.5, PI*0.5));
}

void main() {
    vec2 coord = (vec2(2.0, -2.0) * qt_TexCoord0 + vec2(-1.0, 1.0)) * 6.0;
    coord.x *= ratio;

    float rho2c = length(coord + dir);
    float rho2m = length(coord);
    float rho2y = length(coord - dir);

    vec3 cmy = vec3(rho2c, rho2m, rho2y);
//    cmy = 1.0/(pow(cmy - radius, vec3(30.0)) + 1.0);
    cmy = sstep(radius - 1.0, cmy) - sstep(radius + 1.0, cmy);
//    cmy = sstep(3.0, cmy);

    vec3 rgb = vec3(1.0) - cmy;
    float alpha = max(max(cmy.x, cmy.y), cmy.z);

    gl_FragColor = vec4(rgb, alpha) * alpha * qt_Opacity;
}
