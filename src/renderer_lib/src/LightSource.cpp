#include "LightSource.h"

// Custom.
#include "shader/ShaderUniformHelpers.hpp"

void LightSource::setToShader(unsigned int iShaderProgramId) const {
    ShaderUniformHelpers::setVector3ToShader(iShaderProgramId, "lightSource.position", position);
    ShaderUniformHelpers::setVector3ToShader(iShaderProgramId, "lightSource.color", color);
}

void LightSource::setLightPosition(const glm::vec3& position) { this->position = position; }

void LightSource::setLightColor(const glm::vec3& color) { this->color = color; }

float* LightSource::getLightPosition() { return glm::value_ptr(position); }
