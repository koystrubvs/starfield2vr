#include "CreationEngineEntry.h"
#include "CreationEngineConstants.h"
#include "imgui.h"
#include <CreationEngine/models/GameFlow.h>
#include <CreationEngine/models/ModSettingsStore.h>
#include <imgui_internal.h>
#include <mods/VR.hpp>
#include <vector>

#include "CreationEngineCameraManager.h"
#include "CreationEngineInputManager.h"
#include "CreationEngineMotionControlModule.h"
#include "CreationEngineRendererModule.h"
#include "CreationEngineSingletonManager.h"
#include "RE/P/PlayerCamera.h"

std::optional<std::string> CreationEngineEntry::on_initialize()
{
    CreationEngineCameraManager::Get()->InstallHooks();
    CreationEngineRendererModule::Get()->InstallHooks();
    CreationEngineInputManager::Get()->Init();
    CreationEngineMotionControlModule::Get(); // initialize motion control module
    return Mod::on_initialize();
}

void CreationEngineEntry::on_draw_ui()
{
    if (!ImGui::CollapsingHeader(get_name().data())) {
        return;
    }
    auto vr = VR::get();
    if(m_hud_scale->draw("Масштаб HUD"))
    {
        GameFlow::gStore.hudSettings.hudScale = m_hud_scale->value();
    }
    if(m_hud_perspective->draw("Перспектива HUD"))
    {
        GameFlow::gStore.hudSettings.perspective = (int) m_hud_perspective->value();
    }
    if(m_alternative_joy_layout->draw("Альт. раскладка джойстика"))
    {
        GameFlow::gStore.internalSettings.alternativeJoyLayout = m_alternative_joy_layout->value();
    }
    if(m_dominant_eye->draw("Ведущий глаз"))
    {
        ModConstants::dominantEye = m_dominant_eye->value();
    }
    if(m_head_tracking_type->draw("Тип прицеливания"))
    {
        ModConstants::headTrackingType = m_head_tracking_type->value();
    }
    if(m_pawn_control_rotation->draw("Поворот персонажа стиком"))
    {
        GameFlow::gStore.internalSettings.pawnControl = m_pawn_control_rotation->value();
    }
    if(m_decoupled_pitch->draw("Отвязать наклон головы"))
    {
        GameFlow::gStore.internalSettings.decoupledPitch = m_decoupled_pitch->value();
    }
    if(m_head_tracking_multiplier->draw("Чувствительность головы"))
    {
        ModConstants::headTrackingMultiplier = m_head_tracking_multiplier->value();
    }
    if(m_taa_anf_nvidia_fix->draw("Фикс NVIDIA DLSS и TAA"))
    {
        GameFlow::gStore.internalSettings.nvidiaAndTAAfix = m_taa_anf_nvidia_fix->value();
    }
    if(m_disable_zoom->draw("Блокировать зум игры")) {
        GameFlow::gStore.internalSettings.preventZoom = m_disable_zoom->value();
    }
    if(m_snap_turn->draw("Поворот щелчком")) {
        GameFlow::gStore.internalSettings.snapTurn = m_snap_turn->value();
    }
    if(m_snap_turn_angle->draw("Угол поворота")) {
        GameFlow::gStore.internalSettings.snapTurnAngle = m_snap_turn_angle->value();
    }
    if(m_motion_control_aiming->draw("Motion control aiming")) {
        GameFlow::gStore.internalSettings.motionControlAiming = m_motion_control_aiming->value();
    }
    if(m_motion_control_smoothing->draw("Aim smoothing")) {
        GameFlow::gStore.internalSettings.motionControlSmoothing = m_motion_control_smoothing->value();
    }

    // Debug: show camera state for snap turn debugging
    {
        ImGui::Separator();
        ImGui::Text("== Debug ==");
        auto p_camera = CreationEngineSingletonManager::GetPlayerCameraSingleton();
        if (p_camera && p_camera->currentState) {
            auto state = p_camera->currentState;
            const char* state_name = "Unknown";
            if (state == (RE::TESCameraState*)p_camera->pFirstPersonModeState) state_name = "FirstPerson";
            else if (state == p_camera->pThirdPersonState) state_name = "ThirdPerson";
            else if (state == p_camera->pFlightCameraState) state_name = "Flight";
            else if (state == p_camera->pShipFarCameraState) state_name = "ShipFarTravel";
            else if (state == p_camera->pShipActionCameraState) state_name = "ShipAction";
            else if (state == p_camera->pShipTargetingCameraState) state_name = "ShipTargeting";
            else if (state == p_camera->pShipCameraOrbitState) state_name = "ShipOrbit";
            else if (state == p_camera->pVehicleCameraState) state_name = "Vehicle";
            else if (state == p_camera->pVanityState) state_name = "Vanity";
            else if (state == p_camera->pIronSightState) state_name = "IronSight";
            else if (state == p_camera->pDialogueCameraSTate) state_name = "Dialogue";
            else if (state == p_camera->pPhotoModeCameraState) state_name = "PhotoMode";
            else if (state == p_camera->pFurnitureCameraState) state_name = "Furniture";

            bool in_ship = GameFlow::isInShipOrVehicle();
            ImGui::Text("Camera: %s", state_name);
            ImGui::Text("InShip: %s", in_ship ? "YES" : "NO");
            ImGui::Text("SnapActive: %s",
                (GameFlow::gStore.internalSettings.snapTurn && !in_ship) ? "YES" : "NO");

            auto p_player = CreationEngineSingletonManager::GetPlayerRef();
            if (p_player) {
                ImGui::Text("actorState: 0x%08X", p_player->actorState);
                ImGui::Text("actorState2: 0x%08X", p_player->actorState2);
                ImGui::Text("FlyState: %d", (p_player->actorState >> 14) & 7);
                ImGui::Text("WeaponDrawn: %s", p_player->IsWeaponDrawn() ? "Y" : "N");
            }

            // Motion control debug info
            auto motionCtrl = CreationEngineMotionControlModule::Get();
            ImGui::Text("MotionCtrl: %s", motionCtrl->IsActive() ? "ACTIVE" : "off");
            if (motionCtrl->IsActive()) {
                auto rot = motionCtrl->GetAimRotation();
                ImGui::Text("CtrlAim: [%.2f,%.2f,%.2f,%.2f]", rot.x, rot.y, rot.z, rot.w);
                auto vel = motionCtrl->GetControllerVelocity();
                ImGui::Text("CtrlVel: [%.2f,%.2f,%.2f]", vel.x, vel.y, vel.z);
            }
        } else {
            ImGui::Text("Camera: N/A");
        }
    }

    for(auto& ui_part : GameFlow::gStore.debugData.ui_parts)
    {
        ImGui::Text("UI: %s", ui_part.data());
    }
#if 0
    if(0){
        auto player = CreationEngineSingletonManager::GetPlayerRef();
        GameFlow::State::DebugWeaponData& debugWeaponData = GameFlow::gState.debugWeaponData;

        if(player){
            ImGui::Checkbox("Debug Weapon [X] - LaserNode [ ] - Projectile", &debugWeaponData.type);
            ImGui::InputInt("Deep Level", &debugWeaponData.deepLevel, 0, 10);

            RE::NiNode* obj_int{};
            if(debugWeaponData.type) {
                obj_int = player->getLaserNode(debugWeaponData.deepLevel);
            } else {
                obj_int = player->getProjectileNode(debugWeaponData.deepLevel);
            }

            ImGui::InputFloat3("yaw,roll,pitch", &debugWeaponData.yaw);

            if(obj_int)
            {
                ImGui::Text("Current Node [%s] %s", debugWeaponData.type? "Laser": "Projectile" ,obj_int->name.data());
                auto local = glm::mat4(*(glm::mat3x4*) &obj_int->local.rotate);
                local = glm::rowMajor4(local);
                auto euler = glm::eulerAngles(glm::quat{local});
                ImGui::Text("Current Euler [%.2f,%.2f,%.2f]", euler.x, euler.y, euler.z);
                auto qua = glm::quat{local};
                ImGui::Text("Current quaternion [x=%.2f,y=%.2f,z=%.2f,w=%.2f]", qua.x, qua.y, qua.z, qua.w);
            }

            {
                RE::NiNode* obj = player->getLaserNode(0);
                std::string laser_hierarchy = "$";
                while(obj)
                {
                    laser_hierarchy = obj->name.data() + std::string(":") + laser_hierarchy;
                    obj = obj->parent;
                }
                obj = player->getProjectileNode(0);
                std::string projectile_hierarchy = "$";
                while(obj)
                {
                    projectile_hierarchy = obj->name.data() + std::string(":") + projectile_hierarchy;
                    obj = obj->parent;
                }
                ImGui::Text("[Laser] %s", laser_hierarchy.data());
                ImGui::Text("[Projectile] %s", projectile_hierarchy.data());
            }
        }
    }


    {
        static auto camera = CreationEngineSingletonManager::GetSceneGraphRoot()->worldCamera;
        auto rt_w    = (float)g_framework->get_rendertarget_width_d3d12();
        auto rt_h    = (float)g_framework->get_rendertarget_height_d3d12();
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(rt_w, rt_h));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("Circle", nullptr,
                     ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);

        auto* draw_list = ImGui::GetWindowDrawList();
        int index = 0;
        std::vector<ImRect> drawnRects;

        {

            for (auto& point : GameFlow::gStore.debugData.points) {

                ImVec2 center{};
                auto wts = camera->WorldToScreenNormalized({ point.x, point.y, point.z });

                // Compute center position in render target space
                center.x = wts.x * rt_w;
                center.y = wts.y * rt_h;

                // Draw the point
                draw_list->AddCircleFilled(center, 5.0f, IM_COL32(255, 0, 0, 255));

                // Create the label string for this point
                char label[64];
                sprintf_s(label, "%d:[%.1f,%.1f,%.1f]", (int)(index / 2), point.x, point.y, point.z);

                // Calculate text size
                ImVec2 labelSize = ImGui::CalcTextSize(label);
                // Start with a zero offset in Y
                float offsetY = 0.0f;
                ImRect currRect(center.x, center.y + 5 + offsetY, center.x + labelSize.x, center.y + 5 + offsetY + labelSize.y);

                // Set a maximum offset value so that it eventually exits if offsets go too high
                const float maxOffset = rt_h;

                while (true) {
                    bool hasOverlap = false;
                    for (const auto& r : drawnRects) {
                        if (currRect.Overlaps(r)) {
                            hasOverlap = true;
                            break;
                        }
                    }
                    if (!hasOverlap || offsetY >= maxOffset) {
                        break;
                    }
                    offsetY += labelSize.y;
                    currRect = ImRect(center.x, center.y + 5 + offsetY, center.x + labelSize.x, center.y + 5 + offsetY + labelSize.y);
                }
                
                // Draw the label and save its rectangle for future overlap checks
                draw_list->AddText(ImVec2(center.x, center.y + 5 + offsetY), IM_COL32(255, 255, 255, 255), label);
                drawnRects.push_back(currRect);

                index++;
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
        GameFlow::gStore.debugData.points.clear();
    }
#endif
}

void CreationEngineEntry::on_config_load(const utility::Config& cfg, bool set_defaults) {
    for (IModValue& option : m_options) {
        option.config_load(cfg, set_defaults);
    }
    ModConstants::dominantEye = m_dominant_eye->value();
    ModConstants::headTrackingMultiplier = m_head_tracking_multiplier->value();
    ModConstants::headTrackingType = m_head_tracking_type->value();
    GameFlow::gStore.internalSettings.nvidiaAndTAAfix = m_taa_anf_nvidia_fix->value();
    GameFlow::gStore.internalSettings.pawnControl = m_pawn_control_rotation->value();
    GameFlow::gStore.internalSettings.preventZoom = m_disable_zoom->value();
    GameFlow::gStore.hudSettings.hudScale = m_hud_scale->value();
    GameFlow::gStore.hudSettings.perspective = (int) m_hud_perspective->value();
    GameFlow::gStore.internalSettings.alternativeJoyLayout = m_alternative_joy_layout->value();
    GameFlow::gStore.internalSettings.decoupledPitch = m_decoupled_pitch->value();
    GameFlow::gStore.internalSettings.snapTurn = m_snap_turn->value();
    GameFlow::gStore.internalSettings.snapTurnAngle = m_snap_turn_angle->value();
    GameFlow::gStore.internalSettings.motionControlAiming = m_motion_control_aiming->value();
    GameFlow::gStore.internalSettings.motionControlSmoothing = m_motion_control_smoothing->value();
}

void CreationEngineEntry::on_config_save(utility::Config& cfg)
{
    for (IModValue& option : m_options) {
        option.config_save(cfg);
    }
}
