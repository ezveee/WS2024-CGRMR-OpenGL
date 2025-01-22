#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture1;
uniform vec3 lightPos;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    textureColor = texture(texture1, TexCoord);
    vec3 result = (ambient + diffuse) * textureColor;
    FragColor = vec4(result, 1.0);
    //FragColor = texture(texture1, TexCoord);
}
