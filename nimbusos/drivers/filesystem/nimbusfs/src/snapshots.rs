//! Snapshots copy-on-write del árbol de inodos.

use std::collections::HashMap;
use crate::Inode;

#[derive(Default, Debug)]
pub struct SnapshotStore {
    next_id: u64,
    snaps: HashMap<u64, (String, HashMap<u64, Inode>)>,
}

impl SnapshotStore {
    pub fn new() -> Self { Self::default() }
    pub fn take(&mut self, label: &str, tree: HashMap<u64, Inode>) -> u64 {
        let id = self.next_id;
        self.next_id += 1;
        self.snaps.insert(id, (label.to_string(), tree));
        id
    }
    pub fn get(&self, id: u64) -> Option<&HashMap<u64, Inode>> {
        self.snaps.get(&id).map(|(_, t)| t)
    }
    pub fn label(&self, id: u64) -> Option<&str> {
        self.snaps.get(&id).map(|(l, _)| l.as_str())
    }
    pub fn count(&self) -> usize { self.snaps.len() }
}
