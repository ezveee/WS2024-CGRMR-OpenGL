#version 330 core
out vec4 FragColor;

in vec2 TexCoord; // texture coordinates
in vec3 Normal; // surface normal
in vec3 FragPos; // fragment position

uniform sampler2D texture1; // diffuse texture
uniform sampler2D normalMap; // normal map

uniform bool useNormalMap; // toggle normal map

uniform vec3 lightPos; // light position
uniform vec3 lightColor; // light color

void main()
{
    // sample diffuse texture
    vec4 textureColor = texture(texture1, TexCoord);

    vec3 finalNormal; // normal for lighting
    if (useNormalMap) {
        // load and transform normal map
        vec3 mapN = texture(normalMap, TexCoord).rgb;
        mapN = mapN * 2.0 - 1.0;
        finalNormal = normalize(mapN);
    }
    else {
        // use interpolated normal
        finalNormal = normalize(Normal);
    }

    // ambient light
    float ambientStrength = 0.2; // ambient factor
    vec3 ambient = ambientStrength * lightColor;

    // diffuse light
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(finalNormal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // final color
    vec4 result = vec4(ambient + diffuse, 1.0) * textureColor;
    FragColor = result;
}
