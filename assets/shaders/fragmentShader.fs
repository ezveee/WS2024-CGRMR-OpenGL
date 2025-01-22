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
    //diff = 1.0;

    vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);

    vec4 textureColor = texture(texture1, TexCoord);
    vec4 result = vec4(diffuse, 1.0) * textureColor;
    FragColor = result;
    //FragColor = textureColor;
}
