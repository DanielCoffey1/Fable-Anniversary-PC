#include "generated/default/fable_anniversary_pch.h"
#include <rex/logging/macros.h>
#include <rex/cvar.h>
#include <Windows.h>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <stdexcept>
#include <vector>
#include <thread>

REXCVAR_DEFINE_INT32(anniversary_frame_limit, 0, "Anniversary",
                    "Host swap-submission cap; zero retains original pacing").range(0, 240);

namespace {
void PaceFrame() {
  using Clock = std::chrono::steady_clock;
  struct Timer {
    HANDLE handle = CreateWaitableTimerExW(nullptr, nullptr,
        CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_MODIFY_STATE | SYNCHRONIZE);
    ~Timer() { if (handle) CloseHandle(handle); }
  };
  thread_local Timer timer;
  thread_local auto deadline = Clock::time_point{};
  thread_local int previous_limit = 0;
  const int limit = REXCVAR_GET(anniversary_frame_limit);
  if (!limit) { deadline = {}; previous_limit = 0; return; }
  const auto period = std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(1.0 / limit));
  const auto now = Clock::now();
  if (deadline == Clock::time_point{} || limit != previous_limit || now > deadline + period) deadline = now;
  if (now < deadline) {
    LARGE_INTEGER due;
    due.QuadPart = -std::max<int64_t>(1, std::chrono::duration_cast<std::chrono::nanoseconds>(deadline - now).count() / 100);
    if (timer.handle && SetWaitableTimer(timer.handle, &due, 0, nullptr, nullptr, FALSE)) WaitForSingleObject(timer.handle, INFINITE);
    else std::this_thread::sleep_until(deadline);
  }
  deadline += period; previous_limit = limit;
}
}

// Guest swap-submission intervals, not a count of displayed unique frames.
REX_EXTERN(__imp__VdSwap) {
  using Clock = std::chrono::steady_clock;
  static auto* original = reinterpret_cast<PPCFunc*>(GetProcAddress(
      GetModuleHandleW(L"rexruntime.dll"), "__imp__VdSwap"));
  if (!original) throw std::runtime_error("SDK swap export is unavailable");
  PaceFrame();
  const auto begin = Clock::now();
  original(ctx, base);
  const auto end = Clock::now();
  static std::mutex mutex;
  const std::lock_guard lock(mutex);
  static auto previous = begin;
  static std::vector<double> intervals;
  static double call_ms = 0;
  intervals.push_back(std::chrono::duration<double, std::milli>(begin - previous).count());
  previous = begin;
  call_ms += std::chrono::duration<double, std::milli>(end - begin).count();
  if (intervals.size() == 300) {
    double sum = 0; unsigned slow = 0;
    for (auto ms : intervals) { sum += ms; slow += ms > 50.0; }
    std::sort(intervals.begin(), intervals.end());
    REXSYS_INFO("[frame-probe] 300 swaps avg_ms={:.3f} p95_ms={:.3f} max_ms={:.3f} over50={} submit_avg_ms={:.3f}", sum / 300, intervals[284], intervals.back(), slow, call_ms / 300);
    intervals.clear(); call_ms = 0;
  }
}
