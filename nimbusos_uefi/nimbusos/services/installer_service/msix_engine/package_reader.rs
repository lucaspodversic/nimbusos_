//! Lector MSIX (zip+manifest).
pub fn is_msix(data: &[u8]) -> bool { data.starts_with(b"PK") }
