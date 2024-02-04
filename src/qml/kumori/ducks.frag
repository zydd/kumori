precision highp float;

uniform vec2 c;
uniform int iter;
uniform float esc;
uniform int pre;
uniform float col;
uniform float col_shift;
uniform float col_shift_r;
uniform float col_shift_g;
uniform float col_shift_b;
uniform float col_variation;
uniform float zoom;
uniform float map_size;
uniform float map_zoom;
uniform float ratio;
uniform vec2 center;
uniform float rotation;
uniform vec2 pixel;

varying vec2 qt_TexCoord0;

const float pi = 3.14159265358979323846;

/***********************************************************************
 * Math functions
 */

mat2 rot(float a) {
    float s = sin(a);
    float c = cos(a);
    return mat2(c, s, -s, c);
}

float carg(vec2 z)  { return atan(z.y, z.x); }
vec2 cconj(vec2 z)  { return vec2(z.x, -z.y); }

vec2 cmul(vec2 a, vec2 b) {
    return vec2(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}

vec2 csqr(vec2 a) {
    return cmul(a, a);
}

vec2 clog(vec2 z) {
    z = vec2(log(length(z)), atan(z.y, z.x));
    if (z.y > 0.0)
        z.y -= 2.0 * pi;
    return z;
}

vec2 cdiv(vec2 a, vec2 b) {
    float d = 1.0 / dot(b, b);
    return vec2(a.x*b.x + a.y*b.y, a.y*b.x - a.x*b.y) * d;
}

vec2 crec(vec2 z) {
  float d = 1.0 / dot(z, z);
  return vec2(z.x, -z.y) * d;
}

float cosh(float z) {
    z = exp(z);
    return (z + 1.0 / z) * 0.5;
}

float tanh(float z) {
    z = exp(z);
    float rec_z = 1.0 / z;
    return (z - rec_z) / (z + rec_z);
}

float sinh(float z) {
    z = exp(z);
    return (z - 1.0 / z) * 0.5;
}

vec2 csinh(vec2 z) {
  return vec2(sinh(z.x) * cos(z.y), cosh(z.x) * sin(z.y));
}

vec2 ccosh(vec2 z) {
  return vec2(cosh(z.x) * cos(z.y), sinh(z.x) * sin(z.y));
}

vec2 ctanh(vec2 z) {
  float chr = cosh(z.x);
  float si = sin(z.y);
  float d = 1.0 / (chr * chr + si * si);
  return vec2(sinh(z.x) * chr, si * chr) * d;
}

vec2 ccos(vec2 z) {
    return vec2(cos(z.x) * cosh(z.y), -sin(z.x) * sinh(z.y));
}

vec2 csin(vec2 z) {
    return vec2(sin(z.x) * cosh(z.y), cos(z.x) * sinh(z.y));
}

vec2 ctan(vec2 z) {
    return cdiv(csin(z), ccos(z));
}


/***********************************************************************
 * Coloring
 */

vec3 plasma(float x) {
    vec4 c0 = vec4(0.050383, 0.029803, 0.527975, 1.0);
    vec4 c1 = vec4(0.494877, 0.01199, 0.657865, 1.0);
    vec4 c2 = vec4(0.798216, 0.280197, 0.469538, 1.0);
    vec4 c3 = vec4(0.973416, 0.585761, 0.25154, 1.0);
    vec4 c4 = vec4(0.063536, 0.028426, 0.533124, 1.0);

    float y = (x + 1.0) * 2.0;
    vec4 c = mix(c0, c1, clamp(y, 0.0, 1.0));
    c = mix(c, c2, clamp(y - 1.0, 0.0, 1.0));
    c = mix(c, c3, clamp(y - 2.0, 0.0, 1.0));
    c = mix(c, c4, clamp(y - 3.0, 0.0, 1.0));

    return c.rgb;
}

vec3 twilight(float x) {
    vec4 c0 = vec4(0.88575016, 0.85000925, 0.88797365, 1.0);
    vec4 c1 = vec4(0.38407269, 0.46139019, 0.73094665, 1.0);
    vec4 c2 = vec4(0.18488036, 0.07942573, 0.21307652, 1.0);
    vec4 c3 = vec4(0.69806082, 0.33828976, 0.32207479, 1.0);
    vec4 c4 = vec4(0.88571155, 0.85002186, 0.88572539, 1.0);

    float y = (x + 1.0) * 2.0;
    vec4 c = mix(c0, c1, clamp(y, 0.0, 1.0));
    c = mix(c, c2, clamp(y - 1.0, 0.0, 1.0));
    c = mix(c, c3, clamp(y - 2.0, 0.0, 1.0));
    c = mix(c, c4, clamp(y - 3.0, 0.0, 1.0));

    return c.rgb;
}

vec3 color_norm_itr_count(vec2 z, int i, float mean) {
    const float k = 2.0; // power
    float sn = float(i) - log2(log2(dot(z,z))) + 4.0;
    float sit = float(i) - log2(log2(dot(z,z))/(log2(esc)))/log2(k);
    vec3 color = vec3(0.0);
    if (i < iter)
        color = 0.5 + 0.5*cos(sit*0.075*k - vec3(col_shift_r, col_shift_g, col_shift_b));
    return color;
}

vec3 color_norm_itr_count2(vec2 z, int i, float mean) {
    const float k = 2.0; // power
    float sn = float(i) - log2(log2(dot(z,z))) + 4.0;
    float sit = float(i) - log2(log2(dot(z,z))/(log2(esc)))/log2(k);
    vec3 color = vec3(0.0);
    if (i < iter)
        color = 0.5 + 0.5*cos(1.9 + sit*0.075*k + vec3(0.0, 0.4, 0.87931));
    return color;
}

vec3 color_cyclic_log_log_yb(vec2 z, int i, float m) {
    float ci = col * log2(1.0 + log2(1.0 + m)) - col_shift;
    return 0.5 + 0.5 * cos(ci - vec3(col_shift_r, col_shift_g, col_shift_b));
}

vec3 color_cyclic_log_log_yb2(vec2 z, int i, float m) {
    float ci = col * log2(1.0 + log2(1.0 + m)) - col_shift;
    return 0.5 + 0.5 * cos(ci + vec3(pi - 0.87931, pi - 0.4, pi - 0.0));
}

vec3 color_cyclic_log_log_pg(vec2 z, int i, float m) {
    float ci = col_variation * (float(i) - log2(log2(m * col))) - col_shift;
    return 0.5 + 0.5 * cos(ci + vec3(0.4, 0.87931, 0.0));
}

vec3 color_combined(vec2 z, int i, float mean) {
    if (i < iter)
        return color_norm_itr_count(z, i, mean);
    else
        return color_cyclic_log_log_yb(z, i, mean);
}

vec3 color_combined2(vec2 z, int i, float mean) {
    if (i < iter)
        return color_norm_itr_count2(z, i, mean);
    else
        return color_cyclic_log_log_yb2(z, i, mean);
}

vec3 color_clamp_cyclic_log(vec2 z, int i, float m) {
    return vec3(1.0-sin(float(i) - clamp(2.0 * log2(m * col), 0.0, 1.0) * pi*0.5 - col_shift));
}

vec3 color_cyclic_log(vec2 z, int i, float m) {
    return vec3(0.5+0.5*sin(float(i) - log2(m * col) - col_shift));
}

vec3 color_iteration(vec2 z, int i, float mean) {
    return vec3(pow(float(i) / float(iter) * col * 1e-3, col_shift));
}

/***********************************************************************
 * Fractal
 */

// #define fractal_fn(z, c) clog(vec2(z.x, abs(z.y))) + c // ***** ducks
// #define fractal_fn(z, c) cmul(abs(z), abs(z))+c // burning ship
// #define fractal_fn(z, c) cmul(z,z)+c // mandelbrot

// #define fractal_fn(z, c) clog(vec2(z.x, pow(abs(z.y), 1.1)))+c // **** duck^1.1 rounder
// #define fractal_fn(z, c) clog(vec2(z.x, pow(abs(z.y), 0.9)))+c // **** duck^0.9 point, warped
// #define fractal_fn(z, c) clog(crec(vec2(z.x, abs(z.y))))+c // **** antiduck
// #define fractal_fn(z, c) clog(vec2(z.y, abs(z.x)))+c // **** duckcud
// #define fractal_fn(z, c) clog(abs(crec(z)) + c) // ** kaleidoscopic
// #define fractal_fn(z, c) crec(vec2(z.y, abs(z.x)))+c // ***
// #define fractal_fn(z, c) clog(vec2(cmul(z, z).x, abs(cmul(z, z).y)))+c // **
// #define fractal_fn(z, c) clog(crec(cmul(z, z)))+c // ** discontinuous, high entropy
// #define fractal_fn(z, c) clog(vec2(z.x, z.y * z.y))+c // ** duck^2 interesting map
// #define fractal_fn(z, c) clog(vec2(z.x, sin(z.y)))+c // *** duck sin - gravitational lensing
// #define fractal_fn(z, c) clog(vec2(z.x, z.x * z.y))+c // ** duck xy - 1/x
// #define fractal_fn(z, c) clog(vec2(abs(z.x), z.y))+c // ** duck rabs - kaleidoscopic
// #define fractal_fn(z, c) clog(vec2(sin(z.x), abs(z.y)))+c // ** duck rsin - high entropy, horizontal repeat
// #define fractal_fn(z, c) clog(vec2(z.x+z.y, abs(z.y)))+c // ** duck x+y - high entropy, slanted
// #define fractal_fn(z, c) clog(vec2(z.x*z.y, abs(z.y)))+c // * duck xy - high entropy, distorted
// #define fractal_fn(z, c) clog(vec2(z.x*z.y, z.y))+c // ** duck xy - gaussian
// #define fractal_fn(z, c) clog(vec2(pow(z.x, 1.1), pow(abs(z.y), 1.1)))+c // * duck^1.1^2 missing left half


// chatgpt

// #define fractal_fn(z, c) clog(vec2(z.x * z.y, z.y * z.y - z.x * z.x)) + c  // ** distorted, high entropy
// #define fractal_fn(z, c) clog(vec2(cos(z.x), sin(z.y))) + c  // ** mandelbrot 4x4 lattice
// #define fractal_fn(z, c) clog(vec2(cos(z.y), sin(z.x))) + c  // ** julia 4x4 lattice
// #define fractal_fn(z, c) clog(vec2(abs(z.x), cos(z.y))) + c  // ** target superposition
// #define fractal_fn(z, c) clog(vec2(sin(z.x*z.y), cos(z.x*z.y))) + c  // ** hyperbolic sauron
// #define fractal_fn(z, c) clog(vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y)) + c // *** kaleidoscopic bird
// #define fractal_fn(z, c) clog(vec2(cos(z.x) * cosh(z.y), sin(z.x) * sinh(z.y))) + c  // *** some cool patterns, 4x4 lattice, discontinuous, hard to navigate
// #define fractal_fn(z, c) clog(vec2(sin(z.x)*cosh(z.y), cos(z.x)*sinh(z.y))) + c  // ** worse version of previous one
// #define fractal_fn(z, c) clog(vec2(exp(-z.y)*cos(z.x), exp(-z.y)*sin(z.x))) + c  // *** 4x4 grid
// #define fractal_fn(z, c) clog(vec2(z.x + cos(z.y), z.y + sin(z.x))) + c


vec3 fractal(vec2 coord, vec2 c) {
    vec2 z = coord;
    float mean = 0.0;
    int i;
    for(i = 0; i < iter; ++i) {
        z = fractal_fn(z, coord);
        float l = length(z);

        if (i > pre) {
            mean += l;

            if (l > esc)
                break;
        }
    }
    mean /= float(i - pre);
    return get_color(z, i, mean);
}


vec3 julia(vec2 coord, vec2 c) {
    vec2 z = coord;
    float mean = 0.0;
    int i;
    for(i = 0; i < iter; ++i) {
        z = fractal_fn(z, c);
        float l = length(z);

        if (i > pre) {
            mean += l;

            if (l > esc)
                break;
        }
    }
    mean /= float(i - pre);

     float ci =  1.0 - log2(.5*log2(mean * 5.0));
    //  return vec3( .5+.5*cos(6.*ci+0.5),.5+.5*cos(6.*ci + 0.33),.5+.5*cos(6.*ci + 0.32) );
    // return vec3(0.55 + 0.45 * sin(clamp(1.95 * (mean - 0.9), 0.0, 1.0) * pi * 4.0));

    return get_color(z, i, mean);
}

vec2 uv2coord(vec2 uv) {
    vec2 coord = uv * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    coord.x *= ratio;
    return rot(rotation) * coord * zoom + center;
}

void main() {
#if defined(MULTISAMPLE_W) && defined(MULTISAMPLE_H) && (MULTISAMPLE_W > 1 || MULTISAMPLE_H > 1)
    vec2 d = pixel * vec2(1.0/float(MULTISAMPLE_W), 1.0/float(MULTISAMPLE_H)) * vec2(2.0 * ratio, -2.0) * zoom;

    vec2 coord = uv2coord(qt_TexCoord0) - d * 0.5 * vec2(float(MULTISAMPLE_W), float(MULTISAMPLE_H));

    vec3 color = vec3(0.0);
    int i, j;
    for (i = 0; i < MULTISAMPLE_W; ++i) {
        for (j = 0; j < MULTISAMPLE_H; ++j) {
            color += main_image(vec2(coord.x, coord.y + float(j) * d.y), c);
        }

        coord.x += d.x;
    }

    color /= float(MULTISAMPLE_W * MULTISAMPLE_H);

#else
    vec3 color = main_image(uv2coord(qt_TexCoord0), c);
#endif

#if SHOW_MAP
    vec2 mapc = qt_TexCoord0 - vec2(1.0 - abs(map_size/ratio), 1.0 - map_size);
    float mapd = length(vec2(mapc.x * ratio, mapc.y));
    if (mapd < map_size) {
        vec2 uv = mapc + vec2(0.5, 0.5);
        vec2 coord = uv * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
        coord.x *= ratio;
        coord = coord * map_zoom + c;
        color = mini_image(coord, center);

        if (mapd < 2.0 * max(pixel.x, pixel.y))
        color = 1.0 - color;
    }
#endif

    gl_FragColor = vec4(color, 1.0);
}

