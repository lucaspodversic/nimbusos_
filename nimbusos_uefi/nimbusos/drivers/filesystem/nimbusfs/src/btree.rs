//! Índice B-tree (en memoria) para nombres → inodo.
//! En disco sería un B+tree empaquetado; aquí basta para host tests.

use std::collections::BTreeMap;

#[derive(Default, Debug)]
pub struct BTree {
    map: BTreeMap<Vec<u8>, u64>,
}

impl BTree {
    pub fn new() -> Self { Self::default() }
    pub fn insert(&mut self, key: &[u8], value: u64) {
        self.map.insert(key.to_vec(), value);
    }
    pub fn get(&self, key: &[u8]) -> Option<u64> {
        self.map.get(key).copied()
    }
    pub fn remove(&mut self, key: &[u8]) -> Option<u64> {
        self.map.remove(key)
    }
    pub fn keys_sorted(&self) -> Vec<Vec<u8>> {
        self.map.keys().cloned().collect()
    }
    pub fn len(&self) -> usize { self.map.len() }
}
