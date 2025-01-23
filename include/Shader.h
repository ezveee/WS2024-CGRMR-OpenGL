//
// Created by vee on 21/01/2025.
//

#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>

class Shader {
public:
    unsigned int ID; // program ID

    Shader(const char* vertexPath, const char* fragmentPath) {
        // read shader code from files
        std::string vertexCode, fragmentCode;
        std::ifstream vShaderFile(vertexPath), fShaderFile(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();

        // compile vertex shader
        unsigned int vertex, fragment;
        vertex = glCreateShader(GL_VERTEX_SHADER); // create vertex shader
        const char* vShaderCode = vertexCode.c_str();
        glShaderSource(vertex, 1, &vShaderCode, NULL); // attach shader source
        glCompileShader(vertex); // compile vertex shader

        // compile fragment shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER); // create fragment shader
        const char* fShaderCode = fragmentCode.c_str();
        glShaderSource(fragment, 1, &fShaderCode, NULL); // attach shader source
        glCompileShader(fragment); // compile fragment shader

        // link shaders into a program
        ID = glCreateProgram(); // create shader program
        glAttachShader(ID, vertex); // attach vertex shader
        glAttachShader(ID, fragment); // attach fragment shader
        glLinkProgram(ID); // link shaders into program

        // delete shaders as theyre linked into program now -> no longer needed
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void use() const {
        glUseProgram(ID); // activate shader program
    }

    void setMat4(const std::string& name, const glm::mat4& value) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value)); // set mat4 uniform
    }

    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); // set vec3 uniform
    }

    void setInt(const std::string &name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); // set int uniform
    }

    void setBool(const std::string &name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); // set bool uniform as int
    }
};

#endif