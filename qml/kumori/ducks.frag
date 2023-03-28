precision highp float;

uniform vec2 c;
uniform int iter;
uniform float esc;
uniform int pre;
uniform float col;
uniform float col_shift;
uniform float zoom;
uniform float map_size;
uniform float map_zoom;
uniform float ratio;
uniform vec2 center;
uniform float rotation;
uniform vec2 pixel;

varying vec2 qt_TexCoord0;

const float pi = 3.14159265358979323846;

vec2 clog(vec2 z) {
    z = vec2(log(length(z)), atan(z.y, z.x));
    if (z.y > 0.0)
        z.y -= 2.0 * pi;
    return z;
}

vec2 cmul(vec2 a, vec2 b) {
    return vec2(a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x);
}

mat2 rot(float a) {
    float s = sin(a);
    float c = cos(a);
    return mat2(c, s, -s, c);
}

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

vec3 getColor(int i, float m, float col) {
    float ci = float(i) + 1.0 - log2(.5 * log2(m * col)) + col_shift;
    return vec3(0.5 + 0.5 * cos(6.0 * ci),
                0.5 + 0.5 * cos(6.0 * ci + 0.4),
                0.5 + 0.5 * cos(6.0 * ci + 0.87931));
//    return vec3(0.5 + 0.5 * cos(6.0 * ci + 0.4),
//                0.5 + 0.5 * cos(6.0 * ci + 0.87931),
//                0.5 + 0.5 * cos(6.0 * ci));
}
// non-standard branch cut, bad PI value too...
vec2 cLog(vec2 a) {
  float b =  atan(a.y,a.x);
  if (b>0.0) b-=2.0*pi;
  return vec2(log(length(a)),b);
}
vec3 ducks(vec2 z) {
    float mean = 0.0;
    int i;
    for(i = 0; i < iter; ++i) {
        z = clog(vec2(z.x, abs(z.y))) + c;
        float l = length(z);
        
        if (i > pre) {
            mean += l;

            if (l > esc)
                break;
        }
    }
    mean /= float(i - pre);
    return getColor(i, mean, col);
}

vec3 map(vec2 coord) {
    vec2 z = coord;
    float mean = 0.0;
    int i;
    for(i = 0; i < iter; ++i) {
        z = clog(vec2(z.x, abs(z.y))) + coord;
        float l = length(z);
        
        if (i > pre) {
            mean += l;

            if (l > esc)
                break;
        }
    }
    mean /= float(i - pre);
    
//     float ci =  1.0 - log2(.5*log2(mean * 5.0));
//     return vec3( .5+.5*cos(6.*ci+0.5),.5+.5*cos(6.*ci + 0.33),.5+.5*cos(6.*ci + 0.32) );
//     return vec3(0.55 + 0.45 * sin(clamp(1.95 * (mean - 0.9), 0.0, 1.0) * pi * 4.0));
    return vec3(sin(clamp(2.0 * log2(mean), 0.0, 1.0) * pi*0.5));
    
    return vec3(sin(1.0 - log2(mean)));
    return getColor(i, mean, col);
}

vec3 mandelbrot(vec2 coord) {
    const float threshold = 64.0;
    const float k = 2.0; // power

//     dvec2 kaka;

    vec2 z = coord;
    int i;
    for(i = 0; i < iter; ++i) {
        z = cmul(z, z) + coord;

        if (dot(z,z) > threshold)
            break;
    }
    
    float sn = float(i) - log2(log2(dot(z,z))) + 4.0;
    float sit = float(i) - log2(log2(dot(z,z))/(log2(threshold)))/log2(k);
    vec3 color = vec3(0.0);
    if (i < iter)
        color = 0.5 + 0.5*cos( 3.0 + sit*0.075*k + vec3(0.0,0.6,1.0));
        
    return color;
    
    return vec3(abs(sin(2.0 * pi * float(i) / float(iter))));
}

vec2 uv2coord(vec2 uv) {
    vec2 coord = uv * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
    coord.x *= ratio;
    return rot(rotation) * coord * zoom + center;
}

#ifndef fractal
#define fractal ducks
#endif

void main() {
#if defined(MULTISAMPLE_W) && defined(MULTISAMPLE_H) && (MULTISAMPLE_W > 1 || MULTISAMPLE_H > 1)
    vec2 d = pixel * vec2(1.0/float(MULTISAMPLE_W), 1.0/float(MULTISAMPLE_H)) * vec2(2.0 * ratio, -2.0) * zoom;

    vec2 coord = uv2coord(qt_TexCoord0) - d * 0.5 * vec2(float(MULTISAMPLE_W), float(MULTISAMPLE_H));

    vec3 color = vec3(0.0);
    int i, j;
    for (i = 0; i < MULTISAMPLE_W; ++i) {
        for (j = 0; j < MULTISAMPLE_H; ++j) {
            color += fractal(vec2(coord.x, coord.y + float(j) * d.y));
        }

        coord.x += d.x;
    }
    
    color /= float(MULTISAMPLE_W * MULTISAMPLE_H);

#else
    vec3 color = fractal(uv2coord(qt_TexCoord0));
#endif

#if SHOW_MAP
    vec2 mapc = qt_TexCoord0 - vec2(1.0 - map_size/ratio, 1.0 - map_size);
    float mapd = length(vec2(mapc.x * ratio, mapc.y));
    if (mapd < map_size) {
        vec2 uv = mapc + vec2(0.5, 0.5);
        vec2 coord = uv * vec2(2.0, -2.0) + vec2(-1.0, 1.0);
        coord.x *= ratio;
        coord = coord * map_zoom + c;
        color = map(coord);

        if (mapd < 2.0 * max(pixel.x, pixel.y))
        color = 1.0 - color;
    }
#endif

    gl_FragColor = vec4(color, 1.0);
}

