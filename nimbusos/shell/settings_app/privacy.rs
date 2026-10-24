//! Configuración — privacy (página stub funcional).
#[derive(Default, Debug)]
pub struct Page { pub dirty: bool }
impl Page {
    pub fn mark_dirty(&mut self) { self.dirty = true; }
    pub fn save(&mut self) { self.dirty = false; }
}
