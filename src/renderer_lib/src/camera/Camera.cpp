#include "Camera.h"

// Standard.
#include <cmath>
#include <algorithm>

void Camera::setFreeCameraForwardMovement(float input) {
    lastInputDirection.x = std::clamp(input, -1.0F, 1.0F);
}

void Camera::setFreeCameraRightMovement(float input) {
    lastInputDirection.y = std::clamp(input, -1.0F, 1.0F);
}

void Camera::setFreeCameraWorldUpMovement(float input) {
    lastInputDirection.z = std::clamp(input, -1.0F, 1.0F);
}

void Camera::setCameraMode(CameraMode mode) {
    std::scoped_lock guard(cameraProperties.mtxData.first);

    cameraProperties.mtxData.second.currentCameraMode = mode;
    if (mode == CameraMode::ORBITAL) {
        // Recalculate rotation.
        MathHelpers::convertCartesianCoordinatesToSpherical(
            cameraProperties.mtxData.second.viewData.worldLocation -
                cameraProperties.mtxData.second.viewData.targetPointWorldLocation,
            cameraProperties.mtxData.second.orbitalModeData.distanceToTarget,
            cameraProperties.mtxData.second.orbitalModeData.theta,
            cameraProperties.mtxData.second.orbitalModeData.phi);

        recalculateBaseVectorsForOrbitalCamera();
    } else {
        // Update target for free camera.
        cameraProperties.mtxData.second.viewData.targetPointWorldLocation =
            cameraProperties.mtxData.second.viewData.worldLocation + cameraForwardDirection;

        // Update rotation.
        setFreeCameraRotation(MathHelpers::convertDirectionToRollPitchYaw(cameraForwardDirection));
    }

    // Mark view matrix as "needs update".
    cameraProperties.mtxData.second.viewData.bViewMatrixNeedsUpdate = true;
}

void Camera::setLocation(const glm::vec3& location) {
    std::scoped_lock guard(cameraProperties.mtxData.first);

    // Update camera properties.
    cameraProperties.mtxData.second.viewData.worldLocation = location;
    if (cameraProperties.mtxData.second.currentCameraMode == CameraMode::ORBITAL) {
        // Calculate rotation based on new location.
        MathHelpers::convertCartesianCoordinatesToSpherical(
            cameraProperties.mtxData.second.viewData.worldLocation -
                cameraProperties.mtxData.second.viewData.targetPointWorldLocation,
            cameraProperties.mtxData.second.orbitalModeData.distanceToTarget,
            cameraProperties.mtxData.second.orbitalModeData.theta,
            cameraProperties.mtxData.second.orbitalModeData.phi);

        recalculateBaseVectorsForOrbitalCamera();
    } else {
        // Update target for free camera.
        cameraProperties.mtxData.second.viewData.targetPointWorldLocation =
            cameraProperties.mtxData.second.viewData.worldLocation + cameraForwardDirection;
    }

    // Mark view matrix as "needs update".
    cameraProperties.mtxData.second.viewData.bViewMatrixNeedsUpdate = true;
}

void Camera::setFreeCameraRotation(const glm::vec3& rotation) {
    std::scoped_lock guard(cameraProperties.mtxData.first);

    // Make sure we are in the free camera mode.
    if (cameraProperties.mtxData.second.currentCameraMode == CameraMode::ORBITAL) [[unlikely]] {
        throw std::runtime_error(
            "an attempt to set free camera rotation was ignored because the camera is not in "
            "the free mode");
        return;
    }

    // Save new rotation.
    cameraRotation.x = MathHelpers::normalizeValue(rotation.x, -360.0F, 360.0F); // NOLINT
    cameraRotation.y = MathHelpers::normalizeValue(rotation.y, -360.0F, 360.0F); // NOLINT
    cameraRotation.z = MathHelpers::normalizeValue(rotation.z, -360.0F, 360.0F); // NOLINT

    // Build rotation matrix.
    const auto rotationMatrix = MathHelpers::buildRotationMatrix(cameraRotation);

    // Recalculate axis.
    cameraForwardDirection = rotationMatrix * glm::vec4(Globals::WorldDirection::forward, 0.0F);
    cameraRightDirection = rotationMatrix * glm::vec4(Globals::WorldDirection::right, 0.0F);
    cameraUpDirection = rotationMatrix * glm::vec4(Globals::WorldDirection::up, 0.0F);

    // Update camera properties.
    cameraProperties.mtxData.second.viewData.targetPointWorldLocation =
        cameraProperties.mtxData.second.viewData.worldLocation + cameraForwardDirection;
    cameraProperties.mtxData.second.viewData.worldUpDirection = cameraUpDirection;

    // Mark view matrix as "needs update".
    cameraProperties.mtxData.second.viewData.bViewMatrixNeedsUpdate = true;
}

