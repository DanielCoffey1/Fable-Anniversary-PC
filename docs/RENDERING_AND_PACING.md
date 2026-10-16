# Item-card corruption and pacing investigation

The user completed the Oakvale deeds after the input synchronization change, including the previously failing conversation and save loading. They reported microstutter and supplied an item card showing a missing Chocolates preview plus multicolored pixels across the description region.

The first diagnostic launch used `tools/run.ps1 -ReadbackResolve full`, retaining original vsync pacing and synchronous shader compilation. The user confirmed that both static and the missing item preview were fixed. Full resolve readback is now the launcher-script and debugger default. This result supports a GPU-to-guest memory-coherency problem for the item UI; the exact producing draw has not been isolated. Immediate readback can stall the GPU, so cheaper readback modes remain possible follow-up comparisons.

`src/frame_diagnostics.cpp` measures intervals between guest VdSwap submissions in 300-sample windows. These are not unique displayed-frame or simulation-speed measurements. Initial startup/menu windows average about 33.3 ms, with p95 intervals often near 50 ms and occasional larger loading spikes. Gameplay and same-route comparison are needed before attributing the stutter.

An optional `-FrameLimit` selects a high-resolution host swap cap and disables SDK vsync; zero (the default) keeps original pacing. The first gameplay comparison is running at 30 to isolate pacing at the original rate; its subjective result is pending. Do not infer 60 FPS safety from Fable III's tests. `-ReadbackResolve` accepts none, fast, some, or full (default). Async shader compilation stays off to avoid skipped draws while pipelines compile.
