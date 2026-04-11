"""Scan Starfield.exe for all byte patterns used by starfield2vr mod."""
import sys
import re

EXE_PATH = r"C:\Zona Downloads\Starfield.Digital.Premium.Edition-InsaneRamZes\Starfield.exe"

# All patterns from offsets.h with their names and types
PATTERNS = [
    # (name, pattern_hex, type, old_steam_offset)
    ("NiCamera::SetFrustumVfunc [147392]",
     "48 89 5C 24 10 48 89 74 24 18 57 48 83 EC 20 48 8B F2 48 8B F9 48 8D 99 08",
     "func", 0x2b65900),

    ("NiCamera::CalcFrustumVfunc [147416]",
     "48 8B C4 48 89 58 10 48 89 70 18 57 48 81 EC D0 00 00 00 C5 F8 29 70 E8 C5 F8 29 78 D8 C5 78 29 40 C8 C5 78 29 48 B8 C5 78 28",
     "func", 0x2b68830),

    ("CreationRenderer::RenderGraphFrameStart [143812]",
     "48 89 5C 24 08 48 89 74 24 10 48 89 7C 24 18 55 41 54 41 55 41 56 41 57 48 81 EC F0",
     "func", 0x29D5B30),

    ("CreationRenderer::GetDXGIState [145355]",
     "8B C1 44 8B C1 25 1C 58 04 00 3D 1C 58 04 00 75 06 B8 C3 0A 00 00 C3 41 F6 C0 02 0F 85 87 00 00 00",
     "func", 0x2A81CE0),

    ("CreationRenderer::GetCommandList [144161]",
     "48 89 5C 24 10 48 89 74 24 18 57 48 83 EC 20 48 8B 99 38",
     "func", 0x2A009B0),

    ("CreationRenderer::GlobalFrameCount [883900]",
     "8B 15 ? ? ? ? 89 55 38 41 8B 4E 08",
     "instr_2_6", 0x5D56484),

    ("CreationRenderer::OnUpdateConstantBufferView [142800]",
     "48 8B C4 48 89 58 10 48 89 70 18 57 41 56 41 57 48 81 EC 40",
     "func", 0x29802D0),

    ("Nvidia::onSetReflexMarkerInternal [141825]",
     "48 89 5C 24 10 56 48 83 EC 20 8B F2",
     "func", 0x290e010),

    ("PlayerCamera::Singleton [937788]",
     "48 8B 05 ? ? ? ? 48 8B 48 10 48 85 C9 74 12 48 3B 88 D8 01 00 00",
     "instr_3_7", 0x60b9c70),

    ("Streamline::SetDlssOptions [944432]",
     "48 8B 05 ? ? ? ? 48 8D 54 24 30 48 8D 4D E0 FF D0 40 38 7B 0C",
     "instr_3_7", 0x60D7508),

    ("GlobalRenderSettings [937583]",
     "48 8B 05 ? ? ? ? C5 FB 10 8E D0 01 00 00",
     "instr_3_7", 0x5EB6390),

    ("GlobalDirectX12Module2 [944397]",
     "48 8B 0D ? ? ? ? 49 8B 43 28",
     "instr_3_7", 0x60D73D8),

    ("GlobalSceneGraphRoot [936470]",
     "48 8B 05 ? ? ? ? 48 8B B8 80 00 00 00 48 8B CB",
     "instr_3_7", 0x5E8D158),

    ("GlobalPlayerRef [922868]",
     "48 8B 05 ? ? ? ? F6 80 22 11 00 00 08 74 19",
     "instr_3_7", 0x5E200A0),

    # VTable patterns (RTTI-based, searched by class name)
    ("FirstPersonState vtable [459617]",
     ".?AVFirstPersonState@@",
     "vtable", 0x4c57c88),

    ("BSFadeNode vtable [472039]",
     ".?AVBSFadeNode@@",
     "vtable", 0x4cbd600),

    ("BSPCGamepadDevice vtable [470133]",
     ".?AVBSPCGamepadDevice@@",
     "vtable", 0x4CAEC98),

    ("Scaleform::MovieImpl vtable [303817]",
     ".?AVMovieImpl@GFx@Scaleform@@",
     "vtable", 0x3b2cee8),

    ("TemporalAA_idTech7 vtable [497712]",
     ".?AVTemporalAA_idTech7RenderPass@CreationRendererPrivate@@",
     "vtable", 0x4E37C78),
]


