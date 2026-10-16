// ReXGlue v0.10.0 defers content creation with a view into guest string memory.
// Keep mount names immutable for the process lifetime so deferred work cannot
// observe a reused caller buffer. The SDK retains all content/save semantics.
#include "generated/default/fable_anniversary_pch.h"
#include <rex/system/kernel_state.h>
#include <rex/logging/macros.h>
#include <windows.h>
#include <cstring>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace {
uint32_t RetainMountName(uint32_t source, uint8_t* base) {
  if (!source) return 0;
  const auto* text = reinterpret_cast<const char*>(base + source);
  const auto length = strnlen(text, 256);
  if (length == 256) throw std::runtime_error("Unterminated content mount name");
  const std::string name(text, length);
  static std::mutex mutex;
  static std::unordered_map<std::string, uint32_t> retained;
  const std::lock_guard lock(mutex);
  if (auto found = retained.find(name); found != retained.end()) return found->second;
  auto* memory = REX_KERNEL_MEMORY();
  const auto address = memory->SystemHeapAlloc(static_cast<uint32_t>(length + 1));
  if (!address) throw std::runtime_error("Cannot retain content mount name");
  std::memcpy(memory->TranslateVirtual(address), name.c_str(), length + 1);
  retained.emplace(name, address);
  REXSYS_INFO("[compat] Retained content mount name ({} bytes; {} distinct names)", length, retained.size());
  return address;
}

void ForwardContentCreate(PPCContext& ctx, uint8_t* base, unsigned argument, const char* export_name) {
  auto* original = reinterpret_cast<PPCFunc*>(GetProcAddress(GetModuleHandleW(L"rexruntime.dll"), export_name));
  if (!original) throw std::runtime_error("SDK content-create export is unavailable");
  auto& root_argument = argument == 3 ? ctx.r3 : ctx.r4;
  const auto previous = root_argument.u64;
  root_argument.u64 = RetainMountName(static_cast<uint32_t>(previous), base);
  original(ctx, base);
  root_argument.u64 = previous;
}
}

REX_EXTERN(__imp__XamContentCreate) {
  ForwardContentCreate(ctx, base, 4, "__imp__XamContentCreate");
}
REX_EXTERN(__imp__XamContentCreateEx) {
  ForwardContentCreate(ctx, base, 4, "__imp__XamContentCreateEx");
}
REX_EXTERN(__imp__XamContentCreateInternal) {
  ForwardContentCreate(ctx, base, 3, "__imp__XamContentCreateInternal");
}
