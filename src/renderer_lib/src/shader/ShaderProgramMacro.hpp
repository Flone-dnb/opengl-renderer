#pragma once

// Standard.
#include <string>
#include <stdexcept>
#include <unordered_set>

// External.
#include "xxHash/xxhash.h"

/** Describes a macro that should be defined in GLSL shader. */
enum class ShaderProgramMacro : unsigned int {
    USE_DIFFUSE_TEXTURE,
    USE_METALLIC_ROUGHNESS_TEXTURE,
    USE_EMISSION_TEXTURE,
    // ... new macros go here, DON'T FORGET to add them to `macroToText` function ...
};

inline std::string macroToText(ShaderProgramMacro macro) {
    switch (macro) {
    case (ShaderProgramMacro::USE_DIFFUSE_TEXTURE): {
        return "USE_DIFFUSE_TEXTURE";
    }
    case (ShaderProgramMacro::USE_METALLIC_ROUGHNESS_TEXTURE): {
        return "USE_METALLIC_ROUGHNESS_TEXTURE";
    }
    case (ShaderProgramMacro::USE_EMISSION_TEXTURE): {
        return "USE_EMISSION_TEXTURE";
    }
    }

    throw std::runtime_error("unhandled case");
}

inline size_t convertMacrosToHash(const std::unordered_set<ShaderProgramMacro>& macros) {
    if (macros.empty()) {
        return 0;
    }

    // Convert macro numbers to string.
    std::string sMacros;
    for (const auto& macro : macros) {
        sMacros += std::to_string(static_cast<int>(macro));
    }

    return XXH3_64bits(sMacros.c_str(), sMacros.size());
}

/** Provides hash operator() for std::unordered<ShaderMacro>. */
struct ShaderProgramMacroUnorderedSetHash {
    /**
     * operator() that calculates the hash.
     *
     * @param item Set of shader macros.
     *
     * @return Hash.
     */
    size_t operator()(std::unordered_set<ShaderProgramMacro> const& item) const {
        return convertMacrosToHash(item);
    }
};
