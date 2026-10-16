#include "generated/default/fable_anniversary_pch.h"
#include "startup_callbacks.h"
#include <rex/runtime.h>
#include <rex/system/function_dispatcher.h>
#include <stdexcept>
#include <array>

REX_EXTERN(sub_8247B120);
#define ANNIVERSARY_VM_ADVANCE(address, handler) REX_EXTERN(handler);
#include "vm_advance_callbacks.inc"
#undef ANNIVERSARY_VM_ADVANCE

namespace {
// Four-instruction VM leaves verified at 835496C0..835496F0 and
// 83549750..83549760: advance bytecode IP once, then tail-call the handler.
template<PPCFunc* Handler>
void AdvanceVmAndDispatch(PPCContext& ctx, uint8_t* base) {
  REX_FUNC_PROLOGUE();
  ctx.r11.u64 = REX_LOAD_U32(ctx.r4.u32 + 0x18);
  ctx.r11.s64 += 1;
  REX_STORE_U32(ctx.r4.u32 + 0x18, ctx.r11.u32);
  Handler(ctx, base);
}
// Verified 0xC-byte leaf at 8247E5D8 in the hash-pinned local executable.
// Keep this separate from the preceding virtual-dispatch routine.
void StartupMemberCallback(PPCContext& ctx, uint8_t* base) {
  REX_FUNC_PROLOGUE();
  ctx.r3.u64 = REX_LOAD_U32(ctx.r3.u32 + 0x34);
  ctx.r4.s64 = 100;
  sub_8247B120(ctx, base);
}

// Verified six-instruction VM dispatch leaf at 8242ADF0.
void ScriptDispatchCallback(PPCContext& ctx, uint8_t* base) {
  REX_FUNC_PROLOGUE();
  ctx.r11.s64 = static_cast<int64_t>(static_cast<int32_t>(0x83F40000u));
  ctx.r3.u64 = REX_LOAD_U32(ctx.r4.u32 + 0x14);
  ctx.r10.s64 = ctx.r11.s64 + 0x1500;
  ctx.r9.u64 = REX_LOAD_U32(ctx.r10.u32 + 0x118);
  ctx.ctr.u64 = ctx.r9.u64;
  REX_CALL_INDIRECT_FUNC(ctx.ctr.u32);
}

// Identical inspected null-checked heap dispatch entries at 83B8BB70/83B8BB98.
void HeapReleaseCallback(PPCContext& ctx, uint8_t* base) {
  REX_FUNC_PROLOGUE();
  ctx.r4.u64 = ctx.r3.u64;
  ctx.cr6.compare<uint32_t>(ctx.r3.u32, 0, ctx.xer);
  if (ctx.cr6.eq) return;
  ctx.r11.s64 = static_cast<int64_t>(static_cast<int32_t>(0x84090000u));
  ctx.r3.u64 = REX_LOAD_U32(ctx.r11.u32 + 0x70B8);
  ctx.r10.u64 = REX_LOAD_U32(ctx.r3.u32);
  ctx.r9.u64 = REX_LOAD_U32(ctx.r10.u32 + 0x14);
  ctx.ctr.u64 = ctx.r9.u64;
  REX_CALL_INDIRECT_FUNC(ctx.ctr.u32);
}

// Inspected argument-shuffling heap dispatch entry at 83B8BB40.
void HeapRequestCallback(PPCContext& ctx, uint8_t* base) {
  REX_FUNC_PROLOGUE();
  ctx.r11.s64 = static_cast<int64_t>(static_cast<int32_t>(0x84090000u));
  ctx.r6.u64 = ctx.r5.u64;
  ctx.r5.u64 = ctx.r4.u64;
  ctx.r4.u64 = ctx.r3.u64;
  ctx.r10.s64 = static_cast<int64_t>(static_cast<int32_t>(0x82290000u));
  ctx.r3.u64 = REX_LOAD_U32(ctx.r11.u32 + 0x70B8);
  ctx.r8.s64 = 120;
  ctx.r7.s64 = ctx.r10.s64 + 0x2D6C;
  ctx.r9.u64 = REX_LOAD_U32(ctx.r3.u32);
  ctx.r11.u64 = REX_LOAD_U32(ctx.r9.u32);
  ctx.ctr.u64 = ctx.r11.u64;
  REX_CALL_INDIRECT_FUNC(ctx.ctr.u32);
}

// Inspected VM extension dispatch family, 8242AE08 and 8242AE30..8242B0D0.
template<unsigned Bank>
void VmExtensionDispatch(PPCContext& ctx, uint8_t* base) {
  REX_FUNC_PROLOGUE();
  ctx.r11.u64 = REX_LOAD_U32(ctx.r4.u32 + 0x18);
  ctx.r10.s64 = static_cast<int64_t>(static_cast<int32_t>(0x83F40000u));
  if constexpr (Bank == 0) {
    ctx.r8.s64 = ctx.r10.s64 + 0x1900;
    ctx.r9.s64 = ctx.r11.s64 + 1;
    ctx.r7.u64 = REX_LOAD_U8(ctx.r11.u32);
    REX_STORE_U32(ctx.r4.u32 + 0x18, ctx.r9.u32);
    ctx.r6.u64 = ctx.r7.u32 << 2;
    ctx.r11.u64 = REX_LOAD_U32(ctx.r6.u32 + ctx.r8.u32);
  } else {
    ctx.r10.s64 += 0x1900;
    ctx.r9.s64 = ctx.r11.s64 + 1;
    ctx.r7.s64 = ctx.r10.s64 + Bank * 0x400;
    ctx.r8.u64 = REX_LOAD_U8(ctx.r11.u32);
    REX_STORE_U32(ctx.r4.u32 + 0x18, ctx.r9.u32);
    ctx.r6.u64 = ctx.r8.u32 << 2;
    ctx.r11.u64 = REX_LOAD_U32(ctx.r6.u32 + ctx.r7.u32);
  }
  ctx.ctr.u64 = ctx.r11.u64;
  REX_CALL_INDIRECT_FUNC(ctx.ctr.u32);
}
}

