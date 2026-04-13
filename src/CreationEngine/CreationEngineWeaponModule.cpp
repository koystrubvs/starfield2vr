#include "CreationEngineWeaponModule.h"
#include "CreationEngineMotionControlModule.h"
#include "CreationEngineSingletonManager.h"
#include <CreationEngine/memory/ScanHelper.h>
#include <CreationEngine/models/GameFlow.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_major_storage.hpp>

void CreationEngineWeaponModule::install_hooks() {

        REL::Relocation<uintptr_t> onUpdateWorldFnAddr{ (uintptr_t )MemoryScan::mod + 0x37de714  };
        m_onUpdateWorldHook = std::make_unique<FunctionHook>(onUpdateWorldFnAddr.address(), reinterpret_cast<uintptr_t >(&onUpdateWorld));
        if(!m_onUpdateWorldHook->create()) {
            spdlog::error("Failed to create hook for m_onUpdateWorldHook");
        }
        /*REL::Relocation<uintptr_t> shouldCastFromMuzzleFnAddr{ (uintptr_t )MemoryScan::mod + 0x1fea818 };
        m_shouldCastFromMuzzleHook = std::make_unique<FunctionHook>(shouldCastFromMuzzleFnAddr.address(), reinterpret_cast<uintptr_t >(&shouldCastFromMuzzle));
        if(!m_shouldCastFromMuzzleHook->create()) {
            spdlog::error("Failed to create hook for m_shouldCastFromMuzzleHook");
        }*/
        REL::Relocation<uintptr_t> getEquippedWeaponFnAddr{ (uintptr_t )MemoryScan::mod + 0x26c2b40 };
        m_getEquippedWeaponHook = std::make_unique<FunctionHook>(getEquippedWeaponFnAddr.address(), reinterpret_cast<uintptr_t >(&getEquippedWeapon));
        if(!m_getEquippedWeaponHook->create()) {
            spdlog::error("Failed to create hook for m_getEquippedWeaponHook");
        }
}

uintptr_t CreationEngineWeaponModule::onUpdateWorld(RE::NiAVObject* a_object, RE::NiUpdateData* a_data) {
    static auto instance = Get();
    using func_t = decltype(CreationEngineWeaponModule::onUpdateWorld);
    GameFlow::State::DebugWeaponData& debugWeaponData = GameFlow::gState.debugWeaponData;

    static auto original_fn = instance->m_onUpdateWorldHook->get_original<func_t>();

    auto player = CreationEngineSingletonManager::GetPlayerRef();
    if(player) {
        RE::NiNode* obj_int{};
        if(debugWeaponData.type) {
            obj_int = player->getLaserNode(debugWeaponData.deepLevel);
        } else {
            obj_int = player->getProjectileNode(debugWeaponData.deepLevel);
        }

        if(obj_int && (uintptr_t )obj_int == (uintptr_t)a_object) {
            auto motionCtrl = CreationEngineMotionControlModule::Get();

            if (motionCtrl->IsActive()) {
                // Motion controls active: override weapon node rotation with controller pose
                auto weaponTransform = motionCtrl->GetWeaponTransform();

                // Extract controller rotation (column-major glm::mat4 → 3x3)
                auto controllerRot = glm::mat4(glm::mat3(weaponTransform));
                controllerRot = glm::rowMajor4(controllerRot);

                // Get parent's world rotation using same cast pattern as existing code
                if (a_object->parent) {
                    auto parentWorld = glm::mat4(*(glm::mat3x4*)&a_object->parent->world.rotate);
                    parentWorld = glm::rowMajor4(parentWorld);
                    auto parentWorldInverse = glm::inverse(glm::mat3(parentWorld));
                    auto localRot = glm::mat4(parentWorldInverse * glm::mat3(controllerRot));
                    localRot = glm::rowMajor4(localRot);
                    a_object->local.rotate = *(RE::NiMatrix3*)&localRot;
                }
            } else {
                // Existing debug rotation logic (when motion controls are off)
                auto rotate_obj = a_object;
                auto rotate = glm::mat4(*(glm::mat3x4*)& rotate_obj->local.rotate);
                rotate = glm::rowMajor4(rotate);
                rotate = glm::rotate(rotate, glm::radians(debugWeaponData.roll), { 1.0f, 0.0f, 0.0f });
                rotate = glm::rotate(rotate, glm::radians(debugWeaponData.pitch), { 0.0f, 1.0f, 0.0f });
                rotate = glm::rotate(rotate, glm::radians(debugWeaponData.yaw), { 0.0f, 0.0f, 1.0f });
                rotate = glm::rowMajor4(rotate);
                rotate_obj->local.rotate = *(RE::NiMatrix3*)&rotate;
            }
        }
    }


    auto result = original_fn(a_object, a_data);
    return result;
}

char CreationEngineWeaponModule::shouldCastFromMuzzle(RE::EquippedWeapon::Data* a_equippedWeapon, RE::PlayerCharacter* a_player)
{
    static auto instance = Get();
    using func_t = decltype(CreationEngineWeaponModule::shouldCastFromMuzzle);
//    static char old_Value = 0;
    /*if(a_equippedWeapon && a_equippedWeapon->aimModule && a_equippedWeapon->aimModule->aim_template && a_equippedWeapon->aimModule->aim_template->opticalSightsData) {
        auto& opticalSightsData = a_equippedWeapon->aimModule->aim_template->opticalSightsData;
        old_Value = opticalSightsData->unk40;
        const auto empty = "";
        opticalSightsData->nodeName = empty;
    }*/
    static auto original_func = instance->m_onUpdateWorldHook->get_original<func_t>();
//    spdlog::info("shouldCastFromMuzzle {} {}", a_equippedWeapon->IsAimFromMuzzle(), a_equippedWeapon->IsSomething());
//    auto result = original_func(a_equippedWeapon, a_player);

    /*if(a_equippedWeapon && a_equippedWeapon->aimModule && a_equippedWeapon->aimModule->aim_template && a_equippedWeapon->aimModule->aim_template->opticalSightsData) {
        auto& opticalSightsData = a_equippedWeapon->aimModule->aim_template->opticalSightsData;
        opticalSightsData->unk40 = old_Value;
    }*/
//    spdlog::info("shouldCastFromMuzzle result {}", (bool)result);
    return original_func(a_equippedWeapon, a_player);
}

char CreationEngineWeaponModule::getEquippedWeapon(RE::PlayerCharacter* a_player, unsigned int id_or_slot, RE::EquippedItem* a_equippedItem)
{
    static auto instance = Get();
    using func_t = decltype(CreationEngineWeaponModule::getEquippedWeapon);
    static auto original_func = instance->m_getEquippedWeaponHook->get_original<func_t>();
    auto result = original_func(a_player, id_or_slot, a_equippedItem);
    if(a_equippedItem && a_equippedItem->weaponInstanceData && a_equippedItem->weaponInstanceData->flags[2] == 48) {
        auto& a_equippedWeapon = a_equippedItem->equippedWeapon;
        if(a_equippedWeapon && a_equippedWeapon->aimModule && a_equippedWeapon->aimModule->aim_template && a_equippedWeapon->aimModule->aim_template->opticalSightsData) {
            auto& opticalSightsData  = a_equippedWeapon->aimModule->aim_template->opticalSightsData;
            opticalSightsData->unk40 = 1;
        }

    }
    return result;

}
