#pragma once

// Custom.
#include "math/GLMath.hpp"

/** Groups global parameters. */
class Globals {
public:
    Globals() = delete;

    /** Groups directions in world space. */
    struct WorldDirection {
        /** World's forward direction. */
        static inline const glm::vec3 forward = glm::vec3(0.0F, 0.0F, -1.0F);

        /** World's right direction. */
        static inline const glm::vec3 right = glm::vec3(1.0F, 0.0F, 0.0F);

        /** World's up direction. */
        static inline const glm::vec3 up = glm::vec3(0.0F, 1.0F, 0.0F);
    };
};
