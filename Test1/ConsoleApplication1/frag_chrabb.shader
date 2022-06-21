#version 330 core
out vec4 FragColor;

in vec2 fragCoord;
uniform sampler2D _MainTex;

void main()
{
    float rFactor = 0.98;
    float gFactor = 0.99;
    float bFactor = 1.0;

    vec2 d = fragCoord - vec2(0.5, 0.5);

    float r = texture(_MainTex, vec2(0.5, 0.5) + d * rFactor).r;
    float g = texture(_MainTex, vec2(0.5, 0.5) + d * gFactor).g;
    float b = texture(_MainTex, vec2(0.5, 0.5) + d * bFactor).b;

    FragColor = vec4(r, g, b, 1.0);
}