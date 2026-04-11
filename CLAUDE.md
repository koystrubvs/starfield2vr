# starfield2vr

VR mod for Starfield, forked from [mutars/starfield2vr](https://github.com/mutars/starfield2vr).

## Project overview

- **Language:** C++23
- **Build system:** CMake 3.27+ with Ninja
- **Compilers:** MSVC (v143) or clang-cl
- **Output:** `dxgi.dll` — DLL proxy loaded by the game at startup
- **Dependencies:** vrframework (submodule), CommonLibSF (submodule, optional), ViGEmClient, Streamline, FidelityFX

## Build

**IMPORTANT: Never delete `build/build-release-msvc/` — it contains downloaded dependencies (_deps). Deleting it forces a full re-download (3-5 min). For incremental rebuilds, just run the build step.**

```bat
# Full build (configure + build) — only needed on first run or after CMake changes
build.bat

# Incremental rebuild (fast, only recompiles changed files)
build.bat rebuild
```

Or manually:
```bat
# From VS Developer Command Prompt (vcvars64)
# First time only:
cmake --preset build-release-msvc -DCMAKE_MAKE_PROGRAM="<path-to-ninja>"
# Incremental build (use this for day-to-day development):
cmake --build build/build-release-msvc --config RelWithDebInfo
```

Output: `build/build-release-msvc/dxgi.dll` → copy to Starfield game directory.

### Presets

| Preset | Compiler | Use |
|--------|----------|-----|
| `build-release-msvc` | MSVC | Primary release build with debug info |
| `build-release-clang-cl` | clang-cl | Alternative, more optimized |
| `build-debug-msvc` | MSVC | Debug build |

### Key CMake options

- `SIGNATURE_SCAN=ON` (default) — runtime byte-pattern scanning, version-resilient
- `USE_STARFIELD_SDK_LITE=ON` (default) — lightweight SDK, no full CommonLibSF dependency
- `XBOX_STORE=ON` — build for Xbox/GamePass version
- `HOOK_METHOD=dxgi` (default) — DLL proxy method

## Architecture

```
src/
├── Main.cpp                         # DLL entry point, spawns init thread
├── ModConfig.cpp/h                  # Mod configuration and UI
└── CreationEngine/
    ├── memory/
    │   ├── offsets_table.h          # Static offset table (Steam + Xbox)
    │   ├── offsets.h                # Offset resolution with byte patterns
    │   └── ScanHelper.h            # Pattern scanning utilities
    ├── CreationEngineCameraManager   # VR camera, head tracking, frustum
    ├── CreationEngineRendererModule  # DX12 hooks, TAA, constant buffers
    ├── CreationEngineGameLoop        # Game loop hooks
    ├── CreationEngineInputManager    # Controller input
    ├── CreationEngineEntry           # Mod initialization and UI overlay
    └── CreationEngineSettings        # Game settings integration
```

## Offset system (critical for updates)

When Starfield updates, memory addresses shift. The mod uses two strategies:

1. **Signature scan** (`SIGNATURE_SCAN` define) — searches for byte patterns in the exe at runtime. Most resilient to updates.
2. **Static offsets** (`offsets_table.h`) — fallback hardcoded addresses per version.

### Updating for a new game version

1. Run `tools/scan_patterns.py` against the new `Starfield.exe`
2. Check which patterns still match (expect 90%+ to survive minor updates)
3. For broken patterns — find the function in a disassembler, extract new bytes
4. Update `offsets_table.h` with new Steam/Xbox addresses
5. Update patterns in `offsets.h` if byte sequences changed
6. Build and test

### Offset types

- **sig** (FuncRelocation) — direct function address via byte pattern
- **instr** (InstructionRelocation) — RIP-relative instruction resolving to a global/singleton
- **vtable** (VTable) — RTTI-based vtable lookup by class name

## Install mod

Copy `dxgi.dll` + `openxr_loader.dll` to game root (next to `Starfield.exe`).

### Requirements
- VR headset configured as OpenXR runtime
- ViGEmBus v1.22.0 driver
- Game set to Windowed mode
- Frame Generation, VSync, Motion Blur, Depth of Field — OFF

## Git remotes

- `origin` — github.com/koystrubvs/starfield2vr (this fork)
- `upstream` — github.com/mutars/starfield2vr (original)

## Code style

- C++23 standard
- Namespaces follow game engine structure (`Steam::MemoryOffsets::*`)
- Hook functions use `safetyhook::create_inline` or `FunctionHook` wrappers
- Logging via spdlog
