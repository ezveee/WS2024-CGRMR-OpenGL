#version 330 core
layout(location = 0) in vec3 aPos; // vertex position
layout(location = 1) in vec2 aTexCoord; // texture coordinates
layout (location = 2) in vec3 aNormal; // vertex normal

uniform mat4 projection; // projection matrix
uniform mat4 model; // model matrix
uniform mat4 view; // view matrix

out vec2 TexCoord; // pass texture coordinates to fragment shader
out vec3 Normal; // pass normal to fragment shader
out vec3 FragPos; // pass fragment position to fragment shader

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0); // final vertex position
    FragPos = vec3(model * vec4(aPos, 1.0)); // calculate fragment position in world space
    TexCoord = aTexCoord; // pass texture coordinates
    Normal = aNormal; // pass normal
}
