#include "CreationEngineMotionControlModule.h"
#include "CreationEngineSingletonManager.h"
#include <CreationEngine/models/GameFlow.h>
#include <CreationEngine/models/ModSettingsStore.h>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <mods/VR.hpp>

namespace {
    const glm::mat4 permutation_pre = {
        1, 0, 0, 0,
        0, 0, 1, 0,
        0, -1, 0, 0,
        0, 0, 0, 1
    };
    const glm::mat4 permutation_post = glm::transpose(permutation_pre);

    glm::mat4 to_havok_space(const glm::mat4& mat) {
        return permutation_pre * mat * permutation_post;
    }
}

void CreationEngineMotionControlModule::Update()
{
    static auto vr = VR::get();

    if (!vr->is_hmd_active() || !vr->is_using_controllers()) {
        return;
    }

    auto right_index = vr->get_right_controller_index();
    if (right_index < 0) {
        return;
    }

    // Get raw controller transform from vrframework (OpenXR space)
    auto controller_transform = vr->get_transform(right_index);

    // Subtract standing origin offset (same as camera manager does for HMD)
    auto standing_origin = vr->get_transform_offset();
    controller_transform[3].x -= standing_origin[3].x;
    controller_transform[3].y -= standing_origin[3].y;
    controller_transform[3].z -= standing_origin[3].z;

    // Convert from OpenXR coordinate space to game/Havok space
    m_rightControllerTransform = to_havok_space(controller_transform);
    m_rightControllerRotation = glm::normalize(glm::quat_cast(m_rightControllerTransform));

    // Get velocity for future gesture detection
    auto velocity = vr->get_velocity(right_index);
    m_rightControllerVelocity = glm::vec3(velocity.x, velocity.y, velocity.z);

    // Apply smoothing via slerp (0 = no smoothing/instant, 1 = max smoothing)
    float smoothing = GameFlow::gStore.internalSettings.motionControlSmoothing;
    float interpolation = 1.0f - smoothing;
    // Clamp to avoid NaN from slerp with very small t
    interpolation = std::clamp(interpolation, 0.05f, 1.0f);
    m_smoothedAimRotation = glm::slerp(m_smoothedAimRotation, m_rightControllerRotation, interpolation);
}

bool CreationEngineMotionControlModule::IsActive() const
{
    static auto vr = VR::get();

    if (!GameFlow::gStore.internalSettings.motionControlAiming) {
        return false;
    }
    if (!vr->is_hmd_active() || !vr->is_using_controllers()) {
        return false;
    }
    if (!GameFlow::isInFirstPerson()) {
        return false;
    }
    if (GameFlow::isInShipOrVehicle()) {
        return false;
    }
    if (GameFlow::isImmovable() || GameFlow::isControlledByAI()) {
        return false;
    }
    return true;
}

bool CreationEngineMotionControlModule::ShouldUseControllerAim() const
{
    return IsActive();
}
