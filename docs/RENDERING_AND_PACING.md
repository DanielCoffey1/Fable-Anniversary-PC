# Item-card corruption and pacing investigation

The user completed the Oakvale deeds after the input synchronization change, including the previously failing conversation and save loading. They reported microstutter and supplied an item card showing a missing Chocolates preview plus multicolored pixels across the description region.

The first diagnostic launch uses `tools/run.ps1 -ReadbackResolve full`, retaining original vsync pacing and synchronous shader compilation. Full readback is a correctness comparison, not a recommended performance default. Its visual result remains pending.

`src/frame_diagnostics.cpp` measures intervals between guest VdSwap submissions in 300-sample windows. These are not unique displayed-frame or simulation-speed measurements. Initial startup/menu windows average about 33.3 ms, with p95 intervals often near 50 ms and occasional larger loading spikes. Gameplay and same-route comparison are needed before attributing the stutter.

An optional `-FrameLimit` selects a high-resolution host swap cap and disables SDK vsync; zero (the default) keeps original pacing. Start comparisons at 30 to isolate pacing at the original rate. This option compiles but has not yet been gameplay validated; do not infer 60 FPS safety from Fable III's tests. `-ReadbackResolve` accepts none (default), fast, some, or full. Async shader compilation stays off to avoid skipped draws while pipelines compile.
