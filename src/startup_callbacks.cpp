#include "generated/default/fable_anniversary_pch.h"
#include "startup_callbacks.h"
#include <rex/runtime.h>
#include <rex/system/function_dispatcher.h>
#include <stdexcept>

REX_EXTERN(sub_8247B120);

namespace {
// Verified 0xC-byte leaf at 8247E5D8 in the hash-pinned local executable.
// Keep this separate from the preceding virtual-dispatch routine.
void StartupMemberCallback(PPCContext& ctx, uint8_t* base) {
  REX_FUNC_PROLOGUE();
  ctx.r3.u64 = REX_LOAD_U32(ctx.r3.u32 + 0x34);
  ctx.r4.s64 = 100;
  sub_8247B120(ctx, base);
}
}

void RegisterAnniversaryStartupCallbacks(rex::Runtime* runtime) {
  if (!runtime->function_dispatcher()->SetFunction(0x8247E5D8, StartupMemberCallback)) {
    throw std::runtime_error("Cannot register Anniversary startup callback");
  }
}
