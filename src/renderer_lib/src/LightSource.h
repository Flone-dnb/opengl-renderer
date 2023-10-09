#pragma once

// Custom.
#include "math/GLMath.hpp"

/** Represents a single light source. */
class LightSource {
public:
    /**
     * Sets light properties to the specified shader program.
     *
     * @param iShaderProgramId  Shader program to set light source properties to.
     * @param iLightSourceIndex Index of this light source to copy the properties to the specified light
     * source in the array of lights (in shaders).
     */
    void setToShader(unsigned int iShaderProgramId, size_t iLightSourceIndex) const;

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
     * Sets light's intensity, valid values range is [0.0F; 1.0F].
     *
     * @param intensity New intensity to use.
     */
    void setLightIntensity(float intensity);

    /**
     * Sets distance where the light intensity is half the maximal intensity,
     * valid values range is [0.01F; +inf].
     *
     * @param distance New distance to use.
     */
    void setLightDistance(float distance);

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

    /** Light intensity, valid values range is [0.0F; 1.0F]. */
    float intensity = 1.0F;

    /**
     * Distance where the light intensity is half the maximal intensity, valid values range is [0.01F; +inf].
     */
    float distance = 10.0F; // NOLINT
};
