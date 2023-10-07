#pragma once

// Custom.
#include "GLMath.hpp"

/** Represents a single light source. */
class LightSource {
public:
    /**
     * Sets light properties to the specified shader program.
     *
     * @param iShaderProgramId Shader program to set light source properties to.
     */
    void setToShader(unsigned int iShaderProgramId) const;

    /**
     * Sets light source position in world space.
     *
     * @param position New position to use.
     */
    void setLightPosition(const glm::vec3& position);

    /**
     * Sets a new light color (intensity).
     *
     * @param color New color to use.
     */
    void setLightColor(const glm::vec3& color);

    /**
     * Returns light source position to be modified in ImGui slider.
     *
     * @return Pointer that points to the start of the light position vector.
     */
    float* getLightPosition();

private:
    /** Light source position in world space. */
    glm::vec3 position = glm::vec3(0.0F, 0.0F, 0.0F);

    /** Color (intensity) of the light source. */
    glm::vec3 color = glm::vec3(1.0F, 1.0F, 1.0F);
};
