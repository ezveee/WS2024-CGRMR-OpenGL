#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture1; // diffuse
uniform sampler2D normalMap; // normalmap

uniform bool useNormalMap;

uniform vec3 lightPos;
uniform vec3 lightColor;

void main()
{
    vec4 textureColor = texture(texture1, TexCoord);

    vec3 finalNormal;
    if (useNormalMap) {
        // load normal from normal map ([0 - 1] -> [-1 - 1])
        vec3 mapN = texture(normalMap, TexCoord).rgb;
        mapN = mapN * 2.0 - 1.0;
        finalNormal = normalize(mapN);
    }
    else {
        finalNormal = normalize(Normal);
    }

    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(finalNormal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec4 result = vec4(ambient + diffuse, 1.0) * textureColor;
    FragColor = result;
}