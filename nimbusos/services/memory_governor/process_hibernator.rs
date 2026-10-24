//! Hiberna procesos de UI minimizados serializando estado a zram-lógico.

use std::collections::HashMap;

#[derive(Clone, Debug)]
pub struct HibernatedProcess {
    pub pid: u32,
    pub name: String,
    pub blob: Vec<u8>,
}

#[derive(Default, Debug)]
pub struct ProcessHibernator {
    pub store: HashMap<u32, HibernatedProcess>,
}

impl ProcessHibernator {
    pub fn hibernate(&mut self, pid: u32, name: &str, state: &[u8]) {
        self.store.insert(pid, HibernatedProcess {
            pid, name: name.into(), blob: state.to_vec(),
        });
    }
    pub fn wake(&mut self, pid: u32) -> Option<HibernatedProcess> {
        self.store.remove(&pid)
    }
    pub fn bytes_used(&self) -> usize {
        self.store.values().map(|p| p.blob.len()).sum()
    }
}
