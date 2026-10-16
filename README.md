# Fable Anniversary PC

Private standalone Xbox 360 static-recompilation research project using ReXGlue SDK v0.10.0. This is a separate project from Fable II and III. Controller input, audio, the loading-noise protection, saving/reloading, and several quests have been user-tested. A later crash while speaking to a character remains under investigation; Windows reported native heap corruption (`C0000374`). Sustained stability, rendering, and pacing validation remain in progress.

## Local content

Supply your own disc files under `game/`, including `default.xex` and `WellingtonGame/`. Game files, ISO images, generated game code, caches, and saves are excluded from Git. The local source ISO is read without modification.

The inspected executable has SHA-256 `2bba8c23d8c2da28741f166030ffacfddf3ab35a4d459e818f07cac58debfa77`, title ID `4D530A87`, media ID `315B4575`, version `0.0.0.2`. Extraction yielded 3,290 files totaling 5,113,391,662 bytes, including the disc system-update directory, which is not executed by the project.

## Development

Install Visual Studio 2022 Build Tools with the C++ desktop, Clang, CMake/Ninja, and Windows SDK components. `tools/setup-sdk.ps1` obtains the pinned SDK; `tools/build.ps1` loads the x64 build environment and verifies the supported executable hash before building.

```powershell
python tools/extract-disc.py "path/to/your.iso" game
./tools/setup-sdk.ps1
./tooling/rexglue-sdk/bin/rexglue.exe codegen fable_anniversary_manifest.toml
./tools/build.ps1 -Jobs 4
```

Do not re-run project initialization over customized sources. Do not reuse Fable II or III function boundaries, timing patches, or visibility workarounds without Anniversary-specific evidence.

## Albion Reborn

Albion Reborn supports both this built local project and a managed Anniversary runtime package. Setup accepts the supported USA/Europe ISO or an extracted `default.xex`, checks the executable hash, and imports the `WellingtonGame` files. The packaged runtime retains borderless fullscreen, selected resolution, full resolve readback, synchronous shader compilation, and the validated 30 FPS cap. The existing Documents/fable_anniversary saves are preserved; the user verified Guild save loading and gameplay from the managed package.

The private release agent includes Anniversary's main revision in its change signature, builds it in an isolated checkout, and packages its executable plus runtime DLLs. Launcher updates replace managed Anniversary runtime files alongside II/III. Save/content-preservation checks run before publication. The agent's scheduled check normally notices main changes within 15 minutes while its host is online. Players obtain packages through Albion Reborn's public releases without private-repository access; their own game content is required.

## Build dependency handling

The project-owned `cmake/AnniversaryRuntime.cmake` adapts the SDK v0.10.0 build glue to use one completion stamp and generated-source byproducts. `tools/Invoke-Codegen.ps1` restores the manifest timestamp only when an exact SHA-256 comparison confirms the SDK's version-stamp rewrite did not change its bytes. This avoids repeated full analysis for native host-source edits while preserving real input dependencies. An unchanged Ninja build was checked to report no work.
