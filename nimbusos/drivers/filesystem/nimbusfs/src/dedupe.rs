//! Deduplicación de bloques por hash (content-addressable).

use std::collections::HashMap;

/// Hash FNV-1a 64-bit (suficiente para host; en prod SHA-256).
pub fn fnv1a64(data: &[u8]) -> u64 {
    let mut h: u64 = 0xcbf29ce484222325;
    for &b in data {
        h ^= b as u64;
        h = h.wrapping_mul(0x100000001b3);
    }
    h
}

#[derive(Default, Debug)]
pub struct DedupeStore {
    /// hash → (refcount, data)
    map: HashMap<u64, (u32, Vec<u8>)>,
}

impl DedupeStore {
    pub fn new() -> Self { Self::default() }
    pub fn put(&mut self, data: &[u8]) -> u64 {
        let h = fnv1a64(data);
        self.map.entry(h).and_modify(|(c, _)| *c += 1)
            .or_insert((1, data.to_vec()));
        h
    }
    pub fn get(&self, h: u64) -> Option<&[u8]> {
        self.map.get(&h).map(|(_, d)| d.as_slice())
    }
    pub fn release(&mut self, h: u64) {
        if let Some((c, _)) = self.map.get_mut(&h) {
            *c = c.saturating_sub(1);
            if *c == 0 { self.map.remove(&h); }
        }
    }
    pub fn unique_blocks(&self) -> usize { self.map.len() }
}
