# Concurrent input polling

A user gameplay crash while speaking to a character produced Windows exception C0000374 (native heap corruption). A debugger replay failed during save loading with C000001D in `std::vector<DeviceInfo>::clear`, reached through `InputSystem::RefreshDevices` and `GetState`. The capture is local under ignored `work/`; stack-scan entries are candidates, not an unwound trace. The register dump's assumed PPC-context pointer is not valid for this native input function.

The SDK input implementation rebuilds `devices_` and `device_owners_` on every capabilities/state/vibration/keystroke query. Enumeration and subsequent device-reference use are not protected by a shared lock. Concurrent queries can clear, grow, or free a vector while another query is using it.

`src/input_compat.cpp` serializes the complete six XAM input entrypoints (including capabilities/keystroke Ex forms) with one process-wide recursive mutex, forwarding all arguments/results to the original DLL exports. Locking only enumeration would leave the later lookups exposed. No input state, device selection, or guest timing is substituted. This is scoped to this executable rather than changing the shared SDK installation.

The rebuilt launch logged four distinct input-polling host threads. Save-loading and conversation reproduction remain pending; the race is a supported explanation, not yet a confirmed resolution of every reported crash.
