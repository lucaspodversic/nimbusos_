//! Caché de miniaturas con tope de memoria.

use std::collections::HashMap;

#[derive(Debug)]
pub struct ThumbnailCache {
    pub max_bytes: usize,
    used: usize,
    map: HashMap<String, Vec<u8>>,
}

impl ThumbnailCache {
    pub fn new(max_bytes: usize) -> Self {
        Self { max_bytes, used: 0, map: HashMap::new() }
    }
    pub fn insert(&mut self, key: &str, data: Vec<u8>) {
        while self.used + data.len() > self.max_bytes && !self.map.is_empty() {
            if let Some(k) = self.map.keys().next().cloned() {
                if let Some(v) = self.map.remove(&k) { self.used -= v.len(); }
            }
        }
        self.used += data.len();
        self.map.insert(key.into(), data);
    }
    pub fn get(&self, key: &str) -> Option<&[u8]> {
        self.map.get(key).map(|v| v.as_slice())
    }
}
