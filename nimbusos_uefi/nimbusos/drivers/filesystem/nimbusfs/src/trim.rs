//! TRIM/DISCARD para SSDs: marca bloques libres.

use std::collections::BTreeSet;

#[derive(Default, Debug)]
pub struct TrimBitmap {
    free: BTreeSet<u64>,
}

impl TrimBitmap {
    pub fn new() -> Self { Self::default() }
    pub fn mark_free(&mut self, block: u64) { self.free.insert(block); }
    pub fn mark_used(&mut self, block: u64) { self.free.remove(&block); }
    pub fn is_free(&self, block: u64) -> bool { self.free.contains(&block) }
    pub fn free_count(&self) -> usize { self.free.len() }
    /// Emite rangos consecutivos (para comando TRIM del dispositivo).
    pub fn ranges(&self) -> Vec<(u64, u64)> {
        let mut out = Vec::new();
        let mut it = self.free.iter().copied();
        if let Some(mut start) = it.next() {
            let mut prev = start;
            for b in it {
                if b == prev + 1 {
                    prev = b;
                } else {
                    out.push((start, prev - start + 1));
                    start = b;
                    prev = b;
                }
            }
            out.push((start, prev - start + 1));
        }
        out
    }
}
