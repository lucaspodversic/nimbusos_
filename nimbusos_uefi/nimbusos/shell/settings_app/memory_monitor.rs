//! Monitor de memoria del sistema (presupuestos).
#[derive(Clone, Debug, Default)]
pub struct MemoryMonitor {
    pub system_used_mb: u32,
    pub ceiling_mb: u32,
}
impl MemoryMonitor {
    pub fn new() -> Self { Self { system_used_mb: 0, ceiling_mb: 1024 } }
    pub fn set_used(&mut self, mb: u32) { self.system_used_mb = mb; }
    pub fn percent(&self) -> f32 {
        self.system_used_mb as f32 / self.ceiling_mb.max(1) as f32 * 100.0
    }
    pub fn under_budget(&self) -> bool { self.system_used_mb <= self.ceiling_mb }
}
