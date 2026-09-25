#ifndef SHADER_H
#define SHADER_H

#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Shader {
public:
    GLuint programID = 0;

    Shader();
    ~Shader();

    bool compile(const char* vertexSource, const char* fragmentSource);
    void use() const;
    void destroy();

    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setVec3(const std::string& name, const glm::vec3& vec) const;
    void setVec4(const std::string& name, const glm::vec4& vec) const;
    void setFloat(const std::string& name, float value) const;
    void setInt(const std::string& name, int value) const;

private:
    GLint getUniformLocation(const std::string& name) const;
};

#endif
