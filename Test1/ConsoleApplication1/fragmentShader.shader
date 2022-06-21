#version 330 core

out vec4 FragColor;

in vec3 color;
in vec2 uv;
in vec3 normal;
in vec4 worldPixel;

uniform sampler2D heightmap;
uniform sampler2D normalMap;

uniform sampler2D dirt, sand, grass;

uniform vec3 lightDirection;
uniform vec3 cameraPosition;

vec3 lerp(vec3 a, vec3 b, float t) {
    return a + (b - a) * t;
}

void main() {
    vec3 normalColor = texture(normalMap, uv).rbg * 2 - 1;
    normalColor.b = -normalColor.b;
    normalColor.r = -normalColor.r;

    vec3 lightDir = normalize(lightDirection);
    float light = max(dot(-lightDir, normalColor), .25);

    vec3 dirtColor = texture(dirt, uv).rgb;
    vec3 sandColor = texture(sand, uv).rgb;
    vec3 grassColor = texture(grass, uv).rgb;

    float ds = clamp((worldPixel.y - 25) / 10, 0, 1);
    float sg = clamp((worldPixel.y - 50) / 10, 0, 1);

    vec3 diffuse = lerp(lerp(dirtColor, sandColor, ds), grassColor, sg);

    float d = distance(worldPixel.xyz, cameraPosition);    // vec3(100, 100, 100));        // need uniform here again
    float fogAmount = clamp((d - 50) / 250, 0, 1);        //

    vec3 bot = vec3(188 / 255.0, 214 / 255.0, 231 / 255.0);

    FragColor = vec4(lerp(diffuse * light, bot, fogAmount), 1.0);

    //FragColor = vec4(1, 1, 1, 1);

    //vec3 camPos = vec3(0, 3, -3);
    //vec3 viewDirection = normalize(worldPixel.xyz - camPos);

    //vec3 lightReflect = normalize(reflect(-lightDir, normal));
    //float specular = pow(max(dot(lightReflect, viewDirection), 0.0), 128);



    //float light = max(dot(-lightDir, normal), .25);

    // diffuseColor* light + specular * diffuseColor;// vec4(diffuseColor.rgb, 1.0f); //vec4(normal * .5 + .5, 0);// 
}