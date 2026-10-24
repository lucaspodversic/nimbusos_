//! Transacciones MSI.
pub fn begin() -> u64 { 1 }
pub fn commit(_t: u64) {}
pub fn abort(_t: u64) {}
