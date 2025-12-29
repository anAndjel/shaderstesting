#version 330 core
out vec4 FragColor;

uniform float iTime;
uniform vec2 iResolution;
uniform float uBass; // bass amplitude from C++

// ---------- rotate vector around Y axis ----------
vec3 rotateY(vec3 p, float angle){
    float c = cos(angle);
    float s = sin(angle);
    return vec3(
        p.x * c + p.z * s,
        p.y,
        -p.x * s + p.z * c
    );
}

// ---------- rotate vector around X axis ----------
vec3 rotateX(vec3 p, float angle){
    float c = cos(angle);
    float s = sin(angle);
    return vec3(
        p.x,
        p.y * c - p.z * s,
        p.y * s + p.z * c
    );
}

// ---------- SDF for a cube ----------
float sdBox(vec3 p, vec3 b){
    vec3 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, max(d.y,d.z)), 0.0);
}

// ---------- simple hash + noise ----------
float hash(vec2 p){ return fract(sin(dot(p, vec2(127.1,311.7)))*43758.5453123); }
float noise(vec2 p){
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash(i);
    float b = hash(i+vec2(1.0,0.0));
    float c = hash(i+vec2(0.0,1.0));
    float d = hash(i+vec2(1.0,1.0));
    vec2 u = f*f*(3.0-2.0*f);
    return mix(a,b,u.x) + (c-a)*u.y*(1.0-u.x) + (d-b)*u.x*u.y;
}

// ---------- procedural cube texture ----------
float cubeTexture(vec3 p){
    float stripes = sin(p.x*20.0) * 0.1 + sin(p.y*25.0)*0.1;
    float grain   = noise(p.xy*5.0 + iTime*0.1) * 0.05;
    float scratches = sin(p.z*40.0 + noise(p.xy*10.0)*10.0) * 0.05;
    return stripes + grain + scratches;
}

void main(){
    vec2 uv = (gl_FragCoord.xy * 2.0 - iResolution.xy) / iResolution.y;

    vec3 ro = vec3(0.0, 0.0, -3.0 - uBass*1.0); // move closer/further with bass
    vec3 rd = normalize(vec3(uv, 1.0));

    float t = 0.0;
    float d = 0.0;
    vec3 finalPos = vec3(0.0);

    for(int i=0; i<80; i++){
        vec3 pos = ro + rd * t;

        // rotate cube faster with bass
        vec3 p = rotateY(pos, iTime*0.2 + uBass*5.0);
        p = rotateX(p, iTime*0.1 + uBass*3.0);

        // scale cube with bass
        vec3 cubeSize = vec3(0.5 + uBass*0.3);
        d = sdBox(p, cubeSize);
        t += max(d, 0.01);

        finalPos = p;
        if(t>10.0) break;
    }

    vec3 col = vec3(0.2,0.25,0.3) * (1.0 - smoothstep(0.0,0.05,d));

    float tex = cubeTexture(finalPos*3.0);
    col -= tex;

    // pulse brightness with bass
    col += uBass * 0.3;

    FragColor = vec4(col,1.0);
}

