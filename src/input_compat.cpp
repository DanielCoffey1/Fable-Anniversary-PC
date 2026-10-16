// SDK v0.10.0 InputSystem refreshes shared device/assignment vectors on every
// query. Anniversary polls from multiple guest threads; serialize full queries
// so another refresh cannot invalidate the first query's device references.
#include "generated/default/fable_anniversary_pch.h"
#include <rex/logging/macros.h>
#include <Windows.h>
#include <mutex>
#include <stdexcept>
#include <unordered_set>

namespace {
std::recursive_mutex input_mutex;
void ForwardInput(PPCContext& ctx, uint8_t* base, PPCFunc* original) {
  if (!original) throw std::runtime_error("SDK input export is unavailable");
  const std::lock_guard lock(input_mutex);
  static std::unordered_set<DWORD> observed_threads;
  if (observed_threads.insert(GetCurrentThreadId()).second) {
    REXSYS_INFO("[input-compat] Serialized input caller {} ({} distinct host threads)", GetCurrentThreadId(), observed_threads.size());
  }
  original(ctx, base);
}
}

#define ANNIVERSARY_INPUT_EXPORT(name) \
  REX_EXTERN(name) { \
    static auto* original = reinterpret_cast<PPCFunc*>(GetProcAddress( \
        GetModuleHandleW(L"rexruntime.dll"), #name)); \
    ForwardInput(ctx, base, original); \
  }
ANNIVERSARY_INPUT_EXPORT(__imp__XamInputGetCapabilities)
ANNIVERSARY_INPUT_EXPORT(__imp__XamInputGetCapabilitiesEx)
ANNIVERSARY_INPUT_EXPORT(__imp__XamInputGetState)
ANNIVERSARY_INPUT_EXPORT(__imp__XamInputSetState)
ANNIVERSARY_INPUT_EXPORT(__imp__XamInputGetKeystroke)
ANNIVERSARY_INPUT_EXPORT(__imp__XamInputGetKeystrokeEx)
#undef ANNIVERSARY_INPUT_EXPORT