def hex_pattern_to_regex(pattern_str):
    """Convert hex pattern with ? wildcards to regex bytes pattern."""
    parts = pattern_str.strip().split()
    regex_parts = []
    for part in parts:
        if part == '?':
            regex_parts.append(b'.')
        elif '?' in part:
            regex_parts.append(b'.')
        else:
            byte_val = int(part, 16)
            # Escape regex special bytes
            regex_parts.append(re.escape(bytes([byte_val])))
    return b''.join(regex_parts)


def scan_for_pattern(data, pattern_str):
    """Scan binary data for a hex byte pattern. Returns list of offsets."""
    regex = hex_pattern_to_regex(pattern_str)
    matches = []
    for m in re.finditer(regex, data):
        matches.append(m.start())
    return matches


def scan_for_rtti_string(data, class_name):
    """Scan for RTTI type descriptor string and find vtable reference."""
    # Search for the type descriptor string
    search_bytes = class_name.encode('ascii')
    matches = []
    pos = 0
    while True:
        idx = data.find(search_bytes, pos)
        if idx == -1:
            break
        matches.append(idx)
        pos = idx + 1
    return matches


def resolve_instruction(data, match_offset, rel_offset_begin, instruction_size):
    """Resolve RIP-relative instruction to absolute offset."""
    import struct
    addr = match_offset
    rel_value = struct.unpack_from('<i', data, addr + rel_offset_begin)[0]
    target = addr + rel_value + instruction_size
    return target


def main():
    print(f"Loading {EXE_PATH}...")
    with open(EXE_PATH, 'rb') as f:
        data = f.read()
    print(f"Loaded {len(data):,} bytes\n")

    found = 0
    not_found = 0
    results = {}

    for name, pattern, ptype, old_offset in PATTERNS:
        if ptype == "vtable":
            offsets = scan_for_rtti_string(data, pattern)
            if offsets:
                print(f"OK   {name}")
                print(f"     RTTI string found at {len(offsets)} location(s): {', '.join(f'0x{o:X}' for o in offsets[:3])}")
                # We'd need full RTTI parsing to get vtable, but string presence = class exists
                found += 1
                results[name] = offsets
            else:
                print(f"FAIL {name}")
                print(f"     RTTI string NOT found in exe")
                not_found += 1
        else:
            offsets = scan_for_pattern(data, pattern)
            if offsets:
                print(f"OK   {name}")
                for o in offsets[:3]:
                    delta = o - old_offset
                    delta_str = f"+0x{delta:X}" if delta >= 0 else f"-0x{-delta:X}"

                    extra = ""
                    if ptype.startswith("instr_"):
                        parts = ptype.split("_")
                        rel_begin = int(parts[1])
                        instr_sz = int(parts[2])
                        target = resolve_instruction(data, o, rel_begin, instr_sz)
                        old_target_delta = target - old_offset
                        extra = f"  -> target=0x{target:X}"

                    print(f"     offset=0x{o:X} (old=0x{old_offset:X}, delta={delta_str}){extra}")
                if len(offsets) > 1:
                    print(f"     WARNING: {len(offsets)} matches found!")
                found += 1
                results[name] = offsets
            else:
                print(f"FAIL {name}")
                print(f"     Pattern not found (old offset was 0x{old_offset:X})")
                not_found += 1
        print()

    print("=" * 60)
    print(f"Results: {found} found, {not_found} not found, {len(PATTERNS)} total")
    print(f"Success rate: {found/len(PATTERNS)*100:.0f}%")


if __name__ == "__main__":
    main()
