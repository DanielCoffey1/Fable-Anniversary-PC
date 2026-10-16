// Verified Anniversary XAPI entrypoints. Switching the guest registers alone
// cannot suspend a recompiled C++ call stack; use the SDK's host-fiber bridge.
#include "generated/default/fable_anniversary_pch.h"
#include <rex/logging/macros.h>
#include <atomic>

REX_EXTERN(rexcrt_ConvertThreadToFiber);
REX_EXTERN(rexcrt_ConvertFiberToThread);
REX_EXTERN(rexcrt_CreateFiber);
REX_EXTERN(rexcrt_DeleteFiber);
REX_EXTERN(rexcrt_SwitchToFiber);

REX_EXTERN(sub_839E5C30) {
  rexcrt_ConvertThreadToFiber(ctx, base);
  static std::atomic_flag reported = ATOMIC_FLAG_INIT;
  if (!reported.test_and_set()) REXSYS_INFO("[compat] Anniversary thread conversion uses native host fibers");
}
REX_EXTERN(sub_839E5CC0) { rexcrt_ConvertFiberToThread(ctx, base); }
REX_EXTERN(sub_839E5D10) { rexcrt_CreateFiber(ctx, base); }
REX_EXTERN(sub_839E5DF0) { rexcrt_DeleteFiber(ctx, base); }
REX_EXTERN(sub_839E5E50) { rexcrt_SwitchToFiber(ctx, base); }
REX_EXTERN(ContextSwitchBody_839F3820) { rexcrt_SwitchToFiber(ctx, base); }
