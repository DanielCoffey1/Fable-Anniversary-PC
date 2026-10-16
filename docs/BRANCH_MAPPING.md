# Initial branch mapping

The initial analysis found six unresolved direct branch targets. Each was inspected in the loaded image of the hash-pinned local executable before mapping it in `[entrypoint.functions]`. No Fable II/III addresses were reused.

| Entry | Size | Boundary evidence |
| --- | --- | --- |
| `83BDB968` | `E4` | Scalar vector-basis leaf with conditional normalization; return at `83BDBA48`, padding at `83BDBA4C`. |
| `83BF1748` | `7C` | Member iterator; early return at `83BF17A0`, virtual tail dispatch at `83BF17C0`. |
| `83A54288` | `24` | Optional field accessor with returns at `83A5429C` and `83A542A8`. |
| `83224998` | `8` | Embedded-member load followed by a direct tail branch. |
| `83224970` | `8` | Embedded-member load followed by a direct tail branch. |
| `83858F98` | `4` | Single direct tail branch, followed by padding and the next prologue. |

Mapped names describe observed behavior; they are not recovered original debug symbols. Loaded bytes and disassembly remain local under ignored `work/`.

Code generation exposed one additional tail edge into a context-switch body that automatic discovery merged with the preceding non-returning bugcheck stub. Inspection verified the stub at `839F3810` occupies `10` bytes; the separate context-switch body starts at `839F3820`, spans `40C` bytes, and tail-calls `KeSetCurrentStackPointers` at `839F3C28`, before padding and the next prologue. Both starts are explicit so the edge has its own callable target. The build gate rejects unresolved-call placeholders as well as unresolved-branch placeholders.

## Indirect leaf callbacks

The first runtime launch failed at unregistered callback `83661E60`. Its bounded control-flow graph spans `F4` bytes and ends in virtual tail dispatch. A scan of read-only pointer tables found 267 additional non-overlapping leaf entries that satisfy conservative checks: every reachable instruction decodes, branches stay within a bounded region or tail-call a known entry, no call instruction or LR modification occurs, and no nonvolatile integer/floating registers or condition fields are used. Each pointer reference must also have an adjacent code pointer, rejecting isolated code-looking values in unrelated data. Entries stop before the next known function and before 1 KiB. The observed startup callback is included.

`anniversary_leaf_callbacks.toml` records each entry's size and a local-image pointer reference. These are Anniversary-specific mappings, not imported game code. Runtime testing is still required; passing discovery is not a gameplay guarantee.

The first broad scan included an isolated value pointing to an epilogue inside another function; the generated-branch gate rejected that split. The adjacent-code-pointer filter removed that candidate and one other before the next runtime test.
