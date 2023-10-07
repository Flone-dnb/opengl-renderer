#pragma once

// Standard.
#include <string>
#include <format>

// Custom.
#include "window/GLFW.hpp"
#include "math/GLMath.hpp"

/** Provides static helper function for working with shader uniforms. */
class ShaderUniformHelpers {
private:
    /**
     * Returns location of a shader uniform with the specified name.
     *
     * @param iShaderProgramId ID of the shader program to query for uniform.
     * @param sUniformName     Name of a uniform.
     *
     * @return Location.
     */
    static inline unsigned int
    getUniformLocation(unsigned int iShaderProgramId, const std::string& sUniformName) {
        const auto iLocation = glGetUniformLocation(iShaderProgramId, sUniformName.c_str());
        if (iLocation < 0) [[unlikely]] {
            throw std::runtime_error(std::format("unable to get location for uniform \"{}\"", sUniformName));
        }
        return iLocation;
    }

public:
    ShaderUniformHelpers() = delete;

    /**
     * Sets the specified matrix to a `uniform` with the specified name in shaders.
     *
     * @param iShaderProgramId ID of the shader program to modify.
     * @param sUniformName     Name of the `uniform` from shaders to set the matrix to.
     * @param matrix           Matrix to set.
     */
    static inline void setMatrix4ToShader(
        unsigned int iShaderProgramId, const std::string& sUniformName, const glm::mat4x4& matrix) {
        glUniformMatrix4fv(
            getUniformLocation(iShaderProgramId, sUniformName), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    /**
     * Sets the specified matrix to a `uniform` with the specified name in shaders.
     *
     * @param iShaderProgramId ID of the shader program to modify.
     * @param sUniformName     Name of the `uniform` from shaders to set the matrix to.
     * @param matrix           Matrix to set.
     */
    static inline void setMatrix3ToShader(
        unsigned int iShaderProgramId, const std::string& sUniformName, const glm::mat3x3& matrix) {
        glUniformMatrix3fv(
            getUniformLocation(iShaderProgramId, sUniformName), 1, GL_FALSE, glm::value_ptr(matrix));
    }

    /**
     * Sets the specified vector to a `uniform` with the specified name in shaders.
     *
     * @param iShaderProgramId ID of the shader program to modify.
     * @param sUniformName     Name of the `uniform` from shaders to set the matrix to.
     * @param vector           Vector to set.
     */
    static inline void setVector3ToShader(
        unsigned int iShaderProgramId, const std::string& sUniformName, const glm::vec3& vector) {
        glUniform3fv(getUniformLocation(iShaderProgramId, sUniformName), 1, glm::value_ptr(vector));
    }

    /**
     * Sets the specified float value to a `uniform` with the specified name in shaders.
     *
     * @param iShaderProgramId ID of the shader program to modify.
     * @param sUniformName     Name of the `uniform` from shaders to set the matrix to.
     * @param value            Value to set.
     */
    static inline void
    setFloatToShader(unsigned int iShaderProgramId, const std::string& sUniformName, float value) {
        glUniform1f(getUniformLocation(iShaderProgramId, sUniformName), value);
    }
};
