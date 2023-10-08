#include "LightSource.h"

// Standard.
#include <algorithm>

// Custom.
#include "shader/ShaderUniformHelpers.hpp"

void LightSource::setToShader(unsigned int iShaderProgramId) const {
    ShaderUniformHelpers::setVector3ToShader(iShaderProgramId, "lightSource.position", position);
    ShaderUniformHelpers::setVector3ToShader(iShaderProgramId, "lightSource.color", color);
    ShaderUniformHelpers::setFloatToShader(iShaderProgramId, "lightSource.intensity", intensity);
    ShaderUniformHelpers::setFloatToShader(iShaderProgramId, "lightSource.distance", distance);
}

void LightSource::setLightPosition(const glm::vec3& position) { this->position = position; }

void LightSource::setLightColor(const glm::vec3& color) { this->color = color; }

void LightSource::setLightIntensity(float intensity) { this->intensity = std::clamp(intensity, 0.0F, 1.0F); }

void LightSource::setLightDistance(float distance) { this->distance = std::max(distance, 0.01F); } // NOLINT

float* LightSource::getLightPosition() { return glm::value_ptr(position); }
