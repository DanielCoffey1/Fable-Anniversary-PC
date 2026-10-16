# Anniversary host fibers

The legacy-engine startup path uses XAPI fibers. Running its original context switch as recompiled C++ changes guest registers without suspending the current native call stack. A debugger capture showed the profiling-stack pop still executing with the next fiber's entry registers (r31=82CB6CF8, LR=839F3810), causing an invalid guest-memory read.

`src/fiber_compat.cpp` routes the verified Anniversary entries to the SDK's native host-fiber implementations:

| Guest entry | Operation |
| --- | --- |
| 839E5C30 | ConvertThreadToFiber |
| 839E5CC0 | ConvertFiberToThread |
| 839E5D10 | CreateFiber |
| 839E5DF0 | DeleteFiber |
| 839E5E50 / 839F3820 | SwitchToFiber |

Identification used the actual supported executable: KTHREAD fiber field +164, context allocation A50, stack metadata, new-fiber entry in r31, and return sentinel 839F3810. The SDK bridges preserve guest context and switch the native stack together. A rebuilt launch passed the prior profiling-stack failure and reached a later VM callback dispatch. Gameplay is not yet validated.

The additional four-instruction VM leaves each increment the bytecode instruction pointer at r4+18 once and tail-call their inspected handler. An initial six-entry fix reached the Oakvale loading screen, which exposed 8354B0E0. The complete 150-entry list in `src/vm_advance_callbacks.inc` matches the exact instruction words `81640018 396B0001 91640018`, followed by a relative unconditional tail branch to an already generated function. Entries must follow a return, padding, or an identical adjacent leaf. Native registrations only fill missing dispatcher entries, preserving generated implementations when present.
