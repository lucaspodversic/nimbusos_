//! Journal de operaciones para recuperación ante caídas.

#[derive(Clone, Debug)]
pub enum JournalOp {
    MkDir { parent: u64, name: String, id: u64 },
    Create { parent: u64, name: String, id: u64 },
    Write { id: u64, offset: u64, len: u64 },
    HardLink { target: u64, parent: u64, name: String },
    Delete { parent: u64, name: String },
}

#[derive(Debug)]
pub struct Journal {
    pub ops: Vec<JournalOp>,
    pub capacity: usize,
}

impl Journal {
    pub fn new(capacity: usize) -> Self {
        Self { ops: Vec::new(), capacity }
    }
    pub fn append(&mut self, op: JournalOp) {
        if self.ops.len() >= self.capacity {
            self.ops.remove(0);
        }
        self.ops.push(op);
    }
    pub fn len(&self) -> usize { self.ops.len() }
    pub fn clear(&mut self) { self.ops.clear(); }
}
