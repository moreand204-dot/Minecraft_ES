#include "Shader.h"
#include "Logger.h"
#include <vector>

Shader::Shader() {}

Shader::~Shader() {
    destroy();
}

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> infoLog(logLength);
        glGetShaderInfoLog(shader, logLength, nullptr, infoLog.data());
        LOGE("Shader compile error: %s", infoLog.data());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool Shader::compile(const char* vertexSource, const char* fragmentSource) {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (vs == 0 || fs == 0) return false;

    programID = glCreateProgram();
    glAttachShader(programID, vs);
    glAttachShader(programID, fs);
    glLinkProgram(programID);

    GLint success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        GLint logLength;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> infoLog(logLength);
        glGetProgramInfoLog(programID, logLength, nullptr, infoLog.data());
        LOGE("Program link error: %s", infoLog.data());
        glDeleteProgram(programID);
        programID = 0;
        glDeleteShader(vs);
        glDeleteShader(fs);
        return false;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return true;
}

void Shader::use() const {
    glUseProgram(programID);
}

void Shader::destroy() {
    if (programID != 0) {
        glDeleteProgram(programID);
        programID = 0;
    }
}

GLint Shader::getUniformLocation(const std::string& name) const {
    return glGetUniformLocation(programID, name.c_str());
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec3(const std::string& name, const glm::vec3& vec) const {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(vec));
}

void Shader::setVec4(const std::string& name, const glm::vec4& vec) const {
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(vec));
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(getUniformLocation(name), value);
}
