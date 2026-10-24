//! Aplica a partición inactiva.
#[derive(Debug)]
pub struct AbState { pub active: char, pub inactive_ready: bool }
impl AbState {
    pub fn new() -> Self { Self { active: 'A', inactive_ready: false } }
    pub fn active(&self) -> String { self.active.to_string() }
    pub fn apply_to_inactive(&mut self) -> Result<(), String> {
        self.inactive_ready = true; Ok(())
    }
    pub fn commit_swap(&mut self) -> Result<(), String> {
        if !self.inactive_ready { return Err("not ready".into()); }
        self.active = if self.active == 'A' { 'B' } else { 'A' };
        self.inactive_ready = false; Ok(())
    }
}
