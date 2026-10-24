# Contribuir

1. Clean-room: **nunca** pegar código de Microsoft.
2. Comentarios en español en módulos de núcleo/servicios.
3. Tests host: `cmake -S . -B out && cmake --build out && ctest --test-dir out`.
4. Rust: `cargo test --workspace`.
5. Marca del producto: **NimbusOS** (no “Windows”).
