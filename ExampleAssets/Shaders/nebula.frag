#version 330 core
out vec4 FragColor;

// --- BATCH ENGINE COMPATIBLE INPUTS ---
in vec2 TexCoord;
in vec4 FragColorOut; // Receives the image color tint and alpha properties per vertex

uniform float u_time;
// uniform vec4 tintColor; // REMOVED: Replaced by vertex attribute FragColorOut

float SEED = 42.0;

float random(float co) { return fract(sin(co*(91.3458)) * 47453.5453); }
float random(vec2 co){ return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); }
float random(vec3 co){ return random(co.xy+random(co.z)); }

float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x)+(c - a)*u.y*(1.0 - u.x)+(d - b) * u.x * u.y;
}

#define OCT 5
float fbm (in vec2 st) {
    float val = 0.0;
    float amp = .5;
    for (int i = 0; i < OCT; i++) {
        val += amp*noise(st);
        amp*=.5;
        st*=2.;
    }
    return val;
}

mat2 rotate(float theta) {
    return mat2(cos(theta),-sin(theta),sin(theta),cos(theta));
}

void main(void) {
    float startRandom = fract(SEED*.001);
    vec3 color;

    // Use your engine's normalized coordinates scaled up slightly
    vec2 uv = TexCoord * 2.0;

    float RES = 400.0;

    float t = .06*u_time*2.*(startRandom-.5);
    float t2= .06*u_time*2.*(random(startRandom)-.5);
    vec2 anch = vec2(.5,-4.);
    uv=(uv-anch)*rotate(t/length(anch))+anch+t2;

    vec2 starres=floor(RES*uv)/RES;

    // Background sky processing
    uv*=8.;
    float perlin=fbm(uv+vec2(fbm(uv),fbm(uv+32.)));
    color=mix(
            vec3(16., 21., 112.)/255.,
            vec3(84., 255., 241.)/255.,
            perlin);
    color*=color;
    color*=color;

    // Star calculations
    float seed = random(starres);
    if (seed >= .992) {
        seed=random(starres+seed);
        color += vec3(.2+seed+0.5*seed*sin(u_time+10.*seed));
    }

    // --- FIX: Apply the vertex baked tintColor directly ---
    FragColor = vec4(color, 1.0) * FragColorOut;
}