void Camera::setOrbitalCameraTargetLocation(const glm::vec3& targetLocation) {
    std::scoped_lock guard(cameraProperties.mtxData.first);

    // Make sure we are in the orbital camera mode.
    if (cameraProperties.mtxData.second.currentCameraMode == CameraMode::FREE) [[unlikely]] {
        throw std::runtime_error(
            "an attempt to set orbital camera target location was ignored because the camera is not in "
            "the orbital mode");
        return;
    }

    // Update camera properties.
    cameraProperties.mtxData.second.viewData.targetPointWorldLocation = targetLocation;

    // Calculate rotation based on new target point location.
    MathHelpers::convertCartesianCoordinatesToSpherical(
        cameraProperties.mtxData.second.viewData.worldLocation -
            cameraProperties.mtxData.second.viewData.targetPointWorldLocation,
        cameraProperties.mtxData.second.orbitalModeData.distanceToTarget,
        cameraProperties.mtxData.second.orbitalModeData.theta,
        cameraProperties.mtxData.second.orbitalModeData.phi);

    recalculateBaseVectorsForOrbitalCamera();

    // Save new camera's up direction to view data (to be used in view matrix).
    cameraProperties.mtxData.second.viewData.worldUpDirection = cameraUpDirection;

    // Mark view matrix as "needs update".
    cameraProperties.mtxData.second.viewData.bViewMatrixNeedsUpdate = true;
}

void Camera::setOrbitalCameraDistanceToTarget(float distanceToTarget) {
    std::scoped_lock guard(cameraProperties.mtxData.first);

    // Make sure we are in the orbital camera mode.
    if (cameraProperties.mtxData.second.currentCameraMode == CameraMode::FREE) [[unlikely]] {
        throw std::runtime_error(
            "an attempt to set orbital camera distance to target was ignored because the camera is not "
            "in the orbital mode");
        return;
    }

    // Apply distance.
    cameraProperties.mtxData.second.orbitalModeData.distanceToTarget = distanceToTarget;

    // Recalculate location.
    cameraProperties.mtxData.second.viewData.worldLocation =
        MathHelpers::convertSphericalToCartesianCoordinates(
            cameraProperties.mtxData.second.orbitalModeData.distanceToTarget,
            cameraProperties.mtxData.second.orbitalModeData.theta,
            cameraProperties.mtxData.second.orbitalModeData.phi) +
        cameraProperties.mtxData.second.viewData.targetPointWorldLocation;

    // Mark view matrix as "needs update".
    cameraProperties.mtxData.second.viewData.bViewMatrixNeedsUpdate = true;
}

void Camera::setOrbitalCameraRotation(float phi, float theta) {
    std::scoped_lock guard(cameraProperties.mtxData.first);

    // Make sure we are in the orbital camera mode.
    if (cameraProperties.mtxData.second.currentCameraMode == CameraMode::FREE) [[unlikely]] {
        throw std::runtime_error(
            "an attempt to set orbital camera rotation was ignored because the camera is not "
            "in the orbital mode");
        return;
    }

    // Apply rotation.
    cameraProperties.mtxData.second.orbitalModeData.phi =
        std::clamp(phi, 0.1F, 180.0F); // NOLINT: don't allow flipping the camera
    cameraProperties.mtxData.second.orbitalModeData.theta = theta;

    // Recalculate location.
    cameraProperties.mtxData.second.viewData.worldLocation =
        MathHelpers::convertSphericalToCartesianCoordinates(
            cameraProperties.mtxData.second.orbitalModeData.distanceToTarget,
            cameraProperties.mtxData.second.orbitalModeData.theta,
            cameraProperties.mtxData.second.orbitalModeData.phi) +
        cameraProperties.mtxData.second.viewData.targetPointWorldLocation;

    recalculateBaseVectorsForOrbitalCamera();

    // Save new camera's up direction to view data (to be used in view matrix).
    cameraProperties.mtxData.second.viewData.worldUpDirection = cameraUpDirection;

    // Mark view matrix as "needs update".
    cameraProperties.mtxData.second.viewData.bViewMatrixNeedsUpdate = true;
}

void Camera::setCameraMovementSpeed(float speed) { cameraMovementSpeed = speed; }

glm::vec3 Camera::getFreeCameraRotation() const { return cameraRotation; }

CameraProperties* Camera::getCameraProperties() { return &cameraProperties; }

void Camera::onBeforeNewFrame(float timeSincePrevCallInSec) {
    // Make sure the input is not zero.
    if (glm::all(glm::epsilonEqual(lastInputDirection, glm::vec3(0.0F, 0.0F, 0.0F), inputDelta))) {
        return;
    }

    // Normalize in order to avoid speed boost when multiple input keys are pressed.
    moveFreeCamera(glm::normalize(lastInputDirection) * timeSincePrevCallInSec * cameraMovementSpeed);
}

void Camera::clearInput() { lastInputDirection = glm::vec3(0.0F, 0.0F, 0.0F); }

void Camera::moveFreeCamera(const glm::vec3& distance) {
    std::scoped_lock guard(cameraProperties.mtxData.first);

    // Make sure we are in the free camera mode.
    if (cameraProperties.mtxData.second.currentCameraMode == CameraMode::ORBITAL) [[unlikely]] {
        throw std::runtime_error(
            "an attempt to move free camera forward was ignored because the camera is not in "
            "the free mode");
    }

    // Apply movement.
    cameraProperties.mtxData.second.viewData.worldLocation += cameraForwardDirection * distance.x;
    cameraProperties.mtxData.second.viewData.worldLocation += cameraRightDirection * distance.y;
    cameraProperties.mtxData.second.viewData.worldLocation += Globals::WorldDirection::up * distance.z;

    // Recalculate look direction.
    cameraProperties.mtxData.second.viewData.targetPointWorldLocation =
        cameraProperties.mtxData.second.viewData.worldLocation + cameraForwardDirection;

    // Mark view matrix as "needs update".
    cameraProperties.mtxData.second.viewData.bViewMatrixNeedsUpdate = true;
}
