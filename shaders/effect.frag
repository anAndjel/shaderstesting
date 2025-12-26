#version 330 core
out vec4 FragColor;

uniform float iTime;
uniform vec2 iResolution;

vec2 rot(vec2 p, float a) {
    float c = cos(a);
    float s = sin(a);
    return vec2(
        p.x * c - p.y * s,
        p.x * s + p.y * c
    );
}

float sdBox(vec2 p, vec2 b) {
  vec2 d = abs(p) - b;
  return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
};

void main() {
    vec2 uv = (gl_FragCoord.xy * 2.0 - iResolution.xy) / iResolution.y;
    vec2 p = uv;
    vec2 center = vec2(0.0, 0.0);
    //p = rot(p, iTime);
    //p.x *= 2.0; // stretch

    float r = length(p);
    float a = atan(p.y, p.x);
    //a += iTime;
    a += 1.567;
    p = vec2(cos(a), sin(a)) * r;
    //float c = length(p) - 0.25;
    //vec3 color = vec3(0.5 + 0.5*sin(iTime), 0.5 + 0.5*sin(iTime + 2.0), 0.5 + 0.5*sin(iTime + 4.0));
    //vec3 col = vec3(color) * circle;

    vec2 scale = vec2(0.25, 0.5);

    scale = sin(scale*rot(p, iTime) + iTime) / 2;

    float b = sdBox(p - center, scale);
    //float d = min(c, b);
    //float trgl = sdEquilateralTriangle(p, 0.5);
    //float d = min(b, trgl);
    float d = length(b);
    //d -= 0;
    d = sin(d*8. + iTime)/8.;
    d = abs(b);
    d = 0.013 / d;
    //float circle = smoothstep(0.01, 0.0, d);
    vec3 color = vec3(1.0, 2.0, 3.0);
    //float rect = smoothstep(0.06, 0.0, d);
    //vec3 col = vec3(color) * d;
    color *= d;
    


    //FragColor = vec4(col, 1.0);
    FragColor = vec4(color, 1.0);
    //FragColor = vec4(uv, 0.0,1.0);
}

