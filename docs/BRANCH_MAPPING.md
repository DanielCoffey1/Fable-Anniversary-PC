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

## Materialized callback addresses

The next runtime launch initialized the standard GPU plugin and audio, then stopped at `8244C078`. Inspection shows a null-checking tail-call wrapper: conditional return followed by a branch to a known function. Its address is constructed in code at `8244C0FC`, not held in a read-only pointer table.

The second callback map contains 176 non-overlapping volatile-register leaves with contiguous bounded control flow and address-construction references. Candidate address construction is limited to eight instructions after a high-half load and stops at branches or potential source-register clobbers. Entries must follow padding or a terminal instruction. The same bounded-CFG restrictions apply; entries with discontiguous reachable spans are rejected. Runtime testing remains necessary.

A conservative reachability pass rejects candidates inside known functions and ambiguous computed-switch ranges (indexed target loads or compiler r12 jump dispatch). Ordinary vtable tail calls terminate the preceding function rather than consuming later callbacks. This retains 176 entries while rejecting seven internal/ambiguous entries, including the three switch labels that produced invalid generated branches. Two adjacent null-check wrappers at 8244C088 and 8244C098 were also inspected explicitly (conditional return, tail branch, then unreachable return) and mapped separately.

## Runtime-specific entries

`src/startup_callbacks.cpp` registers the observed `8247E5D8` member callback through the SDK dispatcher. Its inspected semantics are a member load at offset 34, argument 100, and a tail call to known function `8247B120`. It is outside the preceding virtual-dispatch routine; the conservative indirect-jump filter intentionally did not assume it was safe to map automatically.

The following runtime failure was `83A01978`, an allocation callback with internal block `83A01A88` discovered separately. Its full inspected boundary is 14C bytes, ending before the shared virtual tail dispatcher at `83A01AC4`. An explicit function mapping retains the original allocation instructions.

Further observed missing entries were small VM/heap dispatch leaves. The startup callback source implements the inspected semantics at `8242ADF0`, heap entries `83B8BB40`, `83B8BB70`, and its identical alias `83B8BB98`, plus the inspected 16-bank VM extension dispatch family (`8242AE08`, then `8242AE30` through `8242B0D0`). Existing generated mappings take precedence; native registration only fills missing addresses. After these changes, the standalone runtime displayed its startup video.

The first title-screen transition later reached an unmapped settings accessor at 838F6920. Its inspected body loads a member pointer, copies field 80 to the output, and returns. This confirmed the need to distinguish virtual tail calls from computed switches in the conservative filter; the refined set retains this accessor and similar structurally checked callbacks.
