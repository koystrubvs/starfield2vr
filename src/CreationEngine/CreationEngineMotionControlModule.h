#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class CreationEngineMotionControlModule
{
public:
    static CreationEngineMotionControlModule* Get()
    {
        static auto instance(new CreationEngineMotionControlModule);
        return instance;
    }

    // Called each frame to refresh controller pose data
    void Update();

    // Get controller rotation for weapon mesh tracking (same coordinate space as HMD in UpdateMesh)
    [[nodiscard]] glm::quat GetMeshTrackingRotation() const { return m_meshTrackingRotation; }

    // Get controller rotation in Havok/game space (for aim direction / raycasts)
    [[nodiscard]] glm::quat GetAimRotation() const { return m_smoothedAimRotation; }

    // Get controller transform in Havok/game space (for weapon positioning)
    [[nodiscard]] glm::mat4 GetWeaponTransform() const { return m_rightControllerTransform; }

    // Get controller velocity in game space (for future gesture detection)
    [[nodiscard]] glm::vec3 GetControllerVelocity() const { return m_rightControllerVelocity; }

    // Whether motion controls are currently active
    // (enabled in settings + controllers detected + in first person + not in vehicle)
    [[nodiscard]] bool IsActive() const;

    // Whether controller aim should replace head aim for the game character's aim direction
    // Same as IsActive() but also checks weapon-drawn state
    [[nodiscard]] bool ShouldUseControllerAim() const;

    CreationEngineMotionControlModule(const CreationEngineMotionControlModule&) = delete;
    CreationEngineMotionControlModule& operator=(const CreationEngineMotionControlModule&) = delete;

private:
    CreationEngineMotionControlModule() = default;
    ~CreationEngineMotionControlModule() = default;

    glm::mat4 m_rightControllerTransform{ glm::identity<glm::mat4>() };
    glm::quat m_rightControllerRotation{ glm::identity<glm::quat>() };
    glm::quat m_meshTrackingRotation{ glm::identity<glm::quat>() };  // same coord space as HMD in UpdateMesh
    glm::vec3 m_rightControllerVelocity{ 0.f };
    glm::quat m_smoothedAimRotation{ glm::identity<glm::quat>() };
};
