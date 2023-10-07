#include "LightSource.h"

// Custom.
#include "Application.h"

void LightSource::setToShader(unsigned int iShaderProgramId) const {
    Application::setVector3ToShader(iShaderProgramId, "lightSource.position", position);
    Application::setVector3ToShader(iShaderProgramId, "lightSource.color", color);
}

void LightSource::setLightPosition(const glm::vec3& position) { this->position = position; }

void LightSource::setLightColor(const glm::vec3& color) { this->color = color; }

float* LightSource::getLightPosition() { return glm::value_ptr(position); }
