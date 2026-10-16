# Save-container mount-name lifetime

A native first-chance exception capture identified the title-transition crash in `utf8_hash_fnv1a_case`, called by `ContentManager::OpenContent` during deferred `XamContentCreateEx` work. The pinned SDK captures `root_name.value()` in that work item; `MappedPtr<char>::value()` returns a `std::string_view` into guest memory. The caller can reuse its temporary string buffer before the worker hashes it.

`src/content_compat.cpp` retains each distinct mount name in immutable guest system-heap storage for the process lifetime, then forwards to the original SDK export. Existing content creation, enumeration, file IO, completion results, and save formats remain in the SDK. The wrapper covers Create, CreateEx, and CreateInternal; it retains small mount identifiers, not file contents. Repeated names reuse their allocation.

The next launch retained six distinct mount names and passed the previous invalid-UTF-8 failure point. It subsequently encountered a separate guest null-pointer access during later startup; gameplay and save/reload are not yet validated.
