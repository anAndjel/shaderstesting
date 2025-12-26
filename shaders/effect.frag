#version 330 core
out vec4 FragColor;

uniform float iTime;
uniform vec2 iResolution;

varying vec2 pos;

//float sdBox( in vec2 p, int vec2 b) {
//  vec2 d = abs(p);
//  d - b;
//  return length(max(d,0.0)) + min(max(d.x,d.y),0.0);
//}

//vec3 palette( float t, vec3 a, vec3 b, vec3 c, vec3 d) {
//  return a + b*cos( 6.28318*(c*t+d) );
//}

vec3 palette( float t ) {
  vec3 a = vec3(0.5, 0.5, 0.5);
  vec3 b = vec3(0.5, 0.5, 0.5);
  vec3 c = vec3(1.0, 1.0, 1.0);
  vec3 d = vec3(0.00, 0.10, 0.20);

  return a + b*sin( 6.28318*(c*t+d) );
}


void main() {
    vec2 uv = (gl_FragCoord.xy * 2.0 - iResolution.xy) / iResolution.y;
    vec2 pos = gl_FragCoord.xy / iResolution;
    vec2 uv0 = uv;
    vec2 uv1 = uv;
    vec2 uv2 = uv;
    vec2 uv3 = uv;
    vec3 finalColor = vec3(0.0);

    for (float i = 0.0; i < 3.141592653; i++) {

        uv = fract(uv * 1.5) - 0.5;
        uv2 = sin(uv * iTime)/8.0;
        float len = length(uv) * exp(+length(uv0)) * exp(-length(uv1)) *exp(-length(uv2));

        vec3 col = palette(length(uv0 - uv1) + i*.8 + iTime*.4);

        len = sin(len*8. + iTime)/8.;
        len = abs(len);

        //len = step( 10.01, len);

        len = pow(0.01 / len, 1.2);

        finalColor += col * len;
        //sdBox(uv, iResolution);
        //float waves = sin(10.0 * len - iTime * 10.0);
        //float intensity = 0.5 + 0.5 * waves;
        //float intensity = 0.5 + 0.5 * sdBox(uv, iResolution);
        //float r = 0.5 + 0.5 * sin(uv.x * 10.0 + iTime);
        //float g = 0.5 + 0.5 * sin(uv.y * 10.0 + iTime * 1.2);
        //float b = 0.5 + 0.5 * sin((uv.x + uv.y) * 5.0 + iTime * 0.8);
    }

    vec3 circle = vec3(0.5, 0.5, 0.3);
    float d = length(pos - circle.xy) - circle.z;
    d = smoothstep(0., 0.01, d);

    FragColor = vec4(finalColor, 1.0);
}

