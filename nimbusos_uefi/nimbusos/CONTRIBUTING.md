# Contributing to NimbusOS

## Clean-room rules (mandatory)
1. **Never** paste or translate Microsoft Windows source code.
2. Work from public documentation (MSDN/learn.microsoft.com API docs, PE/COFF spec, RFCs).
3. Do not include proprietary binaries, symbols, fonts, or codecs with unclear licenses.
4. Do not use the trademark “Windows” as the product name; this project is **NimbusOS**.
5. Syscall numbers and internal structures may differ from Windows; document Nimbus ABI changes.

## Coding standards
- **Kernel / HAL / boot:** C11, Spanish detailed comments on every file and non-trivial function.
- **Services / shell / FS:** Rust 2021.
- Prefer small, testable modules. Host tests (`NIMBUS_HOST=1`) must stay green.
- Memory: respect `MmBudget*` limits when adding kernel allocations.

## Pull requests
1. Run `cmake --build build && ctest --test-dir build --output-on-failure`.
2. Run `cargo test --workspace` if you touch Rust crates.
3. Update `PLAN.md` checklist for new paths.
4. Describe the public docs you used (links).

## Security
Report issues privately when possible. No telemetry by design.
