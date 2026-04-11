#pragma once

namespace OffsetsTable
{
    struct OffsetMapping
    {
        int       ID;
        uintptr_t STEAM_OFFSET;
        uintptr_t XBOX_OFFSET;
    };

    // Starfield 1.16.236.0 (Free Lanes update, April 7 2026)
    // Steam offsets updated via signature scan; Xbox offsets need separate verification
    inline OffsetMapping offsetMappings[] = {
        {  147392, 0x2BE68B0, 0x3858468 }, // sig  NiCamera::SetFrustumVfunc
        {  147416, 0x2BE97F0, 0x385728C }, // sig  NiCamera::CalcFrustumVfunc
        {  470133, 0x4CAEC98, 0x2F6EA94 }, // vtable BSPCGamepadDevice (needs RTTI rescan)
        {  303817, 0x3b2cee8, 0x0C6FE10 }, // vtable Scaleform::MovieImpl (needs RTTI rescan)
        {  143812, 0x2A56050, 0x371045C }, // sig  RenderGraphFrameStart
        {  145355, 0x2B02520, 0x376EA3C }, // sig  GetDXGIState
        {  144161, 0x2A80ED0, 0x372CE5C }, // sig  GetCommandList
        {  883900, 0x5E7D8DC, 0x6A8C2B0 }, // instr GlobalFrameCount
        {  142800, 0x29FF5E0, 0x36846FC }, // sig  OnUpdateConstantBufferView
        {  497712, 0x4E37C78, 0x382321C }, // vtable TemporalAA_idTech7 (needs RTTI rescan)
        {  937788, 0x61E47F0, 0x6A6A150 }, // instr PlayerCamera::Singleton
        {  944432, 0x62021D0, 0x6A8E530 }, // instr Streamline::SetDlssOptions
        {  937583, 0x5FE0EF8, 0x6866740 }, // instr GlobalRenderSettings
        {  944397, 0x6202098, 0x6A8C220 }, // instr GlobalDirectX12Module2
        {  936470, 0x5FB7C68, 0x6AADB30 }, // instr GlobalSceneGraphRoot
        {  922868, 0x5F4A5D0, 0x6701238 }, // instr GlobalPlayerRef
        {  141825, 0x298D310, 0x35F06A0 }, // sig  Nvidia::onSetReflexMarker
        {  459617, 0x4c57c88, 0x556CA28 }, // vtable FirstPersonState (needs RTTI rescan)
        {  472039, 0x4cbd600, 0x55D4A88 } // vtable BSFadeNode (needs RTTI rescan)
    };

    inline std::unordered_map<int, OffsetMapping> offsetMap = [] {
        std::unordered_map<int, OffsetMapping> map;
        for (const auto& mapping : offsetMappings) {
            map[mapping.ID] = mapping;
        }
        return map;
    }();

    inline uintptr_t GetOffset(int ID)
    {
        auto offset_map = offsetMap[ID];
        auto offset     = offset_map.STEAM_OFFSET;

#ifdef XBOX_STORE
        offset = offset_map.XBOX_OFFSET;
#endif

#ifndef USE_STARFIELD_SDK_LITE
        uintptr_t library_offset = REL::ID{ a_id }.offset();
        if (offset != library_offset) {
            spdlog::error("Offset does not match offset by ID for id={} scan={:x} lib={:x}", a_id, offset, library_offset);
        }
    }
#endif
    return offset;
}
} // namespace MemoryOffsets
