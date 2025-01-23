#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

// Deine (bisherige) diffuse Textur
uniform sampler2D texture1;

// Neu: zweite Textur für Normal Map
uniform sampler2D normalMap;

// Flag, ob Normal Map genutzt werden soll (true/false)
uniform bool useNormalMap;

// Licht-Parameter
uniform vec3 lightPos;
uniform vec3 lightColor;

void main()
{
    // 1) Diffuse Farbe aus 'texture1' lesen
    vec4 textureColor = texture(texture1, TexCoord);

    // 2) Bestimmen, welche Normale genutzt wird
    vec3 finalNormal;
    if (useNormalMap) {
        // Normale aus Normal Map laden ([0..1] -> [-1..1])
        vec3 mapN = texture(normalMap, TexCoord).rgb;
        mapN = mapN * 2.0 - 1.0;
        finalNormal = normalize(mapN);
    }
    else {
        // Normale aus Vertex-Interpolation
        finalNormal = normalize(Normal);
    }

    // 3) Ambient-Licht
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    // 4) Diffuse-Licht
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(finalNormal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 5) Endergebnis
    vec4 result = vec4(ambient + diffuse, 1.0) * textureColor;
    FragColor = result;
}