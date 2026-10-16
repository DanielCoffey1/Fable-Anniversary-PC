// Reject malformed final PCM before the device conversion clips it into noise.
#include "generated/default/fable_anniversary_pch.h"
#include <rex/logging/macros.h>
#include <rex/system/kernel_state.h>
#include <Windows.h>
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <mutex>
#include <stdexcept>

REX_EXTERN(__imp__XAudioSubmitRenderDriverFrame) {
  static auto* original = reinterpret_cast<PPCFunc*>(GetProcAddress(
      GetModuleHandleW(L"rexruntime.dll"), "__imp__XAudioSubmitRenderDriverFrame"));
  if (!original) throw std::runtime_error("SDK audio submission export is unavailable");
  {
    static std::mutex mutex;
    const std::lock_guard lock(mutex);
    static uint64_t frames = 0, invalid = 0, clipped = 0, rejected = 0;
    static bool reported = false;
    static uint32_t silence = 0;
    static float peak = 0;
    bool malformed = false;
    for (unsigned i = 0; i < 6 * 256; ++i) {
      const float sample = std::bit_cast<float>(REX_LOAD_U32(ctx.r4.u32 + i * 4));
      if (!std::isfinite(sample)) { ++invalid; malformed = true; }
      else {
        peak = std::max(peak, std::abs(sample));
        if (std::abs(sample) > 1.0f) ++clipped;
        // Allow substantial legitimate mixer headroom; captured corruption was
        // ~1e24. This is not a compressor or a normal-volume adjustment.
        if (std::abs(sample) > 32.0f) malformed = true;
      }
    }
    if (malformed) {
      if (!silence) {
        silence = REX_KERNEL_MEMORY()->SystemHeapAlloc(6 * 256 * sizeof(float));
        if (!silence) throw std::runtime_error("Cannot allocate silent audio frame");
        std::memset(base + silence, 0, 6 * 256 * sizeof(float));
      }
      ++rejected;
      if (!reported) {
        REXSYS_WARN("[audio-guard] Malformed loading PCM detected: peak={}, caller={:08X}; submitting silence", peak, static_cast<uint32_t>(ctx.lr));
        reported = true;
      }
    }
    if (++frames % 188 == 0) {
      if (rejected || invalid || clipped) REXSYS_INFO("[audio-guard] frames={} peak={} nonfinite={} over_unity={} silenced={}", frames, peak, invalid, clipped, rejected);
      peak = 0; invalid = 0; clipped = 0; rejected = 0;
    }
    const auto saved = ctx.r4.u64;
    if (malformed) ctx.r4.u64 = silence;
    // SDK copies the frame synchronously; guest mixer memory is untouched.
    original(ctx, base);
    ctx.r4.u64 = saved;
  }
}

