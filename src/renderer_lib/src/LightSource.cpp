#include "LightSource.h"

// Standard.
#include <algorithm>

// Custom.
#include "shader/ShaderUniformHelpers.hpp"

void LightSource::setToShader(unsigned int iShaderProgramId, size_t iLightSourceIndex) const {
    ShaderUniformHelpers::setVector3ToShader(
        iShaderProgramId, std::format("vLightSources[{}].position", iLightSourceIndex), position);
    ShaderUniformHelpers::setVector3ToShader(
        iShaderProgramId, std::format("vLightSources[{}].color", iLightSourceIndex), color);
    ShaderUniformHelpers::setFloatToShader(
        iShaderProgramId, std::format("vLightSources[{}].intensity", iLightSourceIndex), intensity);
    ShaderUniformHelpers::setFloatToShader(
        iShaderProgramId, std::format("vLightSources[{}].distance", iLightSourceIndex), distance);
}

void LightSource::setLightPosition(const glm::vec3& position) { this->position = position; }

void LightSource::setLightColor(const glm::vec3& color) { this->color = color; }

void LightSource::setLightIntensity(float intensity) { this->intensity = std::clamp(intensity, 0.0F, 1.0F); }

void LightSource::setLightDistance(float distance) { this->distance = std::max(distance, 0.01F); } // NOLINT

float* LightSource::getLightPosition() { return glm::value_ptr(position); }