void RegisterAnniversaryStartupCallbacks(rex::Runtime* runtime) {
  auto addMissing = [&](uint32_t address, PPCFunc* callback) {
    auto* dispatcher = runtime->function_dispatcher();
    if (!dispatcher->GetFunction(address) && !dispatcher->SetFunction(address, callback)) {
      throw std::runtime_error("Cannot register Anniversary startup callback");
    }
  };
  addMissing(0x8247E5D8, StartupMemberCallback);
#define ANNIVERSARY_VM_ADVANCE(address, handler) addMissing(address, AdvanceVmAndDispatch<handler>);
#include "vm_advance_callbacks.inc"
#undef ANNIVERSARY_VM_ADVANCE
  addMissing(0x8242ADF0, ScriptDispatchCallback);
  addMissing(0x83B8BB70, HeapReleaseCallback);
  addMissing(0x83B8BB98, HeapReleaseCallback);
  addMissing(0x83B8BB40, HeapRequestCallback);
  const std::array<PPCFunc*, 16> extensions = {
    VmExtensionDispatch<0>, VmExtensionDispatch<1>, VmExtensionDispatch<2>, VmExtensionDispatch<3>,
    VmExtensionDispatch<4>, VmExtensionDispatch<5>, VmExtensionDispatch<6>, VmExtensionDispatch<7>,
    VmExtensionDispatch<8>, VmExtensionDispatch<9>, VmExtensionDispatch<10>, VmExtensionDispatch<11>,
    VmExtensionDispatch<12>, VmExtensionDispatch<13>, VmExtensionDispatch<14>, VmExtensionDispatch<15>
  };
  for (unsigned bank = 0; bank < extensions.size(); ++bank) {
    addMissing(bank == 0 ? 0x8242AE08 : 0x8242AE30 + (bank - 1) * 0x30, extensions[bank]);
  }
}
